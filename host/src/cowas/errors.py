"""Exceptions raised by the host package."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from cowas.protocol.models import HelloData, Response


class CowasError(Exception):
    """Base class."""


class TransportError(CowasError):
    """The port is closed, vanished or cannot be opened."""


class PortNotFound(TransportError):
    pass


class ProgrammingPortRefused(TransportError):
    """The host must not use the Programming port outside the single-cable bench mode (§2)."""


class LinkLost(CowasError):
    """No answer after the retries of §10 and a reconnection attempt."""


class CommandTimeout(CowasError):
    """A long command gave no result within ack.max_s + 30 s (§10), even after a resend."""

    def __init__(self, request_id: str, cmd: str, message: str) -> None:
        super().__init__(message)
        self.request_id = request_id
        self.cmd = cmd


class DeviceRebooted(CowasError):
    """The Due rebooted (new boot_id) while this request was pending. Its id must never be
    resent (§7); the slot of an interrupted procedure is SUSPECT (ROADMAP §3.6)."""

    def __init__(
        self,
        request_id: str,
        cmd: str,
        old_boot_id: str | None,
        new_boot_id: str,
        hello: HelloData | None = None,
    ) -> None:
        super().__init__(
            f"the Due rebooted while {cmd} {request_id} was pending "
            f"(boot_id {old_boot_id} -> {new_boot_id})"
        )
        self.request_id = request_id
        self.cmd = cmd
        self.old_boot_id = old_boot_id
        self.new_boot_id = new_boot_id
        self.hello = hello


class ProtocolMismatch(CowasError):
    """The Due speaks another major protocol version."""


class InvalidArguments(CowasError):
    """Refused by the host before sending (the same checks as the Due, §7 step 5)."""

    def __init__(self, cmd: str, code: str, arg: str | None, message: str) -> None:
        super().__init__(f"{cmd}: {message}")
        self.cmd = cmd
        self.code = code
        self.arg = arg


class CommandFailed(CowasError):
    """The Due answered ok:false."""

    def __init__(self, cmd: str, response: Response) -> None:
        err = response.err
        code = err.code if err else "internal"
        msg = err.msg if err else ""
        super().__init__(f"{cmd} failed: {code}" + (f" ({msg})" if msg else ""))
        self.cmd = cmd
        self.response = response
        self.code = code
