"""Host logging: console (stderr) + rotating files under <data>/logs/, timestamps in UTC.

Files:
- host.log: what this program does (INFO, DEBUG with -vv);
- wire.log: every protocol line sent (>>) and received (<<), for field forensics;
- device.log: the Due's own log lines (written by cowas.transport.devicelog, not here).
"""

from __future__ import annotations

import logging
import logging.handlers
import os
import sys
from datetime import UTC, datetime
from pathlib import Path

from cowas.clock import iso_utc

HOST_LOG = "host.log"
WIRE_LOG = "wire.log"


def default_data_dir() -> Path:
    return Path(os.environ.get("COWAS_DATA_DIR", "data"))


class UtcFormatter(logging.Formatter):
    def formatTime(self, record: logging.LogRecord, datefmt: str | None = None) -> str:
        return iso_utc(datetime.fromtimestamp(record.created, UTC))


def _rotating(path: Path, level: int, fmt: str) -> logging.Handler:
    handler = logging.handlers.RotatingFileHandler(
        path, maxBytes=5_000_000, backupCount=10, encoding="utf-8"
    )
    handler.setLevel(level)
    handler.setFormatter(UtcFormatter(fmt))
    return handler


def configure_logging(log_dir: Path | None, verbosity: int = 0) -> None:
    """verbosity 0: warnings on the console; 1: info; 2: debug; 3: debug + wire lines."""
    base = logging.getLogger("cowas")
    wire = logging.getLogger("cowas.wire")
    for logger in (base, wire):
        for h in list(logger.handlers):
            logger.removeHandler(h)
            h.close()
    base.setLevel(logging.DEBUG)
    base.propagate = False
    wire.setLevel(logging.DEBUG)
    wire.propagate = False

    console = logging.StreamHandler(sys.stderr)
    console.setLevel([logging.WARNING, logging.INFO, logging.DEBUG][min(verbosity, 2)])
    console.setFormatter(logging.Formatter("%(levelname)s %(name)s: %(message)s"))
    base.addHandler(console)
    if verbosity >= 3:
        wire.addHandler(console)

    if log_dir is not None:
        log_dir.mkdir(parents=True, exist_ok=True)
        level = logging.DEBUG if verbosity >= 2 else logging.INFO
        base.addHandler(
            _rotating(log_dir / HOST_LOG, level, "%(asctime)s %(levelname)s %(name)s: %(message)s")
        )
        wire.addHandler(_rotating(log_dir / WIRE_LOG, logging.DEBUG, "%(asctime)s %(message)s"))
