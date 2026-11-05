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
        "not_given",
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
    assert [s.name for s in samples] == ["lake_A"]


def test_spaces_in_a_name_and_a_condition_become_underscores(tmp_path):
    samples = read(tmp_path, "date,time,name,condition\n+0,09:00,lake  A B,filter A\n+0,10:00,  x ,  \n")
    assert [(s.name, s.condition) for s in samples] == [("lake_A_B", "filter_A"), ("x", "not_given")]


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
        rows[1][7] == "3"
        and rows[1][8]
        and rows[1][9] == ""
        and rows[1][10] == sc.RUNNING
    )
    schedule.device_line("TEST|run_sample|3|PASS|")
    rows = rows_of(tmp_path / "schedule.csv")
    assert rows[1][9] and rows[1][10] == sc.PASS


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


# --- the spool: the depth goes to the device (2026-11-02; the next-depth question was archived 2026-11-04)


def test_a_spool_sample_is_sent_with_its_depth_and_a_spool_off_sample_is_not(tmp_path):
    on = sample(name="deep", spool=True, depth_m="2.50")
    off = sample(name="surface")
    schedule, device = make([on, off], tmp_path)
    started(schedule, on, "run_sample 4")
    assert device.sent == ["run_sample 4 depth=2.5"]
    schedule.device_line("TEST|run_sample|4|PASS|")
    started(schedule, off, "run_sample 5")
    assert device.sent[-1] == "run_sample 5"




def test_a_sample_that_leaves_the_spool_out_stops_the_schedule(tmp_path):
    # 2026-11-03 (field): a failed rewind left the tube out and the next sample ran from it. Now nothing more starts.
    first = sample("11:50", name="first", spool=True, depth_m="2")
    second = sample("12:00", name="second", spool=True, depth_m="4")
    schedule, device = make([first, second], tmp_path)
    started(schedule, first, "run_sample 4")
    schedule.device_line("TEST|run_sample|4|FAIL|spool_end=FAILED;failed_at=purge")
    assert schedule._halt.is_set()
    assert any("STOPPED" in line for line in device.said)


def test_a_refused_sample_stops_the_schedule_so_the_rest_are_not_refused_in_a_row(tmp_path):
    first = sample("11:50", name="first", spool=True, depth_m="2")
    second = sample("12:00", name="second", spool=True, depth_m="4")
    schedule, device = make([first, second], tmp_path)
    started(schedule, first, "run_sample 4")
    schedule.device_line("REFUSED | run_sample: O-ring lock state is UNKNOWN - run actuator_home")
    assert first.status == sc.REFUSED and schedule._halt.is_set()
    assert any("STOPPED" in line for line in device.said)


def test_an_x_stops_the_schedule_even_when_the_roll_up_worked(tmp_path):
    # 2026-11-03 (F225): the operator's x stops the sample; the list stops too, it does not run the next row
    first = sample("11:50", name="first", spool=True, depth_m="2")
    second = sample("12:00", name="second", spool=True, depth_m="4")
    schedule, device = make([first, second], tmp_path)
    started(schedule, first, "run_sample 4")
    schedule.device_line("TEST|run_sample|4|FAIL|slot=4;aborted=x;spool_end=rolled_up_after_x")
    assert first.status == sc.FAIL and schedule._halt.is_set()
    assert any("stopped with x" in line and "schedule start <row>" in line for line in device.said)


def test_restart_at_a_row_skips_the_waiting_rows_before_it_and_keeps_the_passed_ones(tmp_path):
    def rows():
        out = [sample("09:00", name=n) for n in ("a", "b", "c", "d")]
        for s, status in zip(out, (sc.WAITING, sc.FAIL, sc.WAITING, sc.PASS)):
            s.status = status
        return out

    before = rows()
    assert [s.status for s in sc.restart_rows(before, 3)] == [sc.SKIPPED, sc.FAIL, sc.WAITING, sc.PASS]
    assert [s.status for s in sc.restart_rows(before, 2)] == [sc.SKIPPED, sc.WAITING, sc.WAITING, sc.PASS]
    assert before[0].status == sc.WAITING  # copies: the list itself is not changed


def test_restart_at_a_passed_row_runs_it_again_and_its_slot_is_cleared(tmp_path):
    s = sample("09:00", name="a")
    s.status, s.slot, s.started = sc.PASS, "12", "2026-11-03T09:00:00"
    out = sc.restart_rows([s], 1)
    assert (out[0].status, out[0].slot, out[0].started) == (sc.WAITING, "", "")


def test_samples_at_the_same_time_keep_the_order_of_the_file(tmp_path):
    rows = [sample("09:30", name=n, date="+0") for n in ("b", "a", "c")]
    resolved = sc.resolve(rows, dt.date(2026, 10, 26))
    assert [s.name for s in resolved] == ["b", "a", "c"]


def test_the_time_order_is_kept_whatever_the_depth(tmp_path):
    # 2026-11-04: the deepest-first sort of 2026-11-02 is archived; the order is date, time, then the file
    rows = [
        sample("10:00", name="late_deep", spool=True, depth_m="15"),
        sample("09:30", name="early_shallow", spool=True, depth_m="1"),
    ]
    resolved = sc.resolve(rows, dt.date(2026, 10, 26))
    assert [s.name for s in resolved] == ["early_shallow", "late_deep"]


def test_a_depth_above_the_longest_tube_is_refused_and_the_longest_is_kept(tmp_path):
    with pytest.raises(ValueError, match="more than 40"):
        read(tmp_path, "date,time,spool,depth_m\n+0,12:00,on,40.5\n")
    assert read(tmp_path, "date,time,spool,depth_m\n+0,12:00,on,40\n")[0].depth_m == "40"


# --- depth conflicts at `schedule start` (2026-11-02, user)


def test_a_spool_sample_deeper_than_the_session_is_a_conflict(tmp_path):
    rows = [
        sample("09:30", name="deep", spool=True, depth_m="15"),
        sample("10:00", name="ok", spool=True, depth_m="5"),
        sample("10:30", name="off"),
    ]
    waiting = sc.resolve(rows, dt.date(2026, 10, 26))
    assert [s.name for s in sc.depth_conflicts(waiting, 10.0)] == ["deep"]
    assert [s.name for s in sc.depth_conflicts(waiting, 15.0)] == []  # equal to the session depth is no conflict


def test_no_session_depth_makes_every_spool_sample_a_conflict(tmp_path):
    rows = [sample("09:30", name="deep", spool=True, depth_m="2"), sample("10:00", name="off")]
    waiting = sc.resolve(rows, dt.date(2026, 10, 26))
    assert [s.name for s in sc.depth_conflicts(waiting, None)] == ["deep"]


def test_a_sample_already_run_is_not_a_conflict_any_more(tmp_path):
    rows = [sample("09:30", name="deep", spool=True, depth_m="15")]
    rows[0].status = sc.PASS
    assert sc.depth_conflicts(rows, 5.0) == []


def test_the_operator_accepts_the_deeper_samples_at_start_and_the_device_gets_y(tmp_path):
    deep = sc.resolve([sample("11:50", name="deep", spool=True, depth_m="15")], dt.date(2026, 10, 26))[0]
    schedule, device = make([deep], tmp_path)
    schedule.accepted_deeper = {sc.sample_key(deep)}
    started(schedule, deep)
    assert schedule.device_line("ASK | run_sample: solve it or proceed? y = proceed to 15.00 m  (type the answer + ENTER)") == "y"


def test_a_deep_sample_the_operator_did_not_accept_is_left_to_the_operator(tmp_path):
    # accepted at start: only the 15 m row. A 20 m row (added or edited later) is not answered by the tool
    deep = sc.resolve([sample("11:50", name="deep", spool=True, depth_m="15")], dt.date(2026, 10, 26))[0]
    other = sc.resolve([sample("12:50", name="other", spool=True, depth_m="20")], dt.date(2026, 10, 26))[0]
    schedule, device = make([other], tmp_path)
    schedule.accepted_deeper = {sc.sample_key(deep)}
    started(schedule, other)
    assert schedule.device_line("ASK | run_sample: solve it or proceed? y = proceed to 20.00 m  (type the answer + ENTER)") is None


# --- the 40 m total tube (depth + spool height), 2026-11-02 (user)


def test_a_depth_plus_the_spool_height_over_40_m_is_an_overflow(tmp_path):
    rows = [
        sample("09:30", name="at_40", spool=True, depth_m="40"),
        sample("10:00", name="at_39", spool=True, depth_m="39.5"),
        sample("10:30", name="off"),
    ]
    waiting = sc.resolve(rows, dt.date(2026, 10, 26))
    # height 0.5 m: 40 m + 0.5 m is over the 40 m total; 39.5 m + 0.5 m is exactly 40 m (no overflow)
    assert [s.name for s in sc.total_overflows(waiting, 50)] == ["at_40"]
    assert [s.name for s in sc.total_overflows(waiting, 0)] == []


def test_an_unknown_spool_height_judges_no_overflow(tmp_path):
    rows = sc.resolve([sample("09:30", name="deep", spool=True, depth_m="40")], dt.date(2026, 10, 26))
    assert sc.total_overflows(rows, None) == []


def test_an_overflow_row_is_accepted_by_its_key_only(tmp_path):
    deep = sc.resolve([sample("11:50", name="deep", spool=True, depth_m="40")], dt.date(2026, 10, 26))[0]
    other = sc.resolve([sample("12:50", name="other", spool=True, depth_m="40")], dt.date(2026, 10, 26))[0]
    schedule, device = make([deep], tmp_path)
    schedule.accepted_deeper = {sc.sample_key(deep)}
    started(schedule, deep)
    assert schedule.device_line("ASK | run_sample: solve it or proceed? y = proceed to 39.50 m  (type the answer + ENTER)") == "y"
    schedule2, _ = make([other], tmp_path)
    schedule2.accepted_deeper = {sc.sample_key(deep)}
    started(schedule2, other)
    assert schedule2.device_line("ASK | run_sample: solve it or proceed? y = proceed to 39.50 m  (type the answer + ENTER)") is None


# --- the operational order (2026-11-02, user): the order the device runs the samples is the order in the logs


def test_the_schedule_runs_and_logs_in_the_operational_order(tmp_path):
    # the rows in file order 1 m, 15 m, 5 m at one timepoint, and one earlier sample: the run and the saved list agree
    rows = [
        sample("09:00", name="one_m", spool=True, depth_m="1"),
        sample("09:00", name="fifteen_m", spool=True, depth_m="15"),
        sample("09:00", name="five_m", spool=True, depth_m="5"),
        sample("08:00", name="early", spool=False),
    ]
    state = tmp_path / "schedule.csv"
    sent = []
    slots = iter(range(1, 20))
    schedule = None

    def send(text):
        sent.append(text)
        if text.startswith("run_sample"):
            threading.Timer(
                0.02, lambda: schedule.device_line(f"TEST|run_sample|{text.split()[1]}|PASS|")
            ).start()

    schedule = sc.Schedule(
        rows,
        str(state),
        pick=lambda: f"run_sample {next(slots)}",
        send=send,
        say=lambda text: None,
        now=lambda: NOON,
    )
    schedule.start(dt.date(2026, 10, 26))
    schedule.thread.join(10)
    assert [t.split()[1] for t in sent if t.startswith("run_sample")] == ["1", "2", "3", "4"]
    names = [line.split(",")[2] for line in state.read_text(encoding="utf-8").splitlines()[1:]]
    assert names == ["early", "one_m", "fifteen_m", "five_m"]  # time order, then the file order at 09:00 (2026-11-04)


# --- the force_slot column (2026-11-03)


def test_a_forced_slot_is_read_checked_and_saved(tmp_path):
    path = tmp_path / "in.csv"
    path.write_text(
        "date,time,name,condition,spool,depth_m,force_slot\n+0,09:30,a,control,,,3\n+0,10:00,b,control,,,\n",
        encoding="utf-8",
    )
    samples = sc.read_schedule(str(path))
    assert [s.force_slot for s in samples] == ["3", ""]
    bad = tmp_path / "bad.csv"
    bad.write_text("date,time,name,condition,spool,depth_m,force_slot\n+0,09:30,a,control,,,15\n", encoding="utf-8")
    with pytest.raises(ValueError, match="force_slot"):
        sc.read_schedule(str(bad))
    state = tmp_path / "state.csv"
    sc.write_state(str(state), samples)
    assert [s.force_slot for s in sc.load_state(str(state))] == ["3", ""]


def test_a_forced_row_is_sent_to_its_own_slot_and_reserves_it(tmp_path):
    forced = sample("09:30", name="forced", spool=False)
    forced.force_slot = "7"
    sent = []

    def send(text):
        sent.append(text)
        if text.startswith("run_sample"):
            threading.Timer(0.02, lambda: schedule.device_line(f"TEST|run_sample|{text.split()[1]}|PASS|")).start()

    schedule = sc.Schedule(
        [forced],
        str(tmp_path / "state.csv"),
        pick=lambda: "run_sample 1",  # the automatic order is not used for a forced row
        send=send,
        say=lambda text: None,
        now=lambda: NOON,
        pick_slot=lambda slot: f"run_sample {slot}",
    )
    assert schedule.reserved_slots() == {7}
    schedule.start(dt.date(2026, 10, 26))
    schedule.thread.join(10)
    assert [t.split()[1] for t in sent if t.startswith("run_sample")] == ["7"]


# --- a failure before the filter was loaded (2026-11-03, user): the same filter, twice, then the next row; no thread


def run_loop(rows, tmp_path, reply):
    """Runs the list's loop (Schedule._run). Each run_sample is answered at once with reply(attempt) (1, 2, ...), a device line.
    pick gives a new slot each time, so a retry that takes another slot shows up in `sent`."""
    resolved = sc.resolve(rows, dt.date(2026, 10, 26))
    device = Device()
    slots = iter(range(1, 100))
    attempts = []

    def send(text):
        device.sent.append(text)
        slot = int(text.split()[1])
        attempts.append(slot)
        schedule.device_line(reply(len(attempts), slot))

    schedule = sc.Schedule(
        resolved, str(tmp_path / "state.csv"), pick=lambda: f"run_sample {next(slots)}", send=send,
        say=device.say, now=lambda: dt.datetime(2026, 10, 26, 10, 0),
        pick_slot=lambda slot: f"run_sample {slot}",
    )
    halt = threading.Event()
    schedule._halt = halt
    schedule._run(halt)
    return schedule, device, halt


PURGE_FAIL = "TEST|run_sample|{slot}|FAIL|slot={slot};failed_at=purge;spool_end=rolled_up_after_failure"


def test_a_failure_before_the_filter_is_tried_again_on_the_same_filter(tmp_path):
    schedule, device, halt = run_loop([sample("09:00", name="a")], tmp_path,
                                      lambda n, slot: PURGE_FAIL.format(slot=slot) if n < 3 else PURGE_FAIL.replace("FAIL", "PASS").format(slot=slot))
    assert device.sent == ["run_sample 1", "run_sample 1", "run_sample 1"]  # the first try and the retries: slot 1 each time
    assert schedule.samples[0].status == sc.PASS and not halt.is_set()
    assert any("Trying again on the same filter 1, 1 of 2" in line for line in device.said)


def test_after_two_retries_the_row_stays_failed_and_the_list_goes_on_with_the_next_row(tmp_path):
    rows = [sample("09:00", name="a"), sample("09:30", name="b")]
    schedule, device, halt = run_loop(rows, tmp_path,
                                      lambda n, slot: PURGE_FAIL.format(slot=slot) if n <= 3 else PURGE_FAIL.replace("FAIL", "PASS").format(slot=slot))
    assert len(device.sent) == 4 and not halt.is_set()  # a: three tries, then b
    assert [s.status for s in schedule.samples] == [sc.FAIL, sc.PASS]
    assert any("Not run again; the list goes on" in line for line in device.said)


def test_an_x_is_never_tried_again(tmp_path):
    schedule, device, halt = run_loop([sample("09:00", name="a")], tmp_path,
                                      lambda n, slot: f"TEST|run_sample|{slot}|FAIL|slot={slot};aborted=x;failed_at=purge;spool_end=rolled_up_after_x")
    assert len(device.sent) == 1 and halt.is_set()


def test_a_failure_with_the_spool_out_is_not_tried_again(tmp_path):
    schedule, device, halt = run_loop([sample("09:00", name="a")], tmp_path,
                                      lambda n, slot: f"TEST|run_sample|{slot}|FAIL|slot={slot};failed_at=purge;spool_end=FAILED")
    assert len(device.sent) == 1 and halt.is_set()


def test_a_failure_after_the_sampling_is_not_tried_again_and_the_list_goes_on(tmp_path):
    rows = [sample("09:00", name="a"), sample("09:30", name="b")]
    schedule, device, halt = run_loop(rows, tmp_path,
                                      lambda n, slot: f"TEST|run_sample|{slot}|FAIL|slot={slot};failed_at=sampling;spool_end=rolled_up_after_failure")
    assert len(device.sent) == 2 and not halt.is_set()
    assert [s.status for s in schedule.samples] == [sc.FAIL, sc.FAIL]
