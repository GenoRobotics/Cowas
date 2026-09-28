"""The client against the simulator and against scripted exchanges (ROADMAP HO1 item 7)."""

from __future__ import annotations

import itertools
import json
import re
from typing import Any

import pytest

from cowas.client import CowasClient
from cowas.errors import (
    CommandFailed,
    CommandTimeout,
    DeviceRebooted,
    InvalidArguments,
    LinkLost,
    ProtocolMismatch,
)
from cowas.protocol import models as m
from cowas.protocol.codec import dumps
from cowas.protocol.spec import ID_RE, Cmd
from cowas.protocol.vectors import load_vectors
from cowas.sim import (
    FakeController,
    LinkDrop,
    Reboot,
    SlowProcedure,
    WatchdogReset,
)
from cowas.transport.devicelog import DeviceLog
from cowas.transport.memory import MemoryTransport

VECTORS = load_vectors()
RUN = {"volume_ml": 2000, "max_duration_s": 1800}


# ---------------------------------------------------------------- basics


def test_connect_reads_identity_and_geometry(make_client):
    client, sim, _ = make_client()
    assert client.boot_id == sim.boot_id
    assert client.info is not None and client.info.slot_count == 15
    assert client.hello_data is not None and client.hello_data.device_id == "cowas-V1"


def test_ids_are_valid_unique_and_session_scoped(make_client):
    """F70: a restarted host must not reuse the ids of the previous session."""
    a, _, ta = make_client()
    b, _, tb = make_client()
    ids = ta.sent_ids() + tb.sent_ids()
    assert len(ids) == len(set(ids))
    assert all(ID_RE.fullmatch(i) for i in ids)
    assert a.session != b.session


def test_arguments_are_checked_before_sending(make_client):
    client, _, transport = make_client()
    sent = len(transport.sent)
    with pytest.raises(InvalidArguments) as exc:
        client.run_cycle(client.info.purge_slot, **RUN)  # the purge slot is not a sample slot
    assert exc.value.arg == "slot"
    with pytest.raises(InvalidArguments):
        client.rotate(client.info.slot_count)
    assert len(transport.sent) == sent  # nothing went out


def test_run_cycle_happy_path_with_heartbeat(make_client):
    client, sim, transport = make_client()
    record = client.run_cycle(3, **RUN, sample_id="T-01")
    assert record.completed and record.end_reason == "volume_reached"
    assert record.sample_id == "T-01" and record.missing_fields() == []
    # the heartbeat kept the link alive for the whole 30-minute cycle (§9)
    assert "LINK_LOST" not in [w.code for w in record.warnings]
    pings = [t for t, r in transport.sent if r["cmd"] == "PING"]
    assert len(pings) >= record.cycle_s / 5 - 2
    assert max(b - a for a, b in itertools.pairwise(pings)) <= 5.5
    assert sim.executions[Cmd.RUN_CYCLE] == 1
    status = client.status()
    assert (status.mode, status.lock, status.slot) == ("IDLE", "locked", 0)
    assert status.last is not None and status.last.end_reason == "volume_reached"


def test_without_heartbeat_the_due_reports_link_lost_but_finishes(make_client):
    client, _, _ = make_client(client_kwargs={"heartbeat": False})
    record = client.run_cycle(3, **RUN)
    assert record.completed
    assert "LINK_LOST" in [w.code for w in record.warnings]


def test_refusal_raises_command_failed(make_client):
    client, _, _ = make_client(boot=True)
    with pytest.raises(CommandFailed) as exc:
        client.rotate(3)
    assert exc.value.code == "wrong_mode"
    assert exc.value.response.err.cause == "POWER_ON"


def test_recover_asks_before_moving(make_client):
    client, sim, _ = make_client(boot=True)
    with pytest.raises(CommandFailed) as exc:
        client.recover(confirm_moving=False)
    assert exc.value.code == "confirm_required"
    assert exc.value.response.err.needs == ["actuator_home"]
    assert sim.lock == "unknown"  # nothing moved
    data = client.recover(confirm_moving=True)
    assert [s.name for s in data.steps] == ["cause_gone", "encoder", "align", "oring", "valves"]
    assert client.status().mode == "IDLE"


# ---------------------------------------------------------------- timeouts and retries


def test_short_command_is_resent_with_the_same_id(make_client):
    client, sim, transport = make_client()
    sim.inject(LinkDrop(duration_s=3.0, direction="h2d"))
    status = client.status()
    assert status.mode == "IDLE"
    status_ids = transport.sent_ids("STATUS")
    assert len(status_ids) >= 2 and len(set(status_ids)) == 1  # retries reuse the id


def test_lost_ack_is_retried_and_the_command_runs_once(make_client):
    """Dedupe on retry (§7): the ack and the result are lost, the resend gets them back."""
    client, sim, transport = make_client()
    sim.inject(LinkDrop(duration_s=5.0, direction="d2h"))
    result = client.rotate(7)
    assert result.slot == 7 and result.lock == "locked"
    rotate_ids = transport.sent_ids("ROTATE")
    assert len(rotate_ids) >= 2 and len(set(rotate_ids)) == 1
    assert sim.executions[Cmd.ROTATE] == 1


def test_retried_run_cycle_gets_a_dup_ack_and_never_runs_twice(make_client):
    client, sim, transport = make_client()
    sim.inject(LinkDrop(duration_s=3.0, direction="d2h"))
    record = client.run_cycle(4, **RUN)
    assert record.completed
    assert sim.executions[Cmd.RUN_CYCLE] == 1
    assert len(set(transport.sent_ids("RUN_CYCLE"))) == 1


def test_resending_a_finished_id_returns_the_stored_result(make_client):
    client, sim, _ = make_client()
    p = client.start_run_cycle(5, **RUN)
    first = client.wait(p)
    again = client.wait(client.send(Cmd.RUN_CYCLE, p.args, request_id=p.id))
    assert again == first
    assert sim.executions[Cmd.RUN_CYCLE] == 1


def test_link_lost_when_nothing_answers(make_client):
    client, sim, _ = make_client(client_kwargs={"reconnect_timeout_s": 20.0})
    sim.inject(LinkDrop(duration_s=600.0))
    with pytest.raises(LinkLost):
        client.status()


def test_command_timeout_after_max_s(make_client):
    """A command that overruns ack.max_s + 30 s: HELLO check, one same-id resend, then
    CommandTimeout (§10). The Due is still busy: the host must not start anything else."""
    client, sim, transport = make_client()
    sim.inject(SlowProcedure(factor=5.0, respect_bounds=False))
    t0 = client.clock.monotonic()
    with pytest.raises(CommandTimeout) as exc:
        client.rotate(8)
    elapsed = client.clock.monotonic() - t0
    assert elapsed >= 30 + 30  # never before max_s + 30 s
    assert exc.value.cmd == Cmd.ROTATE
    assert len(set(transport.sent_ids("ROTATE"))) == 1
    assert sim.executions[Cmd.ROTATE] == 1
    assert client.status().mode == "BUSY"


def test_slow_procedure_within_its_bound_is_waited_for(make_client):
    client, sim, _ = make_client()
    sim.inject(SlowProcedure(factor=1.2))
    record = client.run_cycle(3, volume_ml=500, max_duration_s=900)
    assert record.completed


# ---------------------------------------------------------------- reconnect


def test_reconnects_after_the_port_vanishes(make_client):
    client, sim, transport = make_client()
    sim.inject(LinkDrop(step="sample", after_s=60, duration_s=8.0, disconnect=True))
    record = client.run_cycle(3, **RUN)
    assert record.completed
    assert client.link_recoveries >= 1
    assert sim.executions[Cmd.RUN_CYCLE] == 1
    # after the reconnection, HELLO checked the boot and the pending id was resent
    hellos = [t for t, r in transport.sent if r["cmd"] == "HELLO"]
    assert len(hellos) >= 2
    assert len(set(transport.sent_ids("RUN_CYCLE"))) == 1


# ---------------------------------------------------------------- STOP during RUN_CYCLE


def test_stop_during_run_cycle_from_a_subscriber(make_client):
    client, _, _ = make_client()
    stopped: list[m.StopData] = []

    def stop_after_a_minute_of_water(item: Any) -> None:
        tel = item.message
        if (
            isinstance(tel, m.Telemetry)
            and tel.phase == "water"
            and not stopped
            and (tel.volume_ml or 0) > 100
        ):
            stopped.append(client.stop())

    client.subscribe(stop_after_a_minute_of_water)
    record = client.run_cycle(3, **RUN)
    assert stopped and stopped[0].interrupted["cmd"] == "RUN_CYCLE"
    assert record.completed is False
    assert record.end_reason == "stopped"
    assert record.abort == m.Abort(step="sample", reason="stopped", fault="USER_STOP")
    assert 100 < (record.volume_ml or 0) < 200  # the partial record keeps the volume
    assert client.last_mode == "FAULT"
    with pytest.raises(CommandFailed):
        client.run_cycle(4, **RUN)  # nothing starts until CLEAR_FAULT + RECOVER
    assert client.clear_fault() == "USER_STOP"
    client.recover(confirm_moving=False)  # stopped while locked at the slot: nothing to move
    assert client.status().mode == "IDLE"


def test_stop_with_the_non_blocking_api(make_client):
    client, sim, _ = make_client()
    p = client.start_run_cycle(3, **RUN)
    client.poll(120.0)  # in the purge
    assert sim.running == p.id
    client.stop()
    record = client.result(p)
    assert record.abort.step == "purge" and record.end_reason == "stopped"


# ---------------------------------------------------------------- the log separation


def test_device_log_lines_never_reach_the_protocol(make_client):
    client, _, _ = make_client(bench_mode=True)
    protocol: list[Any] = []
    device: list[Any] = []
    client.subscribe(lambda item: (protocol if item.message else device).append(item))
    client.rotate(2)
    assert device and protocol
    assert not any(isinstance(i.message, m.LogMessage) for i in protocol)
    assert {d.device_line.source for d in device} == {"log", "text"}
    log_file = client.router.device_log.path
    lines = log_file.read_text(encoding="utf-8").splitlines()
    assert lines
    stamp = re.compile(r"^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d\.\d{3}Z\t")
    assert all(stamp.match(line) for line in lines)
    assert not any('"t":"res"' in line or '"t":"evt"' in line for line in lines)


def test_garbage_lines_are_logged_not_dispatched(tmp_path):
    transport = MemoryTransport()
    transport.open()
    log = DeviceLog(tmp_path)
    client = CowasClient(transport, device_log=log, heartbeat=False)
    seen: list[Any] = []
    client.subscribe(seen.append)
    transport.feed(
        '{"t":"res","id":"x",', "Programm started", '{"t":"log","ms":1,"lvl":"info","msg":"hi"}'
    )
    client.poll(1.0)
    assert [i.device_line.source for i in seen] == ["garbage", "text", "log"]
    assert client.router.garbage_lines == 1
    log.close()


# ---------------------------------------------------------------- reboots


def test_reboot_mid_cycle_is_detected_via_hello(make_client):
    """The port re-enumerates, the boot event is lost; the HELLO after the reconnection
    shows a new boot_id. The pending id is never resent."""
    client, sim, transport = make_client()
    old_boot = client.boot_id
    sim.inject(Reboot(step="sample", after_s=30, usb_gap_s=2.0))
    p = client.start_run_cycle(3, **RUN)
    with pytest.raises(DeviceRebooted) as exc:
        client.wait(p)
    assert exc.value.request_id == p.id
    assert exc.value.old_boot_id == old_boot and exc.value.new_boot_id == sim.boot_id
    assert exc.value.hello is not None  # detected by HELLO, not by the boot event
    assert exc.value.hello.recovery_cause == "POWER_ON"
    reboot_t = next(t for t, r in transport.sent if r["cmd"] == "HELLO" and t > 0)
    assert all(t < reboot_t for t, r in transport.sent if r["id"] == p.id)
    assert client.reboots[-1].interrupted == [(p.id, "RUN_CYCLE")]
    assert client.hello().mode == "RECOVERY_REQUIRED"


def test_reboot_detected_via_the_boot_event(make_client):
    client, sim, transport = make_client()
    sim.inject(Reboot(step="purge", after_s=20, usb_gap_s=0))
    with pytest.raises(DeviceRebooted) as exc:
        client.run_cycle(3, **RUN)
    assert exc.value.hello is None  # the boot event came first
    assert len(set(transport.sent_ids("RUN_CYCLE"))) == 1
    assert len(transport.sent_ids("RUN_CYCLE")) == 1


def test_watchdog_reset_mid_cycle(make_client):
    client, sim, _ = make_client()
    sim.inject(WatchdogReset(step="sample", after_s=45))
    with pytest.raises(DeviceRebooted):
        client.run_cycle(3, **RUN)
    hello = client.hello()
    assert hello.recovery_cause == "WATCHDOG"


# ---------------------------------------------------------------- scripted exchanges


def scripted(sequence_name: str, hello: dict[str, Any]) -> MemoryTransport:
    """Answers HELLO/GET_INFO/PING, and replays a vector sequence for its first request."""
    seq = next(s for s in VECTORS["sequences"] if s["name"] == sequence_name)
    info = FakeController().info.model_dump(mode="json")
    replies = [i["msg"] for i in seq["messages"] if i["dir"] == "d2h"]
    vector_id = json.loads(next(i["line"] for i in seq["messages"] if i["dir"] == "h2d"))["id"]

    def respond(line: str) -> list[str]:
        req = json.loads(line)
        if req["cmd"] == "HELLO":
            return [dumps({"t": "res", "id": req["id"], "ok": True, "mode": "IDLE", "data": hello})]
        if req["cmd"] == "GET_INFO":
            return [dumps({"t": "res", "id": req["id"], "ok": True, "mode": "IDLE", "data": info})]
        if req["cmd"] == "PING":
            return [
                dumps({"t": "res", "id": req["id"], "ok": True, "mode": "BUSY", "data": {"ms": 1}})
            ]
        out = []
        for msg in replies:
            if msg["t"] == "res" and msg.get("id") != vector_id:
                continue  # answers to the sequence's other requests
            if msg.get("id") == vector_id:
                msg = {**msg, "id": req["id"]}
            out.append(dumps(msg))
        return out

    return MemoryTransport(respond)


HELLO_V1 = {
    "proto": "1.0",
    "device_id": "cowas-V1",
    "boot_id": "9f3c21a7",
    "uptime_s": 12.4,
    "fw": {"version": "fw7", "git": "62a222d", "dirty": False, "built": "2026-10-20T10:00:00Z"},
    "mode": "IDLE",
    "recovery_required": False,
    "recovery_cause": None,
    "fault": None,
    "caps": {"p_in": True, "q_in": True},
}


def test_client_consumes_the_happy_path_sequence():
    client = CowasClient(scripted("run_cycle_happy_path", HELLO_V1))
    client.connect()
    record = client.run_cycle(3, **RUN)
    seq = next(s for s in VECTORS["sequences"] if s["name"] == "run_cycle_happy_path")
    expected = m.SampleRecord.model_validate(seq["messages"][-1]["msg"]["data"])
    assert record == expected


def test_client_consumes_the_reboot_sequence():
    transport = scripted("reboot_mid_cycle", HELLO_V1)
    client = CowasClient(transport)
    client.connect()
    with pytest.raises(DeviceRebooted) as exc:
        client.run_cycle(3, **RUN)
    assert (exc.value.old_boot_id, exc.value.new_boot_id) == ("9f3c21a7", "4b7e0d12")
    assert sum(1 for line in transport.written if '"RUN_CYCLE"' in line) == 1


def test_protocol_mismatch_is_refused():
    client = CowasClient(scripted("run_cycle_happy_path", {**HELLO_V1, "proto": "2.0"}))
    with pytest.raises(ProtocolMismatch):
        client.connect()
