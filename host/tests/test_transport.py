"""cowas.transport: port selection (VID/PID, never 1200 baud), line framing, routing, device log."""

from __future__ import annotations

from dataclasses import dataclass
from datetime import UTC, datetime
from typing import ClassVar

import pytest
import serial

from cowas.clock import ManualClock
from cowas.errors import PortNotFound, ProgrammingPortRefused, TransportError
from cowas.protocol import models as m
from cowas.transport.devicelog import DeviceLog, DeviceLogEntry
from cowas.transport.routing import classify
from cowas.transport.serial_port import (
    SerialTransport,
    check_baud,
    list_ports,
    select_port,
)


@dataclass
class RawPort:  # what pyserial's comports() returns, reduced
    device: str
    vid: int | None
    pid: int | None
    serial_number: str | None = None
    description: str = ""


NATIVE = RawPort("COM7", 0x2341, 0x003E, "A1", "Arduino Due")
PROGRAMMING = RawPort("COM5", 0x2341, 0x003D, "B2", "Arduino Due Prog. Port")
OTHER = RawPort("COM1", None, None, None, "Communications Port")


def ports(*raw: RawPort):
    return list_ports(raw)


# ---------------------------------------------------------------- port selection


def test_the_native_port_is_found_by_vid_pid():
    assert select_port(ports(OTHER, PROGRAMMING, NATIVE)).device == "COM7"


def test_the_programming_port_is_never_picked_automatically():
    with pytest.raises(PortNotFound, match="Programming port"):
        select_port(ports(OTHER, PROGRAMMING))


def test_the_programming_port_is_refused_even_by_name():
    with pytest.raises(ProgrammingPortRefused):
        select_port(ports(PROGRAMMING, NATIVE), "com5")


def test_bench_mode_allows_the_programming_port():
    assert select_port(ports(PROGRAMMING), allow_programming_port=True).device == "COM5"
    assert select_port(ports(PROGRAMMING, NATIVE), allow_programming_port=True).device == "COM7"


def test_several_dues_need_a_choice():
    second = RawPort("COM9", 0x2341, 0x003E, "C3")
    with pytest.raises(PortNotFound, match="several"):
        select_port(ports(NATIVE, second))
    assert select_port(ports(NATIVE, second), serial_number="C3").device == "COM9"


def test_an_unlisted_port_name_is_trusted():
    assert select_port(ports(), "/dev/serial/by-id/usb-Arduino_Due").kind == "other"


def test_no_due():
    with pytest.raises(PortNotFound):
        select_port(ports(OTHER))


# ---------------------------------------------------------------- never 1200 baud


def test_1200_baud_is_refused():
    with pytest.raises(ValueError, match="1200"):
        check_baud(1200)
    with pytest.raises(ValueError):
        SerialTransport("COM7", baudrate=1200)


class FakeSerial:
    """A pyserial stand-in: `chunks` are returned by successive read() calls."""

    opened: ClassVar[list[dict]] = []

    def __init__(self, **kwargs) -> None:
        FakeSerial.opened.append(kwargs)
        self.chunks: list[bytes] = []
        self.written = bytearray()
        self.fail = False

    @property
    def in_waiting(self) -> int:
        return len(self.chunks[0]) if self.chunks else 0

    def read(self, n: int) -> bytes:
        if self.fail:
            raise serial.SerialException("device disconnected")
        return self.chunks.pop(0) if self.chunks else b""

    def write(self, data: bytes) -> int:
        if self.fail:
            raise serial.SerialException("device disconnected")
        self.written += data
        return len(data)

    def flush(self) -> None:
        pass

    def close(self) -> None:
        pass


def make_transport(port_list=(NATIVE,)):
    FakeSerial.opened = []
    clock = ManualClock()
    t = SerialTransport(clock=clock, serial_factory=FakeSerial, lister=lambda: ports(*port_list))
    return t, clock


def test_opens_the_native_port_at_a_safe_baud_rate():
    t, _ = make_transport((PROGRAMMING, NATIVE))
    t.open()
    assert FakeSerial.opened == [
        {"port": "COM7", "baudrate": 115200, "timeout": 0.05, "write_timeout": 2.0}
    ]
    assert all(o["baudrate"] != 1200 for o in FakeSerial.opened)


def test_lines_are_split_and_cr_stripped():
    t, _ = make_transport()
    t.open()
    ser = t._ser
    ser.chunks = [b'{"t":"log"', b',"ms":1}\r\nsystem init', b"ialized\n\n"]
    assert t.read_line(1.0) == '{"t":"log","ms":1}'
    assert t.read_line(1.0) == "system initialized"
    assert t.read_line(1.0) == ""


def test_read_times_out_on_the_clock():
    t, clock = make_transport()
    t.open()
    real_read = t._ser.read

    def slow_read(n):
        clock.sleep(0.05)
        return real_read(n)

    t._ser.read = slow_read
    assert t.read_line(0.2) is None
    assert clock.monotonic() == pytest.approx(0.2, abs=0.06)


def test_write_adds_the_newline_and_refuses_long_lines():
    t, _ = make_transport()
    t.open()
    t.write_line('{"t":"req"}')
    assert bytes(t._ser.written) == b'{"t":"req"}\n'
    with pytest.raises(ValueError):
        t.write_line("x" * 2048)


def test_a_vanished_port_raises_and_reconnect_finds_it_again():
    port_list = [NATIVE]
    FakeSerial.opened = []
    clock = ManualClock()
    t = SerialTransport(clock=clock, serial_factory=FakeSerial, lister=lambda: ports(*port_list))
    t.open()
    t._ser.fail = True
    with pytest.raises(TransportError):
        t.read_line(0.1)
    assert not t.is_open
    port_list.clear()  # re-enumerating
    with pytest.raises(TransportError):
        t.reconnect(3.0)
    port_list.append(RawPort("COM8", 0x2341, 0x003E, "A1"))  # back, under another name
    t.reconnect(3.0)
    assert t.is_open and t.name == "COM8"
    assert all(o["baudrate"] == 115200 for o in FakeSerial.opened)


# ---------------------------------------------------------------- routing and device log

NOW = datetime(2026, 10, 15, 8, 0, tzinfo=UTC)


def test_routing():
    assert classify("   ", NOW) is None
    text = classify("Button B_start initiated", NOW)
    assert text.message is None and text.device_line.source == "text"
    log = classify('{"t":"log","ms":5,"lvl":"warn","msg":"low flow"}', NOW)
    assert log.message is None and log.device_line.lvl == "warn" and log.device_line.ms == 5
    evt = classify('{"t":"evt","ms":5,"type":"link","state":"lost","silent_s":30.0}', NOW)
    assert isinstance(evt.message, m.LinkEvent) and evt.device_line is None
    bad = classify('{"t":"evt","ms":"5"', NOW)
    assert bad.message is None and bad.device_line.source == "garbage"
    indented = classify('  {"t":"ack","id":"a","cmd":"LOCK","max_s":20}', NOW)
    assert isinstance(indented.message, m.Ack)


def test_device_log_file_is_utc_and_rotates(tmp_path):
    log = DeviceLog(tmp_path, max_bytes=300, backup_count=2)
    for i in range(20):
        log.write(DeviceLogEntry(NOW, "log", f"line {i}\nwith newline", lvl="info", ms=i))
    log.close()
    files = sorted(p.name for p in tmp_path.iterdir())
    assert files == ["device.log", "device.log.1", "device.log.2"]
    last = (tmp_path / "device.log").read_text(encoding="utf-8").splitlines()[-1]
    assert last == "2026-10-15T08:00:00.000Z\t19\tinfo\tline 19\\nwith newline"
    assert len(log.recent) == 20
