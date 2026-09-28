"""The transport interface: a line-oriented, reconnectable link to one controller."""

from __future__ import annotations

from typing import Protocol

from cowas.clock import Clock


class Transport(Protocol):
    """One JSON-lines link. Implementations: SerialTransport, SimTransport, MemoryTransport.

    `read_line` waits at most `timeout` seconds *on `clock`* and returns one line without its
    line ending, or None. Every method raises `cowas.errors.TransportError` when the port is
    gone; `reconnect` then tries to get it back until its own deadline.
    """

    clock: Clock
    name: str

    def open(self) -> None: ...

    def close(self) -> None: ...

    @property
    def is_open(self) -> bool: ...

    def write_line(self, line: str) -> None: ...

    def read_line(self, timeout: float) -> str | None: ...

    def reconnect(self, timeout: float) -> None: ...
