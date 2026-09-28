"""The serial transport (pyserial): the Due's Native USB port, found by USB VID/PID (§2).

Safety rules enforced here:
- **never 1200 baud**: opening the Due's port at 1200 baud erases its flash (bootloader
  "touch"). `check_baud` refuses it, and the baud rate cannot be changed after construction.
- the Programming port (2341:003D) resets the Due when opened. It is refused unless the
  single-cable bench mode is asked for explicitly (`allow_programming_port=True`).
"""

from __future__ import annotations

import contextlib
import logging
from collections.abc import Callable, Iterable
from dataclasses import dataclass
from typing import Any

import serial
from serial.tools import list_ports as _list_ports

from cowas.clock import Clock, SystemClock
from cowas.errors import PortNotFound, ProgrammingPortRefused, TransportError
from cowas.protocol.spec import (
    FORBIDDEN_BAUD,
    HOST_BAUD,
    MAX_LINE,
    USB_PID_DUE_NATIVE,
    USB_PID_DUE_PROGRAMMING,
    USB_VID_ARDUINO,
)

log = logging.getLogger(__name__)

NATIVE_IDS: frozenset[tuple[int, int]] = frozenset({(USB_VID_ARDUINO, USB_PID_DUE_NATIVE)})
PROGRAMMING_IDS: frozenset[tuple[int, int]] = frozenset(
    {(USB_VID_ARDUINO, USB_PID_DUE_PROGRAMMING)}
)
MAX_BUFFER = 64 * 1024  # bytes without a newline before the buffer is flushed as garbage


@dataclass(frozen=True)
class PortInfo:
    device: str
    vid: int | None
    pid: int | None
    serial_number: str | None
    description: str
    kind: str  # "native" | "programming" | "other"

    @property
    def usb_id(self) -> str:
        if self.vid is None or self.pid is None:
            return "-"
        return f"{self.vid:04X}:{self.pid:04X}"


def check_baud(baudrate: int) -> int:
    if baudrate == FORBIDDEN_BAUD:
        raise ValueError(
            "refusing 1200 baud: on an Arduino Due that erases the flash (bootloader touch)"
        )
    return baudrate


def _kind(vid: int | None, pid: int | None) -> str:
    if (vid, pid) in NATIVE_IDS:
        return "native"
    if (vid, pid) in PROGRAMMING_IDS:
        return "programming"
    return "other"


def list_ports(raw_ports: Iterable[Any] | None = None) -> list[PortInfo]:
    """Every serial port, with its kind. `raw_ports` (pyserial ListPortInfo-like) for tests."""
    ports = _list_ports.comports() if raw_ports is None else raw_ports
    out = [
        PortInfo(
            device=p.device,
            vid=p.vid,
            pid=p.pid,
            serial_number=getattr(p, "serial_number", None),
            description=getattr(p, "description", "") or "",
            kind=_kind(p.vid, p.pid),
        )
        for p in ports
    ]
    return sorted(out, key=lambda p: p.device)


def select_port(
    ports: list[PortInfo],
    port: str | None = None,
    *,
    serial_number: str | None = None,
    allow_programming_port: bool = False,
) -> PortInfo:
    """Choose the port to open. Pure, so the rules are unit-tested without hardware."""
    if port is not None:
        for p in ports:
            if p.device.lower() == port.lower():
                if p.kind == "programming" and not allow_programming_port:
                    raise ProgrammingPortRefused(
                        f"{port} is the Due's Programming port: opening it resets the Due. "
                        "Use the Native USB port, or --bench for the single-cable bench mode."
                    )
                return p
        # Not enumerated (a symlink such as /dev/serial/by-id/..., a pty): trust the user.
        return PortInfo(port, None, None, None, "not enumerated", "other")

    def wanted(p: PortInfo, kind: str) -> bool:
        return p.kind == kind and (serial_number is None or p.serial_number == serial_number)

    natives = [p for p in ports if wanted(p, "native")]
    if len(natives) == 1:
        return natives[0]
    if len(natives) > 1:
        names = ", ".join(p.device for p in natives)
        raise PortNotFound(f"several Due Native USB ports ({names}): choose one with --port")
    programming = [p for p in ports if wanted(p, "programming")]
    if programming and allow_programming_port:
        return programming[0]
    if programming:
        raise PortNotFound(
            f"no Native USB port found, only the Programming port ({programming[0].device}). "
            "Plug the Due's Native USB port too, or use --bench for the single-cable bench "
            "mode."
        )
    raise PortNotFound("no Arduino Due found (USB 2341:003E)")


class SerialTransport:
    """JSON lines over pyserial. Not thread-safe: one reader, the client."""

    def __init__(
        self,
        port: str | None = None,
        *,
        allow_programming_port: bool = False,
        serial_number: str | None = None,
        baudrate: int = HOST_BAUD,
        clock: Clock | None = None,
        serial_factory: Callable[..., Any] | None = None,
        lister: Callable[[], list[PortInfo]] | None = None,
        chunk_timeout_s: float = 0.05,
    ) -> None:
        self._baudrate = check_baud(baudrate)
        self._port = port
        self._allow_programming = allow_programming_port
        self._serial_number = serial_number
        self.clock: Clock = clock or SystemClock()
        self._factory = serial_factory or serial.Serial
        self._lister = lister or list_ports
        self._chunk_timeout = chunk_timeout_s
        self._ser: Any = None
        self._buf = bytearray()
        self.port_info: PortInfo | None = None
        self.name = port or "auto"

    @property
    def baudrate(self) -> int:
        return self._baudrate

    @property
    def is_open(self) -> bool:
        return self._ser is not None

    def open(self) -> None:
        if self._ser is not None:
            return
        info = select_port(
            self._lister(),
            self._port,
            serial_number=self._serial_number,
            allow_programming_port=self._allow_programming,
        )
        if info.kind == "programming":
            log.warning(
                "bench mode: using the Programming port %s (opening it resets the Due)", info.device
            )
        try:
            # DTR stays asserted (pyserial default), as §2 asks.
            self._ser = self._factory(
                port=info.device,
                baudrate=self._baudrate,
                timeout=self._chunk_timeout,
                write_timeout=2.0,
            )
        except (serial.SerialException, OSError, ValueError) as exc:
            raise TransportError(f"cannot open {info.device}: {exc}") from exc
        self._buf.clear()
        self.port_info = info
        self.name = info.device
        log.info("opened %s (%s)", info.device, info.usb_id)

    def close(self) -> None:
        ser, self._ser = self._ser, None
        if ser is not None:
            with contextlib.suppress(serial.SerialException, OSError):
                ser.close()

    def _require(self) -> Any:
        if self._ser is None:
            raise TransportError("port not open")
        return self._ser

    def write_line(self, line: str) -> None:
        data = (line + "\n").encode("utf-8")
        if len(data) > MAX_LINE:
            raise ValueError(f"request longer than {MAX_LINE} bytes")
        ser = self._require()
        try:
            ser.write(data)
            ser.flush()
        except (serial.SerialException, OSError) as exc:
            self.close()
            raise TransportError(f"write failed: {exc}") from exc

    def _pop_line(self) -> str | None:
        idx = self._buf.find(b"\n")
        if idx < 0:
            if len(self._buf) > MAX_BUFFER:
                junk = bytes(self._buf)
                self._buf.clear()
                return junk.decode("utf-8", errors="replace")
            return None
        raw = bytes(self._buf[:idx])
        del self._buf[: idx + 1]
        if raw.endswith(b"\r"):
            raw = raw[:-1]
        return raw.decode("utf-8", errors="replace")

    def read_line(self, timeout: float) -> str | None:
        ser = self._require()
        deadline = self.clock.monotonic() + max(0.0, timeout)
        while True:
            line = self._pop_line()
            if line is not None:
                return line
            try:
                chunk = ser.read(max(1, ser.in_waiting))
            except (serial.SerialException, OSError) as exc:
                self.close()
                raise TransportError(f"read failed: {exc}") from exc
            if chunk:
                self._buf += chunk
                continue
            if self.clock.monotonic() >= deadline:
                return None

    def reconnect(self, timeout: float) -> None:
        """Close and reopen (re-detecting the port: its name may change after a re-enumeration).
        Opening the Native port does not reset the Due."""
        self.close()
        deadline = self.clock.monotonic() + timeout
        while True:
            try:
                self.open()
                return
            except TransportError as exc:
                remaining = deadline - self.clock.monotonic()
                if remaining <= 0:
                    raise TransportError(f"reconnect failed: {exc}") from exc
                log.info("reconnect: %s; retrying", exc)
                self.clock.sleep(min(1.0, remaining))
