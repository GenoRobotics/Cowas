"""pytest for the TEST-line parser of tools/hw_test_log.py (run: pytest tools/)."""
import csv
import datetime as dt
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import hw_test_log as h  # noqa: E402


def test_parses_a_full_line():
    r = h.parse_test_line("TEST|rotate_all|3|PASS|slot=3;clock=41;err_deg=0.42\r\n")
    assert r["name"] == "rotate_all"
    assert r["id"] == "3"
    assert r["result"] == "PASS"
    assert r["data"] == {"slot": "3", "clock": "41", "err_deg": "0.42"}
    assert r["data_raw"] == "slot=3;clock=41;err_deg=0.42"
    assert r["raw"] == "TEST|rotate_all|3|PASS|slot=3;clock=41;err_deg=0.42"


def test_empty_data_and_every_result():
    for res in ("PASS", "FAIL", "SKIP"):
        r = h.parse_test_line(f"TEST|valve_check|V1|{res}|")
        assert r["result"] == res
        assert r["data"] == {}


def test_value_may_contain_equals_after_the_first():
    r = h.parse_test_line("TEST|x|1|PASS|where=a=b;flag")
    assert r["data"] == {"where": "a=b", "flag": ""}


def test_leading_garbage_is_tolerated():
    r = h.parse_test_line("\xff\x00TEST|lock_cycle|summary|FAIL|pass=4;fail=1;skip=0")
    assert r["id"] == "summary"
    assert r["data"]["fail"] == "1"


def test_pipes_in_data_stay_in_data():
    # the firmware sanitizes '|', but the parser must not lose data if one slips through
    r = h.parse_test_line("TEST|a|b|PASS|k=v|w")
    assert r["data_raw"] == "k=v|w"


def test_rejects_invalid_lines():
    for line in [
        None, "", "hello", "TEST|", "TEST|a|b|PASS",  # too few fields
        "TEST|a|b|OK|x=1",                             # unknown result
        "TEST||b|PASS|", "TEST|a||PASS|",              # empty name / id
        "test|a|b|PASS|",                              # case matters
        "REFUSED | rotateMotor: O-ring lock state is UNKNOWN",
    ]:
        assert h.parse_test_line(line) is None, line


def test_log_paths_sanitize_the_tag():
    c, l = h.log_paths("fw2 bench/1", root="logs", today=dt.date(2026, 9, 28))
    assert c == os.path.join("logs", "2026-09-28_fw2_bench_1.csv")
    assert l == os.path.join("logs", "2026-09-28_fw2_bench_1.log")
    assert h.log_paths("", root="r", today=dt.date(2026, 1, 2))[0] == os.path.join("r", "2026-01-02_untagged.csv")


def test_append_csv_writes_header_once(tmp_path):
    p = tmp_path / "x.csv"
    now = dt.datetime(2026, 9, 28, 10, 0, 0)
    for line in ["TEST|a|1|PASS|k=1", "TEST|a|2|FAIL|k=2"]:
        h.append_csv(str(p), h.parse_test_line(line), "fw2", now=now)
    rows = list(csv.reader(open(p, newline="", encoding="utf-8")))
    assert rows[0] == h.CSV_COLUMNS
    assert rows[1] == ["2026-09-28T10:00:00", "fw2", "a", "1", "PASS", "k=1", "TEST|a|1|PASS|k=1"]
    assert rows[2][4] == "FAIL"
    assert len(rows) == 3


def test_firmware_format_round_trip():
    # Mirrors core::format_test_line(): 'TEST|' + name + '|' + id + '|' + RESULT + '|' + kv
    line = "|".join(["TEST", "leak_hold", "5", "PASS",
                     "slot=5;mode=sensorless;water_reason=volume;water_ml=50;leak=no"])
    r = h.parse_test_line(line)
    assert r["data"]["water_reason"] == "volume"
    assert r["data"]["leak"] == "no"
