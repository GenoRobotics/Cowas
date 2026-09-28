"""FakeController behaviour: the injectable faults and the protocol rules beyond the vectors."""

from __future__ import annotations

import json

import pytest

from cowas.errors import CommandFailed
from cowas.protocol.codec import dumps
from cowas.sim import (
    Clog,
    FakeController,
    FlowmeterDead,
    LinkDrop,
    PressureTrip,
    Reboot,
    SlowProcedure,
    WatchdogReset,
)
from cowas.sim.controller import messages
from cowas.sim.geometry import regular_geometry


def req(rid: str, cmd: str, **args) -> str:
    return dumps({"t": "req", "id": rid, "cmd": cmd, "args": args})


def run_cycle_line(rid: str, slot: int = 3, volume=2000, max_s: int = 1800, **extra) -> str:
    return req(
        rid, "RUN_CYCLE", slot=slot, purge_ml=None, volume_ml=volume, max_duration_s=max_s, **extra
    )


def result(sim: FakeController, rid: str, lines: list[str]) -> dict:
    return next(m for m in messages(lines) if m["t"] == "res" and m.get("id") == rid)


def events(lines: list[str], etype: str) -> list[dict]:
    return [m for m in messages(lines) if m["t"] == "evt" and m["type"] == etype]


def test_happy_cycle_record_and_rest_state():
    sim = FakeController()
    sim.feed_line(run_cycle_line("a"))
    sim.run_until_idle()
    out = sim.drain()
    rec = result(sim, "a", out)["data"]
    assert rec["completed"] and rec["end_reason"] == "volume_reached"
    assert rec["volume_ml"] == pytest.approx(2000, abs=2)
    assert [p["name"] for p in rec["phases"]] == [
        "purge",
        "water",
        "air_push",
        "shield",
        "air_push",
        "empty",
    ]
    assert rec["purge_ml_target"] == 424 and rec["purge_ml_measured"] >= 424
    assert rec["t_end_to_shield_s"] > 0 and rec["shield_ms"] == 250000
    assert (sim.mode, sim.lock, sim.slot) == ("IDLE", "locked", 0)
    tel = [m for m in messages(out) if m["t"] == "tel"]
    assert len(tel) == pytest.approx(rec["cycle_s"], abs=2)  # ~1 Hz while BUSY
    assert all(t["id"] == "a" for t in tel)


def test_telemetry_never_when_idle():
    sim = FakeController()
    sim.advance(20)
    assert not [m for m in messages(sim.drain()) if m["t"] == "tel"]


def test_pressure_trip_ends_with_pressure_cap():
    sim = FakeController()
    sim.inject(PressureTrip(step="sample", after_s=120))
    sim.feed_line(run_cycle_line("a"))
    sim.run_until_idle()
    out = sim.drain()
    res = result(sim, "a", out)
    rec = res["data"]
    assert res["ok"] is True and res["mode"] == "FAULT"  # started, so ok:true + partial record
    assert rec["end_reason"] == "pressure_cap"
    assert rec["abort"] == {"step": "sample", "reason": "fault", "fault": "OVER_PRESSURE"}
    assert rec["faults"][0]["code"] == "OVER_PRESSURE"
    assert sim.pump_pct == 0  # safe state
    assert [e["code"] for e in events(out, "fault")] == ["OVER_PRESSURE"]


def test_clog_is_a_normal_end():
    sim = FakeController()
    sim.inject(Clog())
    sim.feed_line(run_cycle_line("a", volume=None, max_s=3000))
    sim.run_until_idle()
    rec = result(sim, "a", sim.drain())["data"]
    assert rec["completed"] is True and rec["end_reason"] == "clogged"
    assert 300 < rec["volume_ml"] < 2000


@pytest.mark.parametrize("volume,end_reason", [(2000, "timeout"), (None, "duration_reached")])
def test_dead_flowmeter(volume, end_reason):
    sim = FakeController()
    sim.inject(FlowmeterDead())
    sim.feed_line(run_cycle_line("a", volume=volume, max_s=600))
    sim.run_until_idle()
    rec = result(sim, "a", sim.drain())["data"]
    assert rec["end_reason"] == end_reason
    assert rec["volume_ml"] is None and rec["purge_ml_measured"] is None
    assert rec["flow_ml_min"] == {"mean": None, "min": None, "final": None}
    assert "FLOW_SILENT" in [w["code"] for w in rec["warnings"]]  # rule 5: never silent


def test_slow_procedure_within_bounds_hits_the_firmware_timeout():
    sim = FakeController()
    sim.inject(SlowProcedure(factor=3.0))
    sim.feed_line(req("p", "PURGE", volume_ml=None))
    sim.run_until_idle()
    res = result(sim, "p", sim.drain())
    assert res["data"]["abort"]["reason"] == "timeout"
    assert res["data"]["abort"]["fault"] == "TIMEOUT"
    assert sim.mode == "FAULT"


def test_link_loss_never_stops_the_running_cycle():
    """§9: no request for 30 s -> LINK_LOST, the cycle finishes, nothing new starts."""
    sim = FakeController()
    sim.feed_line(run_cycle_line("a"))
    sim.run_until_idle()
    out = sim.drain()
    rec = result(sim, "a", out)["data"]
    assert rec["completed"] is True
    assert "LINK_LOST" in [w["code"] for w in rec["warnings"]]
    assert events(out, "link")[0]["state"] == "lost"
    sim.feed_line(req("b", "STATUS"))
    out = messages(sim.drain())
    assert out[0]["type"] == "link" and out[0]["state"] == "restored"
    assert out[-1]["data"]["link_ok"] is True


def test_dropped_lines_are_lost_in_one_direction_only():
    sim = FakeController()
    sim.inject(LinkDrop(duration_s=5, direction="d2h"))
    sim.advance(0.25)
    sim.feed_line(req("r", "ROTATE", slot=5))
    assert sim.drain() == []  # the ack is lost ...
    assert sim.running == "r"  # ... but the command was received and runs
    sim.run_until_idle()
    assert result(sim, "r", sim.drain())["ok"] is True


def test_reboot_loses_the_command_and_the_dedupe_table():
    sim = FakeController()
    sim.feed_line(req("r", "ROTATE", slot=5))
    sim.run_until_idle()
    sim.feed_line(run_cycle_line("a"))
    old = sim.boot_id
    sim.inject(Reboot(step="sample", after_s=10, usb_gap_s=0))
    sim.advance(600)
    out = sim.drain()
    assert not [m for m in messages(out) if m["t"] == "res" and m["id"] == "a"]
    boot = events(out, "boot")[0]
    assert boot["boot_id"] == sim.boot_id != old and boot["reset_cause"] == "POWER_ON"
    assert (sim.mode, sim.lock, sim.running) == ("RECOVERY_REQUIRED", "unknown", None)
    sim.feed_line(req("r", "ROTATE", slot=5))  # the old id is unknown now: evaluated again
    assert messages(sim.drain())[-1]["err"]["code"] == "wrong_mode"


def test_watchdog_reset_hangs_then_reboots():
    sim = FakeController()
    sim.inject(WatchdogReset(step="purge", after_s=5, hang_s=4, usb_gap_s=0))
    sim.feed_line(run_cycle_line("a"))
    sim.advance(6)
    sim.drain()
    sim.feed_line(req("p", "PING"))
    sim.advance(2)
    assert sim.drain() == []  # hung: no answer, no telemetry
    sim.advance(3)
    out = sim.drain()
    assert events(out, "boot")[0]["reset_cause"] == "WATCHDOG"
    assert sim.mode == "RECOVERY_REQUIRED"


def test_dedupe_keeps_16_ids_and_the_last_4_procedure_records():
    sim = FakeController()
    for i in range(5):
        sim.feed_line(req(f"e{i}", "EMPTY"))
        sim.run_until_idle()
    sim.drain()
    sim.feed_line(req("e4", "EMPTY"))  # recent: the full stored result, not a second run
    assert messages(sim.drain())[-1]["data"]["completed"] is True
    sim.feed_line(req("e0", "EMPTY"))  # its full record was evicted: summary only
    err = messages(sim.drain())[-1]["err"]
    assert err["code"] == "result_expired"
    assert err["summary"] == {"ok": True, "end_reason": None, "completed": True}
    assert sim.executions["EMPTY"] == 5
    for i in range(16):
        sim.feed_line(req(f"l{i}", "LOCK"))
        sim.run_until_idle()
    sim.feed_line(req("e4", "EMPTY"))  # beyond 16 ids: forgotten, so it runs again (§7)
    assert sim.running == "e4"


def test_small_results_are_stored_whole():
    sim = FakeController()
    sim.feed_line(req("r", "ROTATE", slot=5))
    sim.run_until_idle()
    first = result(sim, "r", sim.drain())
    sim.feed_line(req("r", "ROTATE", slot=5))
    assert messages(sim.drain()) == [first]


def test_recovery_after_stop_mid_rotation():
    sim = FakeController()
    sim.feed_line(req("r", "ROTATE", slot=7))
    sim.advance(8)  # unlocked, rotating
    sim.feed_line(req("s", "STOP"))
    sim.advance(1)
    assert (sim.lock, sim.aligned, sim.mode) == ("unlocked", False, "FAULT")
    sim.feed_line(req("c", "CLEAR_FAULT"))
    sim.feed_line(req("v", "RECOVER", confirm_moving=False))
    assert messages(sim.drain())[-1]["err"]["needs"] == ["align", "lock"]
    sim.feed_line(req("v2", "RECOVER", confirm_moving=True))
    sim.run_until_idle()
    res = result(sim, "v2", sim.drain())
    assert res["ok"] and res["mode"] == "IDLE"
    assert (sim.lock, sim.aligned) == ("locked", True)


def test_recover_cannot_align_with_the_lock_unknown():
    """Finding F71: no rotation with the lock unknown, and ACTUATOR_HOME needs alignment."""
    sim = FakeController.from_state(
        {
            "mode": "RECOVERY_REQUIRED",
            "lock": "unknown",
            "slot": None,
            "aligned": False,
            "recovery_cause": "POWER_ON",
        }
    )
    sim.feed_line(req("v", "RECOVER", confirm_moving=True))
    sim.run_until_idle()
    res = result(sim, "v", sim.drain())
    assert res["err"]["code"] == "recovery_failed" and res["err"]["step"] == "align"
    assert sim.mode == "RECOVERY_REQUIRED" and sim.lock == "unknown"


def test_unlock_leaves_a_warning():
    sim = FakeController()
    sim.feed_line(req("u", "UNLOCK"))
    sim.run_until_idle()
    out = sim.drain()
    assert "UNLOCKED_AT_REST" in [e["code"] for e in events(out, "warning")]
    sim.feed_line(req("s", "STATUS"))
    assert messages(sim.drain())[-1]["data"]["warnings"][0]["code"] == "UNLOCKED_AT_REST"


def test_estop_cause_persists_until_released():
    sim = FakeController()
    sim.press_estop()
    assert sim.mode == "ESTOP"
    sim.feed_line(req("c", "CLEAR_FAULT"))
    assert messages(sim.drain())[-1]["err"]["code"] == "cause_persists"
    sim.release_estop()
    sim.feed_line(req("c", "CLEAR_FAULT"))
    assert messages(sim.drain())[-1]["data"] == {"cleared": "ESTOP"}
    assert sim.mode == "RECOVERY_REQUIRED" and sim.recovery_cause == "ESTOP"


def test_service_mode_refuses_host_commands_but_not_stop():
    sim = FakeController()
    assert sim.enter_service()
    sim.feed_line(req("r", "ROTATE", slot=2))
    assert messages(sim.drain())[-1]["err"]["code"] == "wrong_mode"
    sim.feed_line(req("s", "STOP"))
    assert sim.mode == "FAULT"


def test_log_copies_and_bench_mode_text():
    sim = FakeController(bench_mode=True)
    sim.feed_line(req("r", "ROTATE", slot=2))
    sim.run_until_idle()
    out = sim.drain()
    logs = [json.loads(line) for line in out if line.startswith('{"t":"log"')]
    text = [line for line in out if not line.startswith("{")]
    assert logs and text
    assert len(text) == len(logs)  # every console line also goes to the host port


@pytest.mark.parametrize("raw", [16, 24])
def test_any_slot_count(raw):
    sim = FakeController(info=regular_geometry(raw))
    last = raw - 1
    sim.feed_line(run_cycle_line("a", slot=last, volume=200, max_s=600))
    sim.run_until_idle()
    rec = result(sim, "a", sim.drain())["data"]
    assert rec["slot"] == last and rec["completed"]
    sim.feed_line(req("g", "GET_INFO"))
    assert messages(sim.drain())[-1]["data"]["slot_count"] == raw


def test_client_side_view_of_a_refusal(make_client):
    client, sim, _ = make_client()
    sim.press_estop()
    with pytest.raises(CommandFailed) as exc:
        client.rotate(3)
    assert exc.value.code == "wrong_mode"
