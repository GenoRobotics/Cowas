#!/usr/bin/env python3
"""CoWaS on-device test logger (FW2, ROADMAP §5).

Opens the Due's serial console, mirrors everything to the screen, sends what you type
(one line, LF-terminated, like `pio device monitor`), and appends every
    TEST|<name>|<id>|<PASS/FAIL/SKIP>|<key=value;...>
line to test_logs/<YYYY-MM-DD>_<tag>.csv. The full transcript goes to the matching .log.

Usage:
    python tools/hw_test_log.py --port COM5 --tag fw2
    python tools/hw_test_log.py --port /dev/ttyACM0 --tag hw7 --baud 9600
Quit with Ctrl+C or by typing :quit.

Needs pyserial (bundled with PlatformIO's Python:
%USERPROFILE%\\.platformio\\penv\\Scripts\\python.exe). The parser below has no dependency
and is tested with pytest (tools/tests/).
"""

import argparse
import csv
import datetime as _dt
import os
import sys
import threading

RESULTS = ("PASS", "FAIL", "SKIP")
CSV_COLUMNS = ["host_time", "tag", "name", "id", "result", "data", "raw"]


def parse_kv(text):
    """'a=1;b=x' -> {'a': '1', 'b': 'x'}. Pieces without '=' are kept with value ''."""
    out = {}
    for piece in text.split(";"):
        piece = piece.strip()
        if not piece:
            continue
        key, sep, value = piece.partition("=")
        out[key] = value if sep else ""
    return out


def parse_test_line(line):
    """Parses one TEST line; returns a dict or None if the line is not a valid TEST line.

    Leading junk before 'TEST|' (e.g. a garbage byte after a reset) is tolerated; the line
    must then have 5 '|'-separated fields and a known result.
    """
    if line is None:
        return None
    line = line.rstrip("\r\n")
    start = line.find("TEST|")
    if start < 0:
        return None
    fields = line[start:].split("|", 4)
    if len(fields) != 5:
        return None
    _, name, test_id, result, data = fields
    if not name or not test_id or result not in RESULTS:
        return None
    return {
        "name": name,
        "id": test_id,
        "result": result,
        "data": parse_kv(data),
        "data_raw": data,
        "raw": line[start:],
    }


def log_paths(tag, root="test_logs", today=None):
    """(csv_path, log_path) for a tag: test_logs/<date>_<tag>.csv / .log."""
    today = today or _dt.date.today()
    safe = "".join(c if c.isalnum() or c in "-_" else "_" for c in tag) or "untagged"
    base = os.path.join(root, f"{today.isoformat()}_{safe}")
    return base + ".csv", base + ".log"


def append_csv(path, record, tag, now=None):
    """Appends one parsed record; writes the header if the file is new."""
    now = now or _dt.datetime.now()
    new = not os.path.exists(path) or os.path.getsize(path) == 0
    with open(path, "a", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        if new:
            w.writerow(CSV_COLUMNS)
        w.writerow([now.isoformat(timespec="seconds"), tag, record["name"], record["id"],
                    record["result"], record["data_raw"], record["raw"]])


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", required=True, help="serial port, e.g. COM5 or /dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=9600)
    ap.add_argument("--tag", required=True, help="log name suffix, e.g. the step (fw2) or the rig")
    ap.add_argument("--dir", default="test_logs")
    args = ap.parse_args(argv)

    import serial  # pyserial; imported here so the parser tests don't need it

    os.makedirs(args.dir, exist_ok=True)
    csv_path, log_path = log_paths(args.tag, args.dir)
    print(f"Logging TEST lines to {csv_path}, transcript to {log_path}. Ctrl+C or :quit to exit.")

    # Note: opening the Programming port resets the Due (ROADMAP §3.4).
    port = serial.Serial(args.port, args.baud, timeout=0.2)
    stop = threading.Event()
    counts = {r: 0 for r in RESULTS}

    def reader():
        buf = b""
        with open(log_path, "a", encoding="utf-8") as log:
            while not stop.is_set():
                chunk = port.read(256)
                if not chunk:
                    continue
                buf += chunk
                while b"\n" in buf:
                    raw, buf = buf.split(b"\n", 1)
                    line = raw.decode("utf-8", errors="replace").rstrip("\r")
                    print(line, flush=True)
                    log.write(f"{_dt.datetime.now().isoformat(timespec='seconds')} {line}\n")
                    log.flush()
                    rec = parse_test_line(line)
                    if rec:
                        append_csv(csv_path, rec, args.tag)
                        counts[rec["result"]] += 1

    t = threading.Thread(target=reader, daemon=True)
    t.start()
    try:
        for line in sys.stdin:
            line = line.rstrip("\r\n")
            if line == ":quit":
                break
            port.write((line + "\n").encode("utf-8"))  # LF only: a CR breaks the console (F37)
    except KeyboardInterrupt:
        pass
    finally:
        stop.set()
        t.join(timeout=1)
        port.close()
        print(f"Session: {counts['PASS']} PASS, {counts['FAIL']} FAIL, {counts['SKIP']} SKIP -> {csv_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
