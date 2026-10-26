"""Tests of the timed sample list (sample_schedule.py): reading the file, saving it, and running it."""

import csv
import datetime as dt
import threading
import time

import pytest
import sample_schedule as sc

TODAY = dt.date(2026, 10, 26)
NOON = dt.datetime(2026, 10, 26, 12, 0)


def write_text(tmp_path, text, name="input.csv", bom=False):
    # not schedule.csv: in these tests that name is the saved list
    path = tmp_path / name
    path.write_text(text, encoding="utf-8-sig" if bom else "utf-8", newline="")
    return str(path)


def read_file(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def rows_of(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.reader(f))


def read(tmp_path, text):
    return sc.read_schedule(write_text(tmp_path, text))


def wait_for(condition, seconds=5.0):
    end = time.time() + seconds
    while time.time() < end:
        if condition():
            return True
        time.sleep(0.02)
    return False


def sample(time_text="09:30", name="A", date="2026-10-26", **kw):
    return sc.Sample(
        date,
        time_text,
        name,
        kw.pop("condition", "x"),
        kw.pop("spool", False),
        kw.pop("depth_m", ""),
        **kw,
    )


class Device:
    """What the schedule sends and says, instead of a serial port."""

    def __init__(self):
        self.sent = []
        self.said = []

    def send(self, text):
        self.sent.append(text)

    def say(self, text):
        self.said.append(text)


def make(samples, tmp_path, now=lambda: NOON, pick=None, device=None):
    device = device or Device()
    counter = iter(range(1, 100))
    pick = pick or (lambda: f"run_sample {next(counter)}")
    schedule = sc.Schedule(
        samples,
        str(tmp_path / "schedule.csv"),
        pick=pick,
        send=device.send,
        say=device.say,
        now=now,
    )
    return schedule, device


def started(schedule, s, line="run_sample 1"):
    """Puts `s` on the device, as the thread does (no thread here)."""
    schedule._halt = threading.Event()
    schedule._start_sample(s, line, schedule._halt)


# --- reading the file


def test_defaults_are_no_name_condition_not_given_and_spool_off(tmp_path):
    s = read(tmp_path, "date,time\n2026-10-26,9:30\n")[0]
    assert (s.date, s.time, s.name, s.condition, s.spool, s.depth_m) == (
        "2026-10-26",
        "09:30",
        "",
        "not given",
        False,
        "",
    )


@pytest.mark.parametrize(
    "text, expected",
    [
        ("2026-10-27", "2026-10-27"),
        ("27/10/2026", "2026-10-27"),
        ("+1", "+1"),
        ("3", "+3"),
    ],
)
def test_a_date_is_a_full_date_or_days_after_the_start(tmp_path, text, expected):
    assert read(tmp_path, f"date,time\n{text},09:00\n")[0].date == expected


@pytest.mark.parametrize(
    "line, text",
    [
        ("date,time\n2026-13-01,09:00\n", "2026-13-01"),
        ("date,time\n+0,25:00\n", "25:00"),
        ("date,time\n+0,9.30\n", "9.30"),
    ],
)
def test_a_bad_value_is_refused_with_its_line(tmp_path, line, text):
    with pytest.raises(ValueError, match="line 2"):
        read(tmp_path, line)


def test_a_file_must_name_the_date_and_time_columns(tmp_path):
    with pytest.raises(ValueError, match="date and time are required"):
        read(tmp_path, "time,name\n09:00,a\n")
    with pytest.raises(ValueError, match="unknown column"):
        read(tmp_path, "date,time,volume\n+0,09:00,100\n")
    with pytest.raises(ValueError, match="no sample rows"):
        read(tmp_path, "date,time\n\n")


def test_the_order_of_the_file_is_kept_when_read(tmp_path):
    assert [
        s.name
        for s in read(tmp_path, "date,time,name\n+1,00:30,night\n+0,09:30,early\n")
    ] == ["night", "early"]


def test_french_excel_with_semicolons_and_a_decimal_comma_is_read(tmp_path):
    s = read(tmp_path, "Date;Time;name;spool;depth_m\n+0;09:30;lac;on;2,5\n")[0]
    assert (s.name, s.spool, s.depth_m) == ("lac", True, "2.5")


def test_bom_empty_rows_and_a_quoted_line_break_in_a_name_are_handled(tmp_path):
    path = write_text(
        tmp_path, 'date,time,name,\n\n+0,09:00,"lake\nA",\n,,,\n', bom=True
    )
    samples = sc.read_schedule(path)
    assert [s.name for s in samples] == ["lake A"]


@pytest.mark.parametrize(
    "line, message",
    [
        ("date,time,spool,depth_m\n+0,12:00,on,\n", "depth_m is needed"),
        ("date,time,spool,depth_m\n+0,12:00,off,2\n", "only for spool on"),
        ("date,time,spool,depth_m\n+0,12:00,on,two\n", "must be a number"),
    ],
)
def test_spool_and_depth_must_agree(tmp_path, line, message):
    with pytest.raises(ValueError, match=message):
        read(tmp_path, line)


def test_spool_on_with_a_decimal_comma_in_a_semicolon_file_is_read(tmp_path):
    s = read(tmp_path, "date;time;spool;depth_m\n+0;12:00;on;2,5\n")[0]
    assert (s.spool, s.depth_m) == (True, "2.5")


# --- resolving the dates (at `schedule start`)


def test_resolve_counts_plus_days_from_the_start_sorts_and_keeps_the_status(tmp_path):
    late = read(tmp_path, "date,time,name\n+1,00:30,night\n+0,10:45,\n+0,09:30,\n")
    late[0].status = "PASS"
    done = sc.resolve(late, TODAY)
    assert [(s.date, s.time, s.name) for s in done] == [
        ("2026-10-26", "09:30", "2026-10-26 09:30"),
        ("2026-10-26", "10:45", "2026-10-26 10:45"),
        ("2026-10-27", "00:30", "night"),
    ]
    assert done[2].status == "PASS"
    assert late[0].date == "+1"  # the input is not changed


# --- the saved list (schedule.csv in the run folder)


def test_a_saved_list_keeps_its_status_slot_and_unresolved_dates(tmp_path):
    state = str(tmp_path / "schedule.csv")
    samples = read(
        tmp_path, "date,time,name,spool,depth_m\n+0,09:00,a,on,2.5\n+1,09:00,b,,\n"
    )
    samples[0].status, samples[0].slot = "PASS", "3"
    sc.write_state(state, samples)
    loaded = sc.load_state(state)
    assert [(s.date, s.name, s.status, s.slot) for s in loaded] == [
        ("+0", "a", "PASS", "3"),
        ("+1", "b", "waiting", ""),
    ]
    assert (loaded[0].spool, loaded[0].depth_m) == (True, "2.5")


def test_a_sample_that_was_running_when_closed_is_interrupted(tmp_path):
    state = str(tmp_path / "schedule.csv")
    running = sample()
    running.status = sc.RUNNING
    sc.write_state(state, [running])
    assert sc.load_state(state)[0].status == sc.INTERRUPTED


def test_load_plan_reads_the_file_and_saves_it_before_any_start(tmp_path):
    said = []
    state = str(tmp_path / "schedule.csv")
    assert (
        sc.load_plan(None, state, said.append) is None
    )  # no file, no saved list: nothing
    plan = sc.load_plan(
        write_text(tmp_path, "date,time\n+1,09:00\n"), state, said.append
    )
    assert plan[0].date == "+1" and "\n+1,09:00," in read_file(state)
    assert any("Nothing runs until `schedule start`" in line for line in said)


def test_load_plan_replaces_a_saved_list_that_has_not_started(tmp_path):
    state = str(tmp_path / "schedule.csv")
    sc.write_state(state, [sample(name="old")])
    plan = sc.load_plan(
        write_text(tmp_path, "date,time,name\n+0,10:00,new\n"), state, lambda line: None
    )
    assert [s.name for s in plan] == ["new"]


def test_load_plan_keeps_a_saved_list_that_has_started_and_says_so(tmp_path):
    state = str(tmp_path / "schedule.csv")
    done = sample(name="done")
    done.status = "PASS"
    sc.write_state(state, [done, sample("11:00", name="left")])
    said = []
    plan = sc.load_plan(
        write_text(tmp_path, "date,time,name\n+0,10:00,new\n"), state, said.append
    )
    assert [s.name for s in plan] == ["done", "left"]
    assert any("has started" in line and "not read" in line for line in said)


def test_load_plan_without_a_file_resumes_and_reports_an_interrupted_sample(tmp_path):
    state = str(tmp_path / "schedule.csv")
    running = sample(name="cut")
    running.status = sc.RUNNING
    sc.write_state(state, [running])
    said = []
    assert sc.load_plan(None, state, said.append)[0].status == sc.INTERRUPTED
    assert any("not run again" in line and "cut" in line for line in said)


def test_load_plan_refuses_a_broken_saved_list(tmp_path):
    state = tmp_path / "schedule.csv"
    state.write_text("date,time\nnot-a-date,09:00\n", encoding="utf-8")
    said = []
    assert sc.load_plan(None, str(state), said.append) is None
    assert any("cannot be read" in line for line in said)


# --- what the operator sees


def test_table_marks_a_passed_time_only_for_a_full_date_and_names_an_empty_name():
    lines = sc.table_lines(
        [sample("08:00", name="early"), sample("08:00", name="", date="+1")], NOON
    )
    assert lines[0].endswith(
        "early  x  spool off  [waiting]  (time passed: runs as soon as the one before has finished)"
    )
    assert lines[1] == "  +1 08:00  (date and time)  x  spool off  [waiting]"


# --- the run


def test_nothing_runs_until_start_even_when_the_times_have_passed(tmp_path):
    schedule, device = make([sample("09:30")], tmp_path)
    time.sleep(0.2)
    assert device.sent == [] and not schedule.active()


def test_preview_shows_the_dates_and_changes_nothing(tmp_path):
    schedule, _ = make([sample(date="+1", name="")], tmp_path)
    assert schedule.preview(TODAY) == [
        "  2026-10-27 09:30  2026-10-27 09:30  x  spool off  [waiting]"
    ]
    assert schedule.samples[0].date == "+1" and schedule.samples[0].name == ""


def test_start_fixes_the_dates_and_saves_them(tmp_path):
    schedule, _ = make([sample(date="+1", name="")], tmp_path)
    assert schedule.start(TODAY) is True
    schedule.stop()
    assert (
        schedule.samples[0].date == "2026-10-27"
        and schedule.samples[0].name == "2026-10-27 09:30"
    )
    assert "2026-10-27,09:30,2026-10-27 09:30" in read_file(tmp_path / "schedule.csv")


def test_start_refuses_when_nothing_is_left(tmp_path):
    done = sample()
    done.status = sc.PASS
    schedule, _ = make([done], tmp_path)
    assert schedule.start(TODAY) is False


def test_the_device_questions_are_answered_from_the_row(tmp_path):
    s = sample(name="lake A", condition="filter A")
    schedule, device = make([s], tmp_path)
    started(schedule, s, "run_sample 4")
    assert device.sent == ["run_sample 4"] and s.slot == "4" and s.status == sc.RUNNING
    assert (
        schedule.device_line("ASK | sample name  (type the answer + ENTER, x = cancel)")
        == "lake A"
    )
    assert (
        schedule.device_line(
            "ASK | condition (e.g. control, blank, filter A)  (type the answer)"
        )
        == "filter A"
    )
    assert schedule.device_line("ASK | notes (optional)  (type the answer)") == ""
    assert (
        schedule.device_line("ASK | something else  (type the answer)") is None
    )  # left to the operator
    assert any("answer it by hand" in line for line in device.said)


@pytest.mark.parametrize("result", ["PASS", "FAIL", "SKIP"])
def test_the_test_line_ends_the_sample_with_its_result(tmp_path, result):
    s = sample()
    schedule, _ = make([s], tmp_path)
    started(schedule, s)
    schedule.device_line(
        "TEST|run_sample|summary|PASS|"
    )  # the summary line is not the end
    schedule.device_line("TEST|fill_test|1|PASS|")  # nor is another command's line
    assert s.status == sc.RUNNING
    schedule.device_line(f"TEST|run_sample|1|{result}|x=1")
    assert s.status == result and s.finished and not schedule.busy()


@pytest.mark.parametrize(
    "line",
    [
        "REFUSED | run_sample: no session yet - send `session` first (site, date, experiment)",
        "REFUSED | run_sample: O-ring lock state is UNKNOWN - run actuator_home",
        "CANCELLED | x typed",
        "Sample not started.",
    ],
)
def test_a_refused_or_cancelled_sample_ends_as_refused(tmp_path, line):
    s = sample()
    schedule, _ = make([s], tmp_path)
    started(schedule, s)
    schedule.device_line(line)
    assert s.status == sc.REFUSED and not schedule.busy()


def test_the_saved_list_has_the_real_times_at_each_change(tmp_path):
    s = sample(name="lake A", condition="control", spool=True, depth_m="2.5")
    schedule, _ = make([s], tmp_path)
    started(schedule, s, "run_sample 3")
    rows = rows_of(tmp_path / "schedule.csv")
    assert rows[0] == sc.INPUT_COLUMNS + sc.STATE_COLUMNS
    assert rows[1][:6] == ["2026-10-26", "09:30", "lake A", "control", "on", "2.5"]
    assert (
        rows[1][6] == "3"
        and rows[1][7]
        and rows[1][8] == ""
        and rows[1][9] == sc.RUNNING
    )
    schedule.device_line("TEST|run_sample|3|PASS|")
    rows = rows_of(tmp_path / "schedule.csv")
    assert rows[1][8] and rows[1][9] == sc.PASS


def test_a_sample_due_while_the_one_before_runs_waits_for_it(tmp_path):
    device = Device()
    samples = [sample("09:30", "A"), sample("09:31", "B")]
    schedule, _ = make(samples, tmp_path, device=device)
    schedule.start(TODAY)
    assert wait_for(lambda: device.sent == ["run_sample 1"])
    time.sleep(0.3)
    assert device.sent == ["run_sample 1"]  # B is due, A has not finished
    schedule.device_line("TEST|run_sample|1|PASS|")
    assert wait_for(lambda: device.sent == ["run_sample 1", "run_sample 2"])
    schedule.device_line("TEST|run_sample|2|PASS|")
    schedule.thread.join(timeout=5)
    assert [s.status for s in samples] == [sc.PASS, sc.PASS]


def test_a_sample_waits_for_its_day_and_time(tmp_path):
    clock = {"now": dt.datetime(2026, 10, 26, 23, 0)}
    device = Device()
    schedule, _ = make(
        [sample("00:10", "night", date="+1")],
        tmp_path,
        now=lambda: clock["now"],
        device=device,
    )
    schedule.start(TODAY)
    time.sleep(0.3)
    assert device.sent == []
    clock["now"] = dt.datetime(2026, 10, 27, 0, 10, 1)
    assert wait_for(lambda: device.sent == ["run_sample 1"])


def test_a_sample_with_no_sample_left_is_marked_and_skipped(tmp_path):
    device = Device()
    samples = [sample()]
    schedule, _ = make(samples, tmp_path, pick=lambda: None, device=device)
    schedule.start(TODAY)
    schedule.thread.join(timeout=5)
    assert device.sent == [] and samples[0].status == sc.NO_SAMPLE_LEFT


def test_stop_lets_the_sample_on_the_device_finish_and_start_goes_on_with_the_rest(
    tmp_path,
):
    device = Device()
    samples = [sample("09:30", "A"), sample("09:31", "B")]
    schedule, _ = make(samples, tmp_path, device=device)
    schedule.start(TODAY)
    assert wait_for(lambda: device.sent == ["run_sample 1"])
    schedule.stop()
    schedule.device_line("TEST|run_sample|1|PASS|")
    time.sleep(0.3)
    assert (
        device.sent == ["run_sample 1"] and samples[1].status == sc.WAITING
    )  # nothing else started
    assert schedule.start(TODAY) is True
    assert wait_for(lambda: device.sent == ["run_sample 1", "run_sample 2"])
    schedule.device_line("TEST|run_sample|2|PASS|")


def test_closing_the_tool_keeps_the_waiting_samples_waiting_in_the_file(tmp_path):
    device = Device()
    schedule, _ = make([sample("23:59", "A")], tmp_path, device=device)
    schedule.start(TODAY)
    time.sleep(0.2)
    schedule.stop_now()
    schedule.thread.join(timeout=5)
    assert device.sent == []
    assert [s.status for s in sc.load_state(str(tmp_path / "schedule.csv"))] == [
        sc.WAITING
    ]


def test_an_interrupted_sample_is_not_run_again(tmp_path):
    cut = sample("09:30", "cut")
    cut.status = sc.INTERRUPTED
    schedule, _ = make([cut, sample("09:31", "next")], tmp_path)
    assert [s.name for s in schedule.pending()] == ["next"]


def test_a_run_by_hand_after_a_stop_says_how_to_continue_once(tmp_path):
    device = Device()
    schedule, _ = make(
        [sample("09:30", "A"), sample("23:59", "B")], tmp_path, device=device
    )
    schedule.start(TODAY)
    assert wait_for(lambda: device.sent == ["run_sample 1"])
    schedule.device_line("TEST|run_sample|1|PASS|")
    schedule.stop(hint=True)
    schedule.device_line("TEST|run_sample|9|PASS|")  # the run by hand ends
    assert any(
        "Type `schedule start`" in line and "1 sample(s) left" in line
        for line in device.said
    )
    count = len(device.said)
    schedule.device_line("TEST|run_sample|9|PASS|")
    assert len(device.said) == count


def test_a_sample_with_no_test_line_is_reported_again(tmp_path, monkeypatch):
    monkeypatch.setattr(sc, "POLL_S", 0.02)
    monkeypatch.setattr(sc, "REMIND_S", 0.1)
    device = Device()
    schedule, _ = make([sample()], tmp_path, device=device)
    schedule.start(TODAY)
    assert wait_for(lambda: any("no TEST line yet" in line for line in device.said))
    schedule.device_line("TEST|run_sample|1|PASS|")
    schedule.thread.join(timeout=5)


def test_an_error_in_the_thread_is_reported_and_the_list_can_be_started_again(tmp_path):
    device = Device()
    calls = []

    def pick_once_broken():
        calls.append(1)
        if len(calls) == 1:
            raise RuntimeError("boom")
        return "run_sample 1"

    schedule, _ = make([sample()], tmp_path, pick=pick_once_broken, device=device)
    schedule.start(TODAY)
    schedule.thread.join(timeout=5)
    assert any("STOPPED by an error" in line and "boom" in line for line in device.said)
    assert schedule.pending()
    assert schedule.start(TODAY) is True
    assert wait_for(lambda: device.sent == ["run_sample 1"])
