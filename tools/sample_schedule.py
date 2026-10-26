"""The timed sample list of the session tool.

Use (docs/field/operator-tutorial.md, section 4):
1. Fill a CSV file, one row per sample (any file name, for example schedule.csv).
2. `session_log.py --schedule FILE.csv` reads it and shows it. Nothing runs.
3. `schedule start` shows the full dates, asks for a yes, then runs each sample at its time: it types `run_sample` (the next
   available sample, sample_slots.py) and answers the device's name, condition and notes questions from the row.
   A sample due while the one before is still on the device waits for that sample's TEST line.
4. `schedule stop` lets the sample on the device finish and starts no other. `schedule start` runs what is left.

The list is saved in the run folder as schedule.csv, with the full dates and, for each sample, its slot, real start and
finish time and status. Closing the tool and opening it again with the same tag loads that file. A sample that ran is not
run again; a sample that was running when the tool closed is `interrupted` and is not run again either.

Columns of the input file (first line, any case; ',' or ';' as separator, so French Excel works):
    date        2026-10-27, or 27/10/2026, or +N = N days after the day `schedule start` is first typed (+0 = that day).
    time        clock time of the PC, 24 h, HH:MM (09:30). Required.
    name        empty = the date and time.
    condition   e.g. control, blank, filter A. Empty = "not given" (the device needs one).
    spool       on or off. Empty = off. The spool is disabled in the firmware (SPOOL_USE = false): on is only recorded.
    depth_m     metres, only when spool is on (2.5 or 2,5). Empty otherwise.

Pure logic and one thread. No serial port here: session_log.py sends the lines and passes in the device's lines.
"""

import csv
import datetime as dt
import io
import os
import re
import threading
from dataclasses import dataclass, replace

INPUT_COLUMNS = ["date", "time", "name", "condition", "spool", "depth_m"]
STATE_COLUMNS = ["slot", "started_pc", "finished_pc", "status"]
STATE_NAME = "schedule.csv"  # in the run folder
NO_CONDITION = "not given"
POLL_S = 0.5  # how often the waits look at the clock
REMIND_S = 600  # while a sample has no TEST line, the log says so this often

# Status of a sample. Only WAITING samples are run by `schedule start`.
WAITING = "waiting"
RUNNING = "running"  # sent to the device, no TEST line yet
INTERRUPTED = (
    "interrupted"  # the tool was closed while it ran: not run again, check the device
)
NO_SAMPLE_LEFT = "no sample left"
REFUSED = "refused"  # the device did not start it: its slot is still unused
PASS, FAIL, SKIP = "PASS", "FAIL", "SKIP"  # the device's result
STARTED = (RUNNING, INTERRUPTED, NO_SAMPLE_LEFT, REFUSED, PASS, FAIL, SKIP)

# The device's lines (Tests_mech.cpp)
ASK_PREFIX = "ASK | "
REFUSED_PREFIX = "REFUSED | run_sample"
CANCELLED_PREFIX = "CANCELLED"
NOT_STARTED = "Sample not started"
RUN_TEST = "TEST|run_sample|"
TEST_RESULT = re.compile(r"\|(PASS|FAIL|SKIP)\|")

TIME = re.compile(r"^(\d{1,2}):(\d{2})$")
RELATIVE_DATE = re.compile(r"^\+?(\d{1,3})$")
ISO_DATE = re.compile(r"^(\d{4})-(\d{1,2})-(\d{1,2})$")
DMY_DATE = re.compile(r"^(\d{1,2})/(\d{1,2})/(\d{4})$")


@dataclass
class Sample:
    # date: "2026-10-27", or "+N" until `schedule start` (then resolved to a full date and kept)
    date: str
    time: str  # HH:MM
    name: str  # "" until resolved: then the date and time
    condition: str
    spool: bool
    depth_m: str = ""  # "" when the spool is off
    slot: str = ""
    started: str = ""  # PC time the run_sample was sent
    finished: str = ""  # PC time of its TEST line, or of the refusal
    status: str = WAITING

    @property
    def due(self):
        """The clock time of the sample. Only for a resolved date."""
        return dt.datetime.strptime(f"{self.date} {self.time}", "%Y-%m-%d %H:%M")


def stamp():
    """Local time with offset and milliseconds (the PC clock is the trusted clock)."""
    return dt.datetime.now().astimezone().isoformat(timespec="milliseconds")


def parse_time(text):
    """'9:30' or '09:30' -> '09:30'. Anything else raises ValueError."""
    found = TIME.match(text.strip())
    if not found or int(found.group(1)) > 23 or int(found.group(2)) > 59:
        raise ValueError(f"time must be HH:MM, 00:00 to 23:59, not {text.strip()!r}")
    return f"{int(found.group(1)):02d}:{found.group(2)}"


def parse_date(text):
    """'2026-10-27', '27/10/2026' -> '2026-10-27'; '+N' or 'N' -> '+N'. Anything else raises ValueError."""
    text = text.strip()
    if RELATIVE_DATE.match(text):
        return f"+{int(text.lstrip('+'))}"
    try:
        found = ISO_DATE.match(text)
        if found:
            return dt.date(*(int(g) for g in found.groups())).isoformat()
        found = DMY_DATE.match(text)
        if found:
            day, month, year = (int(g) for g in found.groups())
            return dt.date(year, month, day).isoformat()
    except ValueError:
        raise ValueError(f"not a real date: {text!r}") from None
    raise ValueError(
        f"date must be 2026-10-27 or 27/10/2026, or +N (days after the start day), not {text!r}"
    )


def sample_from_cells(cells):
    """One row ({column: text}) as a Sample. Raises ValueError with the reason."""
    if not cells.get("date", ""):
        raise ValueError("date is empty")
    spool_text = cells.get("spool", "").lower()
    if spool_text in ("", "off", "n", "no"):
        spool = False
    elif spool_text in ("on", "y", "yes"):
        spool = True
    else:
        raise ValueError(f"spool must be on or off, not {spool_text!r}")
    depth = cells.get("depth_m", "").replace(",", ".")
    if spool and not depth:
        raise ValueError("depth_m is needed when spool is on")
    if depth:
        try:
            value = float(depth)
        except ValueError:
            raise ValueError(
                f"depth_m must be a number of metres, not {depth!r}"
            ) from None
        if not spool:
            raise ValueError(
                "depth_m is only for spool on: leave it empty, or set spool to on"
            )
        if value < 0:
            raise ValueError("depth_m cannot be negative")
    return Sample(
        date=parse_date(cells["date"]),
        time=parse_time(cells.get("time", "")),
        name=cells.get("name", ""),
        condition=cells.get("condition", "") or NO_CONDITION,
        spool=spool,
        depth_m=depth,
    )


def _rows(path):
    """(line number, {column: text}) for each row of a CSV file. The header is checked; empty rows are skipped."""
    with open(
        path, encoding="utf-8-sig", newline=""
    ) as f:  # utf-8-sig: Excel may write a byte-order mark
        text = f.read()
    first = next((line for line in text.splitlines() if line.strip()), "")
    delimiter = (
        ";" if first.count(";") > first.count(",") else ","
    )  # French Excel saves ';'
    reader = csv.reader(io.StringIO(text, newline=""), delimiter=delimiter)
    header, rows = None, []
    for row in reader:
        cells = [" ".join(cell.split()) for cell in row]  # one line per cell
        if not any(cells):
            continue
        if header is None:
            header = [cell.lower() for cell in cells]
            if "date" not in header or "time" not in header:
                raise ValueError(
                    f"line {reader.line_num}: the first line must name the columns "
                    f"{','.join(INPUT_COLUMNS)} (date and time are required)"
                )
            unknown = [
                h for h in header if h and h not in INPUT_COLUMNS + STATE_COLUMNS
            ]
            if unknown:
                raise ValueError(
                    f"line {reader.line_num}: unknown column(s): {', '.join(unknown)}"
                )
            continue
        if len(cells) > len(header):
            raise ValueError(f"line {reader.line_num}: more cells than columns")
        rows.append((reader.line_num, dict(zip(header, cells))))
    return rows


def read_schedule(path):
    """The samples of a list file, in the order of the file. Raises ValueError (with the line) or OSError."""
    samples = []
    for number, cells in _rows(path):
        try:
            samples.append(sample_from_cells(cells))
        except ValueError as error:
            raise ValueError(f"line {number}: {error}") from None
    if not samples:
        raise ValueError("the file has no sample rows (only the columns, or nothing)")
    return samples


def resolve(samples, base):
    """The samples with full dates and names, in time order. `base` is the day the schedule starts: +N counts from it.
    Returns new objects; the status and results of each sample are kept."""
    out = []
    for s in samples:
        date = (
            (base + dt.timedelta(days=int(s.date[1:]))).isoformat()
            if s.date.startswith("+")
            else s.date
        )
        out.append(replace(s, date=date, name=s.name or f"{date} {s.time}"))
    return sorted(
        out, key=lambda s: (s.date, s.time)
    )  # ISO dates and HH:MM sort correctly as text


def load_state(path):
    """The samples saved in a run folder. A sample that was RUNNING when the tool was closed becomes INTERRUPTED."""
    samples = []
    for number, cells in _rows(path):
        try:
            sample = sample_from_cells(cells)
        except ValueError as error:
            raise ValueError(f"line {number}: {error}") from None
        sample.slot = cells.get("slot", "")
        sample.started = cells.get("started_pc", "")
        sample.finished = cells.get("finished_pc", "")
        sample.status = cells.get("status", "") or WAITING
        if sample.status == RUNNING:
            sample.status = INTERRUPTED
        samples.append(sample)
    return samples


def write_state(path, samples):
    """Writes the samples to `path`. A temporary file is replaced in one step, so a cut write leaves the old file."""
    tmp = path + ".tmp"
    with open(tmp, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(INPUT_COLUMNS + STATE_COLUMNS)
        for s in samples:
            w.writerow(
                [
                    s.date,
                    s.time,
                    s.name,
                    s.condition,
                    "on" if s.spool else "off",
                    s.depth_m,
                    s.slot,
                    s.started,
                    s.finished,
                    s.status,
                ]
            )
    os.replace(tmp, path)


def load_plan(path_arg, state_path, say):
    """The list this run uses, or None when there is none.

    - A saved list that has started is used; the file given is not read.
    - A saved list that has not started is replaced by the file given, so an edited file takes effect.
    - No saved list: the file given is read and saved in state_path.
    """
    if os.path.exists(state_path):
        try:
            samples = load_state(state_path)
        except (OSError, ValueError) as error:
            say(
                f"The saved schedule {state_path} cannot be read ({error}). Fix or delete it. The samples run by hand."
            )
            return None
        if any(s.status in STARTED for s in samples) or not path_arg:
            if path_arg:
                say(
                    f"The schedule of this folder has started: {state_path} is used, {path_arg} is not read."
                )
            for s in samples:
                if s.status == INTERRUPTED:
                    say(
                        f"schedule: {s.name or s.date + ' ' + s.time} was running when the tool was closed: not run "
                        "again. Check the device; run it by hand if needed."
                    )
            say(
                f"Schedule loaded from {state_path}: {sum(s.status == WAITING for s in samples)} sample(s) left to run."
            )
            return samples
        say(f"The saved schedule has not started yet: replaced by {path_arg}.")
    if not path_arg:
        return None
    try:
        samples = read_schedule(path_arg)
    except (OSError, ValueError) as error:
        say(f"Schedule not read: {error}. The samples run by hand.")
        return None
    write_state(state_path, samples)
    say(
        f"Schedule read from {path_arg} and saved in {state_path}. Nothing runs until `schedule start`."
    )
    return samples


def table_lines(samples, now):
    """One line per sample: date, time, name, condition, spool, status. A waiting sample whose time has passed is marked."""
    lines = []
    for s in samples:
        spool = f"spool on at {s.depth_m} m" if s.spool else "spool off"
        late = ""
        if s.status == WAITING and not s.date.startswith("+") and s.due < now:
            late = "  (time passed: runs as soon as the one before has finished)"
        name = s.name or "(date and time)"
        lines.append(
            f"  {s.date} {s.time}  {name}  {s.condition}  {spool}  [{s.status}]{late}"
        )
    return lines


def spool_note(samples):
    """The line about the spool, or None when no sample has it on."""
    if any(s.spool for s in samples):
        return "Spool: the depth is only written to the log. The spool is disabled in the firmware (SPOOL_USE = false)."
    return None


def run_outcome(text):
    """What one device line says about a run_sample: REFUSED (it did not start), its TEST result, or None."""
    if text.startswith((REFUSED_PREFIX, CANCELLED_PREFIX, NOT_STARTED)):
        return REFUSED
    at = text.find(RUN_TEST)
    if at >= 0 and not text[at + len(RUN_TEST) :].startswith(
        "summary"
    ):  # the summary line is not the end of a run
        found = TEST_RESULT.search(text)
        return found.group(1) if found else "done"
    return None


class Schedule:
    """Runs the list, one sample after the other, each at its time. Nothing runs until start() is called.

    pick()      -> the line for the next sample ("run_sample 4"), or None when no sample is left
    send(text)  sends one line to the device (also the answers to its questions)
    say(text)   shows and logs one line of the tool
    now         the clock (the tests pass their own)
    The list is saved in state_path at every change.
    """

    def __init__(self, samples, state_path, pick, send, say, now=None):
        self.samples = samples
        self.state_path = state_path
        self.pick = pick
        self.send = send
        self.say = say
        self.now = now or dt.datetime.now
        self.current = None  # the Sample whose run_sample is on the device
        self.answers = {}  # question label -> answer, for the current sample
        self.hint_pending = (
            False  # a run by hand ended after `stop`: say how to continue, once
        )
        self._lock = threading.Lock()  # the thread and the reader both change the state
        self._file_lock = threading.Lock()
        self._idle = (
            threading.Event()
        )  # set when no sample of the list is on the device
        self._idle.set()
        self._halt = threading.Event()  # set = no further sample starts
        self._halt.set()
        self.thread = None

    # --- what the operator sees

    def pending(self):
        return [s for s in self.samples if s.status == WAITING]

    def active(self):
        """True while the list runs (started and not stopped)."""
        return (
            self.thread is not None
            and self.thread.is_alive()
            and not self._halt.is_set()
        )

    def busy(self):
        """True while a sample of the list is on the device."""
        return self.current is not None

    def table(self):
        return table_lines(self.samples, self.now())

    def preview(self, base):
        """The list with the dates it would have if started on `base`. Nothing is changed."""
        return table_lines(resolve(self.samples, base), self.now())

    # --- control

    def start(self, base):
        """`schedule start`: fixes the dates (+N counts from `base`) and runs what is left. False if it runs already."""
        if self.active() or not self.pending():
            return False
        self.samples[:] = resolve(self.samples, base)
        self.write()
        self._halt = threading.Event()
        self.hint_pending = False
        self.thread = threading.Thread(
            target=self._run_safely, args=(self._halt,), daemon=True
        )
        self.thread.start()
        return True

    def stop(self, hint=False):
        """`schedule stop`, or a run typed by hand (hint=True: say how to continue when that run ends).
        No further sample starts; the sample on the device finishes."""
        self._halt.set()
        self.hint_pending = hint

    def stop_now(self):
        """The tool is closing. The list stops; the waiting samples stay waiting in the file."""
        self._halt.set()
        self.write()

    def write(self):
        with self._file_lock:
            try:
                write_state(self.state_path, self.samples)
            except OSError as error:
                self.say(
                    f"LOG ERROR: {os.path.basename(self.state_path)} not written ({error})"
                )

    # --- the thread

    def _run_safely(self, halt):
        try:
            self._run(halt)
        except Exception as error:  # noqa: BLE001 - a bug must not end the session silently: say so, keep the list
            with self._lock:
                if self.current is not None:
                    self.current.status = INTERRUPTED
                    self.current = None
                self._idle.set()
            halt.set()
            self.write()
            self.say(
                f"schedule STOPPED by an error ({error!r}). The samples not run yet are kept: check the device, "
                "then `schedule start` continues them."
            )

    def _run(self, halt):
        for sample in list(self.samples):
            if sample.status != WAITING:
                continue
            if not self._wait_for_time(sample, halt):
                return
            if not self._idle.is_set():
                self.say(
                    f"schedule: {sample.name} is due, the sample before is still on the device: waiting for it"
                )
            if not self._wait_idle(halt):
                return
            line = self.pick()
            if line is None:
                with self._lock:
                    sample.status = NO_SAMPLE_LEFT
                self.write()
                self.say(f"schedule: {sample.name}: no sample left, not run")
                continue
            if not self._start_sample(sample, line, halt):
                return
        if self._wait_idle(halt):
            self.say("schedule: every sample has run, been refused or been skipped")

    def _wait_for_time(self, sample, halt):
        while not halt.is_set():
            left = (sample.due - self.now()).total_seconds()
            if left <= 0:
                return True
            halt.wait(min(left, POLL_S))
        return False

    def _wait_idle(self, halt):
        """Waits until no sample of the list is on the device. False when stopped first."""
        waited = 0.0
        while not halt.is_set():
            if self._idle.wait(POLL_S):
                return (
                    not halt.is_set()
                )  # stopped while the device was busy: no next sample from this thread
            waited += POLL_S
            if waited >= REMIND_S and self.current is not None:
                waited = 0.0
                self.say(
                    f"schedule: still waiting for {self.current.name} (started {self.current.started}): "
                    "no TEST line yet. Check the device."
                )
        return False

    def _start_sample(self, sample, line, halt):
        with self._lock:
            if halt.is_set():  # stopped while this sample waited: it does not start
                return False
            self.current = sample
            self.answers = {
                "sample name": sample.name,
                "condition": sample.condition,
                "notes": "",
            }
            sample.slot = line.split()[1] if len(line.split()) > 1 else ""
            sample.started = stamp()
            sample.status = RUNNING
            self._idle.clear()
        self.write()
        self.say(
            f"schedule: {sample.name} ({sample.date} {sample.time}): sending {line}"
        )
        self.send(line)
        return True

    # --- the device's lines (called by the reader thread)

    def device_line(self, text):
        """Called with each device line. Returns the answer when the device asks the sample on the list a question.
        Ends the sample when its TEST line arrives, or when the device refuses it."""
        with self._lock:
            outcome = run_outcome(text)
            if self.current is None:
                if outcome and self.hint_pending:
                    self.hint_pending = False
                    left = len(self.pending())
                    if left:
                        self.say(
                            f"The run by hand has ended. Type `schedule start` to run the {left} sample(s) left."
                        )
                return None
            if text.startswith(ASK_PREFIX):
                label = text[len(ASK_PREFIX) :]
                for key in list(self.answers):
                    if label.startswith(key):
                        return self.answers.pop(key)
                self.say(
                    f"schedule: the device asks something the list does not answer ({label.strip()}): "
                    "answer it by hand"
                )
                return None
            if outcome:
                self._end(outcome, text.strip())
        return None

    def _end(self, outcome, device_text):
        sample, self.current = self.current, None
        sample.finished = stamp()
        sample.status = outcome
        self.write()
        self.say(
            f"schedule: {sample.name} ({sample.date} {sample.time}): {outcome} at {sample.finished}  ({device_text})"
        )
        self._idle.set()  # last: the thread goes on only after the result is shown
