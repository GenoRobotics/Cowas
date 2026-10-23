"""An in-memory transport on a manual clock, for tests and scripted replays.

`responder(line)` is called for every line the host writes and returns the lines the "device"
answers; `feed()` queues unsolicited lines (events, telemetry, logs).
"""

from __future__ import annotations

from collections import deque
from collections.abc import Callable

from cowas.clock import ManualClock
from cowas.errors import TransportError


class MemoryTransport:
    def __init__(
        self,
        responder: Callable[[str], list[str]] | None = None,
        clock: ManualClock | None = None,
    ) -> None:
        self.clock = clock or ManualClock()
        self.name = "memory"
        self.responder = responder
        self.written: list[str] = []
        self._inbox: deque[str] = deque()
        self._open = False
        self.connected = True  # False = the port vanished (reads/writes raise)
        self.reconnects = 0

    @property
    def is_open(self) -> bool:
        return self._open

    def open(self) -> None:
        if not self.connected:
            raise TransportError("memory port gone")
        self._open = True

    def close(self) -> None:
        self._open = False

    def feed(self, *lines: str) -> None:
        self._inbox.extend(lines)

    def _check(self) -> None:
        if not self._open or not self.connected:
            self._open = False
            raise TransportError("memory port closed")

    def write_line(self, line: str) -> None:
        self._check()
        self.written.append(line)
        if self.responder is not None:
            self._inbox.extend(self.responder(line))

    def read_line(self, timeout: float) -> str | None:
        self._check()
        if self._inbox:
            return self._inbox.popleft()
        self.clock.sleep(timeout)
        return None

    def reconnect(self, timeout: float) -> None:
        self.reconnects += 1
        self.close()
        if not self.connected:
            self.clock.sleep(timeout)
            raise TransportError("memory port gone")
        self.open()
