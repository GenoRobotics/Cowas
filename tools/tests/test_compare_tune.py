"""pytest for tools/compare_tune.py (run: pytest tools/)."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import compare_tune as c  # noqa: E402


def line(slot, label, **kv):
    base = {"end": "completed", "pmax": "2.10", "ovs": "0.10", "rise": "5.0", "err": "0.02", "rip": "0.20",
            "pwp2p": "10", "rev": "4", "sat": "0"}
    base.update({k: str(v) for k, v in kv.items()})
    data = f"slot={slot};label={label};" + ";".join(f"{k}={v}" for k, v in base.items())
    return f"TEST|pressure_ramp|{slot}|PASS|{data}"


def run_file(tmp_path, lines):
    p = tmp_path / "t.log"
    p.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return str(p)


def test_better_candidate(tmp_path):
    f = run_file(tmp_path, [line(11, "base", ovs=0.40, rip=0.60), line(11, "base", ovs=0.42, rip=0.62),
                            line(11, "new", ovs=0.10, rip=0.20), line(11, "new", ovs=0.12, rip=0.22)])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 0
    text = "\n".join(lines)
    assert "ovs" in text and "BETTER" in text and "WORSE" not in text


def test_worse_on_one_slot_fails_the_whole_comparison(tmp_path):
    f = run_file(tmp_path, [line(10, "base"), line(10, "new", rip=0.21),
                            line(12, "base"), line(12, "new", rip=0.90)])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 1
    assert any("WORSE" in ln for ln in lines)


def test_noise_is_not_an_improvement(tmp_path):
    # the repeats of the baseline differ by 0.3, so a 0.2 difference is noise
    f = run_file(tmp_path, [line(11, "base", rip=0.30), line(11, "base", rip=0.60), line(11, "new", rip=0.38)])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 0
    assert not any("BETTER" in ln for ln in lines)


def test_unsafe_candidate_run(tmp_path):
    f = run_file(tmp_path, [line(12, "base"), line(12, "new", end="soft_limit")])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 1
    assert any("UNSAFE" in ln for ln in lines)


def test_rise_never_reached_is_worse(tmp_path):
    f = run_file(tmp_path, [line(11, "base", rise=6.0), line(11, "new", rise=-1)])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 1


def test_no_common_slot(tmp_path):
    f = run_file(tmp_path, [line(10, "base"), line(11, "new")])
    lines, code = c.compare(c.load_runs([f]), "base", "new")
    assert code == 2


def test_other_tests_are_ignored(tmp_path):
    f = run_file(tmp_path, ["TEST|leak_hold|5|PASS|slot=5;label=base", line(11, "base"), line(11, "new")])
    runs = c.load_runs([f])
    assert len(runs) == 2


def test_reads_the_csv_of_hw_test_log(tmp_path):
    p = tmp_path / "t.csv"
    p.write_text("host_time,tag,name,id,result,data,raw\n"
                 "2026-10-16T10:00:00,tune,pressure_ramp,11,PASS,\"slot=11;label=base;end=completed;ovs=0.1;rip=0.2\",x\n"
                 "2026-10-16T10:05:00,tune,pressure_ramp,11,PASS,\"slot=11;label=new;end=completed;ovs=0.1;rip=0.2\",x\n",
                 encoding="utf-8")
    lines, code = c.compare(c.load_runs([str(p)]), "base", "new")
    assert code == 0
