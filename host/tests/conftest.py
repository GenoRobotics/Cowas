from __future__ import annotations

import json
from collections.abc import Callable
from typing import Any

import pytest

from cowas.client import CowasClient
from cowas.protocol.vectors import load_vectors
from cowas.sim import FakeController, SimTransport
from cowas.transport.devicelog import DeviceLog


@pytest.fixture(scope="session")
def vectors() -> dict[str, Any]:
    return load_vectors()


class RecordingSimTransport(SimTransport):
    """A SimTransport that remembers every line the host wrote, with its virtual time."""

    def __init__(self, controller: FakeController, **kwargs: Any) -> None:
        super().__init__(controller, **kwargs)
        self.sent: list[tuple[float, dict[str, Any]]] = []

    def write_line(self, line: str) -> None:
        self.sent.append((self.clock.monotonic(), json.loads(line)))
        super().write_line(line)

    def sent_ids(self, cmd: str | None = None) -> list[str]:
        return [req["id"] for _, req in self.sent if cmd is None or req["cmd"] == cmd]


@pytest.fixture
def make_client(tmp_path) -> Callable[..., tuple[CowasClient, FakeController, Any]]:
    """make_client(**controller_kwargs) -> (connected client, controller, transport)."""
    logs: list[DeviceLog] = []

    def make(
        controller: FakeController | None = None,
        client_kwargs: dict[str, Any] | None = None,
        **controller_kwargs: Any,
    ) -> tuple[CowasClient, FakeController, RecordingSimTransport]:
        sim = controller or FakeController(**controller_kwargs)
        transport = RecordingSimTransport(sim)
        log = DeviceLog(tmp_path / f"logs{len(logs)}")
        logs.append(log)
        client = CowasClient(transport, device_log=log, **(client_kwargs or {}))
        client.connect()
        return client, sim, transport

    yield make
    for log in logs:
        log.close()
