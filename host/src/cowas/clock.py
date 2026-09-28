"""Clocks. The client measures every timeout on its transport's clock, so the same code runs
in real time on the serial port and in virtual time against the simulator."""

from __future__ import annotations

import time
from collections.abc import Callable
from datetime import UTC, datetime, timedelta
from typing import Protocol


class Clock(Protocol):
    def monotonic(self) -> float: ...

    def utcnow(self) -> datetime: ...

    def sleep(self, seconds: float) -> None: ...


class SystemClock:
    def monotonic(self) -> float:
        return time.monotonic()

    def utcnow(self) -> datetime:
        return datetime.now(UTC)

    def sleep(self, seconds: float) -> None:
        if seconds > 0:
            time.sleep(seconds)


class ManualClock:
    """Virtual time. `sleep()` advances it, through `on_sleep` when a simulator owns the time."""

    def __init__(
        self,
        start_utc: datetime | None = None,
        on_sleep: Callable[[float], None] | None = None,
    ) -> None:
        self._t = 0.0
        self._start_utc = start_utc or datetime(2026, 10, 15, 8, 0, tzinfo=UTC)
        self.on_sleep = on_sleep

    def monotonic(self) -> float:
        return self._t

    def utcnow(self) -> datetime:
        return self._start_utc + timedelta(seconds=self._t)

    def set(self, t: float) -> None:
        if t < self._t:
            raise ValueError("virtual time never goes back")
        self._t = t

    def sleep(self, seconds: float) -> None:
        if seconds <= 0:
            return
        if self.on_sleep is not None:
            self.on_sleep(seconds)
        else:
            self._t += seconds


def iso_utc(ts: datetime) -> str:
    """2026-10-15T08:00:00.123Z"""
    return ts.astimezone(UTC).isoformat(timespec="milliseconds").replace("+00:00", "Z")
