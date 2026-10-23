"""The `cowas` command line (ROADMAP HO1 item 6).

    cowas ports                         list serial ports; show the Due's Native USB port
    cowas hello | status | info         identity / live state / geometry and limits
    cowas stop                          STOP (safe state, FAULT USER_STOP)
    cowas clear-fault                   FAULT -> RECOVERY_REQUIRED
    cowas recover [--yes]               RECOVER (asks before anything moves)
    cowas rotate SLOT | lock | unlock   motion commands
    cowas run-cycle --slot N (--volume ML | --no-volume-cap) [--max-duration S] ...
    cowas monitor [--duration S]        live telemetry, events and device logs
    cowas logs [-n N] [--follow]        the device log file

Every command takes --sim (a simulated Due in virtual time; each invocation starts a fresh
one) and --json. Ctrl-C during a long command sends STOP and waits for the partial result.
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from collections.abc import Callable
from pathlib import Path
from typing import Any

from pydantic import BaseModel

from cowas import __version__
from cowas.client import CowasClient, PendingRequest
from cowas.clock import iso_utc
from cowas.errors import (
    CommandFailed,
    CommandTimeout,
    CowasError,
    DeviceRebooted,
    InvalidArguments,
    LinkLost,
    ProtocolMismatch,
    TransportError,
)
from cowas.logging_config import configure_logging, default_data_dir
from cowas.protocol import models as m
from cowas.protocol.codec import dumps, parse_result
from cowas.protocol.spec import STAGES, Cmd
from cowas.transport.devicelog import DEVICE_LOG_NAME, DeviceLog
from cowas.transport.routing import Incoming
from cowas.transport.serial_port import SerialTransport, list_ports, select_port

EXIT_OK = 0
EXIT_FAILED = 1  # the Due answered ok:false, or the result says the procedure did not complete
EXIT_USAGE = 2
EXIT_LINK = 3  # transport / link / timeout
EXIT_REBOOTED = 4
EXIT_INTERRUPTED = 130


# ============================================================================ output helpers


class OutputClosed(Exception):
    """stdout went away (e.g. `cowas monitor | head`)."""


def _out(text: str = "") -> None:
    try:
        print(text, flush=True)
    except (BrokenPipeError, OSError) as exc:  # Windows reports a closed pipe as EINVAL
        raise OutputClosed from exc


def _dump(obj: Any) -> None:
    if isinstance(obj, BaseModel):
        obj = obj.model_dump(mode="json", by_alias=True)
    _out(json.dumps(obj, indent=2))


def _fmt(v: Any, unit: str = "", nd: int = 1) -> str:
    if v is None:
        return "-"
    if isinstance(v, float):
        return f"{v:.{nd}f}{unit}"
    return f"{v}{unit}"


def format_item(item: Incoming) -> str | None:
    """One line for the monitor: events, telemetry, device log lines (acks/results: None)."""
    ts = iso_utc(item.received_utc)[11:23]
    if item.device_line is not None:
        d = item.device_line
        return f"{ts}  log   {d.lvl or d.source:<5} {d.msg}"
    msg = item.message
    if isinstance(msg, m.Telemetry):
        return (
            f"{ts}  tel   {msg.step or '-'}/{msg.phase or '-'} {_fmt(msg.elapsed_s, 's', 0)} "
            f"slot {_fmt(msg.slot)} {msg.lock or '-'} p_in {_fmt(msg.p_in, ' bar', 2)} "
            f"flow {_fmt(msg.flow_ml_min, ' mL/min')} vol {_fmt(msg.volume_ml, ' mL')} "
            f"pump {_fmt(msg.pump_pct, '%', 0)}"
        )
    if isinstance(msg, m.ModeEvent):
        return f"{ts}  evt   mode {msg.from_} -> {msg.to} ({msg.cause})"
    if isinstance(msg, (m.FaultEvent, m.WarningEvent)):
        kind = "FAULT" if isinstance(msg, m.FaultEvent) else "warning"
        return f"{ts}  evt   {kind} {msg.code}: {msg.msg}" + (
            f" [{msg.phase}]" if msg.phase else ""
        )
    if isinstance(msg, m.StepStartEvent):
        return f"{ts}  evt   {msg.cmd} step {msg.step} started"
    if isinstance(msg, m.StepEndEvent):
        return (
            f"{ts}  evt   {msg.cmd} step {msg.step} "
            f"{'done' if msg.ok else 'FAILED'} ({_fmt(msg.duration_s, ' s')})"
        )
    if isinstance(msg, m.PhaseEvent):
        return f"{ts}  evt   phase {msg.from_} -> {msg.to}"
    if isinstance(msg, m.BootEvent):
        return f"{ts}  evt   BOOT boot_id {msg.boot_id} reset {msg.reset_cause} fw {msg.fw.git}"
    if isinstance(msg, m.LinkEvent):
        return f"{ts}  evt   link {msg.state} (silent {_fmt(msg.silent_s, ' s')})"
    if isinstance(msg, m.Event):
        return f"{ts}  evt   {msg.type} {dumps(msg.model_extra or {})}"
    return None


def print_hello(h: m.HelloData) -> None:
    caps = [k for k, v in {**h.caps.model_dump(), **(h.caps.model_extra or {})}.items() if v]
    _out(f"device    {h.device_id}  (protocol {h.proto})")
    _out(
        f"firmware  {h.fw.version} git {h.fw.git}{' +dirty' if h.fw.dirty else ''} "
        f"built {h.fw.built}"
    )
    _out(f"boot_id   {h.boot_id}  uptime {h.uptime_s:.1f} s")
    _out(
        f"mode      {h.mode}"
        + (f"  (recovery required: {h.recovery_cause})" if h.recovery_required else "")
    )
    if h.fault:
        _out(f"fault     {h.fault.code}: {h.fault.msg}")
    _out(f"installed {', '.join(caps) or 'none'}")


def print_status(s: m.StatusData) -> None:
    _out(
        f"mode      {s.mode}"
        + (f"  (recovery required: {s.recovery_cause})" if s.recovery_required else "")
    )
    if s.fault:
        _out(f"fault     {s.fault.code}: {s.fault.msg}")
    _out(f"manifold  slot {_fmt(s.slot)}  angle {_fmt(s.angle_deg, ' deg', 2)}  O-ring {s.lock}")
    if s.busy:
        b = s.busy
        _out(
            f"busy      {b.cmd} {b.id} step {b.step} phase {b.phase} "
            f"{_fmt(b.elapsed_s, ' s', 0)} / max {_fmt(b.max_s, ' s', 0)}"
        )
    for sensor in s.sensors:
        _out(
            f"sensor    {sensor.name} = {_fmt(sensor.value, '', 2)} {sensor.unit or ''} "
            f"({sensor.stat})"
        )
    for w in s.warnings:
        _out(f"warning   {w.code}: {w.msg}")
    _out(
        f"link      {'ok' if s.link_ok else 'LOST'}  tx_dropped {s.tx_dropped}  "
        f"boot_id {s.boot_id}  uptime {s.uptime_s:.1f} s"
    )
    if s.last:
        _out(f"last      {s.last.cmd} {s.last.id} ok={s.last.ok} end_reason={s.last.end_reason}")


def print_info(i: m.GetInfoData) -> None:
    _out(f"slots     {i.slot_count} (purge {i.purge_slot}, samples {len(i.sample_slots)})")
    _out(f"fill      {' '.join(str(s) for s in i.fill_order)}")
    lim = i.limits
    _out(
        f"volume    {lim.volume_ml_min:g}..{lim.volume_ml_max:g} mL, max duration "
        f"{lim.max_duration_s_max} s, purge >= {lim.purge_ml_min:g} mL "
        f"(default {lim.purge_ml_default:g})"
    )
    _out(f"shield    default {lim.shield_ms_default} ms; pressure cap {lim.pressure_cap_bar} bar")
    _out(f"placeholders  {', '.join(i.placeholders) or 'none (all calibrated)'}")


def print_motion(r: m.MotionResult) -> None:
    _out(f"slot {_fmt(r.slot)}  O-ring {r.lock}  angle {_fmt(r.angle_deg, ' deg', 2)}")


def print_record(r: m.SampleRecord) -> None:
    status = "COMPLETED" if r.completed else "NOT COMPLETED"
    _out(
        f"{status}  slot {r.slot}  end_reason {r.end_reason or '-'}"
        + (f"  sample_id {r.sample_id}" if r.sample_id else "")
    )
    if r.abort:
        _out(f"abort     step {r.abort.step}: {r.abort.reason} ({r.abort.fault or '-'})")
    _out(
        f"volume    {_fmt(r.volume_ml, ' mL')} in {_fmt(r.duration_s, ' s')}  "
        f"(cycle {_fmt(r.cycle_s, ' s')})"
    )
    _out(
        f"purge     {_fmt(r.purge_ml_target, ' mL')} asked, {_fmt(r.purge_ml_measured, ' mL')} "
        "measured"
    )
    if r.p_in:
        _out(
            f"P_in      max {_fmt(r.p_in.max, '', 2)} mean {_fmt(r.p_in.mean, '', 2)} "
            f"final {_fmt(r.p_in.final, '', 2)} bar"
        )
    if r.flow_ml_min:
        f = r.flow_ml_min
        _out(f"flow      mean {_fmt(f.mean)} min {_fmt(f.min)} final {_fmt(f.final)} mL/min")
    if r.phases:
        _out("phases    " + ", ".join(f"{p.name} {_fmt(p.duration_s, 's')}" for p in r.phases))
    _out(
        f"shield    {_fmt(r.shield_ms, ' ms')}  (water end -> shield "
        f"{_fmt(r.t_end_to_shield_s, ' s')})"
    )
    for f in r.faults:
        _out(f"fault     {f.code}: {f.msg}")
    for w in r.warnings:
        _out(f"warning   {w.code}: {w.msg}")
    missing = r.missing_fields()
    if missing:
        _out(f"(the Due did not send: {', '.join(missing)})")


# ============================================================================ plumbing


class Context:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        base: Path = args.data_dir or default_data_dir()
        # Simulated runs never mix with the real Due's logs and results.
        self.data_dir = base / "sim" if args.sim else base
        self.device_log: DeviceLog | None = None
        self.sim: Any = None

    def client(self) -> CowasClient:
        args = self.args
        self.device_log = DeviceLog(self.data_dir / "logs")
        if args.sim:
            from cowas.sim import FakeController, SimTransport

            self.sim = FakeController(boot=args.sim_boot, bench_mode=args.bench)
            transport: Any = SimTransport(self.sim, realtime_speed=args.sim_speed)
        else:
            transport = SerialTransport(args.port, allow_programming_port=args.bench)
        client = CowasClient(transport, device_log=self.device_log)
        client.connect()
        return client

    def close(self) -> None:
        if self.device_log is not None:
            self.device_log.close()

    def save_result(self, p: PendingRequest, res: m.Response, client: CowasClient) -> Path:
        """Keep every procedure result on disk (HO2 will store records properly)."""
        out_dir = self.data_dir / "results"
        out_dir.mkdir(parents=True, exist_ok=True)
        stamp = iso_utc(client.clock.utcnow()).replace(":", "").replace("-", "")[:15]
        path = out_dir / f"{stamp}Z_{p.cmd}_{p.id}.json"
        payload = {
            "saved_utc": iso_utc(client.clock.utcnow()),
            "host": __version__,
            "sim": bool(self.args.sim),
            "boot_id": client.boot_id,
            "request": {"id": p.id, "cmd": p.cmd, "args": p.args},
            "response": res.model_dump(mode="json", by_alias=True),
        }
        path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        return path


def _wait_with_stop(client: CowasClient, p: PendingRequest) -> m.Response:
    """Wait for a long command; Ctrl-C sends STOP, then fetches the (partial) result by
    resending the same id (§7: the stored result, never a second run)."""
    try:
        return client.wait(p)
    except KeyboardInterrupt:
        _out("\nCtrl-C: sending STOP ...")
        stop = client.stop()
        _out(f"STOP accepted (interrupted: {stop.interrupted})")
        again = client.send(p.cmd, p.args, request_id=p.id, validate=False)
        return client.wait(again)


def _progress_printer(verbose: bool) -> Callable[[Incoming], None]:
    last_tel = [-1e9]

    def show(item: Incoming) -> None:
        if isinstance(item.message, m.Telemetry):
            t = item.message.elapsed_s or 0.0
            if not verbose and t - last_tel[0] < 30:
                return
            last_tel[0] = t
        elif item.device_line is not None and not verbose:
            return
        line = format_item(item)
        if line:
            _out(line)

    return show


# ============================================================================ commands


def cmd_ports(ctx: Context) -> int:
    if ctx.args.sim:
        _out("sim       (simulated Due)")
        return EXIT_OK
    ports = list_ports()
    if not ports:
        _out("no serial ports")
    for p in ports:
        tag = {
            "native": "Due Native USB (host port)",
            "programming": "Due Programming port (console; resets the Due when opened)",
        }.get(p.kind, "")
        _out(f"{p.device:<14} {p.usb_id:<10} {p.description[:40]:<40} {tag}")
    try:
        chosen = select_port(ports, allow_programming_port=ctx.args.bench)
        _out(f"\nauto-detected: {chosen.device}")
    except TransportError as exc:
        _out(f"\n{exc}")
    return EXIT_OK


def cmd_hello(ctx: Context, client: CowasClient) -> int:
    h = client.hello()
    _dump(h) if ctx.args.json else print_hello(h)
    return EXIT_OK


def cmd_status(ctx: Context, client: CowasClient) -> int:
    s = client.status()
    _dump(s) if ctx.args.json else print_status(s)
    return EXIT_OK


def cmd_info(ctx: Context, client: CowasClient) -> int:
    i = client.info or client.get_info()
    _dump(i) if ctx.args.json else print_info(i)
    return EXIT_OK


def cmd_stop(ctx: Context, client: CowasClient) -> int:
    s = client.stop()
    if ctx.args.json:
        _dump(s)
    else:
        _out(f"STOP accepted; interrupted: {s.interrupted or 'nothing'}")
        _out("Next: `cowas clear-fault`, then `cowas recover`.")
    return EXIT_OK


def cmd_clear_fault(ctx: Context, client: CowasClient) -> int:
    code = client.clear_fault()
    _out(
        json.dumps({"cleared": code})
        if ctx.args.json
        else f"cleared {code}; now RECOVERY_REQUIRED: run `cowas recover`"
    )
    return EXIT_OK


def cmd_recover(ctx: Context, client: CowasClient) -> int:
    try:
        data = client.recover(confirm_moving=False)
    except CommandFailed as exc:
        if exc.code != "confirm_required":
            raise
        needs = exc.response.err.needs if exc.response.err else None
        _out(f"RECOVER needs to MOVE: {', '.join(needs or [])}")
        if not ctx.args.yes:
            if not sys.stdin.isatty():
                _out("not confirmed (use --yes)")
                return EXIT_FAILED
            answer = input("Is the machine clear to move? [y/N] ").strip().lower()
            if answer not in ("y", "yes"):
                _out("not confirmed: nothing moved")
                return EXIT_FAILED
        p = client.send(Cmd.RECOVER, {"confirm_moving": True})
        data = CowasClient._data(Cmd.RECOVER, _wait_with_stop(client, p))
    if ctx.args.json:
        _dump(data)
    else:
        for step in data.steps:
            _out(
                f"{step.name:<11} {'ok' if step.ok else 'FAILED'}"
                + (f" ({step.action})" if step.action else "")
            )
        _out("IDLE")
    return EXIT_OK


def _motion(ctx: Context, client: CowasClient, cmd: str, args: dict[str, Any]) -> int:
    unsubscribe = client.subscribe(_progress_printer(ctx.args.verbose > 0))
    try:
        p = client.send(cmd, args)
        res = _wait_with_stop(client, p)
    finally:
        unsubscribe()
    data = CowasClient._data(cmd, res)
    _dump(data) if ctx.args.json else print_motion(data)
    return EXIT_OK


def cmd_rotate(ctx: Context, client: CowasClient) -> int:
    return _motion(ctx, client, Cmd.ROTATE, {"slot": ctx.args.slot})


def cmd_lock(ctx: Context, client: CowasClient) -> int:
    return _motion(ctx, client, Cmd.LOCK, {})


def cmd_unlock(ctx: Context, client: CowasClient) -> int:
    return _motion(ctx, client, Cmd.UNLOCK, {})


def cmd_run_cycle(ctx: Context, client: CowasClient) -> int:
    a = ctx.args
    stages = None
    if a.skip:
        skip = {s.strip() for s in a.skip.split(",") if s.strip()}
        unknown = skip - set(STAGES)
        if unknown:
            _out(f"unknown stage(s): {', '.join(sorted(unknown))} (stages: {', '.join(STAGES)})")
            return EXIT_USAGE
        stages = {s: s not in skip for s in STAGES}
    volume = None if a.no_volume_cap else a.volume
    args = CowasClient.run_cycle_args(
        a.slot,
        volume_ml=volume,
        max_duration_s=a.max_duration,
        purge_ml=a.purge_ml,
        stages=stages,
        sample_id=a.sample_id,
    )
    _out(
        f"RUN_CYCLE slot {a.slot}, volume "
        f"{'no cap' if volume is None else f'{volume:g} mL'}, max water phase "
        f"{a.max_duration} s" + (f", stages {stages}" if stages else "")
    )
    unsubscribe = client.subscribe(_progress_printer(a.verbose > 0))
    try:
        p = client.send(Cmd.RUN_CYCLE, args)
        res = _wait_with_stop(client, p)
    finally:
        unsubscribe()
    if not res.ok:
        raise CommandFailed(Cmd.RUN_CYCLE, res)
    path = ctx.save_result(p, res, client)
    record = parse_result(Cmd.RUN_CYCLE, res.data or {})
    assert isinstance(record, m.SampleRecord)
    if a.json:
        _dump(record)
    else:
        print_record(record)
        _out(f"saved     {path}")
    return EXIT_OK if record.completed else EXIT_FAILED


def cmd_monitor(ctx: Context, client: CowasClient) -> int:
    show = _progress_printer(verbose=True)
    client.subscribe(show)
    if ctx.sim is not None:
        rid = ctx.sim.start_demo()
        _out(f"(sim: a demo RUN_CYCLE {rid} was started on the simulated Due)")
    _out("monitoring (Ctrl-C to quit) ...")
    deadline = ctx.args.duration
    t0 = client.clock.monotonic()
    try:
        while deadline is None or client.clock.monotonic() - t0 < deadline:
            client.poll(1.0)
    except KeyboardInterrupt:
        _out("")
    return EXIT_OK


def cmd_logs(ctx: Context) -> int:
    path = ctx.data_dir / "logs" / DEVICE_LOG_NAME
    if not path.exists():
        _out(f"no device log yet ({path})")
        return EXIT_OK
    with open(path, encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
        for line in lines[-ctx.args.lines :]:
            print(line, end="")
        if not ctx.args.follow:
            return EXIT_OK
        try:
            while True:
                line = fh.readline()
                if line:
                    print(line, end="", flush=True)
                else:
                    time.sleep(0.5)
        except KeyboardInterrupt:
            return EXIT_OK


# ============================================================================ parser


def _common(parser: argparse.ArgumentParser, *, suppress: bool) -> None:
    def d(value: Any) -> Any:
        return argparse.SUPPRESS if suppress else value

    g = parser.add_argument_group("common options")
    g.add_argument(
        "--port",
        default=d(None),
        help="serial port (default: auto-detect the Due's Native USB port)",
    )
    g.add_argument(
        "--bench",
        action="store_true",
        default=d(False),
        help="single-cable bench mode: allow the Programming port (resets the Due)",
    )
    g.add_argument(
        "--sim",
        action="store_true",
        default=d(False),
        help="talk to a simulated Due (virtual time)",
    )
    g.add_argument(
        "--sim-boot",
        action="store_true",
        default=d(False),
        help="the simulated Due starts as after a power-on (RECOVERY_REQUIRED)",
    )
    g.add_argument(
        "--sim-speed",
        type=float,
        default=d(None),
        help="simulated time speed-up vs real time (default: as fast as possible; monitor: 30)",
    )
    g.add_argument(
        "--data-dir",
        type=Path,
        default=d(None),
        help="where logs and results go (default: $COWAS_DATA_DIR or ./data; "
        "--sim uses <data>/sim)",
    )
    g.add_argument("--json", action="store_true", default=d(False), help="JSON output")
    g.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=d(0),
        help="more output (-v info, -vv debug, -vvv protocol lines)",
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="cowas", description="CoWaS host: talk to the controller (protocol v1)."
    )
    parser.add_argument("--version", action="version", version=f"cowas {__version__}")
    _common(parser, suppress=False)
    sub = parser.add_subparsers(dest="command", required=True, metavar="COMMAND")

    def add(name: str, help_text: str) -> argparse.ArgumentParser:
        p = sub.add_parser(name, help=help_text, description=help_text)
        _common(p, suppress=True)
        return p

    add("ports", "list serial ports and the auto-detected Due")
    add("hello", "identity, versions, mode, capabilities (HELLO)")
    add("status", "the live state (STATUS)")
    add("info", "geometry, fill order, limits, placeholders (GET_INFO)")
    add("stop", "STOP: safe state, FAULT(USER_STOP)")
    add("clear-fault", "CLEAR_FAULT: FAULT -> RECOVERY_REQUIRED")
    p = add("recover", "RECOVER: the recovery checklist -> IDLE")
    p.add_argument("--yes", action="store_true", help="confirm moving steps without asking")
    p = add("rotate", "ROTATE to a slot (unlock -> rotate -> lock)")
    p.add_argument("slot", type=int)
    add("lock", "LOCK the O-ring at the current slot")
    add("unlock", "UNLOCK the O-ring (service; leaves it unlocked)")
    p = add("run-cycle", "RUN_CYCLE: purge -> sample -> shield -> empty on one filter")
    p.add_argument("--slot", type=int, required=True, help="a sample slot (see `cowas info`)")
    vol = p.add_mutually_exclusive_group(required=True)
    vol.add_argument("--volume", type=float, help="target filtered volume, mL")
    vol.add_argument(
        "--no-volume-cap", action="store_true", help="filter until clogged or --max-duration"
    )
    p.add_argument(
        "--max-duration",
        type=int,
        default=1800,
        help="limit of the water pumping phase, s (default 1800)",
    )
    p.add_argument(
        "--purge-ml",
        type=float,
        default=None,
        help="purge volume, mL (default: the firmware's; raised to its minimum)",
    )
    p.add_argument(
        "--skip", default=None, help=f"comma-separated stages to skip ({', '.join(STAGES)})"
    )
    p.add_argument("--sample-id", default=None, help="echoed in the result (<= 40 chars)")
    p = add("monitor", "live telemetry, events and device logs")
    p.add_argument("--duration", type=float, default=None, help="stop after S seconds")
    p = add("logs", "print the device log")
    p.add_argument("-n", "--lines", type=int, default=50)
    p.add_argument("--follow", "-f", action="store_true")
    return parser


COMMANDS: dict[str, Callable[[Context, CowasClient], int]] = {
    "hello": cmd_hello,
    "status": cmd_status,
    "info": cmd_info,
    "stop": cmd_stop,
    "clear-fault": cmd_clear_fault,
    "recover": cmd_recover,
    "rotate": cmd_rotate,
    "lock": cmd_lock,
    "unlock": cmd_unlock,
    "run-cycle": cmd_run_cycle,
    "monitor": cmd_monitor,
}


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.command == "monitor" and args.sim and args.sim_speed is None:
        args.sim_speed = 30.0
    ctx = Context(args)
    configure_logging(ctx.data_dir / "logs", args.verbose)
    try:
        if args.command == "ports":
            return cmd_ports(ctx)
        if args.command == "logs":
            return cmd_logs(ctx)
        client = ctx.client()
        try:
            return COMMANDS[args.command](ctx, client)
        finally:
            client.close()
    except InvalidArguments as exc:
        _out(f"refused by the host (nothing sent): {exc}")
        return EXIT_USAGE
    except CommandFailed as exc:
        err = exc.response.err
        _out(f"{exc}")
        if err is not None:
            extra = {
                k: v
                for k, v in err.model_dump(exclude_none=True).items()
                if k not in ("code", "msg")
            }
            if extra:
                _out(f"  {json.dumps(extra)}")
        return EXIT_FAILED
    except DeviceRebooted as exc:
        _out(f"THE DUE REBOOTED: {exc}")
        _out("The interrupted slot is SUSPECT. Check the machine, then `cowas recover`.")
        return EXIT_REBOOTED
    except ProtocolMismatch as exc:
        _out(f"protocol mismatch: {exc}")
        return EXIT_LINK
    except (TransportError, LinkLost, CommandTimeout) as exc:
        _out(f"link problem: {exc}")
        return EXIT_LINK
    except CowasError as exc:
        _out(f"error: {exc}")
        return EXIT_FAILED
    except KeyboardInterrupt:
        return EXIT_INTERRUPTED
    except OutputClosed:
        return EXIT_OK
    finally:
        ctx.close()


if __name__ == "__main__":
    raise SystemExit(main())
