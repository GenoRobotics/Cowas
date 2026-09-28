"""The device log: every human line from the Due, timestamped in UTC on reception (§3.6).

Two sources end up here: `{"t":"log"}` copies (Native USB), and non-JSON lines (the
single-cable bench mode, where console text and protocol share the Programming port).
Files rotate by size under <data>/logs/device.log[.1 .. .N].
"""

from __future__ import annotations

import logging
import logging.handlers
from collections import deque
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

from cowas.clock import iso_utc

DEVICE_LOG_NAME = "device.log"


@dataclass(frozen=True)
class DeviceLogEntry:
    received_utc: datetime
    source: str  # "log" ({"t":"log"} copy), "text" (a non-JSON line), "garbage" (bad JSON)
    msg: str
    lvl: str | None = None
    ms: int | None = None

    def format(self) -> str:
        ms = "-" if self.ms is None else str(self.ms)
        lvl = self.lvl or self.source
        text = self.msg.replace("\r", "\\r").replace("\n", "\\n")
        return f"{iso_utc(self.received_utc)}\t{ms}\t{lvl}\t{text}"


class DeviceLog:
    """Rotating device-log files, plus the last lines in memory (for the monitor and tests).

    `directory=None` keeps the lines in memory only.
    """

    def __init__(
        self,
        directory: Path | None,
        *,
        max_bytes: int = 5_000_000,
        backup_count: int = 20,
        keep_in_memory: int = 1000,
    ) -> None:
        self.recent: deque[DeviceLogEntry] = deque(maxlen=keep_in_memory)
        self.path: Path | None = None
        self._handler: logging.handlers.RotatingFileHandler | None = None
        if directory is not None:
            directory.mkdir(parents=True, exist_ok=True)
            self.path = directory / DEVICE_LOG_NAME
            self._handler = logging.handlers.RotatingFileHandler(
                self.path, maxBytes=max_bytes, backupCount=backup_count, encoding="utf-8"
            )
            self._handler.setFormatter(logging.Formatter("%(message)s"))

    def write(self, entry: DeviceLogEntry) -> None:
        self.recent.append(entry)
        if self._handler is not None:
            self._handler.handle(logging.makeLogRecord({"msg": entry.format(), "args": None}))

    def close(self) -> None:
        if self._handler is not None:
            self._handler.close()
            self._handler = None
