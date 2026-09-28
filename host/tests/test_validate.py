"""The request checks shared by the simulator and the client (§7 steps 1, 2, 5): boundaries."""

from __future__ import annotations

import pytest

from cowas.protocol.models import Limits
from cowas.protocol.spec import MAX_LINE
from cowas.protocol.validate import (
    CheckedArgs,
    Envelope,
    Refusal,
    check_args,
    check_command,
    parse_request_line,
)
from cowas.sim.geometry import cowas_v1, regular_geometry

INFO = cowas_v1()
LIM = INFO.limits


def run_cycle(**over):
    args = {"slot": 3, "purge_ml": None, "volume_ml": 2000, "max_duration_s": 1800}
    args.update(over)
    return check_args(Envelope("h-1", "RUN_CYCLE", args), INFO)


def refused_arg(result) -> str | None:
    return result.extra.get("arg") if isinstance(result, Refusal) else None


# ---------------------------------------------------------------- framing (steps 1-2)


def test_crlf_and_lf_are_stripped():
    for line in ('{"t":"req","id":"a","cmd":"PING"}\r\n', '{"t":"req","id":"a","cmd":"PING"}\n'):
        assert isinstance(parse_request_line(line), Envelope)


def test_line_length_limit_counts_the_newline():
    prefix = '{"t":"req","id":"a","cmd":"PING","args":{"x":"'
    fits = prefix + "a" * (MAX_LINE - 1 - len(prefix) - 3) + '"}}'
    assert len(fits) == MAX_LINE - 1
    assert isinstance(parse_request_line(fits), Envelope)
    too_long = prefix + "a" * (MAX_LINE - len(prefix) - 3) + '"}}'
    result = parse_request_line(too_long)
    assert isinstance(result, Refusal) and result.code == "line_too_long"
    assert result.extra == {"max": MAX_LINE}


@pytest.mark.parametrize(
    "line,code",
    [
        ('{"t":"req","id":"a","cmd":"PING","args":{"x":NaN}}', "bad_json"),
        (b'\xff\xfe{"t":"req"}', "bad_json"),
        ('{"t":"req","id":"a","cmd":7}', "bad_request"),
        ('{"t":"req","id":"a","cmd":"PING","args":null}', "bad_request"),
    ],
)
def test_malformed(line, code):
    result = parse_request_line(line)
    assert isinstance(result, Refusal) and result.code == code


def test_reserved_before_unknown():
    assert check_command(Envelope("a", "SPOOL_DOWN", {})).code == "not_available"
    assert check_command(Envelope("a", "Spool_down", {})).code == "unknown_cmd"
    assert check_command(Envelope("a", "PING", {})) is None


# ---------------------------------------------------------------- args (step 5)


@pytest.mark.parametrize(
    "value,ok",
    [
        (LIM.volume_ml_min - 0.001, False),
        (LIM.volume_ml_min, True),
        (LIM.volume_ml_max, True),
        (LIM.volume_ml_max + 0.001, False),
        (None, True),  # no volume cap
        (True, False),  # a bool is not a number
        (float("nan"), False),
    ],
)
def test_volume_boundaries(value, ok):
    result = run_cycle(volume_ml=value)
    assert isinstance(result, CheckedArgs) == ok
    if not ok:
        assert refused_arg(result) == "volume_ml"


@pytest.mark.parametrize(
    "value,effective,clamped",
    [
        (None, LIM.purge_ml_default, False),
        (1, LIM.purge_ml_min, True),
        (LIM.purge_ml_min - 0.5, LIM.purge_ml_min, True),
        (LIM.purge_ml_min, LIM.purge_ml_min, False),
        (LIM.volume_ml_max, LIM.volume_ml_max, False),
    ],
)
def test_purge_is_raised_to_the_minimum(value, effective, clamped):
    """§3.7 rule 1: a purge below the minimum is raised, with PURGE_CLAMPED."""
    result = run_cycle(purge_ml=value)
    assert isinstance(result, CheckedArgs)
    assert result.args["purge_ml"] == effective
    assert [code for code, _ in result.warnings] == (["PURGE_CLAMPED"] if clamped else [])


@pytest.mark.parametrize("value", [0, -1, LIM.volume_ml_max + 1, "424", False])
def test_purge_refused(value):
    assert refused_arg(run_cycle(purge_ml=value)) == "purge_ml"


@pytest.mark.parametrize(
    "value,ok",
    [
        (0, False),
        (1, True),
        (LIM.max_duration_s_max, True),
        (LIM.max_duration_s_max + 1, False),
        (1800.0, False),
        (True, False),
    ],
)
def test_max_duration_boundaries(value, ok):
    assert isinstance(run_cycle(max_duration_s=value), CheckedArgs) == ok


def test_stages():
    assert run_cycle(stages={"shield": False}).args["stages"] == {
        "purge": True,
        "sample": True,
        "shield": False,
        "empty": True,
    }
    assert refused_arg(run_cycle(stages={"purge": 0})) == "stages.purge"
    assert refused_arg(run_cycle(stages={"rinse": True})) == "stages.rinse"
    all_off = dict.fromkeys(("purge", "sample", "shield", "empty"), False)
    assert refused_arg(run_cycle(stages=all_off)) == "stages"


def test_sample_id_length():
    assert isinstance(run_cycle(sample_id="x" * 40), CheckedArgs)
    assert refused_arg(run_cycle(sample_id="x" * 41)) == "sample_id"


def test_unknown_argument_is_reported_first():
    result = run_cycle(slot="bad", volume=500)
    assert refused_arg(result) == "volume"


def test_slot_rules_follow_the_geometry():
    """No hard-coded slot count: the limits come from GET_INFO (rule 6)."""
    info24 = regular_geometry(24)
    rotate = lambda s, info: check_args(Envelope("a", "ROTATE", {"slot": s}), info)  # noqa: E731
    assert isinstance(rotate(23, info24), CheckedArgs)
    assert refused_arg(rotate(24, info24)) == "slot"
    assert refused_arg(rotate(15, INFO)) == "slot"
    sample = check_args(
        Envelope("a", "SAMPLE", {"slot": 23, "volume_ml": None, "max_duration_s": 60}), info24
    )
    assert isinstance(sample, CheckedArgs)
    purge_slot = check_args(
        Envelope("a", "SAMPLE", {"slot": 0, "volume_ml": None, "max_duration_s": 60}), info24
    )
    assert refused_arg(purge_slot) == "slot"


def test_limits_come_from_get_info():
    info = cowas_v1(Limits(volume_ml_max=100))
    result = check_args(
        Envelope("a", "SAMPLE", {"slot": 1, "volume_ml": 101, "max_duration_s": 60}), info
    )
    assert refused_arg(result) == "volume_ml"


def test_hello_proto_mismatch():
    result = check_args(Envelope("a", "HELLO", {"proto": 2}), INFO)
    assert isinstance(result, Refusal) and result.code == "proto_mismatch"
    assert isinstance(check_args(Envelope("a", "HELLO", {"proto": 1}), INFO), CheckedArgs)
    assert refused_arg(check_args(Envelope("a", "HELLO", {"proto": "1"}), INFO)) == "proto"


def test_recover_needs_a_real_bool():
    for value in (1, 0, "true", None):
        result = check_args(Envelope("a", "RECOVER", {"confirm_moving": value}), INFO)
        assert refused_arg(result) == "confirm_moving"


def test_dna_shield_default_duration():
    result = check_args(Envelope("a", "DNA_SHIELD", {"slot": 2}), INFO)
    assert isinstance(result, CheckedArgs)
    assert result.args["duration_ms"] == LIM.shield_ms_default
