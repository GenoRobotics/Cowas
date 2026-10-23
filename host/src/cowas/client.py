"""The protocol v1 client (docs/software/protocol.md §3, §7, §9, §10).

Single-threaded by design: the client "pumps" its transport while it waits, measuring every
timeout on the transport's clock. The same code therefore runs in real time on the serial
port and in virtual time against the simulator, and the tests are deterministic.

What it does for the caller:
- ids: `h<session>-<n>`. The session part is random per client, because the Due's dedupe
  table outlives a host restart: a restarted host that reused `h-00005` could get an old
  stored result instead of a new action (finding F70).
- acks and timeouts: short commands 2 s; long commands 2 s for the ack, then
  `ack.max_s + 30 s` (§10). A timeout resends **the same id** (safe, §7), up to 3 times;
  then the link is treated as lost.
- link recovery: reconnect the transport, HELLO, compare `boot_id`, then resend every pending
  request with its own id to get its stored result (§9).
- reboot detection: a new `boot_id` (from HELLO or a `boot` event) fails every pending request
  with DeviceRebooted. Their ids are never resent (§7).
- heartbeat: a PING whenever nothing was sent for `ping_period_s` (5 s), also while BUSY.
- subscription: every routed item (events, telemetry, acks, results, device log lines) goes
  to the subscribers. Subscribers may call the client (e.g. STOP from a callback).
"""

from __future__ import annotations

import logging
import math
import secrets
from collections.abc import Callable
from dataclasses import dataclass, field
from datetime import datetime
from typing import Any

from pydantic import ValidationError

from cowas import __version__
from cowas.errors import (
    CommandFailed,
    CommandTimeout,
    DeviceRebooted,
    InvalidArguments,
    LinkLost,
    ProtocolMismatch,
    TransportError,
)
from cowas.protocol import models as m
from cowas.protocol.codec import ProtocolParseError, parse_result
from cowas.protocol.spec import (
    ACK_TIMEOUT_S,
    KNOWN_COMMANDS,
    LONG_MARGIN_S,
    PING_PERIOD_S,
    PROTO_MAJOR,
    SHORT_RETRIES,
    SHORT_TIMEOUT_S,
    Cmd,
    is_long,
)
from cowas.protocol.validate import Envelope, Refusal, check_args
from cowas.transport.base import Transport
from cowas.transport.devicelog import DeviceLog
from cowas.transport.routing import Incoming, LineRouter

log = logging.getLogger(__name__)

Subscriber = Callable[[Incoming], None]


@dataclass
class PendingRequest:
    id: str
    cmd: str
    args: dict[str, Any]
    line: str
    long: bool
    heartbeat: bool = False
    sends: int = 0
    last_send: float = 0.0
    ack: m.Ack | None = None
    ack_at: float | None = None
    dup_acks: int = 0
    hard_deadline: float | None = None
    final_check: bool = False
    link_recoveries: int = 0
    response: m.Response | None = None
    reboot: DeviceRebooted | None = None
    messages: list[m.Message] = field(default_factory=list)  # evt/tel carrying this id

    @property
    def done(self) -> bool:
        return self.response is not None or self.reboot is not None


@dataclass(frozen=True)
class RebootInfo:
    old_boot_id: str | None
    new_boot_id: str
    detected_utc: datetime
    interrupted: list[tuple[str, str]]  # (request id, cmd) of the pending requests
    hello: m.HelloData | None


class CowasClient:
    def __init__(
        self,
        transport: Transport,
        *,
        device_log: DeviceLog | None = None,
        session: str | None = None,
        client_name: str = f"cowas-host {__version__}",
        heartbeat: bool = True,
        ping_period_s: float | None = None,
        short_timeout_s: float = SHORT_TIMEOUT_S,
        short_retries: int = SHORT_RETRIES,
        ack_timeout_s: float = ACK_TIMEOUT_S,
        long_margin_s: float = LONG_MARGIN_S,
        reconnect_timeout_s: float = 60.0,
    ) -> None:
        self.transport = transport
        self.router = LineRouter(transport, device_log)
        self.clock = transport.clock
        self.session = session or secrets.token_hex(3)
        self.client_name = client_name
        self.heartbeat_enabled = heartbeat
        self._ping_period_arg = ping_period_s
        self.ping_period_s = ping_period_s or PING_PERIOD_S
        self.short_timeout_s = short_timeout_s
        self.short_retries = short_retries
        self.ack_timeout_s = ack_timeout_s
        self.long_margin_s = long_margin_s
        self.reconnect_timeout_s = reconnect_timeout_s

        self.boot_id: str | None = None
        self.hello_data: m.HelloData | None = None
        self.info: m.GetInfoData | None = None
        self.last_mode: str | None = None
        self.reboots: list[RebootInfo] = []
        self.stray_messages = 0
        self.link_recoveries = 0

        self._counter = 0
        self._pending: dict[str, PendingRequest] = {}
        self._subscribers: list[Subscriber] = []
        self._connected = False
        self._recovering = False
        self._last_tx = -math.inf
        self._heartbeat: PendingRequest | None = None

    # ------------------------------------------------------------------ lifecycle

    def connect(self) -> m.HelloData:
        """Open the transport, HELLO (identity, reboot reference), GET_INFO (geometry, limits)."""
        if not self.transport.is_open:
            self.transport.open()
        hello = self.hello()
        self.info = self.get_info()
        if self._ping_period_arg is None:
            self.ping_period_s = float(self.info.limits.ping_period_s)
        self._connected = True
        return hello

    def close(self) -> None:
        self._connected = False
        self.transport.close()

    def __enter__(self) -> CowasClient:
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()

    def subscribe(self, callback: Subscriber) -> Callable[[], None]:
        self._subscribers.append(callback)

        def unsubscribe() -> None:
            if callback in self._subscribers:
                self._subscribers.remove(callback)

        return unsubscribe

    @property
    def pending(self) -> list[PendingRequest]:
        return [p for p in self._pending.values() if not p.heartbeat]

    # ------------------------------------------------------------------ sending

    def new_id(self) -> str:
        self._counter += 1
        return f"h{self.session}-{self._counter:05d}"

    def send(
        self,
        cmd: str,
        args: dict[str, Any] | None = None,
        *,
        request_id: str | None = None,
        validate: bool = True,
    ) -> PendingRequest:
        """Send a request without waiting. `request_id` resends an earlier id on purpose (§7)."""
        args = dict(args or {})
        rid = request_id or self.new_id()
        if validate and self.info is not None and cmd in KNOWN_COMMANDS:
            checked = check_args(Envelope(rid, cmd, args), self.info)
            if isinstance(checked, Refusal):
                raise InvalidArguments(cmd, checked.code, checked.extra.get("arg"), checked.msg)
        return self._send_raw(cmd, args, rid)

    def _send_raw(
        self, cmd: str, args: dict[str, Any], rid: str | None = None, *, heartbeat: bool = False
    ) -> PendingRequest:
        rid = rid or self.new_id()
        if rid in self._pending:
            raise ValueError(f"request id {rid} is already pending")
        line = m.Request(id=rid, cmd=cmd, args=args).to_line()
        p = PendingRequest(rid, cmd, args, line, is_long(cmd), heartbeat=heartbeat)
        self._pending[rid] = p
        self._transmit(p)
        return p

    def _transmit(self, p: PendingRequest) -> None:
        p.sends += 1
        p.last_send = self.clock.monotonic()
        self._last_tx = p.last_send
        if p.sends > 1:
            log.info("resending %s %s (attempt %d)", p.cmd, p.id, p.sends)
        try:
            self.router.write(p.line)
        except TransportError as exc:
            if self._recovering:
                raise LinkLost(f"write failed during recovery: {exc}") from exc
            self._recover_link()  # resends every pending request, p included

    # ------------------------------------------------------------------ waiting

    def wait(self, p: PendingRequest) -> m.Response:
        """Block (pumping the transport) until `p` has its result. Raises DeviceRebooted,
        CommandTimeout or LinkLost."""
        try:
            while True:
                if p.reboot is not None:
                    raise p.reboot
                if p.response is not None:
                    return p.response
                now = self.clock.monotonic()
                deadline = self._deadline(p)
                if now >= deadline:
                    self._on_timeout(p)
                    continue
                self._pump(min(deadline - now, self._until_heartbeat()))
        finally:
            # Done, failed or given up: the caller owns the id now (it may resend it with
            # send(..., request_id=p.id) to fetch a stored result).
            self._pending.pop(p.id, None)

    def call(self, cmd: str, args: dict[str, Any] | None = None) -> m.Response:
        return self.wait(self.send(cmd, args))

    def poll(self, duration_s: float) -> None:
        """Pump for `duration_s` (events, telemetry, logs to subscribers; heartbeat PINGs)."""
        end = self.clock.monotonic() + duration_s
        while True:
            now = self.clock.monotonic()
            if now >= end:
                return
            self._pump(min(end - now, self._until_heartbeat()))

    def _deadline(self, p: PendingRequest) -> float:
        if p.hard_deadline is not None:
            return p.hard_deadline
        if p.long and p.ack is not None and p.ack_at is not None:
            return p.ack_at + p.ack.max_s + self.long_margin_s
        return p.last_send + (self.ack_timeout_s if p.long else self.short_timeout_s)

    def _on_timeout(self, p: PendingRequest) -> None:
        if p.long and p.ack is not None:
            # Past ack.max_s + 30 s. Check for a reboot, then ask once for the stored result.
            if p.final_check:
                raise CommandTimeout(
                    p.id,
                    p.cmd,
                    f"{p.cmd} {p.id}: no result {p.ack.max_s:g} + {self.long_margin_s:g} s "
                    "after its ack",
                )
            p.final_check = True
            log.warning("%s %s overran max_s=%g: checking the Due", p.cmd, p.id, p.ack.max_s)
            self.hello()
            if p.done:
                return
            p.hard_deadline = self.clock.monotonic() + self.short_timeout_s * (
                self.short_retries + 1
            )
            self._transmit(p)
            return
        if p.sends <= self.short_retries:
            self._transmit(p)
            return
        if self._recovering or p.link_recoveries >= 1:
            raise LinkLost(f"no answer to {p.cmd} {p.id} after {p.sends} attempts")
        p.link_recoveries += 1
        self._recover_link()

    # ------------------------------------------------------------------ the pump

    def _pump(self, timeout: float) -> None:
        try:
            item = self.router.read(max(0.0, timeout))
        except TransportError as exc:
            if self._recovering:
                raise LinkLost(f"transport lost during recovery: {exc}") from exc
            log.warning("transport error: %s", exc)
            self._recover_link()
            return
        if item is not None:
            self._dispatch(item)
        self._service_heartbeat()

    def _dispatch(self, item: Incoming) -> None:
        msg = item.message
        if isinstance(msg, m.Response):
            self._on_response(msg)
        elif isinstance(msg, m.Ack):
            p = self._pending.get(msg.id)
            if p is None:
                self.stray_messages += 1
                log.info("ack for an unknown id %s", msg.id)
            elif p.ack is None:
                p.ack, p.ack_at = msg, self.clock.monotonic()
            else:
                p.dup_acks += 1  # keep the first ack's time: the deadline does not move
        elif isinstance(msg, m.BootEvent):
            if self.boot_id is not None and msg.boot_id != self.boot_id:
                self._on_reboot(msg.boot_id, None, exclude=None)
            self.boot_id = msg.boot_id
        elif isinstance(msg, m.ModeEvent):
            self.last_mode = msg.to
        if isinstance(msg, (m.Event, m.Telemetry)):
            rid = getattr(msg, "id", None)
            if isinstance(rid, str) and rid in self._pending:
                self._pending[rid].messages.append(msg)
        for callback in list(self._subscribers):
            callback(item)

    def _on_response(self, res: m.Response) -> None:
        if res.id is None:
            log.error("the Due refused a line from this host: %s", res.err)
            self.stray_messages += 1
            return
        p = self._pending.get(res.id)
        if p is None:
            self.stray_messages += 1
            log.info("result for an unknown or finished id %s", res.id)
            return
        self.last_mode = res.mode
        if p.cmd == Cmd.HELLO and res.ok and res.data is not None:
            try:
                hello = m.HelloData.model_validate(res.data)
            except ValidationError as exc:
                log.error("invalid HELLO data: %s", exc)
            else:
                if self.boot_id is not None and hello.boot_id != self.boot_id:
                    self._on_reboot(hello.boot_id, hello, exclude=p.id)
                self.boot_id = hello.boot_id
                self.hello_data = hello
        p.response = res
        if p.heartbeat:
            self._pending.pop(p.id, None)
            if self._heartbeat is p:
                self._heartbeat = None

    def _on_reboot(self, new_boot_id: str, hello: m.HelloData | None, exclude: str | None) -> None:
        old = self.boot_id
        interrupted: list[tuple[str, str]] = []
        for p in list(self._pending.values()):
            if p.id == exclude or p.done:
                continue
            if p.heartbeat:
                self._pending.pop(p.id, None)
                self._heartbeat = None
                continue
            p.reboot = DeviceRebooted(p.id, p.cmd, old, new_boot_id, hello)
            interrupted.append((p.id, p.cmd))
        self.reboots.append(RebootInfo(old, new_boot_id, self.clock.utcnow(), interrupted, hello))
        self.boot_id = new_boot_id
        log.warning(
            "the Due rebooted (boot_id %s -> %s); interrupted: %s",
            old,
            new_boot_id,
            ", ".join(f"{c} {i}" for i, c in interrupted) or "nothing",
        )

    # ------------------------------------------------------------------ heartbeat

    def _heartbeat_active(self) -> bool:
        return self.heartbeat_enabled and self._connected and not self._recovering

    def _until_heartbeat(self) -> float:
        if not self._heartbeat_active():
            return math.inf
        now = self.clock.monotonic()
        if self._heartbeat is not None:
            return max(0.0, self._deadline(self._heartbeat) - now)
        return max(0.0, self._last_tx + self.ping_period_s - now)

    def _service_heartbeat(self) -> None:
        if not self._heartbeat_active():
            return
        now = self.clock.monotonic()
        hb = self._heartbeat
        if hb is not None:
            if hb.done:
                self._heartbeat = None
            elif now >= self._deadline(hb):
                if hb.sends <= self.short_retries:
                    self._transmit(hb)
                else:
                    self._pending.pop(hb.id, None)
                    self._heartbeat = None
                    log.warning("heartbeat PING unanswered: reconnecting")
                    self._recover_link()
            return
        if now - self._last_tx >= self.ping_period_s:
            self._heartbeat = self._send_raw(Cmd.PING, {}, heartbeat=True)

    # ------------------------------------------------------------------ link recovery

    def _recover_link(self) -> None:
        """Reconnect, HELLO (reboot check), resend the pending requests with their ids (§9)."""
        if self._recovering:
            raise LinkLost("link lost during recovery")
        self._recovering = True
        self.link_recoveries += 1
        try:
            deadline = self.clock.monotonic() + self.reconnect_timeout_s
            last_exc: Exception | None = None
            while True:
                remaining = deadline - self.clock.monotonic()
                if remaining <= 0:
                    raise LinkLost(f"could not restore the link: {last_exc}")
                try:
                    self.transport.reconnect(remaining)
                    hello = self._send_raw(Cmd.HELLO, m.hello_args(self.client_name))
                    self.wait(hello)
                    break
                except (TransportError, LinkLost) as exc:
                    last_exc = exc
                    log.info("link recovery: %s", exc)
            log.warning("link restored (boot_id %s)", self.boot_id)
            for p in list(self._pending.values()):
                if p.done or p.heartbeat:
                    continue
                p.sends = 0
                self._transmit(p)
        finally:
            self._recovering = False

    # ------------------------------------------------------------------ typed commands

    @staticmethod
    def _data(cmd: str, res: m.Response) -> Any:
        if not res.ok or res.data is None:
            raise CommandFailed(cmd, res)
        try:
            return parse_result(cmd, res.data)
        except ProtocolParseError as exc:
            raise CommandFailed(cmd, res) from exc

    def result(self, p: PendingRequest) -> Any:
        """Wait for `p` and return its typed data; raise CommandFailed on ok:false."""
        return self._data(p.cmd, self.wait(p))

    def hello(self) -> m.HelloData:
        res = self.call(Cmd.HELLO, m.hello_args(self.client_name))
        if not res.ok and res.err is not None and res.err.code == "proto_mismatch":
            raise ProtocolMismatch(f"the Due speaks protocol {res.err.proto}")
        hello: m.HelloData = self._data(Cmd.HELLO, res)
        if hello.proto_major != PROTO_MAJOR:
            raise ProtocolMismatch(
                f"the Due speaks protocol {hello.proto}, this host {PROTO_MAJOR}"
            )
        return hello

    def ping(self) -> int:
        return self._data(Cmd.PING, self.call(Cmd.PING)).ms

    def status(self) -> m.StatusData:
        return self._data(Cmd.STATUS, self.call(Cmd.STATUS))

    def get_info(self) -> m.GetInfoData:
        return self._data(Cmd.GET_INFO, self.call(Cmd.GET_INFO))

    def stop(self) -> m.StopData:
        """STOP: always accepted, never deduplicated (§5.6). Safe state + FAULT(USER_STOP)."""
        return self._data(Cmd.STOP, self.call(Cmd.STOP))

    def clear_fault(self) -> str:
        return self._data(Cmd.CLEAR_FAULT, self.call(Cmd.CLEAR_FAULT)).cleared

    def recover(self, confirm_moving: bool) -> m.RecoverData:
        """RECOVER (§5.6). With confirm_moving=False, CommandFailed(code='confirm_required')
        lists in `response.err.needs` the moving steps that would run."""
        return self._data(Cmd.RECOVER, self.call(Cmd.RECOVER, {"confirm_moving": confirm_moving}))

    def actuator_home(self) -> m.MotionResult:
        return self._data(Cmd.ACTUATOR_HOME, self.call(Cmd.ACTUATOR_HOME))

    def rotate(self, slot: int) -> m.MotionResult:
        return self._data(Cmd.ROTATE, self.call(Cmd.ROTATE, {"slot": slot}))

    def lock(self) -> m.MotionResult:
        return self._data(Cmd.LOCK, self.call(Cmd.LOCK))

    def unlock(self) -> m.MotionResult:
        return self._data(Cmd.UNLOCK, self.call(Cmd.UNLOCK))

    @staticmethod
    def run_cycle_args(
        slot: int,
        *,
        volume_ml: float | None,
        max_duration_s: int,
        purge_ml: float | None = None,
        stages: dict[str, bool] | None = None,
        sample_id: str | None = None,
    ) -> dict[str, Any]:
        args: dict[str, Any] = {
            "slot": slot,
            "purge_ml": purge_ml,
            "volume_ml": volume_ml,
            "max_duration_s": max_duration_s,
        }
        if stages is not None:
            args["stages"] = stages
        if sample_id is not None:
            args["sample_id"] = sample_id
        return args

    def start_run_cycle(self, slot: int, **kwargs: Any) -> PendingRequest:
        return self.send(Cmd.RUN_CYCLE, self.run_cycle_args(slot, **kwargs))

    def run_cycle(self, slot: int, **kwargs: Any) -> m.SampleRecord:
        """RUN_CYCLE (§5.8). ok:true = the record, complete or partial (read `completed`,
        `end_reason`, `abort`); CommandFailed = refused, nothing moved."""
        return self.result(self.start_run_cycle(slot, **kwargs))

    def purge(self, volume_ml: float | None = None) -> m.SampleRecord:
        return self._data(Cmd.PURGE, self.call(Cmd.PURGE, {"volume_ml": volume_ml}))

    def sample(self, slot: int, volume_ml: float | None, max_duration_s: int) -> m.SampleRecord:
        args = {"slot": slot, "volume_ml": volume_ml, "max_duration_s": max_duration_s}
        return self._data(Cmd.SAMPLE, self.call(Cmd.SAMPLE, args))

    def dna_shield(self, slot: int, duration_ms: int | None = None) -> m.SampleRecord:
        args: dict[str, Any] = {"slot": slot}
        if duration_ms is not None:
            args["duration_ms"] = duration_ms
        return self._data(Cmd.DNA_SHIELD, self.call(Cmd.DNA_SHIELD, args))

    def empty(self) -> m.SampleRecord:
        return self._data(Cmd.EMPTY, self.call(Cmd.EMPTY))

    def prime_shield(self, duration_ms: int | None = None) -> m.SampleRecord:
        args = {} if duration_ms is None else {"duration_ms": duration_ms}
        return self._data(Cmd.PRIME_SHIELD, self.call(Cmd.PRIME_SHIELD, args))
