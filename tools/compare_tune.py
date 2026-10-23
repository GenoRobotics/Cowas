#!/usr/bin/env python3
"""Compares two sets of `pressure_ramp` runs, to say whether a PID change helped (S1-1b).

Every run prints one TEST|pressure_ramp|<slot>|...|label=<text>;ovs=..;rise=..;... line.
Runs with the same slot and the same label are repeats of one setting. This script takes a baseline
label and a candidate label and compares them slot by slot, metric by metric. Lower is better for all
of them. A difference smaller than the run-to-run spread (or a floor) counts as SAME, so noise is not
read as an improvement.

Usage:
    python tools/compare_tune.py docs/logs/test-logs/2026-10-15_tune.csv --base base --cand kp150
    python tools/compare_tune.py a.log b.log --base base --cand kp150     (transcripts work too)

Exit code 0 = the candidate is not worse on any slot, 1 = it is worse somewhere (or unsafe), 2 = no data.
Stdlib only; the logic is tested with pytest (tools/tests/test_compare_tune.py).
"""

import argparse
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from hw_test_log import parse_kv, parse_test_line  # noqa: E402

# metric -> (floor of "same", what it means)
METRICS = {
    "pmax": (0.05, "highest pressure (bar)"),
    "ovs": (0.05, "overshoot over the target (bar)"),
    "rise": (0.5, "seconds to 90 % of the target"),
    "err": (0.05, "|mean P - target| in the second half of the hold (bar)"),
    "rip": (0.05, "pressure ripple, max - min (bar)"),
    "pwp2p": (3.0, "power swing (%)"),
    "rev": (2.0, "power reversals (chatter)"),
    "sat": (5.0, "% of the time at max power"),
}
NEVER = 999.0  # a rise time of -1 means the 90 % was never reached


def load_runs(paths):
    """List of {'slot','label','data','result'} for every pressure_ramp TEST line in the files."""
    runs = []
    for path in paths:
        if path.lower().endswith(".csv"):
            with open(path, newline="", encoding="utf-8") as f:
                for row in csv.DictReader(f):
                    if row.get("name") != "pressure_ramp":
                        continue
                    runs.append({"slot": row["id"], "result": row["result"], "data": parse_kv(row["data"])})
        else:
            with open(path, encoding="utf-8", errors="replace") as f:
                for line in f:
                    rec = parse_test_line(line)
                    if rec and rec["name"] == "pressure_ramp":
                        runs.append({"slot": rec["id"], "result": rec["result"], "data": rec["data"]})
    for r in runs:
        r["label"] = r["data"].get("label", "")
    return runs


def metric_value(run, name):
    """The number of one metric in one run, or None if the run does not have it."""
    raw = run["data"].get(name)
    if raw is None:
        return None
    try:
        v = float(raw)
    except ValueError:
        return None
    if name == "rise" and v < 0:
        return NEVER
    if name == "err":
        return abs(v)
    return v


def summarize(runs, name):
    """(mean, spread, n) of a metric over repeated runs; None if no run has it."""
    vals = [v for v in (metric_value(r, name) for r in runs) if v is not None]
    if not vals:
        return None
    return sum(vals) / len(vals), max(vals) - min(vals), len(vals)


def judge(base, cand, floor):
    """'BETTER', 'WORSE' or 'SAME': the difference must beat the larger of the spreads and the floor."""
    tol = max(base[1], cand[1], floor)
    diff = cand[0] - base[0]
    if diff > tol:
        return "WORSE"
    if -diff > tol:
        return "BETTER"
    return "SAME"


def compare_slot(base_runs, cand_runs):
    """Rows [(metric, base_mean, cand_mean, verdict)] and a list of problems for one slot."""
    rows = []
    problems = []
    for name, (floor, _) in METRICS.items():
        b, c = summarize(base_runs, name), summarize(cand_runs, name)
        if b is None or c is None:
            continue
        rows.append((name, b[0], c[0], judge(b, c, floor)))
    base_ok = all(r["data"].get("end") == "completed" for r in base_runs)
    for r in cand_runs:
        end = r["data"].get("end", "?")
        if end in ("soft_limit", "cap"):
            problems.append(f"UNSAFE: a candidate run ended with {end}")
        elif end != "completed" and base_ok:
            problems.append(f"a candidate run ended with {end} (the baseline completed)")
    return rows, problems


def compare(runs, base_label, cand_label):
    """Returns (report lines, exit code)."""
    slots = sorted({r["slot"] for r in runs if r["label"] in (base_label, cand_label)}, key=lambda s: (len(s), s))
    lines = []
    worse = False
    seen = False
    for slot in slots:
        base_runs = [r for r in runs if r["slot"] == slot and r["label"] == base_label]
        cand_runs = [r for r in runs if r["slot"] == slot and r["label"] == cand_label]
        if not base_runs or not cand_runs:
            lines.append(f"slot {slot}: missing runs (base {len(base_runs)}, cand {len(cand_runs)}), skipped")
            continue
        seen = True
        rows, problems = compare_slot(base_runs, cand_runs)
        lines.append(f"slot {slot}  ({len(base_runs)} base run(s), {len(cand_runs)} cand run(s))")
        lines.append(f"  {'metric':<7}{'base':>9}{'cand':>9}  verdict")
        for name, b, c, v in rows:
            lines.append(f"  {name:<7}{b:>9.2f}{c:>9.2f}  {v}   {METRICS[name][1]}")
            worse = worse or v == "WORSE"
        for p in problems:
            lines.append("  !! " + p)
            worse = True
    if not seen:
        return lines + ["No slot has runs for both labels."], 2
    lines.append("")
    lines.append("RESULT: " + ("the candidate is WORSE (or unsafe) somewhere: keep the baseline" if worse
                              else "the candidate is not worse on any slot"))
    return lines, 1 if worse else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="+", help="hw_test_log .csv or .log files")
    ap.add_argument("--base", required=True, help="label of the baseline runs")
    ap.add_argument("--cand", required=True, help="label of the candidate runs")
    args = ap.parse_args(argv)
    runs = load_runs(args.files)
    lines, code = compare(runs, args.base, args.cand)
    print("\n".join(lines))
    return code


if __name__ == "__main__":
    sys.exit(main())
