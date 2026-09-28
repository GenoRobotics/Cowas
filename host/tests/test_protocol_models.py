"""cowas.protocol: models and codec against every vector of protocol/vectors.json."""

from __future__ import annotations

import json

import pytest
from pydantic import ValidationError

from cowas.protocol import models as m
from cowas.protocol.codec import RESULT_MODELS, ProtocolParseError, parse_message, parse_result
from cowas.protocol.spec import (
    MOTION_COMMANDS,
    PROCEDURE_COMMANDS,
    Cmd,
    Mode,
    accepted_in,
    is_reserved,
    ms_elapsed,
)
from cowas.protocol.validate import Envelope, parse_request_line
from cowas.protocol.vectors import load_vectors, matches, mismatch, request_line
from cowas.sim.geometry import cowas_v1, regular_geometry

VECTORS = load_vectors()

# Which command produced each result example (a result does not carry its command).
RESULT_CMD = {
    "res_run_cycle_": Cmd.RUN_CYCLE,
    "res_purge_": Cmd.PURGE,
    "res_recover_": Cmd.RECOVER,
    "res_rotate_": Cmd.ROTATE,
}


def _cmd_for(name: str) -> str:
    for prefix, cmd in RESULT_CMD.items():
        if name.startswith(prefix):
            return cmd
    raise KeyError(name)


@pytest.mark.parametrize("vec", VECTORS["messages"], ids=lambda v: v["name"])
def test_every_message_vector_parses(vec):
    assert vec["valid"] is True
    msg = parse_message(vec["line"])
    obj = json.loads(vec["line"])
    assert msg.t == obj["t"]
    if isinstance(msg, m.Response):
        if msg.ok:
            data = parse_result(_cmd_for(vec["name"]), msg.data or {})
            if isinstance(data, m.SampleRecord):
                assert data.missing_fields() == []
        else:
            assert msg.err is not None and msg.err.code
    if obj["t"] == "evt":
        expected = m.EVENT_MODELS.get(obj["type"], m.Event)
        assert type(msg) is expected


def test_unknown_event_type_is_kept_generic():
    msg = parse_message('{"t":"evt","ms":5000,"type":"door_open","where":"lid"}')
    assert type(msg) is m.Event
    assert msg.model_extra == {"where": "lid"}


def test_unknown_fields_are_ignored_but_readable():
    vec = next(v for v in VECTORS["messages"] if v["name"] == "tel_extra_field")
    msg = parse_message(vec["line"])
    assert isinstance(msg, m.Telemetry)
    assert msg.model_extra == {"water_temp": 11.9}


def test_unknown_t_is_not_an_error():
    assert isinstance(parse_message('{"t":"xyz","a":1}'), m.UnknownMessage)


@pytest.mark.parametrize(
    "line",
    [
        '{"t":"tel","ms":1,"p_in":NaN}',  # §2: NaN is never sent
        '{"t":"ack","id":"h-1","cmd":"ROTATE","max_s":"30"}',  # a string is not a number
        '{"t":"log","ms":true,"lvl":"info","msg":"x"}',  # a bool is not an int
        '{"t":"res","id":"h-1","ok":true,"mode":"IDLE"}',  # ok:true without data
        '{"t":"res","id":"h-1","ok":false,"mode":"IDLE"}',  # ok:false without err
        '{"t":"evt","ms":1,"type":"mode","to":"IDLE"}',  # a known event missing a field
        "[1,2]",
        '{"no_t":1}',
        "{",
    ],
)
def test_invalid_messages_are_rejected(line):
    with pytest.raises(ProtocolParseError):
        parse_message(line)


@pytest.mark.parametrize("seq", VECTORS["sequences"], ids=lambda s: s["name"])
def test_sequences_parse_both_ways(seq):
    for item in seq["messages"]:
        if item["dir"] == "d2h":
            parse_message(json.dumps(item["msg"]))
        elif item["dir"] == "h2d":
            req = m.Request.model_validate(json.loads(item["line"]))
            assert json.loads(req.to_line()) == {"args": {}, **json.loads(item["line"])}


@pytest.mark.parametrize("vec", VECTORS["requests"], ids=lambda v: v["name"])
def test_well_formed_request_lines_round_trip_through_the_request_model(vec):
    """What the host builds with `Request` is byte-compatible with the vector lines."""
    parsed = parse_request_line(request_line(vec))
    if not isinstance(parsed, Envelope):
        return  # malformed on purpose: checked against the simulator (test_sim_vectors)
    obj = json.loads(request_line(vec))
    if set(obj) - {"t", "id", "cmd", "args"}:
        with pytest.raises(ValidationError):  # the host never sends unknown envelope keys
            m.Request.model_validate(obj)
        return
    req = m.Request(id=parsed.id, cmd=parsed.cmd, args=parsed.args)
    assert json.loads(req.to_line()) == {"args": {}, **obj}


def test_request_model_refuses_bad_ids():
    for bad in ("", "x" * 33, "h 1", "h/1"):
        with pytest.raises(ValidationError):
            m.Request(id=bad, cmd="PING")
    assert m.Request(id="x" * 32, cmd="PING").to_line() == (
        '{"t":"req","id":"' + "x" * 32 + '","cmd":"PING","args":{}}'
    )


def test_get_info_must_be_consistent():
    good = cowas_v1().model_dump()
    m.GetInfoData.model_validate(good)
    for change in (
        {"fill_order": [1, 2, 3]},  # not every sample slot
        {"sample_slots": [0, *range(2, 15)]},  # the purge slot among the samples
        {"purge_slot": 15},
    ):
        with pytest.raises(ValidationError):
            m.GetInfoData.model_validate({**good, **change})


@pytest.mark.parametrize("raw,omitted", [(16, ()), (24, ()), (24, (4, 12))])
def test_geometries_of_any_size_are_valid(raw, omitted):
    info = regular_geometry(raw, omitted)
    assert info.slot_count == raw - len(omitted)
    assert sorted(info.fill_order) == info.sample_slots
    assert len(info.positions) == info.slot_count


def test_v1_geometry_matches_the_firmware_clock_labels():
    labels = [p.clock for p in cowas_v1().positions]
    assert labels == [30, 34, 38, 41, 49, 53, 56, 0, 4, 8, 11, 15, 19, 23, 26]


def test_sample_record_reports_missing_fields():
    rec = m.SampleRecord.model_validate({"slot": 3, "completed": True})
    assert "end_reason" in rec.missing_fields()
    assert "slot" not in rec.missing_fields()


def test_result_models_cover_every_command():
    assert set(RESULT_MODELS) == set(Cmd)
    for cmd in PROCEDURE_COMMANDS:
        assert RESULT_MODELS[cmd] is m.SampleRecord
    for cmd in MOTION_COMMANDS - {Cmd.RECOVER}:
        assert RESULT_MODELS[cmd] is m.MotionResult


def test_mode_table_spot_checks():
    """§5.2."""
    for mode in Mode:
        assert accepted_in(Cmd.STOP, mode)
        assert accepted_in(Cmd.STATUS, mode)
        assert accepted_in(Cmd.RUN_CYCLE, mode) == (mode == Mode.IDLE)
        assert accepted_in(Cmd.RECOVER, mode) == (mode == Mode.RECOVERY_REQUIRED)
        assert accepted_in(Cmd.CLEAR_FAULT, mode) == (mode in (Mode.FAULT, Mode.ESTOP))
    assert is_reserved("SPOOL_UP") and is_reserved("NEG_CONTROL")
    assert not is_reserved("PING")


def test_ms_clock_wraps():
    assert ms_elapsed(100, 350) == 250
    assert ms_elapsed(0xFFFFFF00, 0x10) == 0x110


def test_vector_matcher():
    assert matches({"a": "$num"}, {"a": 1.5, "b": 2})
    assert not matches({"a": "$num"}, {"a": True})
    assert not matches({"a": True}, {"a": 1})
    assert not matches([1], [1, 2])
    assert matches({"a": "$any"}, {"a": None})
    assert mismatch({"a": {"b": 1}}, {"a": {"b": 2}}) == "$.a.b: expected 1, got 2"
