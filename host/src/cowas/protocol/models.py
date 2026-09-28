"""Pydantic models for protocol v1 messages and results (docs/PROTOCOL.md §3, §5, §6).

Due -> host models are strict about types (a string is never a number, a bool is never an
int) but accept unknown fields (§1.6 forward compatibility); unknown fields stay readable
in `model_extra`. Result fields that the spec says are "always present, maybe null" default
to None here, so a firmware that forgets one does not make the host lose a sample record:
`SampleRecord.missing_fields()` reports them instead.
"""

from __future__ import annotations

from typing import Any, Literal

from pydantic import BaseModel, ConfigDict, Field, model_validator

from cowas.protocol.spec import ID_RE, PROTO_VERSION


class _Model(BaseModel):
    model_config = ConfigDict(extra="allow", strict=True, frozen=True, populate_by_name=True)


# --------------------------------------------------------------------------- host -> Due


class Request(BaseModel):
    """§3.1. The only message the host sends."""

    model_config = ConfigDict(extra="forbid", strict=True, frozen=True)

    t: Literal["req"] = "req"
    id: str = Field(pattern=ID_RE.pattern)
    cmd: str
    args: dict[str, Any] = Field(default_factory=dict)

    def to_line(self) -> str:
        """The wire form: compact JSON, no newline (the transport adds it)."""
        return self.model_dump_json(exclude_none=False)


# --------------------------------------------------------------------------- Due -> host


class Ack(_Model):
    """§3.2: accepted and started (long commands only)."""

    t: Literal["ack"]
    id: str
    cmd: str
    max_s: float
    dup: bool = False


class ErrorInfo(_Model):
    """§3.3 / §8: `err` of a refused or failed request. Extra fields depend on `code`."""

    code: str
    msg: str = ""
    arg: str | None = None
    field: str | None = None
    mode: str | None = None
    cause: str | None = None
    needs: list[str] | None = None
    step: str | None = None
    steps: list[dict[str, Any]] | None = None
    fault: str | None = None
    cmd: str | None = None
    summary: dict[str, Any] | None = None
    sensor: str | None = None
    angle_deg: float | None = None
    max: int | None = None
    proto: str | None = None


class Response(_Model):
    """§3.3: the final answer to one request id."""

    t: Literal["res"]
    id: str | None
    ok: bool
    mode: str
    data: dict[str, Any] | None = None
    err: ErrorInfo | None = None

    @model_validator(mode="after")
    def _data_or_err(self) -> Response:
        if self.ok and self.data is None:
            raise ValueError("ok:true without data")
        if not self.ok and self.err is None:
            raise ValueError("ok:false without err")
        return self


class Event(_Model):
    """§3.4. The base class is also what an unknown event type parses into (hosts ignore it)."""

    t: Literal["evt"]
    ms: int
    type: str


class FwInfo(_Model):
    version: str
    git: str
    dirty: bool
    built: str


class BootEvent(Event):
    type: Literal["boot"]
    boot_id: str
    reset_cause: str
    proto: str
    fw: FwInfo


class ModeEvent(Event):
    type: Literal["mode"]
    from_: str = Field(alias="from")
    to: str
    cause: str | None = None


class FaultEvent(Event):
    type: Literal["fault"]
    code: str
    msg: str = ""
    phase: str | None = None


class FaultClearedEvent(Event):
    type: Literal["fault_cleared"]
    code: str


class WarningEvent(Event):
    type: Literal["warning"]
    code: str
    msg: str = ""
    phase: str | None = None


class StepStartEvent(Event):
    type: Literal["step_start"]
    id: str
    cmd: str
    step: str


class StepEndEvent(Event):
    type: Literal["step_end"]
    id: str
    cmd: str
    step: str
    ok: bool
    duration_s: float | None = None


class PhaseEvent(Event):
    type: Literal["phase"]
    id: str | None = None
    from_: str = Field(alias="from")
    to: str


class LinkEvent(Event):
    type: Literal["link"]
    state: Literal["lost", "restored"]
    silent_s: float | None = None


EVENT_MODELS: dict[str, type[Event]] = {
    "boot": BootEvent,
    "mode": ModeEvent,
    "fault": FaultEvent,
    "fault_cleared": FaultClearedEvent,
    "warning": WarningEvent,
    "step_start": StepStartEvent,
    "step_end": StepEndEvent,
    "phase": PhaseEvent,
    "link": LinkEvent,
}


class Telemetry(_Model):
    """§3.5: ~1 Hz while a long command runs. Every value may be null."""

    t: Literal["tel"]
    ms: int
    id: str | None = None
    cmd: str | None = None
    step: str | None = None
    phase: str | None = None
    elapsed_s: float | None = None
    slot: int | None = None
    lock: str | None = None
    p_in: float | None = None
    flow_ml_min: float | None = None
    volume_ml: float | None = None
    pump_pct: float | None = None


class LogMessage(_Model):
    """§3.6: a copy of a human console line. Never parsed for control."""

    t: Literal["log"]
    ms: int
    lvl: str
    msg: str


class UnknownMessage(_Model):
    """A JSON object with a `t` this host does not know (a later minor version)."""

    t: str


Message = Ack | Response | Event | Telemetry | LogMessage | UnknownMessage


# --------------------------------------------------------------------------- result data


class Caps(_Model):
    """§5.3: what is installed and configured. Unknown capabilities are kept in model_extra."""

    p_in: bool = False
    p_out: bool = False
    q_in: bool = False
    q_out: bool = False
    estop_input: bool = False
    spool: bool = False
    water_temp: bool = False
    leak: bool = False
    oring_home_switch: bool = False


class FaultInfo(_Model):
    code: str
    msg: str = ""


class HelloData(_Model):
    proto: str
    device_id: str
    boot_id: str
    uptime_s: float
    fw: FwInfo
    mode: str
    recovery_required: bool
    recovery_cause: str | None = None
    fault: FaultInfo | None = None
    caps: Caps

    @property
    def proto_major(self) -> int | None:
        head = self.proto.split(".", 1)[0]
        return int(head) if head.isdigit() else None


class PingData(_Model):
    ms: int


class BusyInfo(_Model):
    id: str
    cmd: str
    step: str | None = None
    phase: str | None = None
    elapsed_s: float | None = None
    max_s: float | None = None


class SensorReading(_Model):
    name: str
    value: float | None = None
    unit: str | None = None
    stat: str | None = None


class WarningEntry(_Model):
    code: str
    msg: str = ""
    phase: str | None = None


class LastCommand(_Model):
    id: str
    cmd: str
    ok: bool
    end_reason: str | None = None


class StatusData(_Model):
    mode: str
    boot_id: str
    uptime_s: float
    recovery_required: bool
    recovery_cause: str | None = None
    fault: FaultInfo | None = None
    lock: str
    slot: int | None = None
    angle_deg: float | None = None
    busy: BusyInfo | None = None
    sensors: list[SensorReading] = Field(default_factory=list)
    warnings: list[WarningEntry] = Field(default_factory=list)
    link_ok: bool = True
    tx_dropped: int = 0
    last: LastCommand | None = None


class Position(_Model):
    slot: int
    angle_deg: float | None = None
    clock: int | None = None


class Limits(_Model):
    """§5.7 `limits`. Defaults = the PROTOCOL.md example (the real ones come from GET_INFO)."""

    volume_ml_min: float = 10
    volume_ml_max: float = 5000
    max_duration_s_max: int = 3600
    purge_ml_min: float = 424
    purge_ml_default: float = 424
    shield_ms_default: int = 250000
    duration_ms_max: int = 600000
    pressure_cap_bar: float = 2.5
    ping_period_s: float = 5
    link_timeout_s: float = 30
    max_line: int = 2048


class GetInfoData(_Model):
    """§5.7. The host sizes everything from this: it never assumes a slot count."""

    slot_count: int
    purge_slot: int
    sample_slots: list[int]
    fill_order: list[int]
    raw_positions: int | None = None
    clock_dir: str | None = None
    positions: list[Position] = Field(default_factory=list)
    limits: Limits = Field(default_factory=Limits)
    placeholders: list[str] = Field(default_factory=list)

    @model_validator(mode="after")
    def _consistent(self) -> GetInfoData:
        if self.slot_count < 1:
            raise ValueError("slot_count must be >= 1")
        slots = set(range(self.slot_count))
        if self.purge_slot not in slots:
            raise ValueError("purge_slot out of range")
        if len(set(self.sample_slots)) != len(self.sample_slots):
            raise ValueError("sample_slots has duplicates")
        if not set(self.sample_slots) <= slots - {self.purge_slot}:
            raise ValueError("sample_slots must be slots other than the purge slot")
        if sorted(self.fill_order) != sorted(self.sample_slots):
            raise ValueError("fill_order must list every sample slot exactly once")
        return self


class StopData(_Model):
    interrupted: dict[str, Any] | None = None


class ClearFaultData(_Model):
    cleared: str


class RecoverStep(_Model):
    name: str
    ok: bool
    action: str | None = None


class RecoverData(_Model):
    steps: list[RecoverStep]


class MotionResult(_Model):
    """§5.10: ROTATE, LOCK, UNLOCK, ACTUATOR_HOME."""

    slot: int | None = None
    lock: str
    angle_deg: float | None = None


class Abort(_Model):
    step: str | None = None
    reason: str
    fault: str | None = None


class PInStats(_Model):
    max: float | None = None
    mean: float | None = None
    final: float | None = None


class FlowStats(_Model):
    mean: float | None = None
    min: float | None = None
    final: float | None = None


class PhaseDuration(_Model):
    name: str
    duration_s: float | None = None


class SampleRecord(_Model):
    """§6: the RUN_CYCLE result (also PURGE, SAMPLE, DNA_SHIELD, EMPTY, PRIME_SHIELD)."""

    slot: int | None = None
    sample_id: str | None = None
    completed: bool | None = None
    abort: Abort | None = None
    end_reason: str | None = None
    volume_ml: float | None = None
    duration_s: float | None = None
    cycle_s: float | None = None
    purge_ml_target: float | None = None
    purge_ml_measured: float | None = None
    p_in: PInStats | None = None
    flow_ml_min: FlowStats | None = None
    phases: list[PhaseDuration] = Field(default_factory=list)
    shield_ms: int | None = None
    t_end_to_shield_s: float | None = None
    faults: list[WarningEntry] = Field(default_factory=list)
    warnings: list[WarningEntry] = Field(default_factory=list)
    sensors: list[SensorReading] = Field(default_factory=list)
    p_out: float | None = None
    q_out: float | None = None
    dp: float | None = None
    r_filter: float | None = None
    clog_index: float | None = None

    def missing_fields(self) -> list[str]:
        """Fields the spec says are always present but that the Due did not send."""
        return [name for name in type(self).model_fields if name not in self.model_fields_set]


def hello_args(client: str) -> dict[str, Any]:
    return {"proto": int(PROTO_VERSION.split(".")[0]), "client": client[:40]}
