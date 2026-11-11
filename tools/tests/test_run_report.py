"""pytest for tools/run_report.py (run: pytest tools/)."""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import run_report as r  # noqa: E402

FIXTURE = """\
2026-11-05T16:52:05.169+01:00 < Step purge started
2026-11-05T16:52:21.298+01:00 < Pump P1 started, output 0 % (the controller ramps it from here)
2026-11-05T16:53:05.000+01:00 < Flow measured : 53
2026-11-05T16:53:05.000+01:00 < Pressure bar : 0.10
2026-11-05T16:54:05.000+01:00 < Flow measured : 222
2026-11-05T16:54:05.000+01:00 < Pressure bar : 0.04
2026-11-05T16:55:05.000+01:00 < Flow measured : 309
2026-11-05T16:56:05.000+01:00 < Flow measured : 393
2026-11-05T16:57:05.000+01:00 < Flow measured : 480
2026-11-05T16:57:22.000+01:00 < Step purge ended
2026-11-05T17:00:00.000+01:00 < PAUSE 112 in this run (cap margin, normal): pressure 3.37 bar, pump off until 3.15 bar
2026-11-05T17:00:00.200+01:00 < RESUME after 200 ms: reading 1.70 bar is at or below 3.15 bar, pump restarts from 0 % (ramp 5.0 %/s)
2026-11-05T17:00:00.200+01:00 < WARNING | pause of only 200 ms: the pressure was already 1.70 bar when the pump restarted. Check the pressure reading (F249)
2026-11-05T17:01:00.000+01:00 < PAUSE at 3.36 bar (3): waiting for 3.15 bar
2026-11-05T17:01:00.400+01:00 < RESUME at 2.00 bar, restart from 0 % (ramp 5.0 %/s)
2026-11-05T17:02:00.000+01:00 < Spool prime (try 1 of 3): flow check FAIL, end=timeout, 0 mL counted, peak 0.04 bar
2026-11-05T17:03:00.000+01:00 < Spool prime (try 2 of 3): flow check PASS, end=cond, 101 mL counted, peak 0.11 bar
2026-11-05T17:04:00.000+01:00 < WARNING | spool: origin switch held at 3468 cm out, not at the origin: ignored, the move goes on
2026-11-05T17:04:10.000+01:00 < Spool move: up to the origin switch | time 5.0 s | encoder 0 -> -2 cm ( 2 cm passed) | average 0.4 cm/s | start 30 % | stalled (encoder not counting)
2026-11-05T17:04:20.000+01:00 < Spool move: down to 4000 cm | time 3.8 s | encoder 16 -> 16 cm ( 0 cm passed) | average 0.0 cm/s | start 40 % | aborted with x
2026-11-05T17:04:30.000+01:00 < ERROR | spool_goto: aborted with x
2026-11-05T17:04:40.000+01:00 < Vent: 0.44 bar before, 0.30 bar after the wait, 0.30 bar after the air vent (NOT released)
2026-11-05T17:04:50.000+01:00 < REFUSED | rotateMotor: manifold not moved
2026-11-05T17:05:00.000+01:00 ! schedule: still waiting for E2_T1_20M_R2 (started 2026-11-05T17:39:22.278+01:00): no TEST line yet.
"""


def report(text=FIXTURE):
    return r.build_report(text.splitlines(keepends=True))


def test_pauses_are_counted_with_drop_and_length_in_both_text_versions():
    p = report()["pauses"]
    assert p["count"] == 2
    assert p["under_1s"] == 2  # both pauses (0.2 s and 0.4 s) are under a second
    assert round(p["seconds_median"], 2) == 0.3
    assert round(p["drop_median"], 2) == round(((3.37 - 1.70) + (3.36 - 2.00)) / 2, 2)


def test_purge_flow_is_read_at_each_minute_and_stops_at_the_purge_end():
    purge = report()["purge"]
    assert purge["at_minutes"] == {1: 53, 2: 222, 3: 309, 4: 393, 5: 480}
    assert purge["pressure_max"] == 0.10


def test_prime_tries_and_spool_warnings_are_counted():
    rep = report()
    assert rep["prime"] == {"pass": 1, "fail": 1}
    assert rep["spool"]["origin_held_warnings"] == 1
    assert rep["spool"]["stalled_moves"] == 1
    assert rep["spool"]["aborted_moves"] == 1


def test_error_warning_and_refusal_lines_are_counted():
    stops = report()["stops"]
    assert stops["errors"] == 1
    assert stops["aborts_x"] == 2  # the spool move line and the ERROR line that reports it
    assert stops["refused"] == 1
    assert stops["vent_not_released"] == 1
    assert stops["warnings"] == 2  # the pause warning and the origin-switch warning


def test_a_log_without_a_purge_or_pauses_gives_empty_results():
    rep = r.build_report(["2026-11-05T10:00:00.000+01:00 < Pump P1 stopped\n"])
    assert rep["purge"] is None
    assert rep["pauses"]["count"] == 0
    assert rep["pauses"]["drop_median"] is None
    text = r.render(rep)
    assert "no purge in this log" in text
    assert "count 0" in text


def test_render_lists_each_section(tmp_path):
    path = tmp_path / "session.log"
    path.write_text(FIXTURE, encoding="utf-8")
    assert r.main([str(tmp_path)]) == 0
    assert r.main([str(path)]) == 0
    assert r.main([str(tmp_path / "missing")]) == 1
    text = r.render(report())
    for heading in ("Pressure cap pauses", "Purge flow", "Spool prime tries", "Spool:", "Lines:"):
        assert heading in text
