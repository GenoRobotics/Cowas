"""SimTransport: plugs a FakeController in where a serial port would be.

Reading with a timeout advances the controller's virtual time, so a 30-minute cycle runs in
well under a second. `realtime_speed` (e.g. 20.0) slows it down to 20x real time for a
human watching `cowas monitor --sim`.
"""

from __future__ import annotations

import time

from cowas.errors import TransportError
from cowas.sim.controller import TICK, FakeController


class SimTransport:
    def __init__(self, controller: FakeController, *, realtime_speed: float | None = None) -> None:
        self.controller = controller
        self.clock = controller.clock
        self.name = "sim"
        self.realtime_speed = realtime_speed
        self._open = False
        controller.host_attached = False

    @property
    def is_open(self) -> bool:
        return self._open

    def open(self) -> None:
        if not self.controller.port_present():
            raise TransportError("simulated port absent")
        self._open = True
        self.controller.host_attached = True

    def close(self) -> None:
        self._open = False
        self.controller.host_attached = False

    def _check(self) -> None:
        if not self._open:
            raise TransportError("simulated port closed")
        if not self.controller.port_present():
            self.close()
            raise TransportError("simulated port vanished (USB re-enumeration)")

    def _advance(self, seconds: float) -> None:
        self.controller.advance(seconds)
        if self.realtime_speed:
            time.sleep(seconds / self.realtime_speed)

    def write_line(self, line: str) -> None:
        self._check()
        self.controller.feed_line(line)

    def read_line(self, timeout: float) -> str | None:
        self._check()
        deadline = self.clock.monotonic() + max(0.0, timeout)
        while True:
            line = self.controller.read_output()
            if line is not None:
                return line
            remaining = deadline - self.clock.monotonic()
            if remaining <= 1e-9:
                return None
            self._advance(min(remaining, TICK))
            self._check()

    def reconnect(self, timeout: float) -> None:
        self.close()
        deadline = self.clock.monotonic() + timeout
        while not self.controller.port_present():
            remaining = deadline - self.clock.monotonic()
            if remaining <= 0:
                raise TransportError("simulated port still absent")
            self._advance(min(0.5, remaining))
        self._open = True
        self.controller.host_attached = True
