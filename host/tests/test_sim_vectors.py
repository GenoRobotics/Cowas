"""The simulator answers every request vector exactly as protocol/vectors.json says, and
replays the three sequences (PROTOCOL.md §13).

Log copies ({"t":"log"}) are left out of the comparison: the vectors list the protocol
messages only (finding F72).
"""

from __future__ import annotations

import json
from typing import Any

import pytest

from cowas.protocol.vectors import load_vectors, merged_state, mismatch, request_line
from cowas.sim import FakeController, Reboot
from cowas.sim.controller import messages

VECTORS = load_vectors()


def protocol_only(lines: list[str]) -> list[dict[str, Any]]:
    return [msg for msg in messages(lines) if msg["t"] != "log"]


@pytest.mark.parametrize("vec", VECTORS["requests"], ids=lambda v: v["name"])
def test_request_vector(vec):
    state = merged_state(VECTORS["defaults"]["state"], vec.get("state"))
    sim = FakeController.from_state(state)
    sim.feed_line(request_line(vec))
    out = protocol_only(sim.drain())
    problem = mismatch(vec["expect"], out)
    assert problem is None, f"{problem}\nactual: {json.dumps(out)}"


def test_a_refused_request_moves_nothing(vectors):
    """No refusal ever starts a command (§1.4)."""
    for vec in vectors["requests"]:
        if not vec["expect"] or vec["expect"][-1].get("t") != "res":
            continue
        if vec["expect"][-1].get("ok") is not False:
            continue
        state = merged_state(vectors["defaults"]["state"], vec.get("state"))
        sim = FakeController.from_state(state)
        before = (sim.mode, sim.lock, sim.slot, sim.running)
        sim.feed_line(request_line(vec))
        sim.advance(60)
        assert sum(sim.executions.values()) == 0, vec["name"]
        if before[3] is None:
            assert (sim.mode, sim.lock, sim.slot, sim.running) == before, vec["name"]


# ---------------------------------------------------------------- sequences

KEYS = ("t", "type", "step", "from", "to", "ok")


def project(msg: dict[str, Any], ids: dict[str, str]) -> tuple:
    """The shape of a message: its kind, its request (by role), step, mode change, ok."""
    rid = msg.get("id")
    return (*(msg.get(k) for k in KEYS), ids.get(rid, rid) if rid else None)


def is_milestone(msg: dict[str, Any]) -> bool:
    return msg["t"] in ("ack", "res") or (
        msg["t"] == "evt" and msg["type"] in ("mode", "step_start", "step_end", "boot", "fault")
    )


def expected_milestones(seq: dict[str, Any]) -> list[tuple]:
    return [
        project(i["msg"], {})
        for i in seq["messages"]
        if i["dir"] == "d2h" and is_milestone(i["msg"])
    ]


def h2d(seq: dict[str, Any]) -> list[str]:
    return [i["line"] for i in seq["messages"] if i["dir"] == "h2d"]


def sequence(name: str) -> dict[str, Any]:
    return next(s for s in VECTORS["sequences"] if s["name"] == name)


def milestones(lines: list[str]) -> list[tuple]:
    return [project(m, {}) for m in protocol_only(lines) if is_milestone(m)]


def test_sequence_run_cycle_happy_path():
    seq = sequence("run_cycle_happy_path")
    run, ping = h2d(seq)
    sim = FakeController()
    sim.feed_line(run)
    sim.advance(17.0)
    sim.feed_line(ping)
    sim.run_until_idle()
    out = sim.drain()
    assert milestones(out) == expected_milestones(seq)
    final = protocol_only(out)[-1]
    expected_final = seq["messages"][-1]["msg"]
    assert set(final["data"]) == set(expected_final["data"])  # every record field present
    assert final["data"]["completed"] is True
    assert final["data"]["end_reason"] == "volume_reached"
    assert (sim.lock, sim.slot) == ("locked", 0)  # the rest state, at the purge slot


def test_sequence_stop_mid_purge():
    seq = sequence("stop_mid_purge")
    run, stop, clear, recover = h2d(seq)
    sim = FakeController()
    sim.feed_line(run)
    sim.advance(47.0)
    sim.feed_line(stop)
    sim.advance(2.0)
    sim.feed_line(clear)
    sim.feed_line(recover)
    sim.run_until_idle()
    out = sim.drain()
    assert milestones(out) == expected_milestones(seq)
    record = next(m for m in protocol_only(out) if m.get("id") == "h-0110" and m["t"] == "res")
    assert record["data"]["abort"] == {"step": "purge", "reason": "stopped", "fault": "USER_STOP"}
    assert record["data"]["end_reason"] == "stopped"
    assert record["mode"] == "FAULT"
    assert sim.mode == "IDLE"


def test_sequence_reboot_mid_cycle():
    seq = sequence("reboot_mid_cycle")
    run, hello = h2d(seq)
    sim = FakeController()
    old_boot = sim.boot_id
    sim.feed_line(run)
    before = sim.drain()  # the host reads the ack before the reset loses unsent output
    sim.inject(Reboot(step="purge", after_s=30, cause="WATCHDOG", usb_gap_s=0))
    sim.advance(60)
    sim.feed_line(hello)
    out = protocol_only(before + sim.drain())
    expected = expected_milestones(seq)
    got = [project(m, {}) for m in out if is_milestone(m)]
    # The vector is a short form: its milestones appear in this order among ours.
    it = iter(got)
    assert all(any(e == g for g in it) for e in expected), got
    res = out[-1]
    assert res["data"]["boot_id"] != old_boot
    assert res["data"]["recovery_cause"] == "WATCHDOG"
    assert res["mode"] == "RECOVERY_REQUIRED"
    assert not any(m["t"] == "res" and m.get("id") == "h-0120" for m in out)
