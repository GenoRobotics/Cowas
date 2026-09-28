"""FakeController: protocol v1 as the Due should speak it, in virtual time (ROADMAP HO1 item 5).

It follows docs/PROTOCOL.md: the §7 check order, the §5.2 mode table, dedupe of the last 16
ids with the full results of the last 4 procedures, long commands (events, ~1 Hz telemetry,
ack then result), STOP -> FAULT(USER_STOP) -> CLEAR_FAULT -> RECOVER, link loss (the running
command finishes, nothing new starts), `boot_id` per boot, and a copy of every human log line.
It answers every request vector of protocol/vectors.json (tests/test_sim_vectors.py).

The physics is deliberately crude (constant clean flow, a quadratic clog, fixed move times):
enough to exercise the host, not a model of the machine. Timings are the PROTOCOL.md §10
budgets; the real values come from the Due's `ack.max_s`.

Time only moves in `advance()`: one generator per running command is stepped every TICK.
"""

from __future__ import annotations

import contextlib
import json
import math
import random
from collections import Counter, OrderedDict, deque
from collections.abc import Generator
from dataclasses import dataclass, field
from typing import Any

from cowas.clock import ManualClock
from cowas.protocol.codec import dumps
from cowas.protocol.models import Caps, FwInfo, GetInfoData, Limits
from cowas.protocol.spec import (
    LONG_COMMANDS,
    PROCEDURE_COMMANDS,
    PROTO_VERSION,
    PUMPING_COMMANDS,
    Cmd,
    Mode,
    accepted_in,
)
from cowas.protocol.validate import (
    CheckedArgs,
    Envelope,
    Refusal,
    check_args,
    check_command,
    parse_request_line,
)
from cowas.sim import faults as F
from cowas.sim.geometry import cowas_v1

TICK = 0.25  # s of virtual time per simulation step

# Move and stage budgets (PROTOCOL.md §10, FW0 values).
UNLOCK_S = 6.4
LOCK_S = 7.0
HOME_S = 7.5
ROTATE_BASE_S = 2.0
ROTATE_PER_RAW_S = 0.6
MOVE_BOUND_S = 30  # unlock 6.5 + rotation timeout 15 + lock 7, rounded up
PURGE_PUMP_TIMEOUT_S = 300
AIR_PUSH_S = 70.0
AIR_PUSH_BOUND_S = 120
SHIELD_AIR_PUSH_S = 20.0
EMPTY_S = 95.0
EMPTY_BOUND_S = 300
RECOVER_BOUND_S = 60
LOCK_CMD_BOUND_S = 20
PURGE_FLOW_FACTOR = 1.1  # the purge path (no filter) flows a little more
P_CLEAN_BAR = 1.0  # P_in through a clean filter at the nominal flow
P_PURGE_BAR = 0.8
P_IDLE_BAR = 0.02
FLOW_BLANKING_S = 10.0  # stats ignore the start of the water phase (FW8)
CLOG_SETTLE_S = 60.0
CLOG_RATIO = 0.30
CLOG_T_S = 30.0
FLOW_SILENT_AFTER_S = 5.0
MAX_OUTPUT_LINES = 4096
MAX_REMEMBERED = 16
MAX_FULL_RECORDS = 4
MAX_SMALL_RESULT = 256
MAX_RECORD_LIST = 8

SIM_FW = FwInfo(version="sim", git="0000000", dirty=False, built="2026-09-25T00:00:00Z")


class _Abort(Exception):
    """Unwinds a running command: reason = stopped | fault | timeout."""

    def __init__(self, reason: str, fault: str | None) -> None:
        super().__init__(reason)
        self.reason = reason
        self.fault = fault


def _r(x: float | None, nd: int = 1) -> float | None:
    return None if x is None else round(x, nd)


@dataclass
class _Record:
    """Builds the §6 sample record."""

    slot: int | None
    sample_id: str | None = None
    end_reason: str | None = None
    volume_ml: float | None = None
    duration_s: float | None = None
    purge_ml_target: float | None = None
    purge_ml_measured: float | None = None
    p_values: list[float] = field(default_factory=list)
    flow_values: list[float] = field(default_factory=list)
    phases: list[dict[str, Any]] = field(default_factory=list)
    shield_ms: int | None = None
    water_end_t: float | None = None
    t_end_to_shield_s: float | None = None
    faults: list[dict[str, Any]] = field(default_factory=list)
    warnings: list[dict[str, Any]] = field(default_factory=list)

    def _bounded(self, items: list[dict[str, Any]]) -> list[dict[str, Any]]:
        if len(items) <= MAX_RECORD_LIST:
            return list(items)
        more = len(items) - (MAX_RECORD_LIST - 1)
        return [*items[: MAX_RECORD_LIST - 1], {"code": "MORE", "msg": f"{more} more"}]

    def to_data(
        self, *, completed: bool, abort: dict[str, Any] | None, cycle_s: float, p_in_present: bool
    ) -> dict[str, Any]:
        pv = self.p_values if p_in_present else []
        fv = self.flow_values
        return {
            "slot": self.slot,
            "sample_id": self.sample_id,
            "completed": completed,
            "abort": abort,
            "end_reason": self.end_reason,
            "volume_ml": _r(self.volume_ml),
            "duration_s": _r(self.duration_s),
            "cycle_s": _r(cycle_s),
            "purge_ml_target": self.purge_ml_target,
            "purge_ml_measured": _r(self.purge_ml_measured),
            "p_in": {
                "max": _r(max(pv), 2) if pv else None,
                "mean": _r(sum(pv) / len(pv), 2) if pv else None,
                "final": _r(pv[-1], 2) if pv else None,
            },
            "flow_ml_min": {
                "mean": _r(sum(fv) / len(fv)) if fv else None,
                "min": _r(min(fv)) if fv else None,
                "final": _r(fv[-1]) if fv else None,
            },
            "phases": self.phases[:12],
            "shield_ms": self.shield_ms,
            "t_end_to_shield_s": _r(self.t_end_to_shield_s),
            "faults": self._bounded(self.faults),
            "warnings": self._bounded(self.warnings),
            "sensors": [],
            "p_out": None,
            "q_out": None,
            "dp": None,
            "r_filter": None,
            "clog_index": None,
        }


@dataclass
class _Job:
    id: str
    cmd: str
    args: dict[str, Any]
    max_s: int
    started: float
    gen: Generator[None, None, None] | None = None
    step: str | None = None
    step_started: float = 0.0
    stage_deadline: float | None = None
    phase: str = "idle"
    phase_started: float = 0.0
    phase_volume: float | None = None
    record: _Record | None = None
    abort_request: tuple[str, str | None] | None = None
    next_tel: float = 0.0


@dataclass
class _Remembered:
    cmd: str
    running: bool = True
    res: dict[str, Any] | None = None  # the full result when <= 256 bytes
    summary: dict[str, Any] | None = None


@dataclass
class _Armed:
    fault: F.Fault
    armed_at: float
    fired: bool = False


class FakeController:
    """A simulated Due. Feed it host lines with `feed_line`, read its lines with
    `read_output`, move time with `advance`. `SimTransport` wraps it as a transport."""

    def __init__(
        self,
        *,
        info: GetInfoData | None = None,
        caps: Caps | None = None,
        boot: bool = False,
        bench_mode: bool = False,
        flow_ml_min: float = 100.0,
        device_id: str = "cowas-V1",
        seed: int = 0,
    ) -> None:
        self.clock = ManualClock(on_sleep=self.advance)
        self.info = info or cowas_v1()
        self.caps = caps or Caps(p_in=True, q_in=True)
        self.bench_mode = bench_mode
        self.q_nominal = flow_ml_min
        self.device_id = device_id
        self._rng = random.Random(seed)

        self._out: deque[str] = deque()
        self.tx_dropped = 0
        # False while no host has the port open (SimTransport sets it); used alone (tests),
        # the controller keeps its output.
        self.host_attached = True
        self.executions: Counter[str] = Counter()  # accepted long commands, per cmd
        self.boot_count = 0
        self._next_tick = 0.0
        self._armed: list[_Armed] = []
        self._step_last_start: dict[str, float] = {}

        # link / port
        self._last_req_t = 0.0
        self._link_lost = False
        self._drop_until = -math.inf
        self._drop_dir = "both"
        self._drop_disconnect = False
        self._port_gone_until = -math.inf
        self._boot_evt_pending = False
        self._hang_until: float | None = None
        self._hang_usb_gap = 0.0

        # injected conditions
        self._slow: F.SlowProcedure | None = None
        self._clog: F.Clog | None = None
        self._flow_dead = False
        self._pressure_trip = False

        # machine
        self.slot: int | None = self.info.purge_slot
        self.aligned = True
        self.angle_deg: float | None = self._slot_angle(self.info.purge_slot)
        self.lock = "locked"
        self.pump_pct = 0
        self.p_in = P_IDLE_BAR
        self.flow_reading: float | None = 0.0
        self.estop_pressed = False
        self.cause_persists_code: str | None = None  # an injected cause CLEAR_FAULT can't clear
        self.encoder_ok = True

        # protocol state
        self._boot(cause="POWER_ON")
        if not boot:
            self.mode = Mode.IDLE
            self.recovery_cause = None
            self.lock = "locked"
        self._out.clear()

    # ================================================================== construction helpers

    @classmethod
    def from_state(cls, state: dict[str, Any], **kwargs: Any) -> FakeController:
        """Build the controller described by a vector's `state` (PROTOCOL.md §13)."""
        caps = Caps(**state.get("caps", {}))
        limits = Limits(**state.get("limits", {}))
        sim = cls(info=cowas_v1(limits), caps=caps, **kwargs)
        sim.mode = Mode(state.get("mode", "IDLE"))
        sim.lock = state.get("lock", "locked")
        sim.aligned = bool(state.get("aligned", True))
        slot = state.get("slot", sim.info.purge_slot)
        if sim.aligned and slot is not None:
            sim.slot, sim.angle_deg = slot, sim._slot_angle(slot)
        else:
            sim.slot = None
            base = sim._slot_angle(sim.info.purge_slot) or 0.0
            sim.angle_deg = round((base - 11.0) % 360.0, 2)
        fault = state.get("fault")
        sim.fault = (
            None
            if fault is None
            else {"code": fault["code"], "msg": fault.get("msg", fault["code"])}
        )
        sim.recovery_cause = state.get("recovery_cause")
        sim.estop_pressed = bool(state.get("estop_pressed", False))
        busy = state.get("busy")
        if busy is not None:
            sim._start_placeholder_job(busy)
        for entry in state.get("remembered", []):
            rem = _Remembered(entry["cmd"])
            res = entry.get("res")
            if entry.get("full_evicted"):
                rem.running, rem.summary = False, entry.get("summary")
            elif res is not None:
                rem.running = False
                rem.summary = sim._summary(res)
                if len(dumps(res)) <= MAX_SMALL_RESULT:
                    rem.res = res
                if entry["cmd"] in PROCEDURE_COMMANDS:
                    sim._full_records[entry["id"]] = res
            sim._remembered[entry["id"]] = rem
        sim._out.clear()
        return sim

    def _start_placeholder_job(self, busy: dict[str, Any]) -> None:
        """A running command set up by a vector state (it holds until stopped)."""
        cmd = busy["cmd"]
        args = self._default_args(cmd)
        job = _Job(busy["id"], cmd, args, self._max_s(cmd, args), self.now)
        job.step = busy.get("step") or cmd.lower()
        job.phase = busy.get("phase") or "idle"
        job.step_started = job.phase_started = self.now
        job.next_tel = self.now + 1.0
        if cmd in PROCEDURE_COMMANDS:
            job.record = _Record(slot=args.get("slot", self.info.purge_slot))
            if cmd == Cmd.RUN_CYCLE:
                job.record.purge_ml_target = args["purge_ml"]
        job.gen = self._job_body(job, self._hold_step(job))
        self._job = job
        self._remembered.setdefault(job.id, _Remembered(cmd))

    def _default_args(self, cmd: str) -> dict[str, Any]:
        lim = self.info.limits
        first = self.info.fill_order[0] if self.info.fill_order else self.info.purge_slot
        return {
            Cmd.RUN_CYCLE: {
                "slot": first,
                "purge_ml": float(lim.purge_ml_default),
                "volume_ml": 2000.0,
                "max_duration_s": 1800,
                "stages": dict.fromkeys(("purge", "sample", "shield", "empty"), True),
                "sample_id": None,
            },
            Cmd.PURGE: {"volume_ml": float(lim.purge_ml_default)},
            Cmd.SAMPLE: {"slot": first, "volume_ml": 2000.0, "max_duration_s": 1800},
            Cmd.DNA_SHIELD: {"slot": first, "duration_ms": lim.shield_ms_default},
            Cmd.PRIME_SHIELD: {"duration_ms": 10000},
            Cmd.ROTATE: {"slot": first},
            Cmd.RECOVER: {"confirm_moving": True},
        }.get(cmd, {})

    # ================================================================== public API

    @property
    def now(self) -> float:
        return self.clock.monotonic()

    @property
    def running(self) -> str | None:
        """The id of the running long command, or None."""
        return self._job.id if self._job is not None else None

    def inject(self, fault: F.Fault) -> None:
        """Arm a fault; with no step and no delay it takes effect at once."""
        armed = _Armed(fault, self.now)
        self._armed.append(armed)
        if fault.step is None and fault.after_s <= 0:
            armed.fired = True
            self._apply_fault(fault)

    def port_present(self) -> bool:
        now = self.now
        if now < self._port_gone_until:
            return False
        return not (self._drop_disconnect and now < self._drop_until)

    def _dir_up(self, direction: str) -> bool:
        if self.now >= self._drop_until or self._drop_disconnect:
            return True
        return self._drop_dir not in ("both", direction)

    def read_output(self) -> str | None:
        return self._out.popleft() if self._out else None

    def drain(self) -> list[str]:
        out = list(self._out)
        self._out.clear()
        return out

    def feed_line(self, line: str) -> None:
        """A line from the host (without or with its "\\n")."""
        if not self.port_present() or not self._dir_up("h2d") or self._hang_until is not None:
            return
        self._handle_line(line)

    def advance(self, seconds: float) -> None:
        end = self.now + max(0.0, seconds)
        while self._next_tick <= end + 1e-9:
            self.clock.set(max(self.now, self._next_tick))
            self._tick()
            self._next_tick += TICK
        self.clock.set(end)

    def run_until_idle(self, limit_s: float = 7200.0) -> None:
        """Advance until no command runs (tests)."""
        end = self.now + limit_s
        while self._job is not None and self.now < end:
            self.advance(TICK)

    def press_estop(self) -> None:
        self.estop_pressed = True
        self._latch_fault("ESTOP", "E-stop pressed")

    def release_estop(self) -> None:
        self.estop_pressed = False

    def enter_service(self) -> bool:
        """A person types `service` on the console (refused unless IDLE)."""
        if self.mode != Mode.IDLE:
            return False
        self._set_mode(Mode.SERVICE, "SERVICE")
        return True

    def exit_service(self) -> None:
        if self.mode == Mode.SERVICE:
            self._set_mode(Mode.IDLE, "SERVICE")

    def reboot(self, cause: str = "POWER_ON", usb_gap_s: float = 1.5) -> None:
        self._boot(cause=cause, usb_gap_s=usb_gap_s)

    def start_demo(self, slot: int | None = None) -> str:
        """Start a RUN_CYCLE as if another host had asked (the CLI's `monitor --sim`)."""
        rid = f"sim-demo-{self.boot_count}-{self.executions[Cmd.RUN_CYCLE]}"
        target = slot if slot is not None else self.info.fill_order[0]
        req = {
            "t": "req",
            "id": rid,
            "cmd": "RUN_CYCLE",
            "args": {"slot": target, "purge_ml": None, "volume_ml": 500, "max_duration_s": 900},
        }
        self._handle_line(dumps(req))
        return rid

    # ================================================================== output

    def _ms(self) -> int:
        return int((self.now - self._boot_t) * 1000) & 0xFFFFFFFF

    def _emit(self, obj: dict[str, Any]) -> None:
        self._emit_line(dumps(obj))

    def _emit_line(self, line: str) -> None:
        if self._hang_until is not None or not self.port_present() or not self._dir_up("d2h"):
            return
        if not self.host_attached:  # nobody has the port open: the line is lost
            return
        if len(self._out) >= MAX_OUTPUT_LINES:
            self.tx_dropped += 1
            return
        self._out.append(line)

    def _evt(self, etype: str, **fields: Any) -> None:
        self._emit({"t": "evt", "ms": self._ms(), "type": etype, **fields})

    def _log(self, lvl: str, msg: str) -> None:
        text = msg if len(msg) <= 200 else msg[:197] + "..."
        self._emit({"t": "log", "ms": self._ms(), "lvl": lvl, "msg": text})
        if self.bench_mode:
            self._emit_line(f"[{lvl}] {text}")

    def _res_ok(self, rid: str | None, data: dict[str, Any]) -> dict[str, Any]:
        res = {"t": "res", "id": rid, "ok": True, "mode": self.mode.value, "data": data}
        self._emit(res)
        return res

    def _res_err(self, rid: str | None, err: dict[str, Any]) -> dict[str, Any]:
        err = {**err, "msg": str(err.get("msg", ""))[:60]}
        res = {"t": "res", "id": rid, "ok": False, "mode": self.mode.value, "err": err}
        self._emit(res)
        return res

    def _refuse(self, refusal: Refusal) -> None:
        self._res_err(refusal.id, refusal.err())

    # ================================================================== modes and faults

    def _set_mode(self, new: Mode, cause: str | None) -> None:
        if new == self.mode:
            return
        old, self.mode = self.mode, new
        self._evt("mode", **{"from": old.value, "to": new.value, "cause": cause})
        self._log("info", f"mode {old.value} -> {new.value} ({cause})")

    def _safe_state(self) -> None:
        # Pumps off, valves closed, manifold motor stopped; the O-ring stays as it is.
        self.pump_pct = 0
        self.p_in = P_IDLE_BAR
        self.flow_reading = None if self._flow_dead else 0.0

    def _latch_fault(self, code: str, msg: str) -> None:
        job = self._job
        phase = job.phase if job is not None and job.phase != "idle" else None
        self._safe_state()
        if self.mode in (Mode.FAULT, Mode.ESTOP):
            if job is not None and job.abort_request is None:
                job.abort_request = ("fault", code)
            return
        self.fault = {"code": code, "msg": msg}
        self._set_mode(Mode.ESTOP if code == "ESTOP" else Mode.FAULT, code)
        self._evt("fault", code=code, msg=msg, phase=phase)
        self._log("error", f"FAULT {code}: {msg}")
        if job is not None:
            if job.record is not None:
                job.record.faults.append({"code": code, "msg": msg, "phase": phase})
            if job.abort_request is None:
                reason = (
                    "stopped"
                    if code == "USER_STOP"
                    else ("timeout" if code == "TIMEOUT" else "fault")
                )
                job.abort_request = (reason, code)

    def _warn(self, job: _Job | None, code: str, msg: str, phase: str | None = None) -> None:
        self._evt("warning", code=code, msg=msg, phase=phase)
        self._log("warn", f"WARNING {code}: {msg}")
        if job is not None and job.record is not None:
            job.record.warnings.append({"code": code, "msg": msg, "phase": phase})

    def _active_warnings(self) -> list[dict[str, Any]]:
        out = []
        if self._link_lost:
            out.append({"code": "LINK_LOST", "msg": "no host request", "phase": None})
        if self.lock == "unlocked" and self._job is None:
            out.append({"code": "UNLOCKED_AT_REST", "msg": "O-ring unlocked", "phase": None})
        return out

    # ================================================================== boot

    def _boot(self, cause: str, usb_gap_s: float = 0.0) -> None:
        self._out.clear()  # the reset loses whatever was not sent yet
        self.boot_count += 1
        self.boot_id = f"{self._rng.getrandbits(32):08x}"
        self._boot_t = self.now
        self.reset_cause = cause
        self.mode = Mode.BOOT
        self.recovery_cause = cause
        self.fault: dict[str, Any] | None = None
        self.lock = "unknown"  # F18: RAM only
        self._job: _Job | None = None
        self._remembered: OrderedDict[str, _Remembered] = OrderedDict()
        self._full_records: OrderedDict[str, dict[str, Any]] = OrderedDict()
        self.last: dict[str, Any] | None = None
        self._link_lost = False
        self._last_req_t = self.now
        self._hang_until = None
        self._safe_state()
        self._port_gone_until = self.now + usb_gap_s if usb_gap_s > 0 else -math.inf
        self._boot_evt_pending = True
        if usb_gap_s <= 0:
            self._boot_ready()

    def _boot_ready(self) -> None:
        """The host port is up again after a reset: `boot` event, then RECOVERY_REQUIRED."""
        self._boot_evt_pending = False
        self._evt(
            "boot",
            boot_id=self.boot_id,
            reset_cause=self.reset_cause,
            proto=PROTO_VERSION,
            fw=SIM_FW.model_dump(),
        )
        self._set_mode(Mode.RECOVERY_REQUIRED, self.reset_cause)

    # ================================================================== time

    def _tick(self) -> None:
        now = self.now
        if self._hang_until is not None:
            if now >= self._hang_until:
                self._hang_until = None
                self._boot("WATCHDOG", usb_gap_s=self._hang_usb_gap)
            return
        if self._boot_evt_pending and self.port_present():
            self._boot_ready()
        self._fire_faults()
        if self._hang_until is not None:
            return
        job = self._job
        if job is not None and job.gen is not None:
            with contextlib.suppress(StopIteration):
                next(job.gen)
            job = self._job
        if job is not None and now >= job.next_tel:
            self._emit_tel(job)
            job.next_tel += 1.0
        limit = self.info.limits.link_timeout_s
        if not self._link_lost and now - self._last_req_t >= limit:
            self._link_lost = True
            silent = round(now - self._last_req_t, 1)
            self._evt("link", state="lost", silent_s=silent)
            self._warn(
                self._job,
                "LINK_LOST",
                f"no host request for {silent:g} s",
                self._job.phase if self._job and self._job.phase != "idle" else None,
            )

    def _fire_faults(self) -> None:
        for armed in self._armed:
            if armed.fired:
                continue
            f = armed.fault
            if f.step is None:
                due = self.now >= armed.armed_at + f.after_s
            else:
                t0 = self._step_last_start.get(f.step)
                due = t0 is not None and t0 >= armed.armed_at and self.now >= t0 + f.after_s
            if due:
                armed.fired = True
                self._apply_fault(f)

    def _apply_fault(self, f: F.Fault) -> None:
        if isinstance(f, F.PressureTrip):
            self._pressure_trip = True
        elif isinstance(f, F.Clog):
            self._clog = f
        elif isinstance(f, F.SlowProcedure):
            self._slow = f
        elif isinstance(f, F.FlowmeterDead):
            self._flow_dead = True
        elif isinstance(f, F.LinkDrop):
            self._drop_until = self.now + f.duration_s
            self._drop_dir = f.direction
            self._drop_disconnect = f.disconnect
            if f.disconnect:
                self._out.clear()
        elif isinstance(f, F.WatchdogReset):
            self._hang_until = self.now + f.hang_s
            self._hang_usb_gap = f.usb_gap_s
        elif isinstance(f, F.Reboot):
            self._boot(f.cause, usb_gap_s=f.usb_gap_s)

    def _slow_factor(self) -> float:
        return self._slow.factor if self._slow is not None else 1.0

    def _emit_tel(self, job: _Job) -> None:
        pumping = job.phase in ("purge", "water")
        self._emit(
            {
                "t": "tel",
                "ms": self._ms(),
                "id": job.id,
                "cmd": job.cmd,
                "step": job.step,
                "phase": job.phase,
                "elapsed_s": _r(self.now - job.started),
                "slot": self.slot,
                "lock": self.lock,
                "p_in": _r(self.p_in, 2) if self.caps.p_in else None,
                "flow_ml_min": _r(self.flow_reading),
                "volume_ml": _r(job.phase_volume) if pumping else None,
                "pump_pct": self.pump_pct,
            }
        )

    # ================================================================== request handling

    def _handle_line(self, line: str) -> None:
        parsed = parse_request_line(line)
        if parsed is None:
            return
        if isinstance(parsed, Refusal):
            self._refuse(parsed)
            return
        env = parsed
        self._touch_link()
        rem = self._remembered.get(env.id)
        if rem is not None:
            self._answer_remembered(env, rem)
            return
        refusal = check_command(env)
        if refusal is not None:
            self._refuse(refusal)
            return
        checked = check_args(env, self.info)
        if isinstance(checked, Refusal):
            self._refuse(checked)
            return
        refusal = self._check_mode(env) or self._check_preconditions(env, checked)
        if refusal is not None:
            self._refuse(refusal)
            return
        handler = {
            Cmd.HELLO: self._cmd_hello,
            Cmd.PING: self._cmd_ping,
            Cmd.STATUS: self._cmd_status,
            Cmd.GET_INFO: self._cmd_get_info,
            Cmd.STOP: self._cmd_stop,
            Cmd.CLEAR_FAULT: self._cmd_clear_fault,
        }.get(Cmd(env.cmd))
        if handler is not None:
            handler(env, checked)
        else:
            self._start_job(env, checked)

    def _touch_link(self) -> None:
        if self._link_lost:
            silent = round(self.now - self._last_req_t, 1)
            self._link_lost = False
            self._evt("link", state="restored", silent_s=silent)
            self._log("info", f"host link restored after {silent:g} s")
        self._last_req_t = self.now

    @staticmethod
    def _summary(res: dict[str, Any]) -> dict[str, Any]:
        data = res.get("data") or {}
        return {
            "ok": res.get("ok"),
            "end_reason": data.get("end_reason"),
            "completed": data.get("completed"),
        }

    def _answer_remembered(self, env: Envelope, rem: _Remembered) -> None:
        if rem.cmd != env.cmd:
            self._res_err(
                env.id, {"code": "id_conflict", "msg": f"id used for {rem.cmd}", "cmd": rem.cmd}
            )
            return
        if rem.running:
            job = self._job
            max_s = job.max_s if job is not None and job.id == env.id else 0
            self._emit({"t": "ack", "id": env.id, "cmd": env.cmd, "max_s": max_s, "dup": True})
            return
        full = rem.res or self._full_records.get(env.id)
        if full is not None:
            self._emit(full)  # unchanged, including its `mode` (§7)
            return
        self._res_err(
            env.id,
            {
                "code": "result_expired",
                "msg": "full result evicted",
                "cmd": rem.cmd,
                "summary": rem.summary,
            },
        )

    def _check_mode(self, env: Envelope) -> Refusal | None:
        cmd = env.cmd
        busy_with = self._job if cmd in LONG_COMMANDS else None
        if accepted_in(cmd, self.mode) and busy_with is None:
            return None
        extra: dict[str, Any] = {"mode": self.mode.value}
        if self.mode == Mode.RECOVERY_REQUIRED and self.recovery_cause:
            extra["cause"] = self.recovery_cause
        elif self.mode == Mode.FAULT and self.fault:
            extra["cause"] = self.fault["code"]
        job = self._job
        if job is not None:
            msg = f"busy with {job.cmd} {job.id}"
        else:
            msg = f"{cmd} not accepted in {self.mode.value}"
        return Refusal("wrong_mode", msg[:60], env.id, extra)

    def _recover_needs(self) -> list[str]:
        needs = []
        if not self.aligned:
            needs.append("align")
        if self.lock == "unknown":
            needs.append("actuator_home")
        elif self.lock == "unlocked" or not self.aligned:
            needs.append("lock")
        return needs

    def _cause_persists(self) -> str | None:
        if self.estop_pressed:
            return "ESTOP"
        return self.cause_persists_code

    def _check_preconditions(self, env: Envelope, checked: CheckedArgs) -> Refusal | None:
        cmd, args = env.cmd, checked.args
        if cmd in PUMPING_COMMANDS and not self.caps.p_in:
            return Refusal("sensor_unavailable", "no trusted P_in", env.id, {"sensor": "p_in"})
        needs_q = cmd == Cmd.SAMPLE or (cmd == Cmd.RUN_CYCLE and args["stages"]["sample"])
        if needs_q and args.get("volume_ml") is not None and not self.caps.q_in:
            return Refusal(
                "sensor_unavailable", "volume target needs q_in", env.id, {"sensor": "q_in"}
            )
        if (cmd in (Cmd.ROTATE, Cmd.LOCK, Cmd.UNLOCK) or cmd in PROCEDURE_COMMANDS) and (
            self.lock == "unknown"
        ):
            return Refusal("lock_unknown", "O-ring state unknown: ACTUATOR_HOME", env.id)
        if cmd in (Cmd.LOCK, Cmd.ACTUATOR_HOME) and not self.aligned:
            return Refusal(
                "not_aligned",
                "manifold not on a slot",
                env.id,
                {"angle_deg": self.angle_deg if self.angle_deg is not None else 0.0},
            )
        if cmd == Cmd.CLEAR_FAULT:
            persists = self._cause_persists()
            if persists is not None:
                return Refusal(
                    "cause_persists", f"{persists} still active", env.id, {"fault": persists}
                )
        if cmd == Cmd.RECOVER and not args["confirm_moving"]:
            needs = self._recover_needs()
            if needs:
                return Refusal(
                    "confirm_required", "recovery needs to move", env.id, {"needs": needs}
                )
        return None

    # ================================================================== short commands

    def _cmd_hello(self, env: Envelope, checked: CheckedArgs) -> None:
        client = checked.args.get("client")
        if client:
            self._log("info", f"host: {client}")
        self._res_ok(
            env.id,
            {
                "proto": PROTO_VERSION,
                "device_id": self.device_id,
                "boot_id": self.boot_id,
                "uptime_s": _r(self.now - self._boot_t),
                "fw": SIM_FW.model_dump(),
                "mode": self.mode.value,
                "recovery_required": self.mode == Mode.RECOVERY_REQUIRED,
                "recovery_cause": self.recovery_cause
                if self.mode == Mode.RECOVERY_REQUIRED
                else None,
                "fault": self.fault,
                "caps": self.caps.model_dump(),
            },
        )

    def _cmd_ping(self, env: Envelope, checked: CheckedArgs) -> None:
        self._res_ok(env.id, {"ms": self._ms()})

    def _cmd_status(self, env: Envelope, checked: CheckedArgs) -> None:
        job = self._job
        busy = None
        if job is not None and self.mode == Mode.BUSY:
            busy = {
                "id": job.id,
                "cmd": job.cmd,
                "step": job.step,
                "phase": job.phase,
                "elapsed_s": _r(self.now - job.started),
                "max_s": job.max_s,
            }
        sensors = [
            {
                "name": "p_in",
                "value": _r(self.p_in, 2) if self.caps.p_in else None,
                "unit": "bar",
                "stat": "ok" if self.caps.p_in else "absent",
            },
            {
                "name": "flow_ml_min",
                "value": _r(self.flow_reading),
                "unit": "mL/min",
                "stat": ("stale" if self._flow_dead else "ok") if self.caps.q_in else "absent",
            },
        ]
        self._res_ok(
            env.id,
            {
                "mode": self.mode.value,
                "boot_id": self.boot_id,
                "uptime_s": _r(self.now - self._boot_t),
                "recovery_required": self.mode == Mode.RECOVERY_REQUIRED,
                "recovery_cause": self.recovery_cause
                if self.mode == Mode.RECOVERY_REQUIRED
                else None,
                "fault": self.fault,
                "lock": self.lock,
                "slot": self.slot if self.aligned else None,
                "angle_deg": self.angle_deg,
                "busy": busy,
                "sensors": sensors,
                "warnings": self._active_warnings(),
                "link_ok": not self._link_lost,
                "tx_dropped": self.tx_dropped,
                "last": self.last,
            },
        )

    def _cmd_get_info(self, env: Envelope, checked: CheckedArgs) -> None:
        self._res_ok(env.id, self.info.model_dump(mode="json"))

    def _cmd_stop(self, env: Envelope, checked: CheckedArgs) -> None:
        job = self._job
        interrupted = {"id": job.id, "cmd": job.cmd} if job is not None else None
        if self.mode in (Mode.FAULT, Mode.ESTOP):
            self._safe_state()  # re-applied, nothing else changes
            if job is not None and job.abort_request is None:
                job.abort_request = ("stopped", "USER_STOP")
        else:
            self._latch_fault("USER_STOP", "STOP from host")
        self._res_ok(env.id, {"interrupted": interrupted})

    def _cmd_clear_fault(self, env: Envelope, checked: CheckedArgs) -> None:
        code = self.fault["code"] if self.fault else "USER_STOP"
        self._remembered[env.id] = _Remembered(env.cmd)
        self._evt("fault_cleared", code=code)
        self.fault = None
        self.recovery_cause = code
        self._set_mode(Mode.RECOVERY_REQUIRED, "CLEAR_FAULT")
        res = self._res_ok(env.id, {"cleared": code})
        self._store_result(env.id, env.cmd, res)

    # ================================================================== dedupe store

    def _store_result(self, rid: str, cmd: str, res: dict[str, Any]) -> None:
        rem = self._remembered.setdefault(rid, _Remembered(cmd))
        rem.running = False
        rem.summary = self._summary(res)
        rem.res = res if len(dumps(res)) <= MAX_SMALL_RESULT else None
        if cmd in PROCEDURE_COMMANDS:
            self._full_records[rid] = res
            while len(self._full_records) > MAX_FULL_RECORDS:
                self._full_records.popitem(last=False)
        while len(self._remembered) > MAX_REMEMBERED:
            self._remembered.popitem(last=False)

    # ================================================================== long commands

    def _max_s(self, cmd: str, a: dict[str, Any]) -> int:
        lim = self.info.limits
        purge = MOVE_BOUND_S + PURGE_PUMP_TIMEOUT_S
        empty = MOVE_BOUND_S + EMPTY_BOUND_S

        def sample(max_d: int) -> int:
            return 3 * MOVE_BOUND_S + max_d + AIR_PUSH_BOUND_S

        def shield(ms: int) -> int:
            return 2 * MOVE_BOUND_S + math.ceil(ms / 1000) + 30

        if cmd == Cmd.RUN_CYCLE:
            st = a["stages"]
            return int(
                (purge if st["purge"] else 0)
                + (sample(a["max_duration_s"]) if st["sample"] else 0)
                + (shield(lim.shield_ms_default) if st["shield"] else 0)
                + (empty if st["empty"] else 0)
            )
        return int(
            {
                Cmd.PURGE: purge,
                Cmd.SAMPLE: sample(a.get("max_duration_s", 0)),
                Cmd.DNA_SHIELD: shield(a.get("duration_ms", 0)),
                Cmd.EMPTY: empty,
                Cmd.PRIME_SHIELD: 2 * MOVE_BOUND_S + math.ceil(a.get("duration_ms", 0) / 1000),
                Cmd.ROTATE: MOVE_BOUND_S,
                Cmd.LOCK: LOCK_CMD_BOUND_S,
                Cmd.UNLOCK: LOCK_CMD_BOUND_S,
                Cmd.ACTUATOR_HOME: LOCK_CMD_BOUND_S,
                Cmd.RECOVER: RECOVER_BOUND_S,
            }[cmd]
        )

    def _start_job(self, env: Envelope, checked: CheckedArgs) -> None:
        cmd, args = env.cmd, checked.args
        job = _Job(env.id, cmd, args, self._max_s(cmd, args), self.now)
        job.next_tel = self.now + 1.0
        self._remembered[env.id] = _Remembered(cmd)
        self.executions[cmd] += 1
        self._job = job
        if cmd != Cmd.RECOVER:
            self._set_mode(Mode.BUSY, cmd)
        body = {
            Cmd.RUN_CYCLE: self._body_run_cycle,
            Cmd.PURGE: self._body_purge,
            Cmd.SAMPLE: self._body_sample,
            Cmd.DNA_SHIELD: self._body_dna_shield,
            Cmd.EMPTY: self._body_empty,
            Cmd.PRIME_SHIELD: self._body_prime_shield,
            Cmd.ROTATE: self._body_rotate,
            Cmd.LOCK: self._body_lock,
            Cmd.UNLOCK: self._body_unlock,
            Cmd.ACTUATOR_HOME: self._body_actuator_home,
            Cmd.RECOVER: self._body_recover,
        }[Cmd(cmd)](job)
        if cmd in PROCEDURE_COMMANDS:
            job.record = _Record(
                slot=args.get("slot", self.info.purge_slot), sample_id=args.get("sample_id")
            )
        for code, msg in checked.warnings:
            self._warn(job, code, msg, None)
        job.gen = self._job_body(job, body)
        next(job.gen)  # runs up to the first step_start (every body yields once after it)
        self._log("info", f"{cmd} {env.id} started")
        self._emit({"t": "ack", "id": env.id, "cmd": cmd, "max_s": job.max_s})

    def _job_body(
        self, job: _Job, body: Generator[None, None, dict[str, Any] | None]
    ) -> Generator[None, None, None]:
        try:
            data = yield from body
        except _Abort as abort:
            self._set_phase(job, "idle")
            self._finish_aborted(job, abort)
            return
        self._finish_ok(job, data or {})

    def _finish_ok(self, job: _Job, data: dict[str, Any]) -> None:
        self._safe_state()
        if job.cmd == Cmd.RECOVER:
            self.recovery_cause = None
            self._set_mode(Mode.IDLE, Cmd.RECOVER.value)
        elif self.mode == Mode.BUSY:
            self._set_mode(Mode.IDLE, job.cmd)
        if job.record is not None:
            data = job.record.to_data(
                completed=True,
                abort=None,
                cycle_s=self.now - job.started,
                p_in_present=self.caps.p_in,
            )
        self._job = None
        if job.cmd == Cmd.UNLOCK:
            self._warn(None, "UNLOCKED_AT_REST", "O-ring left unlocked (UNLOCK)")
        res = self._res_ok(job.id, data)
        self._conclude(job, res)

    def _finish_aborted(self, job: _Job, abort: _Abort) -> None:
        self._safe_state()
        if job.record is not None:
            rec = job.record
            if (
                job.cmd in (Cmd.RUN_CYCLE, Cmd.SAMPLE)
                and rec.end_reason is None
                and (job.cmd == Cmd.SAMPLE or job.args["stages"]["sample"])
            ):
                rec.end_reason = self._end_reason_for(abort)
            data = rec.to_data(
                completed=False,
                abort={"step": job.step, "reason": abort.reason, "fault": abort.fault},
                cycle_s=self.now - job.started,
                p_in_present=self.caps.p_in,
            )
            self._job = None
            res = self._res_ok(job.id, data)
        else:
            self._job = None
            if job.cmd == Cmd.RECOVER and abort.reason == "recovery_failed":
                err = {
                    "code": "recovery_failed",
                    "msg": f"step {job.step} failed",
                    "step": job.step,
                    "steps": job.args.get("_steps", []),
                }
            elif abort.reason == "stopped":
                err = {"code": "stopped", "msg": f"STOP during {job.cmd}"}
            else:
                err = {
                    "code": "fault",
                    "msg": f"{abort.fault} during {job.cmd}",
                    "fault": abort.fault,
                }
            res = self._res_err(job.id, err)
        self._conclude(job, res)

    @staticmethod
    def _end_reason_for(abort: _Abort) -> str:
        if abort.reason == "stopped":
            return "stopped"
        if abort.fault == "OVER_PRESSURE":
            return "pressure_cap"
        return "fault"

    def _conclude(self, job: _Job, res: dict[str, Any]) -> None:
        self._store_result(job.id, job.cmd, res)
        data = res.get("data") or {}
        self.last = {
            "id": job.id,
            "cmd": job.cmd,
            "ok": res["ok"],
            "end_reason": data.get("end_reason"),
        }
        self._log("info", f"{job.cmd} {job.id} finished ok={res['ok']}")

    # ------------------------------------------------------------------ generator helpers

    def _check_abort(self, job: _Job) -> None:
        if job.abort_request is not None:
            reason, code = job.abort_request
            raise _Abort(reason, code)
        if (
            job.stage_deadline is not None
            and self.now > job.stage_deadline
            and (self._slow is None or self._slow.respect_bounds)
        ):
            self._latch_fault("TIMEOUT", f"step {job.step} over its time limit")
            raise _Abort("timeout", "TIMEOUT")

    def _wait(
        self, job: _Job, seconds: float, *, timed: bool = False
    ) -> Generator[None, None, None]:
        """Wait `seconds`; moves are slowed by SlowProcedure, `timed` phases (the Due's own
        timers: shield, air push, empty, prime) are not."""
        end = self.now + seconds * (1.0 if timed else self._slow_factor())
        while self.now < end - 1e-9:
            yield
            self._check_abort(job)

    def _hold_step(self, job: _Job) -> Generator[None, None, None]:
        """A placeholder command (vector state): runs until stopped."""
        try:
            while True:
                yield
                self._check_abort(job)
        except _Abort:
            self._evt(
                "step_end",
                id=job.id,
                cmd=job.cmd,
                step=job.step,
                ok=False,
                duration_s=_r(self.now - job.step_started),
            )
            raise

    def _stage(
        self, job: _Job, name: str, body: Generator[None, None, Any], bound_s: float | None
    ) -> Generator[None, None, Any]:
        job.step, job.step_started = name, self.now
        self._step_last_start[name] = self.now
        job.stage_deadline = self.now + bound_s if bound_s is not None else None
        self._evt("step_start", id=job.id, cmd=job.cmd, step=name)
        self._log("info", f"{job.cmd} {job.id}: step {name}")
        try:
            yield
            self._check_abort(job)
            result = yield from body
        except _Abort:
            self._set_phase(job, "idle")
            self._evt(
                "step_end",
                id=job.id,
                cmd=job.cmd,
                step=name,
                ok=False,
                duration_s=_r(self.now - job.step_started),
            )
            raise
        job.stage_deadline = None
        self._evt(
            "step_end",
            id=job.id,
            cmd=job.cmd,
            step=name,
            ok=True,
            duration_s=_r(self.now - job.step_started),
        )
        return result

    def _set_phase(self, job: _Job, name: str) -> None:
        if job.phase == name:
            return
        old = job.phase
        if old != "idle" and job.record is not None:
            job.record.phases.append({"name": old, "duration_s": _r(self.now - job.phase_started)})
        job.phase, job.phase_started = name, self.now
        job.phase_volume = 0.0 if name in ("purge", "water") else None
        self._evt("phase", id=job.id, **{"from": old, "to": name})
        if name == "idle":
            self._safe_state()

    # ------------------------------------------------------------------ moves

    def _slot_angle(self, slot: int) -> float | None:
        for pos in self.info.positions:
            if pos.slot == slot:
                return pos.angle_deg
        return None

    def _raw_index(self, slot: int | None) -> int:
        if slot is None:
            return 0
        n = self.info.raw_positions or self.info.slot_count
        angle = self._slot_angle(slot)
        ref = self._slot_angle(self.info.purge_slot)
        if angle is None or ref is None:
            return slot
        return round(((ref - angle) % 360.0) / (360.0 / n))

    def _unlock(self, job: _Job) -> Generator[None, None, None]:
        if self.lock == "unlocked":
            return
        if self.lock == "unknown":  # never reached: preconditions refuse it
            raise _Abort("fault", "ROTATION_REFUSED")
        yield from self._wait(job, UNLOCK_S)
        self.lock = "unlocked"

    def _lock(self, job: _Job) -> Generator[None, None, None]:
        if self.lock == "locked":
            return
        yield from self._wait(job, LOCK_S)
        self.lock = "locked"

    def _rotate(self, job: _Job, target: int) -> Generator[None, None, None]:
        if self.lock != "unlocked":  # the interlock (rule 4): never rotate while locked
            self._latch_fault("ROTATION_REFUSED", "rotation while not unlocked")
            raise _Abort("fault", "ROTATION_REFUSED")
        distance = abs(self._raw_index(target) - self._raw_index(self.slot))
        self.slot, self.aligned = None, False
        yield from self._wait(job, ROTATE_BASE_S + ROTATE_PER_RAW_S * distance)
        self.slot, self.aligned, self.angle_deg = target, True, self._slot_angle(target)

    def _move_to(self, job: _Job, target: int) -> Generator[None, None, None]:
        """unlock -> rotate -> lock (the rest state is locked, §5.1)."""
        if self.slot != target or not self.aligned:
            yield from self._unlock(job)
            yield from self._rotate(job, target)
        yield from self._lock(job)

    # ------------------------------------------------------------------ pumping

    def _pump_tick(self, job: _Job, q_true: float, p: float, pump: int) -> float | None:
        """One pumping tick: the cap check, then the readings. Returns the measured volume."""
        if self._pressure_trip:
            self._pressure_trip = False
            p = self.info.limits.pressure_cap_bar + 0.02
        self.p_in, self.pump_pct = p, pump
        cap = self.info.limits.pressure_cap_bar
        if self.caps.p_in and p > cap:
            self._latch_fault("OVER_PRESSURE", f"P_in {p:.2f} bar > cap {cap:.2f}")
            raise _Abort("fault", "OVER_PRESSURE")
        if self._flow_dead:
            self.flow_reading = None
            return None
        noise = 1.0 + self._rng.uniform(-0.01, 0.01)
        self.flow_reading = q_true * noise
        return q_true * TICK / 60.0

    def _purge(self, job: _Job, purge_ml: float) -> Generator[None, None, None]:
        rec = job.record
        assert rec is not None
        rec.purge_ml_target = purge_ml
        yield from self._move_to(job, self.info.purge_slot)
        self._set_phase(job, "purge")
        q = self.q_nominal * PURGE_FLOW_FACTOR / self._slow_factor()
        time_needed = purge_ml / (self.q_nominal * PURGE_FLOW_FACTOR) * 60.0
        measured: float | None = 0.0
        t0 = self.now
        try:
            while True:
                dv = self._pump_tick(job, q, P_PURGE_BAR, 60)
                if dv is None or measured is None:
                    measured = None  # dead flowmeter: time-based fallback
                    if self.now - t0 >= time_needed * self._slow_factor():
                        break
                else:
                    measured += dv
                    job.phase_volume = measured
                    if measured >= purge_ml:
                        break
                yield
                self._check_abort(job)
        finally:
            rec.purge_ml_measured = measured
        self._set_phase(job, "idle")

    def _water(
        self, job: _Job, volume_ml: float | None, max_duration_s: int
    ) -> Generator[None, None, None]:
        rec = job.record
        assert rec is not None
        cap = self.info.limits.pressure_cap_bar
        target = cap - 0.5
        self._set_phase(job, "water")
        t0 = self.now
        true_volume = 0.0
        measured: float | None = 0.0
        first_minute: list[float] = []
        low_since: float | None = None
        silent_warned = False
        try:
            while True:
                elapsed = self.now - t0
                resistance = 1.0
                if self._clog is not None and true_volume > self._clog.start_ml:
                    resistance += ((true_volume - self._clog.start_ml) / self._clog.scale_ml) ** 2
                p_need = P_CLEAN_BAR * resistance
                if p_need <= target:
                    p, q, pump = p_need, self.q_nominal, min(100, int(45 + 25 * p_need))
                else:
                    p, q, pump = target, self.q_nominal * target / p_need, 100
                q /= self._slow_factor()
                dv = self._pump_tick(job, q, p, pump)
                true_volume += q * TICK / 60.0
                if dv is None:
                    measured = None
                    if not silent_warned and elapsed >= FLOW_SILENT_AFTER_S:
                        silent_warned = True
                        self._warn(job, "FLOW_SILENT", "no flow pulses while pumping", "water")
                elif measured is not None:
                    measured += dv
                job.phase_volume = measured
                if self.caps.p_in:
                    rec.p_values.append(p)
                reading = self.flow_reading
                if reading is not None and elapsed >= FLOW_BLANKING_S:
                    rec.flow_values.append(reading)
                    if elapsed < CLOG_SETTLE_S:
                        first_minute.append(reading)
                rec.volume_ml, rec.duration_s = measured, elapsed
                # End conditions: the first one met sets end_reason (FW8).
                if volume_ml is not None and measured is not None and measured >= volume_ml:
                    rec.end_reason = "volume_reached"
                    break
                if reading is not None and elapsed >= CLOG_SETTLE_S and first_minute:
                    base = sum(first_minute) / len(first_minute)
                    if reading < CLOG_RATIO * base and (pump >= 95 or p >= target - 0.2):
                        low_since = self.now if low_since is None else low_since
                        if self.now - low_since >= CLOG_T_S:
                            rec.end_reason = "clogged"
                            break
                    else:
                        low_since = None
                if elapsed >= max_duration_s:
                    rec.end_reason = "duration_reached" if volume_ml is None else "timeout"
                    break
                yield
                self._check_abort(job)
        except _Abort as abort:
            rec.end_reason = self._end_reason_for(abort)
            raise
        finally:
            rec.water_end_t = self.now
        self._set_phase(job, "air_push")
        self.pump_pct, self.p_in = 80, 0.3
        yield from self._wait(job, AIR_PUSH_S, timed=True)
        self._set_phase(job, "idle")

    def _shield(self, job: _Job, slot: int, duration_ms: int) -> Generator[None, None, None]:
        rec = job.record
        assert rec is not None
        yield from self._move_to(job, slot)
        if rec.water_end_t is not None:
            rec.t_end_to_shield_s = self.now - rec.water_end_t
        self._set_phase(job, "shield")
        t0 = self.now
        try:
            yield from self._wait(job, duration_ms / 1000.0, timed=True)
        finally:  # the on-time actually run, also when interrupted
            rec.shield_ms = round((self.now - t0) * 1000)
        self._set_phase(job, "air_push")
        self.pump_pct, self.p_in = 80, 0.3
        yield from self._wait(job, SHIELD_AIR_PUSH_S, timed=True)
        self._set_phase(job, "idle")
        yield from self._move_to(job, self.info.purge_slot)

    def _empty(self, job: _Job) -> Generator[None, None, None]:
        yield from self._move_to(job, self.info.purge_slot)
        self._set_phase(job, "empty")
        self.pump_pct, self.p_in = 100, 0.4
        yield from self._wait(job, EMPTY_S, timed=True)
        self._set_phase(job, "idle")

    # ------------------------------------------------------------------ command bodies

    def _body_run_cycle(self, job: _Job) -> Generator[None, None, None]:
        a = job.args
        lim = self.info.limits
        st = a["stages"]
        if st["purge"]:
            yield from self._stage(
                job, "purge", self._purge(job, a["purge_ml"]), MOVE_BOUND_S + PURGE_PUMP_TIMEOUT_S
            )
        if st["sample"]:
            yield from self._stage(
                job,
                "sample",
                self._sample_stage(job, a["slot"], a["volume_ml"], a["max_duration_s"]),
                3 * MOVE_BOUND_S + a["max_duration_s"] + AIR_PUSH_BOUND_S,
            )
        if st["shield"]:
            yield from self._stage(
                job,
                "shield",
                self._shield(job, a["slot"], lim.shield_ms_default),
                2 * MOVE_BOUND_S + lim.shield_ms_default / 1000 + 30,
            )
        if st["empty"]:
            yield from self._stage(job, "empty", self._empty(job), MOVE_BOUND_S + EMPTY_BOUND_S)

    def _sample_stage(
        self, job: _Job, slot: int, volume_ml: float | None, max_d: int
    ) -> Generator[None, None, None]:
        yield from self._move_to(job, slot)
        yield from self._water(job, volume_ml, max_d)

    def _body_purge(self, job: _Job) -> Generator[None, None, None]:
        yield from self._stage(
            job,
            "purge",
            self._purge(job, job.args["volume_ml"]),
            MOVE_BOUND_S + PURGE_PUMP_TIMEOUT_S,
        )

    def _body_sample(self, job: _Job) -> Generator[None, None, None]:
        a = job.args
        yield from self._stage(
            job,
            "sample",
            self._sample_stage(job, a["slot"], a["volume_ml"], a["max_duration_s"]),
            3 * MOVE_BOUND_S + a["max_duration_s"] + AIR_PUSH_BOUND_S,
        )

    def _body_dna_shield(self, job: _Job) -> Generator[None, None, None]:
        a = job.args
        yield from self._stage(
            job,
            "shield",
            self._shield(job, a["slot"], a["duration_ms"]),
            2 * MOVE_BOUND_S + a["duration_ms"] / 1000 + 30,
        )

    def _body_empty(self, job: _Job) -> Generator[None, None, None]:
        yield from self._stage(job, "empty", self._empty(job), MOVE_BOUND_S + EMPTY_BOUND_S)

    def _body_prime_shield(self, job: _Job) -> Generator[None, None, None]:
        duration_ms = job.args["duration_ms"]

        def prime() -> Generator[None, None, None]:
            yield from self._move_to(job, self.info.purge_slot)
            self._set_phase(job, "prime")
            yield from self._wait(job, duration_ms / 1000.0, timed=True)
            assert job.record is not None
            job.record.shield_ms = duration_ms
            self._set_phase(job, "idle")

        yield from self._stage(job, "prime_shield", prime(), 2 * MOVE_BOUND_S + duration_ms / 1000)

    def _motion_data(self) -> dict[str, Any]:
        return {
            "slot": self.slot if self.aligned else None,
            "lock": self.lock,
            "angle_deg": self.angle_deg,
        }

    def _body_rotate(self, job: _Job) -> Generator[None, None, dict[str, Any]]:
        yield from self._stage(job, "rotate", self._move_to(job, job.args["slot"]), MOVE_BOUND_S)
        return self._motion_data()

    def _body_lock(self, job: _Job) -> Generator[None, None, dict[str, Any]]:
        yield from self._stage(job, "lock", self._lock(job), LOCK_CMD_BOUND_S)
        return self._motion_data()

    def _body_unlock(self, job: _Job) -> Generator[None, None, dict[str, Any]]:
        yield from self._stage(job, "unlock", self._unlock(job), LOCK_CMD_BOUND_S)
        return self._motion_data()

    def _body_actuator_home(self, job: _Job) -> Generator[None, None, dict[str, Any]]:
        def home() -> Generator[None, None, None]:
            yield from self._wait(job, HOME_S)
            self.lock = "locked"

        yield from self._stage(job, "actuator_home", home(), LOCK_CMD_BOUND_S)
        return self._motion_data()

    def _body_recover(self, job: _Job) -> Generator[None, None, dict[str, Any]]:
        steps: list[dict[str, Any]] = []
        job.args["_steps"] = steps

        def fail(name: str) -> None:
            steps.append({"name": name, "ok": False})
            job.step = name
            raise _Abort("recovery_failed", None)

        def recover() -> Generator[None, None, None]:
            if self._cause_persists():
                fail("cause_gone")
            steps.append({"name": "cause_gone", "ok": True})
            if not self.encoder_ok:
                fail("encoder")
            steps.append({"name": "encoder", "ok": True})
            if not self.aligned:
                if self.lock == "unknown":
                    # No rotation while the lock state is unknown, and ACTUATOR_HOME needs an
                    # aligned manifold: a person must intervene (finding F71).
                    fail("align")
                yield from self._unlock(job)
                nearest = min(
                    self.info.positions,
                    key=lambda pos: abs(
                        ((pos.angle_deg or 0) - (self.angle_deg or 0) + 180) % 360 - 180
                    ),
                )
                yield from self._rotate(job, nearest.slot)
            steps.append({"name": "align", "ok": True})
            if self.lock == "unknown":
                yield from self._wait(job, HOME_S)
                self.lock, action = "locked", "actuator_home"
            elif self.lock == "unlocked":
                yield from self._lock(job)
                action = "lock"
            else:
                action = "none"
            steps.append({"name": "oring", "ok": True, "action": action})
            steps.append({"name": "valves", "ok": True})

        yield from self._stage(job, "recover", recover(), RECOVER_BOUND_S)
        return {"steps": steps}

    # ================================================================== debugging

    def __repr__(self) -> str:
        return (
            f"<FakeController mode={self.mode} lock={self.lock} slot={self.slot} "
            f"job={self.running} t={self.now:.2f}>"
        )


def messages(lines: list[str]) -> list[dict[str, Any]]:
    """Parse output lines (tests): JSON objects only, console text skipped."""
    return [json.loads(line) for line in lines if line.startswith("{")]
