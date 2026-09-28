"""Faults the simulator can inject (ROADMAP HO1 item 5).

Every fault fires once. With `step=None` it fires `after_s` seconds after `inject()`; with a
step name (`purge`, `sample`, `shield`, `empty`, `rotate`, ...) it fires `after_s` seconds
after that step starts.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal


@dataclass(kw_only=True)
class Fault:
    step: str | None = None
    after_s: float = 0.0


@dataclass(kw_only=True)
class PressureTrip(Fault):
    """P_in jumps above the cap at the next pumping tick: FAULT OVER_PRESSURE,
    end_reason pressure_cap (during the water phase)."""


@dataclass(kw_only=True)
class Clog(Fault):
    """The filter clogs: resistance grows with the filtered volume beyond `start_ml`, the flow
    drops at the pressure target, and the FW8 rule ends the water phase with `clogged`."""

    start_ml: float = 300.0
    scale_ml: float = 150.0


@dataclass(kw_only=True)
class SlowProcedure(Fault):
    """Every move and flow is `factor` times slower. With `respect_bounds` the firmware's own
    stage timeouts still apply (FAULT TIMEOUT); without, the command overruns its `max_s`
    (a firmware bug: tests the host timeout)."""

    factor: float = 3.0
    respect_bounds: bool = True


@dataclass(kw_only=True)
class LinkDrop(Fault):
    """The link fails for `duration_s`. `disconnect=True`: the port vanishes (USB unplugged:
    reads/writes raise, the host must reconnect). Otherwise lines are lost silently in
    `direction` ("both", "h2d" host -> Due, "d2h" Due -> host)."""

    duration_s: float = 10.0
    direction: Literal["both", "h2d", "d2h"] = "both"
    disconnect: bool = False


@dataclass(kw_only=True)
class Reboot(Fault):
    """The Due resets (power loss, brown-out): the running command is lost without a result,
    the mode becomes RECOVERY_REQUIRED, the dedupe table is empty, the lock state unknown.
    The USB port re-enumerates for `usb_gap_s`; a `boot` event is sent when it is back (so a
    host that was not reconnected yet misses it and must detect the reboot via HELLO)."""

    cause: str = "POWER_ON"
    usb_gap_s: float = 1.5


@dataclass(kw_only=True)
class WatchdogReset(Fault):
    """The firmware hangs (no output, input ignored) for `hang_s`, then the watchdog resets
    it: a Reboot with cause WATCHDOG."""

    hang_s: float = 4.0
    usb_gap_s: float = 1.5


@dataclass(kw_only=True)
class FlowmeterDead(Fault):
    """No flowmeter pulses from now on: volume and flow unknown (null), warning FLOW_SILENT;
    a volume target then ends on `timeout`, a run without cap on `duration_reached`."""
