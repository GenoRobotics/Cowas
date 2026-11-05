import argparse
import csv
import datetime as dt
import json
import os
import sys
from pathlib import Path

import pytest

sys.path.insert(
    0, str(Path(__file__).resolve().parents[1])
)  # tools/ is a folder of scripts, not a package

import session_log as sl

TODAY = dt.date(2026, 10, 21)

SAMPLE = {
    "t": "sample",
    "fw": "S1-1b-dev",
    "id": "auto-1",
    "purpose": "sample",
    "slot": 5,
    "start_ms": 1000,
    "end_ms": 9000,
    "target_ml": 100.0,
    "vol_ml": 101.25,
    "purge_ml": None,
    "end_reason": "volume",
    "p_mean_bar": 1.235,
    "p_max_bar": 1.5,
    "p_cap_bar": 3.6,
    "lock_before": "UNLOCKED",
    "lock_after": "LOCKED",
    "shield_ml": None,
    "calib": {
        "flow_factor": 77.0,
        "p_gain": 1.041,
        "p_offset_bar": -0.031,
        "abp_full_scale_psi": 60.0,
        "linear_act_locked_steps": 210,
    },
    "flags": ["placeholder:ABP_FULL_SCALE_PSI"],
}


def sample_line(doc=SAMPLE):
    return json.dumps(doc, separators=(",", ":"))


def new_session(tmp_path, tag="s1-1b"):
    return sl.Session(str(tmp_path / "docs/logs/test-logs"), tag, today=TODAY)


# ---- parsers (the old hw_test_log tests cover TEST lines in depth) ----


def test_parse_sample_line():
    doc = sl.parse_sample(sample_line())
    assert doc["id"] == "auto-1"
    assert doc["end_reason"] == "volume"
    assert doc["purge_ml"] is None


def test_parse_sample_tolerates_leading_junk():
    assert sl.parse_sample("\x00\xff" + sample_line())["slot"] == 5


def test_parse_non_sample_lines_are_none():
    assert sl.parse_sample("Step sample ended: volume, pauses 0") is None
    assert sl.parse_sample('{"t":"ack","id":3}') is None


def test_parse_broken_sample_raises():
    with pytest.raises(ValueError):
        sl.parse_sample('{"t":"sample","id":"auto-2",')


def test_parse_test_line_inside_the_merged_tool():
    rec = sl.parse_test_line("junk TEST|slot|slot3|PASS|angle=12.5;n=2")
    assert rec["name"] == "slot" and rec["result"] == "PASS"
    assert rec["data"] == {"angle": "12.5", "n": "2"}


def test_safe_name():
    assert sl.safe_name("auto-3") == "auto-3"
    assert sl.safe_name('a"b/c d') == "a_b_c_d"
    assert sl.safe_name("") == "none"


def test_checked_baud():
    assert sl.checked_baud("9600") == 9600
    with pytest.raises(argparse.ArgumentTypeError):
        sl.checked_baud("1200")  # the erase touch
    with pytest.raises(argparse.ArgumentTypeError):
        sl.checked_baud("abc")
    with pytest.raises(argparse.ArgumentTypeError):
        sl.checked_baud("50")


# ---- rebuild from the transcript (F206) ----


def test_rebuild_writes_the_sample_with_its_run_times(tmp_path):
    run = tmp_path / "01_field-test"
    run.mkdir()
    (run / "session.log").write_text(
        "2026-10-21T10:00:00.000+02:00 < === run_sample ===\n"
        f"2026-10-21T10:00:05.000+02:00 < {sample_line()}\n"
        "2026-10-21T10:00:06.000+02:00 > typed line\n"  # typed lines hold no sample
        "2026-10-21T10:01:00.000+02:00 < TEST|run_sample|3|PASS|slot=3\n",
        encoding="utf-8",
    )
    folder, count = sl.rebuild_samples(str(run))
    assert folder == str(run) and count == 1
    name = "100005000_auto-1.json"
    with open(run / "samples" / name, encoding="utf-8") as f:
        record = json.load(f)
    assert record["pc_received"] == "2026-10-21T10:00:05.000+02:00"
    assert record["run_start_pc"] == "2026-10-21T10:00:00.000+02:00"
    assert record["run_finish_pc"] == "2026-10-21T10:01:00.000+02:00"
    with open(run / "samples" / "samples.csv", encoding="utf-8") as f:
        assert len(list(csv.reader(f))) == 2  # header and one row


def test_rebuild_into_the_next_number_when_the_samples_are_there(tmp_path):
    run = tmp_path / "01_field-test"
    (run / "samples").mkdir(parents=True)
    (run / "samples" / "old.json").write_text("{}", encoding="utf-8")
    (run / "session.log").write_text(
        "2026-10-21T10:00:00.000+02:00 < === run_sample ===\n"
        f"2026-10-21T10:00:05.000+02:00 < {sample_line()}\n",
        encoding="utf-8",
    )
    folder, count = sl.rebuild_samples(str(run))
    assert folder == str(tmp_path / "02_field-test")
    assert count == 1
    assert (tmp_path / "02_field-test" / "session.log").exists()
    assert (tmp_path / "02_field-test" / "samples" / "100005000_auto-1.json").exists()
    assert (run / "samples" / "old.json").read_text(encoding="utf-8") == "{}"  # the old folder is untouched


def test_close_reports_a_missing_sample_file(tmp_path):
    s = new_session(tmp_path)
    s.samples = 1  # one sample was saved this session ...
    s.close()  # ... but its file is not on disk
    with open(s.log_path, encoding="utf-8") as f:
        assert "SAMPLES CHECK: 0 JSON files" in f.read()


# ---- files of a run ----


def test_session_files_are_in_one_numbered_folder(tmp_path):
    s = new_session(tmp_path)
    s.close()
    run = os.path.join(str(tmp_path / "docs/logs/test-logs"), "2026-10-21", "01_s1-1b")
    assert os.path.join(run, "session.log") == s.log_path
    assert os.path.join(run, "tests.csv") == s.test_csv
    assert os.path.join(run, "samples") == s.samples_dir
    assert os.path.isdir(s.samples_dir)


def test_second_run_of_the_day_gets_the_next_number_and_a_restart_reuses_its_folder(tmp_path):
    root = str(tmp_path / "docs/logs/test-logs")
    first = sl.Session(root, "field-a", today=TODAY)
    first.close()
    second = sl.Session(root, "field-b", today=TODAY)
    second.close()
    again = sl.Session(root, "field-a", today=TODAY)
    again.close()
    assert os.path.basename(os.path.dirname(first.log_path)) == "01_field-a"
    assert os.path.basename(os.path.dirname(second.log_path)) == "02_field-b"
    assert os.path.dirname(again.log_path) == os.path.dirname(first.log_path)


def test_sample_is_saved_as_json_and_csv(tmp_path):
    s = new_session(tmp_path)
    name = s.sample(sl.parse_sample(sample_line()))
    s.close()

    assert name.endswith("_auto-1.json")
    with open(os.path.join(s.samples_dir, name), encoding="utf-8") as f:
        saved = json.load(f)
    assert saved["end_reason"] == "volume"
    assert saved["file"] == name
    assert "T" in saved["pc_received"]
    assert saved["calib"]["linear_act_locked_steps"] == 210

    with open(s.samples_csv, newline="", encoding="utf-8") as f:
        rows = list(csv.reader(f))
    assert rows[0] == sl.SAMPLE_CSV_COLUMNS
    assert len(rows) == 2
    row = dict(zip(rows[0], rows[1]))
    assert row["id"] == "auto-1"
    assert row["purge_ml"] == ""  # null is an empty cell, not 0
    assert row["flags"] == "placeholder:ABP_FULL_SCALE_PSI"
    assert s.samples == 1


def test_two_samples_with_the_same_id_do_not_overwrite(tmp_path):
    s = new_session(tmp_path)
    a = s.sample(sl.parse_sample(sample_line()))
    b = s.sample(sl.parse_sample(sample_line(dict(SAMPLE, purpose="purge"))))
    s.close()
    assert a != b
    assert len(os.listdir(s.samples_dir)) == 3  # two files and the csv
    with open(s.samples_csv, newline="", encoding="utf-8") as f:
        rows = list(csv.reader(f))
    assert len(rows) == 3 and rows[2][2] == "purge"


def test_console_lines_are_logged_in_order(tmp_path):
    s = new_session(tmp_path)
    s.line("<", "Step purge: 100 mL")
    s.line(">", "stop")
    s.close()
    with open(s.log_path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    assert lines[0][:4] == "2026"  # ISO time first
    assert lines[0].endswith(" < Step purge: 100 mL")
    assert lines[1].endswith(" > stop")


def test_test_line_goes_to_csv_and_is_counted(tmp_path):
    s = new_session(tmp_path)
    s.test(sl.parse_test_line("TEST|slot|slot1|FAIL|reason=aborted"))
    s.test(sl.parse_test_line("TEST|slot|slot2|PASS|"))
    s.close()
    assert s.counts == {"PASS": 1, "FAIL": 1, "SKIP": 0}
    with open(s.test_csv, newline="", encoding="utf-8") as f:
        rows = list(csv.reader(f))
    assert rows[0] == sl.TEST_CSV_COLUMNS
    assert rows[1][4] == "FAIL" and rows[2][3] == "slot2"


def test_device_line_is_shown_logged_and_recorded(tmp_path, capsys):
    s = new_session(tmp_path)
    sl.on_device_line(s, "TEST|home|h1|PASS|done=1")
    sl.on_device_line(s, sample_line())
    sl.on_device_line(s, "Step sample ended: volume")
    s.close()

    shown = capsys.readouterr().out
    assert "TEST|home|h1|PASS|done=1" in shown
    assert "sample saved" in shown
    assert s.counts["PASS"] == 1 and s.samples == 1
    with open(s.log_path, encoding="utf-8") as f:
        assert len(f.read().splitlines()) == 3


def test_bad_sample_line_is_reported_not_dropped(tmp_path, capsys):
    s = new_session(tmp_path)
    sl.on_device_line(s, '{"t":"sample","id":"auto-9",')
    s.close()
    assert "BAD SAMPLE LINE" in capsys.readouterr().out
    assert s.samples == 0
    with open(s.log_path, encoding="utf-8") as f:
        assert "BAD SAMPLE LINE" in f.read()


def test_write_error_is_reported_and_gap_is_marked(tmp_path, capsys):
    s = new_session(tmp_path)
    # a file where the samples folder should be: every write into it fails
    blocker = tmp_path / "blocked"
    blocker.write_text("x")
    real_dir = s.samples_dir
    s.samples_dir = str(blocker)

    assert s.sample(sl.parse_sample(sample_line())) is None
    assert s.errors == 1
    assert "LOG ERROR" in capsys.readouterr().out

    # the console keeps logging, and the first line that is written says there was a gap
    s.line("<", "still running")
    s.close()
    with open(s.log_path, encoding="utf-8") as f:
        text = f.read()
    assert "LOG GAP" in text
    assert text.rstrip().endswith("< still running")
    assert os.path.isdir(real_dir)


# ---- run times of a run_sample (2026-10-23): start from the "=== run_sample ===" line, finish from its TEST line ----


def test_run_start_and_finish_go_into_every_record_of_the_run(tmp_path):
    s = new_session(tmp_path)
    s.run_begin("2026-10-23T18:00:00+02:00")
    a = s.sample(sl.parse_sample(sample_line(dict(SAMPLE, purpose="purge"))))
    b = s.sample(sl.parse_sample(sample_line()))
    s.run_end("2026-10-23T18:05:00+02:00")
    s.close()
    for name in (a, b):
        with open(os.path.join(s.samples_dir, name), encoding="utf-8") as f:
            saved = json.load(f)
        assert saved["run_start_pc"] == "2026-10-23T18:00:00+02:00"
        assert saved["run_finish_pc"] == "2026-10-23T18:05:00+02:00"
    with open(s.samples_csv, newline="", encoding="utf-8") as f:
        rows = [dict(zip(sl.SAMPLE_CSV_COLUMNS, r)) for r in list(csv.reader(f))[1:]]
    assert [r["run_finish_pc"] for r in rows] == ["2026-10-23T18:05:00+02:00"] * 2


def test_sample_outside_a_run_has_no_run_times(tmp_path):
    s = new_session(tmp_path)
    name = s.sample(sl.parse_sample(sample_line()))
    s.close()
    with open(os.path.join(s.samples_dir, name), encoding="utf-8") as f:
        saved = json.load(f)
    assert saved["run_start_pc"] == "" and saved["run_finish_pc"] == ""


def test_device_lines_of_a_run_sample_stamp_the_run(tmp_path, capsys):
    s = new_session(tmp_path)
    sl.on_device_line(s, "=== run_sample ===")
    sl.on_device_line(s, sample_line(dict(SAMPLE, purpose="purge")))
    sl.on_device_line(s, "TEST|run_sample|7|PASS|slot=7;sampled_ml=1001")
    s.close()
    files = [f for f in os.listdir(s.samples_dir) if f.endswith(".json")]
    assert len(files) == 1
    with open(os.path.join(s.samples_dir, files[0]), encoding="utf-8") as f:
        saved = json.load(f)
    assert saved["run_start_pc"] and saved["run_finish_pc"]
    assert saved["run_finish_pc"] >= saved["run_start_pc"]


def test_summary_test_line_does_not_end_a_run(tmp_path):
    s = new_session(tmp_path)
    sl.on_device_line(s, "=== run_sample ===")
    sl.on_device_line(s, sample_line())
    sl.on_device_line(s, "TEST|run_sample|summary|PASS|pass=1;fail=0;skip=0")
    s.close()
    assert s.run_start is not None  # still running: only the per-slot TEST line ends it


class _FakeSession:
    """The attributes on_device_line sets, with no file behind them."""

    spool_max_m = None
    spool_height_cm = None

    def line(self, direction, text):
        pass


def test_the_session_depth_and_the_spool_height_come_from_the_device(tmp_path):
    session = _FakeSession()
    sl.on_device_line(session, "META|session;site=L;date=2026-11-02;experiment=E;operator=;spool=installed;spool_max_m=12.50")
    assert session.spool_max_m == 12.5
    sl.on_device_line(session, "META|session;site=L;date=2026-11-02;experiment=E;operator=;spool=absent")
    assert session.spool_max_m is None
    sl.on_device_line(session, "META|spool;height_cm=35")
    assert session.spool_height_cm == 35.0
    sl.on_device_line(session, "META|spool;something_else=y")
    assert session.spool_height_cm == 35.0  # a line without the height changes nothing


def test_the_depth_warnings_come_before_the_schedule_list(tmp_path):
    # 2026-11-02 (user): a 15 m sample against a 10 m session is printed before the list, and before any question about it
    import datetime as dt

    import sample_schedule as sc

    csv_path = tmp_path / "in.csv"
    csv_path.write_text("date,time,name,condition,spool,depth_m" + chr(10) + "+0,09:00,deep,control,on,15" + chr(10), encoding="utf-8")
    samples = sc.read_schedule(str(csv_path))
    scheduled = sc.Schedule(samples, str(tmp_path / "state.csv"), pick=lambda: "run_sample 1", send=lambda t: None, say=lambda t: None)
    out = []
    answers = iter(["y", "n"])  # run the conflict anyway? then start? -> no start
    sl.schedule_command(["start"], scheduled, dt.date(2026, 10, 26), lambda prompt: next(answers), out.append,
                        session_max_m=10.0, spool_height_cm=0.0)
    conflict = next(i for i, line in enumerate(out) if line.startswith("CONFLICT"))
    listing = next(i for i, line in enumerate(out) if line.startswith("The schedule will be"))
    assert conflict < listing
    assert not scheduled.active()


# --- at the start: the samples used before this start; the forced slots at `schedule start` (2026-11-03)


def test_the_start_question_lists_the_available_samples_and_the_next_is_the_first_of_them(tmp_path):
    import sample_slots as ss

    book = ss.SlotBook.new(str(tmp_path / "slots.json"), len(ss.LOADING_ORDER))
    answers = iter(["x", "4-7,9,12-14", "y"])  # a bad answer is asked again; the list is applied on y
    out = []
    sl.ask_available_at_start(book, lambda prompt: next(answers), out.append)
    assert [book.status(n) for n in (4, 7, 9, 12)] == [ss.AVAILABLE] * 4
    assert book.status(1) == ss.USED and book.status(8) == ss.USED
    assert book.next_available() == 14  # the first available in the loading order (1, 2, 3, 14, 13, ...)
    assert any(line.startswith("REQUIRED") for line in out)


def test_the_start_list_is_read_back_and_applied_only_on_yes(tmp_path):
    # F244 (2026-11-04): the wrong answer 5-6 for 4-5 was applied with no check. Now it is read back first.
    import sample_slots as ss

    book = ss.SlotBook.new(str(tmp_path / "slots.json"), len(ss.LOADING_ORDER))
    answers = iter(["5-6,8-11", "n", "4-5,8-11", "y"])
    out = []
    sl.ask_available_at_start(book, lambda prompt: next(answers), out.append)
    readbacks = [line for line in out if line.startswith("Read back")]
    assert readbacks[0] == "Read back: available 5, 6, 8, 9, 10, 11; becomes used 1, 2, 3, 4, 7, 12, 13, 14."
    assert book.status(4) == ss.AVAILABLE and book.status(6) == ss.USED


def test_nothing_is_applied_before_the_yes(tmp_path):
    import sample_slots as ss

    book = ss.SlotBook.new(str(tmp_path / "slots.json"), len(ss.LOADING_ORDER))
    answers = iter(["5-6,8-11", "n", ""])  # the list is typed, read back and refused; ENTER then leaves it as it was
    sl.ask_available_at_start(book, lambda prompt: next(answers), lambda t: None)
    assert book.status(6) == ss.AVAILABLE and book.status(1) == ss.AVAILABLE


def test_the_start_question_all_and_enter(tmp_path):
    import sample_slots as ss

    book = ss.SlotBook.new(str(tmp_path / "slots.json"), len(ss.LOADING_ORDER))
    book.mark_used(3)
    answers = iter(["all", "y"])
    sl.ask_available_at_start(book, lambda prompt: next(answers), lambda t: None)
    assert book.status(3) == ss.AVAILABLE
    book.mark_used(5)
    sl.ask_available_at_start(book, lambda prompt: "", lambda t: None)  # ENTER: no change
    assert book.status(5) == ss.USED


def test_a_forced_slot_that_is_used_is_asked_before_the_list_and_nothing_starts_on_n(tmp_path):
    import datetime as _dt

    import sample_slots as ss
    import sample_schedule as sc

    book = ss.SlotBook.new(str(tmp_path / "slots.json"), len(ss.LOADING_ORDER))
    book.mark_used(7)
    csv_path = tmp_path / "in.csv"
    csv_path.write_text("date,time,name,condition,spool,depth_m,force_slot\n+0,09:00,deep,control,,,7\n", encoding="utf-8")
    scheduled = sc.Schedule(sc.read_schedule(str(csv_path)), str(tmp_path / "state.csv"), pick=lambda: "run_sample 1",
                            send=lambda t: None, say=lambda t: None)
    answers = iter(["n"])
    out = []
    sl.schedule_command(["start"], scheduled, _dt.date(2026, 10, 26), lambda p: next(answers), out.append,
                        session_max_m=40.0, spool_height_cm=0.0, book=book)
    assert any(line.startswith("SLOT | forced slot 7") for line in out)
    assert "The schedule will be:" not in out  # stopped before the list
    assert "Not started." in out
    assert book.status(7) == ss.USED and not scheduled.active()


def test_two_rows_forced_to_the_same_slot_block_the_start(tmp_path):
    import datetime as _dt

    import sample_schedule as sc

    csv_path = tmp_path / "in.csv"
    csv_path.write_text(
        "date,time,name,condition,spool,depth_m,force_slot\n+0,09:00,one,control,,,4\n+0,10:00,two,control,,,4\n",
        encoding="utf-8",
    )
    scheduled = sc.Schedule(sc.read_schedule(str(csv_path)), str(tmp_path / "state.csv"), pick=lambda: "run_sample 1",
                            send=lambda t: None, say=lambda t: None)
    out = []
    sl.schedule_command(["start"], scheduled, _dt.date(2026, 10, 26), lambda p: "y", out.append, book=None)
    assert any(line.startswith("BLOCKED | slot 4 is forced by two rows") for line in out)
    assert not scheduled.active()


def test_schedule_start_at_a_row_shows_the_restart_list_and_changes_nothing_until_yes(tmp_path):
    # 2026-11-03 (user): after a failure is solved, the list restarts at the row chosen. Nothing changes until the yes
    import datetime as dt

    import sample_schedule as sc

    csv_path = tmp_path / "in.csv"
    csv_path.write_text("date,time,name,condition,spool,depth_m" + chr(10) + "+0,09:00,a,control,off," + chr(10)
                        + "+0,09:30,b,control,off," + chr(10), encoding="utf-8")
    samples = sc.read_schedule(str(csv_path))
    scheduled = sc.Schedule(samples, str(tmp_path / "state.csv"), pick=lambda: "run_sample 1", send=lambda t: None, say=lambda t: None)
    samples[0].status = sc.FAIL
    out = []
    sl.schedule_command([], scheduled, dt.date(2026, 10, 26), lambda p: "n", out.append)
    assert out[0].startswith("  1 ")  # the rows are numbered
    out.clear()
    sl.schedule_command(["start", "2"], scheduled, dt.date(2026, 10, 26), lambda p: "n", out.append)
    assert any(line.startswith("Restart at row 2") for line in out)
    assert "Not started." in out and samples[0].status == sc.FAIL and samples[1].status == sc.WAITING
    out.clear()
    sl.schedule_command(["start", "9"], scheduled, dt.date(2026, 10, 26), lambda p: "n", out.append)
    assert out[0].startswith("REFUSED")


def test_a_run_stopped_before_its_filter_was_loaded_frees_the_slot(tmp_path):
    # 2026-11-03 (user): a filter is precious. Stopped during the dive or the purge: no filter was used, so it is available.
    # Stopped from the sampling on: the filter is used.
    import sample_slots as ss

    for stopped_at, expect in (("purge", ss.AVAILABLE), ("dive", ss.AVAILABLE), ("sampling", ss.USED)):
        s = new_session(tmp_path / stopped_at)
        book = ss.SlotBook.new(str(tmp_path / stopped_at / "slots.json"), len(ss.LOADING_ORDER))
        s.open_slot = 12
        book.mark_used(12)
        sl.on_device_line(s, f"TEST|run_sample|12|FAIL|slot=12;aborted=x;failed_at={stopped_at}", book)
        assert book.status(12) == expect, stopped_at
        assert s.open_slot is None
