"""The timed sample list of the session tool.

Use (docs/field/operator-tutorial.md, section 2b):
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
    name        empty = the date and time. A space becomes _ (lake A -> lake_A): the device takes no space in a name.
    condition   e.g. control, blank, filter A. A space becomes _ (filter A -> filter_A). Empty = "not_given" (the device needs one).
    spool       on or off. Empty = off. On = the device dives to depth_m before the purge and rolls the tube up after the shield.
                Off = the spool is not moved (the session must still say whether the module is on the machine: it sets the purge).
    depth_m     metres, 0 to 40, only when spool is on (2.5 or 2,5). Empty otherwise.

Samples run in date and time order; samples at the same time run in the order of the file. Every spool sample starts from a dry
tube: the device dives to the session's deepest point, primes, and rises to the sample's depth (2026-11-04: the Monday
"deepest first, keep the tube out" plan is archived in docs/archive/code/superseded). The tube is rolled up and emptied after
each sample, so the device asks no next-depth question.

Pure logic and one thread. No serial port here: session_log.py sends the lines and passes in the device's lines.
"""

import csv
import datetime as dt
import io
import os
import re
import threading
from dataclasses import dataclass, replace

import sample_slots as ss

INPUT_COLUMNS = ["date", "time", "name", "condition", "spool", "depth_m", "force_slot"]
STATE_COLUMNS = ["slot", "started_pc", "finished_pc", "status"]
STATE_NAME = "schedule.csv"  # in the run folder
NO_CONDITION = "not_given"  # no space: the device takes none in a name or condition
MAX_TUBE_M = 40.0  # the total tube, depth + spool height (SPOOL_MAX_LENGTH_CM in Settings.h); the device never goes beyond it
MAX_DEPTH_M = 40.0  # the longest tube of the spool (SPOOL_MAX_LENGTH_CM in Settings.h; the device also checks the height)
POLL_S = 0.5  # how often the waits look at the clock
REMIND_S = 600  # while a sample has no TEST line, the log says so this often
RETRY_MAX = 2  # a row that fails before its filter was loaded is tried again this many times, on the same filter (then the list goes on)

# Status of a sample. Only WAITING samples are run by `schedule start`.
WAITING = "waiting"
RUNNING = "running"  # sent to the device, no TEST line yet
INTERRUPTED = (
    "interrupted"  # the tool was closed while it ran: not run again, check the device
)
NO_SAMPLE_LEFT = "no sample left"
REFUSED = "refused"  # the device did not start it: its slot is still unused
SKIPPED = "skipped"  # waiting, but before the row chosen at `schedule start <row>`: not run
PASS, FAIL, SKIP = "PASS", "FAIL", "SKIP"  # the device's result
STARTED = (RUNNING, INTERRUPTED, NO_SAMPLE_LEFT, REFUSED, SKIPPED, PASS, FAIL, SKIP)
# the statuses that `schedule start <row>` puts back to waiting from the row on (a PASS is never run twice)
RERUN_STATUSES = (FAIL, REFUSED, INTERRUPTED, NO_SAMPLE_LEFT, WAITING)

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
    force_slot: str = ""  # "" = the next available sample; a sample number = this row runs on that slot (2026-11-03)

    @property
    def due(self):
        """The clock time of the sample. Only for a resolved date."""
        return dt.datetime.strptime(f"{self.date} {self.time}", "%Y-%m-%d %H:%M")


def depth_conflicts(samples, session_max_m):
    """The waiting spool samples deeper than the session's deepest point (`session`, spool_max_m). session_max_m None = the
    session has no spool depth: every waiting spool sample counts as a conflict. Checked once, at `schedule start`."""
    out = []
    for s in samples:
        if s.status != WAITING or not s.spool:
            continue
        if session_max_m is None or float(s.depth_m) > session_max_m + 1e-6:
            out.append(s)
    return out


def total_overflows(samples, spool_height_cm):
    """The waiting spool samples whose TOTAL tube (depth + spool height) would pass MAX_TUBE_M: the device cuts them there,
    and the operator is asked at `schedule start` (2026-11-02, user). spool_height_cm None = unknown: nothing is judged."""
    if spool_height_cm is None:
        return []
    out = []
    for s in samples:
        if s.status != WAITING or not s.spool:
            continue
        if float(s.depth_m) + spool_height_cm / 100.0 > MAX_TUBE_M + 1e-6:
            out.append(s)
    return out


def sample_key(sample):
    """What identifies a resolved row for the depth conflict answer (date, time, name and depth)."""
    return (sample.date, sample.time, sample.name, sample.depth_m)


def depth_arg(depth_m):
    """'2.50' -> '2.5': the depth as the device reads it."""
    return f"{float(depth_m):g}"


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
        if value > MAX_DEPTH_M:
            raise ValueError(
                f"depth_m cannot be more than {MAX_DEPTH_M:g} m (the longest tube of the spool)"
            )
    force = cells.get("force_slot", "").strip()
    if force:
        if not force.isdigit() or int(force) not in ss.LOADING_ORDER:
            raise ValueError(
                f"force_slot must be a sample number 1 to {len(ss.LOADING_ORDER)}, or empty, not {force!r}"
            )
        force = str(int(force))
    return Sample(
        force_slot=force,
        date=parse_date(cells["date"]),
        time=parse_time(cells.get("time", "")),
        name=cells.get("name", "").replace(" ", "_"),  # the device takes no space in a sample name
        condition=cells.get("condition", "").replace(" ", "_") or NO_CONDITION,
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
    # ISO dates and HH:MM sort correctly as text. Samples at the same time keep the order of the file (2026-11-04: the
    # deepest-first sort of 2026-11-02 is archived with the keep-out)
    return sorted(
        out, key=lambda s: (s.date, s.time)
    )


def restart_rows(samples, row):
    """The list after `schedule start <row>` (row = 1-based, as `schedule` numbers it). Copies: nothing is changed.
    Rows before `row` that still wait are SKIPPED (not run). The row itself waits again, even if it passed. From it on, every
    row that has not passed waits again (RERUN_STATUSES). PASS stays PASS. Slot and times of a re-run row are cleared; the
    earlier attempt stays in the session log and the sample records."""
    out = []
    for number, s in enumerate(samples, 1):
        t = replace(s)
        if number < row:
            if t.status == WAITING:
                t.status = SKIPPED
        elif number == row or t.status in RERUN_STATUSES:
            t.status = WAITING
            t.slot = t.started = t.finished = ""
        out.append(t)
    return out


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
                    s.force_slot,
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
        if s.force_slot:
            spool += f", forced to slot {s.force_slot}"
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
        return (
            "Spool: the device dives to each depth, samples, then rolls up (or goes straight to the next depth if that "
            "sample is waiting). The session must have the spool answer y, and spool_height must be set after a reset."
        )
    return None


def unused_filter(device_text):
    """True when the run stopped before its filter was loaded: the dive or the purge (the filter is only touched at the
    sampling, D75). The device's TEST line says failed_at=dive or failed_at=purge."""
    return "failed_at=dive" in device_text or "failed_at=purge" in device_text


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

    def __init__(self, samples, state_path, pick, send, say, now=None, pick_slot=None):
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
        self.accepted_deeper = set()  # set by start(): the depth conflicts the operator accepted
        # pick_slot(slot) -> the line for a row that forces its slot (2026-11-03); the default sends it as typed
        self.pick_slot = pick_slot or (lambda slot: f"run_sample {slot}")
        self.thread = None
        self.retries = {}  # id(sample) -> how many times the row was tried again (a failure before its filter was loaded)
        self.retry_slot = {}  # id(sample) -> the filter of the failed try: the retry uses that same filter

    # --- what the operator sees

    def pending(self):
        return [s for s in self.samples if s.status == WAITING]

    def _retry_wanted(self, sample, outcome, device_text):
        """A FAIL before the filter was loaded, with the spool not left out and no x, is tried again (RETRY_MAX times per row).
        Any other failure goes on as before (the halt rules in _end)."""
        return (
            outcome == FAIL
            and unused_filter(device_text)
            and "spool_end=FAILED" not in device_text
            and "aborted=x" not in device_text
            and self.retries.get(id(sample), 0) < RETRY_MAX
        )

    def reserved_slots(self):
        """The slots the waiting rows force: the automatic order must not take them (2026-11-03)."""
        return {int(s.force_slot) for s in self.pending() if s.force_slot}

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

    def preview(self, base, rows=None):
        """The list with the dates it would have if started on `base` (rows: the list a restart would run, see restart_rows).
        Nothing is changed."""
        return table_lines(resolve(self.samples if rows is None else rows, base), self.now())

    # --- control

    def start(self, base, accepted_deeper=(), rows=None):
        """`schedule start`: fixes the dates (+N counts from `base`) and runs what is left. False if it runs already.
        accepted_deeper: the sample_key()s of the rows the operator accepted at start (the depth conflicts). Only those
        get `y` when the device asks about their depth; any other deep sample is left to the operator.
        rows: the list to run instead of the saved one (`schedule start <row>`, see restart_rows)."""
        if self.active():
            return False
        if rows is not None:
            self.samples[:] = rows
        if not self.pending():
            return False
        self.samples[:] = resolve(self.samples, base)
        self.write()
        self.accepted_deeper = set(accepted_deeper)
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
        i = 0
        while i < len(self.samples):
            sample = self.samples[i]
            if sample.status != WAITING:
                i += 1
                continue
            if not self._wait_for_time(sample, halt):
                return
            if not self._idle.is_set():
                self.say(
                    f"schedule: {sample.name} is due, the sample before is still on the device: waiting for it"
                )
            if not self._wait_idle(halt):
                return
            # a retry takes the same filter as the failed try (it was not used): the row's own slot, not the next free one
            retry_slot = self.retry_slot.get(id(sample))
            if retry_slot is not None:
                line = self.pick_slot(retry_slot)
            elif sample.force_slot:
                line = self.pick_slot(int(sample.force_slot))
            else:
                line = self.pick()
            if line is None:
                with self._lock:
                    sample.status = NO_SAMPLE_LEFT
                self.write()
                if retry_slot is not None:
                    self.say(f"schedule: {sample.name}: its filter {retry_slot} is not free for the retry, not run")
                else:
                    self.say(f"schedule: {sample.name}: no sample left, not run")
                i += 1
                continue
            if not self._start_sample(sample, line, halt):
                return
            # the sample ends first. _end either ends the row, or sets it WAITING again for a retry (the same row, next pass)
            if not self._wait_idle(halt):
                return
            if sample.status != WAITING:
                i += 1
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
            if sample.spool:
                line += f" depth={depth_arg(sample.depth_m)}"
                if sample_key(sample) in self.accepted_deeper:
                    self.answers["run_sample: solve it or proceed?"] = "y"
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
                        answer = self.answers.pop(key)
                        return answer() if callable(answer) else answer
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
        if self._retry_wanted(sample, outcome, device_text):
            # 2026-11-03 (user): a failure before the filter was loaded used no filter: the same row is tried again, on the next
            # free filter (the released one comes first). The list does not stop
            used = self.retries.get(id(sample), 0) + 1
            self.retries[id(sample)] = used
            self.retry_slot[id(sample)] = int(sample.slot)  # the same filter: it was not used
            sample.status = WAITING
            sample.slot = sample.started = sample.finished = ""
            self.write()
            self.say(
                f"schedule: {sample.name} ({sample.date} {sample.time}): failed before its filter was loaded "
                f"({device_text.split('failed_at=')[-1].split(';')[0]}). Trying again on the same filter "
                f"{self.retry_slot[id(sample)]}, {used} of {RETRY_MAX}"
            )
            self._idle.set()
            return
        sample.finished = stamp()
        sample.status = outcome
        self.retry_slot.pop(id(sample), None)  # the row is over: no retry pending on its filter
        self.write()
        self.say(
            f"schedule: {sample.name} ({sample.date} {sample.time}): {outcome} at {sample.finished}  ({device_text})"
        )
        # Any of these stops the list: no further sample starts. The samples left stay waiting in the file. The operator
        # fixes the cause, then `schedule start <row>` runs again from the row chosen (see `schedule`).
        restart = "Then `schedule start <row>` runs again from the row you choose (`schedule` shows the numbers)."
        if "spool_end=FAILED" in device_text:
            # the tube may be out: the next sample would dive from a wrong position
            self._halt.set()
            self.say(
                "schedule STOPPED: the spool did not roll up (spool_end=FAILED). The tube may be out: check it, "
                f"spool_home. {restart}"
            )
        elif "aborted=x" in device_text:
            # x (2026-11-03, F225): the operator stopped this sample, so the list stops too
            self._halt.set()
            self.say(f"schedule STOPPED: the sample was stopped with x. Check the device and the tube. {restart}")
        elif outcome == REFUSED:
            # a refusal (O-ring UNKNOWN, no session, height, manifold not aligned, a question not answered) comes back for every
            # sample until the device is fixed: stop, so the samples left are not all refused in a row
            self._halt.set()
            self.say(f"schedule STOPPED: the device refused a sample (the reason is above). Fix it. {restart}")
        elif outcome == FAIL and unused_filter(device_text):
            # the tries of this row are used up (user, 2026-11-03): the row stays FAIL, its filter stays free, and the list goes on
            # with the next row at its own time
            self.say(f"schedule: {sample.name}: {RETRY_MAX + 1} tries failed before the filter was loaded. Not run again; the list goes on.")
        self._idle.set()  # last: the thread goes on only after the result is shown
