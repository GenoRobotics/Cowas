"""Splitting what the Due sends: protocol messages vs device log lines (§2, §3.6).

Rule (ROADMAP HO1): a line starting with '{' is protocol, unless it is a {"t":"log"} copy;
every other line is console text. Both kinds of log line go to the device log, never to the
protocol dispatcher. A '{' line that does not parse is logged as "garbage" (and counted),
never acted upon.
"""

from __future__ import annotations

import logging
from dataclasses import dataclass
from datetime import datetime

from cowas.protocol.codec import ProtocolParseError, parse_message
from cowas.protocol.models import LogMessage, Message
from cowas.transport.base import Transport
from cowas.transport.devicelog import DeviceLog, DeviceLogEntry

wire = logging.getLogger("cowas.wire")
log = logging.getLogger(__name__)


@dataclass(frozen=True)
class Incoming:
    received_utc: datetime
    message: Message | None = None  # a protocol message (never a LogMessage)
    device_line: DeviceLogEntry | None = None  # a log copy or console text


def classify(line: str, received_utc: datetime) -> Incoming | None:
    """Route one received line (without its line ending). None = blank line."""
    if not line.strip():
        return None
    if not line.lstrip().startswith("{"):
        return Incoming(received_utc, device_line=DeviceLogEntry(received_utc, "text", line))
    try:
        msg = parse_message(line)
    except ProtocolParseError as exc:
        log.warning("unparseable line from the Due (%s): %.120s", exc, line)
        return Incoming(received_utc, device_line=DeviceLogEntry(received_utc, "garbage", line))
    if isinstance(msg, LogMessage):
        entry = DeviceLogEntry(received_utc, "log", msg.msg, lvl=msg.lvl, ms=msg.ms)
        return Incoming(received_utc, device_line=entry)
    return Incoming(received_utc, message=msg)


class LineRouter:
    """Reads a transport, writes device lines to the device log, returns everything routed."""

    def __init__(self, transport: Transport, device_log: DeviceLog | None = None) -> None:
        self.transport = transport
        self.device_log = device_log or DeviceLog(None)
        self.garbage_lines = 0

    def read(self, timeout: float) -> Incoming | None:
        line = self.transport.read_line(timeout)
        if line is None:
            return None
        wire.debug("<< %s", line)
        item = classify(line, self.transport.clock.utcnow())
        if item is None:
            return None
        if item.device_line is not None:
            if item.device_line.source == "garbage":
                self.garbage_lines += 1
            self.device_log.write(item.device_line)
        return item

    def write(self, line: str) -> None:
        wire.debug(">> %s", line)
        self.transport.write_line(line)
