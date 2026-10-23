"""The Due's request checks, as pure functions (§3.1, §5, §7 steps 1, 2, 4 and 5, §8).

The simulator uses them to answer like the firmware, and the client uses `check_args` to
refuse a bad request before it is sent. Mode checks and preconditions depend on the
machine state, so they live in the simulator.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field
from typing import Any

from cowas.protocol.codec import loads
from cowas.protocol.models import GetInfoData
from cowas.protocol.spec import (
    ID_RE,
    KNOWN_COMMANDS,
    MAX_LINE,
    PROTO_MAJOR,
    PROTO_VERSION,
    STAGES,
    Cmd,
    is_reserved,
)


@dataclass(frozen=True)
class Refusal:
    """A refusal: `res ok:false` with `err = {code, msg, **extra}`. Nothing moves."""

    code: str
    msg: str
    id: str | None = None
    extra: dict[str, Any] = field(default_factory=dict)

    def err(self) -> dict[str, Any]:
        return {"code": self.code, "msg": self.msg, **self.extra}


@dataclass(frozen=True)
class Envelope:
    id: str
    cmd: str
    args: dict[str, Any]


@dataclass(frozen=True)
class CheckedArgs:
    """Arguments after the checks: defaults filled in, plus warnings to raise (PURGE_CLAMPED)."""

    args: dict[str, Any]
    warnings: list[tuple[str, str]] = field(default_factory=list)


def _is_int(v: Any) -> bool:
    return isinstance(v, int) and not isinstance(v, bool)


def _is_num(v: Any) -> bool:
    return isinstance(v, (int, float)) and not isinstance(v, bool) and math.isfinite(float(v))


def valid_id(v: Any) -> bool:
    return isinstance(v, str) and ID_RE.fullmatch(v) is not None


# --------------------------------------------------------------------------- steps 1-2


def parse_request_line(line: str | bytes) -> Envelope | Refusal | None:
    """§7 steps 1 and 2. None = an empty or whitespace line, ignored silently (§2)."""
    raw = line.encode("utf-8", errors="surrogateescape") if isinstance(line, str) else line
    if raw.endswith(b"\n"):
        raw = raw[:-1]
    if raw.endswith(b"\r"):  # F37: the Due strips one trailing CR
        raw = raw[:-1]
    if len(raw) + 1 > MAX_LINE:
        msg = f"line longer than {MAX_LINE} bytes"
        return Refusal("line_too_long", msg, None, {"max": MAX_LINE})
    if not raw.strip():
        return None
    try:
        obj = loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, ValueError):
        return Refusal("bad_json", "not valid JSON")
    if not isinstance(obj, dict):
        return Refusal("bad_json", "not a JSON object")

    rid = obj.get("id")
    echo = rid if valid_id(rid) else None
    if obj.get("t") != "req":
        return Refusal("bad_request", 't must be "req"', echo, {"field": "t"})
    if echo is None:
        return Refusal("bad_request", "id missing or invalid", None, {"field": "id"})
    cmd = obj.get("cmd")
    if not isinstance(cmd, str):
        return Refusal("bad_request", "cmd missing or not a string", echo, {"field": "cmd"})
    args = obj.get("args", {})
    if not isinstance(args, dict):
        return Refusal("bad_request", "args must be an object", echo, {"field": "args"})
    return Envelope(echo, cmd, args)


def check_command(env: Envelope) -> Refusal | None:
    """§7 step 4: reserved -> not_available; unknown -> unknown_cmd."""
    if is_reserved(env.cmd):
        return Refusal("not_available", f"{env.cmd[:30]} is not available", env.id)
    if env.cmd not in KNOWN_COMMANDS:
        return Refusal("unknown_cmd", f"unknown command {env.cmd[:30]}", env.id)
    return None


# --------------------------------------------------------------------------- step 5


class _ArgError(Exception):
    def __init__(self, arg: str, msg: str) -> None:
        super().__init__(msg)
        self.arg = arg
        self.msg = msg


def _only(args: dict[str, Any], allowed: tuple[str, ...]) -> None:
    for key in args:
        if key not in allowed:
            raise _ArgError(key, f"unknown argument {key[:30]}")


def _required(args: dict[str, Any], key: str) -> Any:
    if key not in args:
        raise _ArgError(key, f"{key} is required")
    return args[key]


def _int_in(args: dict[str, Any], key: str, lo: int, hi: int, *, required: bool) -> int | None:
    if key not in args or (not required and args[key] is None):
        if required:
            raise _ArgError(key, f"{key} is required")
        return None
    v = args[key]
    if not _is_int(v) or not lo <= v <= hi:
        raise _ArgError(key, f"{key} must be an integer in {lo}..{hi}")
    return int(v)


def _sample_slot(args: dict[str, Any], info: GetInfoData) -> int:
    v = _required(args, "slot")
    if not _is_int(v) or v not in info.sample_slots:
        raise _ArgError("slot", "slot must be a sample slot")
    return int(v)


def _volume(args: dict[str, Any], info: GetInfoData, *, key: str = "volume_ml") -> float | None:
    """A required key whose value is a number in the volume limits, or null (no cap)."""
    v = _required(args, key)
    if v is None:
        return None
    lim = info.limits
    if not _is_num(v) or not lim.volume_ml_min <= v <= lim.volume_ml_max:
        raise _ArgError(key, f"{key} must be null or {lim.volume_ml_min:g}..{lim.volume_ml_max:g}")
    return float(v)


def _purge(
    args: dict[str, Any], info: GetInfoData, key: str, *, required_key: bool
) -> tuple[float, list[tuple[str, str]]]:
    """purge_ml / PURGE volume_ml: null = default; below the minimum -> raised (§5.8)."""
    lim = info.limits
    if required_key:
        v = _required(args, key)
    else:
        v = args.get(key)
    if v is None:
        return float(lim.purge_ml_default), []
    if not _is_num(v) or v <= 0 or v > lim.volume_ml_max:
        raise _ArgError(key, f"{key} must be null or > 0 and <= {lim.volume_ml_max:g}")
    if v < lim.purge_ml_min:
        msg = f"purge {v:g} mL raised to {lim.purge_ml_min:g} mL"
        return float(lim.purge_ml_min), [("PURGE_CLAMPED", msg)]
    return float(v), []


def _duration_ms(args: dict[str, Any], info: GetInfoData, default: int) -> int:
    v = _int_in(args, "duration_ms", 1, info.limits.duration_ms_max, required=False)
    return default if v is None else v


def _stages(args: dict[str, Any]) -> dict[str, bool]:
    raw = args.get("stages")
    if raw is None:
        return dict.fromkeys(STAGES, True)
    if not isinstance(raw, dict):
        raise _ArgError("stages", "stages must be an object")
    out = dict.fromkeys(STAGES, True)
    for key, value in raw.items():
        if key not in STAGES:
            raise _ArgError(f"stages.{key}"[:40], f"unknown stage {key[:30]}")
        if not isinstance(value, bool):
            raise _ArgError(f"stages.{key}", "stage flags are booleans")
        out[key] = value
    if not any(out.values()):
        raise _ArgError("stages", "at least one stage must run")
    return out


def _sample_id(args: dict[str, Any]) -> str | None:
    v = args.get("sample_id")
    if v is None:
        return None
    if not isinstance(v, str) or len(v) > 40:
        raise _ArgError("sample_id", "sample_id must be a string of at most 40 characters")
    return v


# The default PRIME_SHIELD_MS is a firmware PLACEHOLDER (§5.9); the host sends it explicitly
# when it matters. This value only fills the simulator's record.
PRIME_SHIELD_MS_DEFAULT = 10000


def _check(cmd: str, args: dict[str, Any], info: GetInfoData) -> CheckedArgs:
    lim = info.limits
    if cmd == Cmd.HELLO:
        _only(args, ("proto", "client"))
        proto = args.get("proto")
        if proto is not None and not _is_int(proto):
            raise _ArgError("proto", "proto must be an integer")
        client = args.get("client")
        if client is not None and (not isinstance(client, str) or len(client) > 40):
            raise _ArgError("client", "client must be a string of at most 40 characters")
        return CheckedArgs({"proto": proto, "client": client})
    if cmd in (
        Cmd.PING,
        Cmd.STATUS,
        Cmd.GET_INFO,
        Cmd.STOP,
        Cmd.CLEAR_FAULT,
        Cmd.ACTUATOR_HOME,
        Cmd.LOCK,
        Cmd.UNLOCK,
        Cmd.EMPTY,
    ):
        _only(args, ())
        return CheckedArgs({})
    if cmd == Cmd.RECOVER:
        _only(args, ("confirm_moving",))
        v = _required(args, "confirm_moving")
        if not isinstance(v, bool):
            raise _ArgError("confirm_moving", "confirm_moving must be a boolean")
        return CheckedArgs({"confirm_moving": v})
    if cmd == Cmd.ROTATE:
        _only(args, ("slot",))
        slot = _int_in(args, "slot", 0, info.slot_count - 1, required=True)
        return CheckedArgs({"slot": slot})
    if cmd == Cmd.PRIME_SHIELD:
        _only(args, ("duration_ms",))
        return CheckedArgs({"duration_ms": _duration_ms(args, info, PRIME_SHIELD_MS_DEFAULT)})
    if cmd == Cmd.RUN_CYCLE:
        _only(args, ("slot", "purge_ml", "volume_ml", "max_duration_s", "stages", "sample_id"))
        slot = _sample_slot(args, info)
        purge_ml, warnings = _purge(args, info, "purge_ml", required_key=True)
        volume = _volume(args, info)
        max_s = _int_in(args, "max_duration_s", 1, lim.max_duration_s_max, required=True)
        stages = _stages(args)
        sample_id = _sample_id(args)
        return CheckedArgs(
            {
                "slot": slot,
                "purge_ml": purge_ml,
                "volume_ml": volume,
                "max_duration_s": max_s,
                "stages": stages,
                "sample_id": sample_id,
            },
            warnings,
        )
    if cmd == Cmd.PURGE:
        _only(args, ("volume_ml",))
        purge_ml, warnings = _purge(args, info, "volume_ml", required_key=False)
        return CheckedArgs({"volume_ml": purge_ml}, warnings)
    if cmd == Cmd.SAMPLE:
        _only(args, ("slot", "volume_ml", "max_duration_s"))
        slot = _sample_slot(args, info)
        volume = _volume(args, info)
        max_s = _int_in(args, "max_duration_s", 1, lim.max_duration_s_max, required=True)
        return CheckedArgs({"slot": slot, "volume_ml": volume, "max_duration_s": max_s})
    if cmd == Cmd.DNA_SHIELD:
        _only(args, ("slot", "duration_ms"))
        slot = _sample_slot(args, info)
        return CheckedArgs(
            {"slot": slot, "duration_ms": _duration_ms(args, info, lim.shield_ms_default)}
        )
    raise _ArgError("cmd", f"no argument rules for {cmd}")  # pragma: no cover


def check_args(env: Envelope, info: GetInfoData) -> CheckedArgs | Refusal:
    """§7 step 5 (+ HELLO's proto_mismatch). Unknown keys are reported first, then the
    declared arguments in their documented order; `err.arg` names the first bad one."""
    try:
        checked = _check(env.cmd, env.args, info)
    except _ArgError as exc:
        return Refusal("bad_args", exc.msg[:60], env.id, {"arg": exc.arg})
    if env.cmd == Cmd.HELLO:
        proto = checked.args.get("proto")
        if proto is not None and proto != PROTO_MAJOR:
            return Refusal(
                "proto_mismatch",
                f"host speaks {proto}, Due speaks {PROTO_VERSION}",
                env.id,
                {"proto": PROTO_VERSION},
            )
    return checked
