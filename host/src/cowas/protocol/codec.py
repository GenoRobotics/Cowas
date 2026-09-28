"""Parsing Due -> host lines into models, and typing results by command (§3, §5, §6)."""

from __future__ import annotations

import json
from typing import Any

from pydantic import BaseModel, ValidationError

from cowas.protocol import models as m
from cowas.protocol.spec import PROCEDURE_COMMANDS, Cmd


class ProtocolParseError(ValueError):
    """A line that starts like JSON but is not a valid protocol v1 message."""


def _reject_constant(name: str) -> Any:
    # NaN and ±inf are never sent (§2): they must arrive as null.
    raise ValueError(f"non-JSON number {name}")


def loads(text: str | bytes) -> Any:
    """json.loads that refuses NaN/Infinity (Python accepts them by default)."""
    return json.loads(text, parse_constant=_reject_constant)


def dumps(obj: Any) -> str:
    """Compact JSON as the protocol uses it (no spaces, ASCII only)."""
    return json.dumps(obj, separators=(",", ":"), ensure_ascii=True, allow_nan=False)


_TOP_MODELS: dict[str, type[BaseModel]] = {
    "ack": m.Ack,
    "res": m.Response,
    "tel": m.Telemetry,
    "log": m.LogMessage,
}


def parse_object(obj: Any) -> m.Message:
    if not isinstance(obj, dict):
        raise ProtocolParseError("not a JSON object")
    t = obj.get("t")
    if not isinstance(t, str):
        raise ProtocolParseError("missing or non-string 't'")
    if t == "evt":
        etype = obj.get("type")
        model: type[BaseModel] = (
            m.EVENT_MODELS.get(etype, m.Event) if isinstance(etype, str) else m.Event
        )
    else:
        model = _TOP_MODELS.get(t, m.UnknownMessage)
    try:
        return model.model_validate(obj)  # type: ignore[return-value]
    except ValidationError as exc:
        raise ProtocolParseError(f"invalid {t!r} message: {exc.errors()[0]['msg']}") from exc


def parse_message(line: str | bytes) -> m.Message:
    """Parse one Due -> host JSON line (without its newline)."""
    try:
        obj = loads(line)
    except ValueError as exc:
        raise ProtocolParseError(f"bad JSON: {exc}") from exc
    return parse_object(obj)


RESULT_MODELS: dict[str, type[BaseModel]] = {
    Cmd.HELLO: m.HelloData,
    Cmd.PING: m.PingData,
    Cmd.STATUS: m.StatusData,
    Cmd.GET_INFO: m.GetInfoData,
    Cmd.STOP: m.StopData,
    Cmd.CLEAR_FAULT: m.ClearFaultData,
    Cmd.RECOVER: m.RecoverData,
    Cmd.ROTATE: m.MotionResult,
    Cmd.LOCK: m.MotionResult,
    Cmd.UNLOCK: m.MotionResult,
    Cmd.ACTUATOR_HOME: m.MotionResult,
    **{cmd: m.SampleRecord for cmd in PROCEDURE_COMMANDS},
}


def parse_result(cmd: str, data: dict[str, Any]) -> BaseModel:
    """Type the `data` of an ok:true response by the command that produced it."""
    model = RESULT_MODELS.get(cmd)
    if model is None:
        raise ProtocolParseError(f"no result model for {cmd}")
    try:
        return model.model_validate(data)
    except ValidationError as exc:
        raise ProtocolParseError(f"invalid {cmd} result: {exc}") from exc
