#!/usr/bin/env python3
"""CoWaS console with a session log: the one tool for talking to the Due and keeping the record.

Shows the console like `pio device monitor` and sends what you type (one line, LF). Everything is also written to
docs/logs/test-logs/ (docs/software/session-log.md). One folder per run, numbered per day:
    <date>/<NN>_<tag>/session.log   the full transcript: every line in (<) and out (>), with the PC time
    <date>/<NN>_<tag>/tests.csv     one row per TEST|<name>|<id>|<PASS/FAIL/SKIP>|<k=v;...> line (the FW2 checklists)
    <date>/<NN>_<tag>/samples/      one JSON per sample or purge (the firmware's {"t":"sample",...} line + pc_received),
                                    and samples.csv, one row per sample
NN is 01, 02, ... for the runs of that day. A new start with the same tag on the same day reuses its folder (appends).
    <date>/<NN>_<tag>/slots.json    the sample availability (tools/sample_slots.py): asked at the start of a new folder
    <date>/<NN>_<tag>/schedule.csv  with --schedule: the real start and finish time of each scheduled sample

At the start of a new folder the tool asks how many samples are loaded and whether they are sorted on the device. Then
`run_sample` (no slot) runs the next available sample in the loading order, and `run_sample <slot>` runs that sample
(an erase question if it is used or not loaded).

With --schedule FILE.csv (tools/sample_schedule.py) the tool shows the timed list of samples. Nothing runs until the
operator types `schedule start` and confirms. Then each sample runs at its time (`run_sample`, with the name, condition
and notes from the file). `schedule stop` stops the list; `schedule` shows it.

Usage:
    python tools/session_log.py --port COM6 --tag s1-1b
    python tools/session_log.py --port COM6 --tag field --schedule tools/schedule_example.csv
    python tools/session_log.py --port /dev/ttyACM0 --tag hw7 --baud 9600
Quit with Ctrl+C or by typing :quit.

The port is opened with DTR and RTS held low, so opening it does not reset the Due (pyserial asserts DTR on open by
default, and the Programming port resets the Due on that). 1200 baud is refused: it erases the flash.
tools/hw_test_log.py is the old name and still works (it calls this file).

Needs pyserial (PlatformIO's Python: %USERPROFILE%\\.platformio\\penv\\Scripts\\python.exe). The parsing and the files
have no dependency and are tested with pytest (tools/tests/test_session_log.py, test_hw_test_log.py).
"""

import argparse
import csv
import datetime as _dt
import json
import os
import re
import shutil
import sys
import threading

import sample_schedule as sched
import sample_slots as ss

RESULTS = ("PASS", "FAIL", "SKIP")
TEST_CSV_COLUMNS = ["host_time", "tag", "name", "id", "result", "data", "raw"]
SAMPLE_CSV_COLUMNS = [
    "pc_received",
    "id",
    "purpose",
    "site",
    "date",
    "condition",
    "slot",
    "end_reason",
    "target_ml",
    "vol_ml",
    "vol_trusted_ml",
    "purge_ml",
    "p_mean_bar",
    "p_max_bar",
    "p_cap_bar",
    "lock_before",
    "lock_after",
    "run_start_pc",
    "run_finish_pc",
    "flags",
    "fw",
    "file",
]
SAMPLE_PREFIX = '{"t":"sample"'
PC_LOG_HELLO = "log_on"  # sent to the Due when this tool starts: run_sample then knows its record is saved (Tests_mech.cpp)
SAMPLE_META = re.compile(r"META\|sample;slot=(\d+)")  # printed by the Due when run_sample has its name: the sample is used
RUN_START_LINE = "=== run_sample ===" # printed by the Due when a run_sample pump run begins
# The default folder is found from this file (tools/ is inside the repository), so the tool works from any folder.
REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_LOG_DIR = os.path.join(REPO_ROOT, "docs", "logs", "test-logs")


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


def parse_sample(line):
    """The sample document in a console line, or None if the line is not a sample.

    Junk before the '{' is tolerated. A line that starts a sample but is not valid JSON raises ValueError: the caller
    reports it, it is never dropped silently.
    """
    start = line.find(SAMPLE_PREFIX)
    if start < 0:
        return None
    return json.loads(line[start:])


def now_iso(now=None):
    """Local time with offset and milliseconds: the PC clock is the trusted clock (the Due has no RTC)."""
    now = now or _dt.datetime.now().astimezone()
    return now.isoformat(timespec="milliseconds")


def safe_name(text):
    """A file-name piece: letters, digits, '-' and '_'; everything else becomes '_'."""
    return "".join(c if c.isalnum() or c in "-_" else "_" for c in str(text)) or "none"


def checked_baud(text):
    """argparse type: refuses 1200 baud (the touch that erases the Due's flash) and non-numbers."""
    try:
        value = int(text)
    except ValueError:
        raise argparse.ArgumentTypeError(f"not a baud rate: {text}")
    if value == 1200:
        raise argparse.ArgumentTypeError(
            "1200 baud is refused: it erases the Due's flash"
        )
    if value < 300:
        raise argparse.ArgumentTypeError(f"baud rate too low: {value}")
    return value


def session_dir(tag, root="docs/logs/test-logs", today=None):
    """The folder of one run: <root>/<date>/<NN>_<tag>. NN is one more than the highest number of that day, from 01.
    A start with a tag that already has a folder that day reuses it (the files are appended), so a restart loses nothing."""
    today = today or _dt.date.today()
    name = safe_name(tag) if tag else "untagged"
    day = os.path.join(root, today.isoformat())
    runs = []
    if os.path.isdir(day):
        runs = [e for e in os.listdir(day) if e[:2].isdigit() and os.path.isdir(os.path.join(day, e))]
    for entry in runs:
        if entry[3:] == name:
            return os.path.join(day, entry)
    number = max((int(e[:2]) for e in runs), default=0) + 1
    return os.path.join(day, f"{number:02d}_{name}")


def log_paths(tag, root="docs/logs/test-logs", today=None):
    """(tests_csv, session_log) for a tag, inside its run folder (see session_dir)."""
    folder = session_dir(tag, root, today)
    return os.path.join(folder, "tests.csv"), os.path.join(folder, "session.log")


def append_csv(path, record, tag, now=None):
    """Appends one parsed TEST record; writes the header if the file is new."""
    now = now or _dt.datetime.now()
    new = not os.path.exists(path) or os.path.getsize(path) == 0
    with open(path, "a", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        if new:
            w.writerow(TEST_CSV_COLUMNS)
        w.writerow(
            [
                now.isoformat(timespec="seconds"),
                tag,
                record["name"],
                record["id"],
                record["result"],
                record["data_raw"],
                record["raw"],
            ]
        )


class Session:
    """The files of one run. A failed write does not stop the console: it prints LOG ERROR, and the next line that does
    get written starts with LOG GAP, so the gap is in the file too."""

    def __init__(self, root, tag="session", today=None):
        self.tag = tag
        self.test_csv, self.log_path = log_paths(tag, root, today)
        self.samples_dir = os.path.join(os.path.dirname(self.log_path), "samples")
        self.samples_csv = os.path.join(self.samples_dir, "samples.csv")
        os.makedirs(self.samples_dir, exist_ok=True)  # also creates root
        self.log = open(self.log_path, "a", encoding="utf-8")
        self.counts = {r: 0 for r in RESULTS}
        self.samples = 0
        self.errors = 0
        self.gap = False
        self.sample_records = []  # every record saved, for samples.csv
        self.run_start = None  # PC time of the run_sample that is running, or None
        self.run_records = []  # the records of that run: their run_finish_pc is written when it ends
        self.open_slot = None  # the slot whose name the device took and whose run has not ended (a refusal frees it)
        self._lock = threading.Lock()

    def _fail(self, what, error):
        self.gap = True
        self.errors += 1
        print(f"LOG ERROR: {what} not written ({error})", flush=True)

    def line(self, direction, text):
        """One console line: '<' from the device, '>' typed by the operator, '!' the tool itself."""
        with self._lock:
            try:
                if self.gap:
                    self.log.write(
                        f"{now_iso()} ! LOG GAP: lines were not written after an error\n"
                    )
                    self.gap = False
                self.log.write(f"{now_iso()} {direction} {text}\n")
                self.log.flush()
            except OSError as error:
                self._fail("transcript line", error)

    def test(self, record):
        """Appends a TEST line to the CSV of the tag and counts it."""
        with self._lock:
            try:
                append_csv(self.test_csv, record, self.tag)
            except OSError as error:
                self._fail("TEST row", error)
                return
            self.counts[record["result"]] += 1

    def sample(self, doc):
        """Saves one sample document, its CSV row, and the run times of a run_sample. Returns the file name, or None if it
        could not be written."""
        with self._lock:
            now = _dt.datetime.now().astimezone()
            stem = f"{now:%H%M%S}{now.microsecond // 1000:03d}_{safe_name(doc.get('id', 'none'))}"
            name, n = f"{stem}.json", 2
            while os.path.exists(
                os.path.join(self.samples_dir, name)
            ):  # two records in the same millisecond
                name, n = f"{stem}_{n}.json", n + 1
            record = dict(
                doc,
                pc_received=now_iso(now),
                file=name,
                run_start_pc=self.run_start or "",
                run_finish_pc="",  # written when the run_sample ends
            )
            try:
                self._write_json(name, record)
                self.sample_records.append(record)
                self._write_samples_csv()
            except OSError as error:
                self._fail(f"sample {name}", error)
                return None
            if self.run_start:
                self.run_records.append(record)
            self.samples += 1
            return name

    def run_begin(self, start):
        """The run_sample pump run starts (start = PC time). The sample records written from now on get it."""
        with self._lock:
            self.run_start = start
            self.run_records = []

    def run_end(self, finish):
        """The run_sample is done (its TEST line arrived, PASS or FAIL). Writes the finish time into the records of the run,
        in their JSON files and in samples.csv."""
        with self._lock:
            records, self.run_records = self.run_records, []
            self.run_start = None
            for record in records:
                record["run_finish_pc"] = finish
                try:
                    self._write_json(record["file"], record)
                except OSError as error:
                    self._fail(f"sample {record['file']} finish time", error)
            if records:
                try:
                    self._write_samples_csv()
                except OSError as error:
                    self._fail("samples.csv finish times", error)

    def _write_json(self, name, record):
        with open(os.path.join(self.samples_dir, name), "w", encoding="utf-8") as f:
            json.dump(record, f, indent=2, ensure_ascii=False)
            f.write("\n")

    def _write_samples_csv(self):
        """samples.csv, one row per record, rewritten whole (a few rows per session; the run times change after a run)."""
        write_samples_csv(self.samples_csv, self.sample_records)

    def close(self):
        """Closes the transcript. Checks that every sample saved in this session is on disk: a mismatch is written to
        the log as SAMPLES CHECK (F206: the JSON files of a run were found missing after the session had closed)."""
        with self._lock:
            on_disk = count_json(self.samples_dir)
            if on_disk != self.samples:
                message = f"SAMPLES CHECK: {on_disk} JSON files in {self.samples_dir}, {self.samples} saved in this session"
                self.log.write(f"{now_iso()} ! {message}\n")
                print(message, flush=True)
            self.log.close()


def count_json(folder):
    """How many sample files (*.json) are in a samples folder; 0 if it does not exist."""
    if not os.path.isdir(folder):
        return 0
    return sum(1 for name in os.listdir(folder) if name.endswith(".json"))


def write_samples_csv(path, records):
    """samples.csv: one row per record, the columns of SAMPLE_CSV_COLUMNS (flags joined with ';', None as empty)."""
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(SAMPLE_CSV_COLUMNS)
        for record in records:
            row = []
            for column in SAMPLE_CSV_COLUMNS:
                value = record.get(column)
                if column == "flags":
                    value = ";".join(value or [])
                row.append("" if value is None else value)
            w.writerow(row)


LOG_LINE = re.compile(r"^(\d{4}-\d\d-\d\dT\S+) ([<>!]) ?(.*)$")  # the transcript format: time, direction, text


def rebuild_samples(run_dir):
    """Rebuilds samples/ (one JSON per sample, and samples.csv) from the session.log of a run folder, for a run whose
    sample files were lost. The same rules as the live tool: the sample record is the firmware line plus pc_received
    (the time of its log line), run_start_pc (the `=== run_sample ===` line) and run_finish_pc (its TEST line).
    Nothing is overwritten: if the samples/ folder already holds sample files, the transcript is copied into the next
    numbered run folder of the same day and the samples are rebuilt there. Returns (folder_rebuilt, records_written)."""
    if count_json(os.path.join(run_dir, "samples")):
        source = run_dir
        run_dir = next_run_folder(source)
        os.makedirs(run_dir)
        shutil.copy2(os.path.join(source, "session.log"), os.path.join(run_dir, "session.log"))
    log_path = os.path.join(run_dir, "session.log")
    samples_dir = os.path.join(run_dir, "samples")
    os.makedirs(samples_dir, exist_ok=True)
    records, pending, run_start, names = [], [], None, set()
    with open(log_path, encoding="utf-8") as f:
        for raw in f:
            match = LOG_LINE.match(raw.rstrip("\n"))
            if not match or match.group(2) != "<":  # only the device lines hold samples
                continue
            stamp, text = match.group(1), match.group(3)
            if text.strip() == RUN_START_LINE:
                run_start, pending = stamp, []
                continue
            rec = parse_test_line(text)
            if rec and rec["name"] == "run_sample" and rec["id"] != "summary" and run_start:
                for done in pending:
                    done["run_finish_pc"] = stamp
                pending, run_start = [], None
            try:
                doc = parse_sample(text)
            except ValueError:
                continue  # a cut line: the live tool reported it as BAD SAMPLE LINE; the transcript keeps it
            if doc is None:
                continue
            name = f"{stamp[11:13]}{stamp[14:16]}{stamp[17:19]}{stamp[20:23]}_{safe_name(doc.get('id', 'none'))}.json"
            record = dict(
                doc,
                pc_received=stamp,
                file=name,
                run_start_pc=run_start or "",
                run_finish_pc="",
            )
            while name in names:  # two records in the same millisecond, as in Session.sample
                name = name[:-5] + "_x.json"
            names.add(name)
            records.append(record)
            if run_start:
                pending.append(record)
    for record in records:
        with open(os.path.join(samples_dir, record["file"]), "w", encoding="utf-8") as f:
            json.dump(record, f, indent=2, ensure_ascii=False)
            f.write("\n")
    write_samples_csv(os.path.join(samples_dir, "samples.csv"), records)
    return run_dir, len(records)


def next_run_folder(run_dir):
    """The next numbered run folder of the same day after run_dir (<date>/<NN+1>_<tag>), not created yet."""
    day, name = os.path.split(os.path.normpath(run_dir))
    tag = name.split("_", 1)[1] if "_" in name else name
    numbers = [int(e[:2]) for e in os.listdir(day) if e[:2].isdigit() and os.path.isdir(os.path.join(day, e))]
    return os.path.join(day, f"{max(numbers, default=0) + 1:02d}_{tag}")


def ask_yes_no(prompt, read):
    """True for y or yes. Anything else (including an empty answer) is no."""
    return read(prompt).strip().lower() in ("y", "yes")


def ask_loaded(read, write):
    """The start of a new folder: how many samples are loaded (the first ones of the loading order), and whether they are
    sorted on the device as numbered. Asks again until the operator confirms. Returns the count."""
    count = len(ss.LOADING_ORDER)
    order = ", ".join(str(s) for s in ss.LOADING_ORDER)
    while True:
        write(f"How many samples are loaded? 0 to {count}. The loaded ones are the first in the loading order: {order}.")
        answer = read("loaded> ").strip()
        if not answer.isdigit() or int(answer) > count:
            write(f"Type a number from 0 to {count}.")
            continue
        n = int(answer)
        write(f"Loaded: {', '.join(str(s) for s in ss.LOADING_ORDER[:n]) or 'none'}. The others are not loaded.")
        if ask_yes_no("Are these samples on the device, numbered as above and in this order? (y/n) ", read):
            return n
        write("Not confirmed. Sort the samples on the device, then answer again.")


SCHEDULE_HELP = "Schedule commands: `schedule` (the list), `schedule start`, `schedule stop`."


def schedule_command(args, scheduled, today, read, write):
    """The `schedule` commands typed in the tool. `schedule start` shows the dates and asks for a yes before anything runs.
    `today` is the day the schedule starts, for the +N dates."""
    if scheduled is None:
        write("No schedule: start the tool with --schedule FILE.csv.")
        return
    if not args:
        for line in scheduled.table():
            write(line)
        return
    if args == ["start"]:
        if scheduled.active():
            write("The schedule is running already.")
            return
        if not scheduled.pending():
            write("Nothing left to run.")
            return
        write("The schedule will be:")
        for line in scheduled.preview(today):
            write(line)
        write("The device needs `session` first (site, date, experiment).")
        if not ask_yes_no("Start the schedule now? (y/n) ", read):
            write("Not started.")
            return
        scheduled.start(today)
        write("Schedule started. Each sample runs at its time. Type `schedule stop` to stop it.")
        return
    if args == ["stop"]:
        if not scheduled.active():
            write("The schedule is not running.")
            return
        scheduled.stop()
        write("Schedule stopped: no further sample starts. The sample on the device now (if any) finishes. "
              "`schedule start` runs the samples left.")
        return
    write(SCHEDULE_HELP)


def route_run_sample(line, book, read, write):
    """What to send to the Due for a typed line, or None when nothing must be sent.

    `run_sample <slot>` sends that slot if it is available; if it is used or not loaded, asks to erase it first (y), and
    sends nothing on n. A bare `run_sample` sends the next available sample of the loading order, or refuses when none is
    left. Other lines (the sample's name, the condition, `stop`, ...) are sent as they are."""
    words = line.split()
    if not words or words[0].lower() != "run_sample":
        return line
    rest = words[1:]
    if rest and rest[0].isdigit():
        slot = int(rest.pop(0))
        status = book.status(slot)
        if status is None:
            write(f"REFUSED | run_sample: {slot} is not a sample (1 to {len(ss.LOADING_ORDER)}). Not sent.")
            return None
        if status != ss.AVAILABLE:
            if not ask_yes_no(f"Sample {slot} is {status}. Erase it and run it anyway? (y/n) ", read):
                write("Not sent.")
                return None
            book.erase(slot)
            write(f"Sample {slot} erased: available again.")
    else:
        slot = book.next_available()
        if slot is None:
            write("REFUSED | run_sample: every loaded sample is used or not loaded. Type run_sample <slot> to force one.")
            return None
        write(f"run_sample: next sample in the loading order: {slot}")
    return " ".join(["run_sample", str(slot)] + rest)


def on_device_line(session, text, book=None):
    """Shows a device line, logs it, and records it: a TEST line in the CSV, a sample as its own file.
    `book` (the sample availability) is told when a run_sample has its name: that sample is used from then on."""
    print(text, flush=True)
    session.line("<", text)
    if book is not None:
        meta = SAMPLE_META.match(text.strip())
        if meta:
            # The name is taken: the sample is used. Its pump run follows (the `=== run_sample ===` line of test_begin)
            session.open_slot = int(meta.group(1))
            book.mark_used(session.open_slot)
        # refused after the name (O-ring unknown): no pump ran, so the sample is free again
        if session.open_slot is not None and text.startswith((sched.REFUSED_PREFIX, sched.NOT_STARTED)):
            slot, session.open_slot = session.open_slot, None
            book.erase(slot)
            session.run_end(now_iso())
            message = f"sample {slot} is available again (its run did not start)"
            session.line("!", message)
            print(message, flush=True)
    if text.strip() == RUN_START_LINE:
        session.run_begin(now_iso())
    rec = parse_test_line(text)
    if rec:
        session.test(rec)
        if rec["name"] == "run_sample" and rec["id"] != "summary":
            session.open_slot = None
            session.run_end(now_iso())  # the run_sample is done: PASS or FAIL
    try:
        doc = parse_sample(text)
    except ValueError as error:
        message = f"BAD SAMPLE LINE ({error}): not saved"
        session.line("!", message)
        print(message, flush=True)
        return
    if doc is not None:
        name = session.sample(doc)
        if name:
            print(
                f"-> sample saved: {os.path.join(session.samples_dir, name)}",
                flush=True,
            )


def main(argv=None):
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument(
        "--port", default=None, help="serial port, e.g. COM6 or /dev/ttyACM0 (required unless --rebuild)"
    )
    ap.add_argument(
        "--rebuild",
        metavar="RUN_FOLDER",
        help="rebuild the samples/ folder of a finished run from its session.log (no port needed), e.g. "
        "docs/logs/test-logs/2026-10-23/02_field-test-20261023",
    )
    ap.add_argument("--baud", type=checked_baud, default=9600)
    ap.add_argument(
        "--tag",
        default="session",
        help="log name suffix, e.g. the step (s1-1b) or the rig",
    )
    ap.add_argument("--dir", default=DEFAULT_LOG_DIR, help="folder for the logs (default: docs/logs/test-logs in this repository)")
    ap.add_argument("--schedule", metavar="FILE.csv", help="the timed list of samples (CSV). Nothing runs until `schedule start` (tools/sample_schedule.py)")
    args = ap.parse_args(argv)

    if args.rebuild:
        folder, count = rebuild_samples(args.rebuild)
        print(f"Rebuilt {count} sample record(s) in {os.path.join(folder, 'samples')}", flush=True)
        return 0
    if not args.port:
        ap.error("--port is required (or --rebuild RUN_FOLDER)")

    import serial  # pyserial; imported here so the parsing tests do not need it

    session = Session(args.dir, args.tag)
    print(
        f"Logging to {session.log_path} (TEST rows: {session.test_csv}, samples: {session.samples_dir}).",
        flush=True,
    )
    print("Ctrl+C or :quit to exit.", flush=True)

    def say(message):
        print(message, flush=True)
        session.line("!", message)

    def read_line(prompt):
        print(prompt, end="", flush=True)
        raw = sys.stdin.readline()
        if not raw:
            raise EOFError("the input is closed")
        answer = raw.rstrip("\r\n")
        session.line(">", answer)
        return answer

    book_path = os.path.join(os.path.dirname(session.log_path), ss.STATE_NAME)
    try:
        book = ss.SlotBook.load(book_path)
        if book is None:  # a new folder: ask the samples (a restart with the same tag keeps its state)
            book = ss.SlotBook.new(book_path, ask_loaded(read_line, say))
        say(f"Samples (loading order): {book.summary()}")
        state_path = os.path.join(os.path.dirname(session.log_path), sched.STATE_NAME)
        plan = sched.load_plan(args.schedule, state_path, say)  # None when there is no list
    except (EOFError, KeyboardInterrupt) as error:
        say(f"Session not started: {error}.")
        session.close()
        return 1

    # DTR and RTS are set before the port opens, so the open does not pulse them (see the module doc)
    port = serial.Serial()
    port.port = args.port
    port.baudrate = args.baud
    port.timeout = 0.2
    port.dtr = False
    port.rts = False
    port.open()
    session.line("!", f"open {args.port} {args.baud} baud, DTR/RTS low")

    stop = threading.Event()

    def lost(error):
        # a cable pulled or a port gone: say so in the log and on screen, and end the session cleanly
        message = f"DISCONNECTED: {error}. Session ends; restart the tool when the port is back."
        session.line("!", message)
        print(message, flush=True)
        stop.set()

    def reader():
        buf = b""
        while not stop.is_set():
            try:
                chunk = port.read(256)
            except (serial.SerialException, OSError) as error:
                lost(error)
                return
            if not chunk:
                continue
            buf += chunk
            while b"\n" in buf:
                raw, buf = buf.split(b"\n", 1)
                text = raw.decode("utf-8", errors="replace").rstrip("\r")
                on_device_line(session, text, book)
                answer = scheduled.device_line(text) if scheduled is not None else None
                if answer is not None:
                    send_scheduled(answer)

    def send_scheduled(text):
        # a line the schedule sends: the run_sample, and the answers to the device's questions (logged with '>')
        session.line(">", f"(schedule) {text}")
        try:
            port.write((text + "\n").encode("utf-8"))
        except (serial.SerialException, OSError) as error:
            lost(error)

    scheduled = None
    if plan:
        scheduled = sched.Schedule(
            plan,
            state_path,
            pick=lambda: route_run_sample("run_sample", book, None, say),  # the bare run_sample asks nothing
            send=send_scheduled,
            say=say,
        )
        for line in scheduled.table():
            say(line)
        note = sched.spool_note(plan)
        if note:
            say(note)

    say(f"sent: {PC_LOG_HELLO} (the Due saves the sample records while this tool runs)")
    try:
        port.write((PC_LOG_HELLO + "\n").encode("utf-8"))
    except (serial.SerialException, OSError) as error:
        lost(error)

    t = threading.Thread(target=reader, daemon=True)
    t.start()
    try:
        while True:
            raw = sys.stdin.readline()
            if not raw:
                break
            line = raw.rstrip("\r\n")
            if line == ":quit":
                break
            session.line(">", line)
            words = line.split()
            if words and words[0].lower() == "schedule":
                schedule_command(words[1:], scheduled, _dt.date.today(), read_line, say)
                continue
            if scheduled is not None and words and words[0].lower() == "run_sample":
                if scheduled.active():  # the list would start a sample on top of this one: ask first
                    if not ask_yes_no("The schedule is running. Stop it so you can run this sample by hand? (y/n) ", read_line):
                        say("Not sent. The schedule keeps running.")
                        continue
                    scheduled.stop(hint=True)
                    say("Schedule stopped: no further sample starts. Its sample on the device, if any, finishes first.")
                if scheduled.busy():
                    say("A sample of the schedule is still on the device. Type run_sample when its TEST line has appeared.")
                    continue
            outgoing = route_run_sample(line, book, read_line, say)
            if outgoing is None:
                continue
            if outgoing != line:
                say(f"sent: {outgoing}")
            try:
                port.write(
                    (outgoing + "\n").encode("utf-8")
                )  # LF only: a CR breaks the console (F37)
            except (serial.SerialException, OSError) as error:
                lost(error)
                break
    except (KeyboardInterrupt, EOFError):
        pass
    finally:
        if scheduled is not None:
            scheduled.stop_now()  # the list stops; the samples not started stay waiting in schedule.csv
        stop.set()
        t.join(timeout=1)
        try:
            port.close()
        except (serial.SerialException, OSError):
            pass  # already gone (unplugged): nothing more to close
        session.line("!", "closed")
        session.close()
        c = session.counts
        print(
            f"Session: {c['PASS']} PASS, {c['FAIL']} FAIL, {c['SKIP']} SKIP, {session.samples} samples, "
            f"{session.errors} write errors -> {session.log_path}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
