#!/usr/bin/env python3
"""Summary of one field run folder, from its session.log (read-only, stdlib only).

Written 2026-11-05 from the field checks of the E2 run: it prints what was checked by hand that day (the purge flow at
1 to 5 minutes, the pauses at the pressure cap, the spool warnings, the prime tries) so that it does not have to be
done again. It never touches the device or the session tool.

    python tools/run_report.py docs/logs/test-logs/2026-11-05/01_E2-DIEL1-session5-2026-11-05

It reads <folder>/session.log (or the path of a log file given directly). Lines look like
    2026-11-05T16:52:05.169+01:00 < Step purge started
"""

import argparse
import os
import re
import statistics
import sys
from datetime import datetime

LINE = re.compile(r"^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d+[+-]\d{2}:\d{2}) [<>!] ?(.*)$")
PURGE_MINUTES = (1, 2, 3, 4, 5)


def _time(stamp):
    return datetime.fromisoformat(stamp)


def parse_events(lines):
    """(time, text) for every line of the session log that has a time stamp."""
    events = []
    for raw in lines:
        m = LINE.match(raw.rstrip("\r\n"))
        if m:
            events.append((_time(m.group(1)), m.group(2)))
    return events


def pauses(events):
    """Each cap-margin pause: (pressure at the stop, reading at the restart, seconds the pump was off).
    Reads both the old text (`PAUSE at X bar (n)`, `RESUME at Y bar`) and the 2026-11-05 text (`PAUSE n in this run`,
    `RESUME after T ms: reading Y bar`). The time is taken from the two lines, so the old text works too."""
    out = []
    start = None
    for t, text in events:
        m = re.match(r"PAUSE (?:at ([\d.]+) bar|\d+ in this run.*pressure ([\d.]+) bar)", text)
        if m:
            start = (t, float(m.group(1) or m.group(2)))
            continue
        m = re.match(r"RESUME (?:at ([\d.]+) bar|after \d+ ms: reading ([\d.]+) bar)", text)
        if m and start is not None:
            out.append((start[1], float(m.group(1) or m.group(2)), (t - start[0]).total_seconds()))
            start = None
    return out


def purge_flow(events):
    """mL counted (the flowmeter total) at each whole minute of the first purge in the log, and the pressure statistics.
    The purge starts at `Step purge started` and ends at the next `Step purge ended` or `Step purge: ... ` line; None when
    there is no purge in the log."""
    start = None
    readings = []
    pressures = []
    for t, text in events:
        if start is None and text.startswith("Step purge started"):
            start = t
            continue
        if start is not None:
            if text.startswith("Step purge ended") or text.startswith("Time to purge"):
                break
            m = re.match(r"Flow measured : (\d+)", text)
            if m:
                readings.append(((t - start).total_seconds(), int(m.group(1))))
            m = re.match(r"Pressure bar : ([-\d.]+)", text)
            if m:
                pressures.append(float(m.group(1)))
    if start is None or not readings:
        return None
    at = {}
    for minute in PURGE_MINUTES:
        seen = [ml for secs, ml in readings if secs <= minute * 60]
        at[minute] = seen[-1] if seen else None
    return {"at_minutes": at, "pressure_mean": statistics.fmean(pressures) if pressures else None,
            "pressure_max": max(pressures) if pressures else None}


def count(events, pattern):
    rx = re.compile(pattern)
    return sum(1 for _, text in events if rx.search(text))


def build_report(lines):
    events = parse_events(lines)
    report = {}
    p = pauses(events)
    drops = [before - after for before, after, _ in p]
    durations = [seconds for _, _, seconds in p]
    report["pauses"] = {
        "count": len(p),
        "drop_median": statistics.median(drops) if drops else None,
        "seconds_median": statistics.median(durations) if durations else None,
        "under_1s": sum(1 for s in durations if s < 1.0),
    }
    report["purge"] = purge_flow(events)
    report["prime"] = {
        "pass": count(events, r"Spool prime \(try \d+ of \d+\): flow check PASS"),
        "fail": count(events, r"Spool prime \(try \d+ of \d+\): flow check FAIL"),
    }
    report["spool"] = {
        "origin_held_warnings": count(events, r"WARNING \| spool: origin switch held"),
        "stalled_moves": count(events, r"Spool move:.*stalled"),
        "aborted_moves": count(events, r"Spool move:.*aborted with x"),
        "depth_in_doubt": count(events, r"depth in doubt"),
    }
    report["stops"] = {
        "errors": count(events, r"^ERROR \|"),
        "warnings": count(events, r"^WARNING \|"),
        "aborts_x": count(events, r"ABORT \||aborted with x"),
        "refused": count(events, r"^REFUSED \|"),
        "vent_not_released": count(events, r"Vent:.*NOT released"),
    }
    return report


def render(report):
    out = []
    pz = report["pauses"]
    out.append("Pressure cap pauses")
    out.append(f"  count {pz['count']}, pump off {pz['seconds_median'] if pz['seconds_median'] is not None else '-'} s (median), "
               f"drop after the stop {round(pz['drop_median'], 2) if pz['drop_median'] is not None else '-'} bar (median), "
               f"under 1 s: {pz['under_1s']}")
    purge = report["purge"]
    out.append("Purge flow (mL counted at each minute)")
    if purge is None:
        out.append("  no purge in this log")
    else:
        at = purge["at_minutes"]
        out.append("  " + ", ".join(f"{m} min: {at[m] if at[m] is not None else '-'}" for m in PURGE_MINUTES))
        if purge["pressure_mean"] is not None:
            out.append(f"  pressure mean {purge['pressure_mean']:.2f} bar, max {purge['pressure_max']:.2f} bar")
    pr = report["prime"]
    out.append(f"Spool prime tries: {pr['pass']} PASS, {pr['fail']} FAIL")
    sp = report["spool"]
    out.append(f"Spool: origin-switch warnings {sp['origin_held_warnings']}, stalled moves {sp['stalled_moves']}, "
               f"moves aborted with x {sp['aborted_moves']}, depth in doubt {sp['depth_in_doubt']}")
    st = report["stops"]
    out.append(f"Lines: ERROR {st['errors']}, WARNING {st['warnings']}, x aborts {st['aborts_x']}, "
               f"REFUSED {st['refused']}, vent not released {st['vent_not_released']}")
    return "\n".join(out)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    parser.add_argument("path", help="run folder (with session.log) or a session.log file")
    args = parser.parse_args(argv)
    path = args.path
    if os.path.isdir(path):
        path = os.path.join(path, "session.log")
    if not os.path.isfile(path):
        print(f"no session log at {path}", file=sys.stderr)
        return 1
    with open(path, encoding="utf-8", errors="replace") as f:
        report = build_report(f.readlines())
    print(render(report))
    return 0


if __name__ == "__main__":
    sys.exit(main())
