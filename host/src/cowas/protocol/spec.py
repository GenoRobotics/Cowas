"""Protocol v1 constants and tables, copied from docs/PROTOCOL.md.

Section numbers (§) refer to docs/PROTOCOL.md. Nothing here hard-codes a slot count:
the geometry always comes from GET_INFO (§5.7).
"""

from __future__ import annotations

import re
from enum import StrEnum

PROTO_MAJOR = 1
PROTO_VERSION = "1.0"

# §2 Transport
MAX_LINE = 2048  # bytes, including the "\n", both directions
ID_RE = re.compile(r"^[A-Za-z0-9._:-]{1,32}$")
USB_VID_ARDUINO = 0x2341
USB_PID_DUE_NATIVE = 0x003E  # the host port
USB_PID_DUE_PROGRAMMING = 0x003D  # console only; opening it resets the Due
HOST_BAUD = 115200  # ignored by USB CDC
FORBIDDEN_BAUD = 1200  # the "1200 baud touch" erases the Due's flash: never

# §9, §10 Heartbeat and timeouts
PING_PERIOD_S = 5.0
LINK_TIMEOUT_S = 30.0
SHORT_TIMEOUT_S = 2.0
SHORT_RETRIES = 3
ACK_TIMEOUT_S = 2.0
LONG_MARGIN_S = 30.0


class Mode(StrEnum):
    BOOT = "BOOT"
    RECOVERY_REQUIRED = "RECOVERY_REQUIRED"
    IDLE = "IDLE"
    BUSY = "BUSY"
    SERVICE = "SERVICE"
    FAULT = "FAULT"
    ESTOP = "ESTOP"


class Cmd(StrEnum):
    HELLO = "HELLO"
    PING = "PING"
    STATUS = "STATUS"
    GET_INFO = "GET_INFO"
    STOP = "STOP"
    CLEAR_FAULT = "CLEAR_FAULT"
    RECOVER = "RECOVER"
    ACTUATOR_HOME = "ACTUATOR_HOME"
    ROTATE = "ROTATE"
    LOCK = "LOCK"
    UNLOCK = "UNLOCK"
    PRIME_SHIELD = "PRIME_SHIELD"
    RUN_CYCLE = "RUN_CYCLE"
    PURGE = "PURGE"
    SAMPLE = "SAMPLE"
    DNA_SHIELD = "DNA_SHIELD"
    EMPTY = "EMPTY"


# §5.1
READ_ONLY_COMMANDS = frozenset({Cmd.HELLO, Cmd.PING, Cmd.STATUS, Cmd.GET_INFO})
LONG_COMMANDS = frozenset(
    {
        Cmd.RECOVER,
        Cmd.ACTUATOR_HOME,
        Cmd.ROTATE,
        Cmd.LOCK,
        Cmd.UNLOCK,
        Cmd.PRIME_SHIELD,
        Cmd.RUN_CYCLE,
        Cmd.PURGE,
        Cmd.SAMPLE,
        Cmd.DNA_SHIELD,
        Cmd.EMPTY,
    }
)
DEDUPE_COMMANDS = LONG_COMMANDS | {Cmd.CLEAR_FAULT}
PROCEDURE_COMMANDS = frozenset(
    {Cmd.RUN_CYCLE, Cmd.PURGE, Cmd.SAMPLE, Cmd.DNA_SHIELD, Cmd.EMPTY, Cmd.PRIME_SHIELD}
)
MOTION_COMMANDS = frozenset({Cmd.ROTATE, Cmd.LOCK, Cmd.UNLOCK, Cmd.ACTUATOR_HOME, Cmd.RECOVER})
# Commands that run the diaphragm pump, so they need a trusted P_in (§5.8, F50).
PUMPING_COMMANDS = frozenset({Cmd.RUN_CYCLE, Cmd.PURGE, Cmd.SAMPLE, Cmd.DNA_SHIELD, Cmd.EMPTY})
# Commands accepted only in IDLE (§5.2).
IDLE_ONLY_COMMANDS = frozenset(
    {
        Cmd.ACTUATOR_HOME,
        Cmd.ROTATE,
        Cmd.LOCK,
        Cmd.UNLOCK,
        Cmd.PRIME_SHIELD,
        Cmd.RUN_CYCLE,
        Cmd.PURGE,
        Cmd.SAMPLE,
        Cmd.DNA_SHIELD,
        Cmd.EMPTY,
    }
)
RESERVED_COMMANDS = frozenset(
    {"NEG_CONTROL", "POS_CONTROL", "LEAK_TEST", "GET_PARAMS", "SET_PARAM"}
)
RESERVED_PREFIXES = ("SPOOL_",)

KNOWN_COMMANDS = frozenset(c.value for c in Cmd)


def is_reserved(cmd: str) -> bool:
    return cmd in RESERVED_COMMANDS or cmd.startswith(RESERVED_PREFIXES)


def is_long(cmd: str) -> bool:
    return cmd in LONG_COMMANDS


def accepted_in(cmd: str, mode: str) -> bool:
    """The §5.2 table: is `cmd` accepted in `mode`? (Reserved/unknown commands never reach it.)"""
    if cmd in READ_ONLY_COMMANDS or cmd == Cmd.STOP:
        return True
    if cmd == Cmd.CLEAR_FAULT:
        return mode in (Mode.FAULT, Mode.ESTOP)
    if cmd == Cmd.RECOVER:
        return mode == Mode.RECOVERY_REQUIRED
    if cmd in IDLE_ONLY_COMMANDS:
        return mode == Mode.IDLE
    return False


# §4.2 fault codes (latched). The list may grow within v1: unknown = a system fault.
FAULT_CODES = frozenset(
    {
        "OVER_PRESSURE",
        "IMPLAUSIBLE_PRESSURE",
        "ESTOP",
        "USER_STOP",
        "TIMEOUT",
        "ENCODER",
        "ROTATION_REFUSED",
        "WATCHDOG_RESET",
        "SENSOR_DEAD",
        "MOTOR",
        "ACTUATOR",
        "LEAK_ENCLOSURE",
    }
)

# §4.3 warning codes (never latched). May grow: unknown codes are stored as they are.
WARNING_CODES = frozenset(
    {
        "FLOW_SILENT",
        "FLOW_LOW",
        "SENSOR_ABSENT",
        "PLACEHOLDER",
        "LINK_LOST",
        "OUTPUT_DROPPED",
        "AIR_PUSH_SHORT",
        "PURGE_CLAMPED",
        "UNLOCKED_AT_REST",
    }
)

# §4.4
RESET_CAUSES = frozenset({"POWER_ON", "WATCHDOG", "SOFTWARE", "USER_RESET", "BROWNOUT"})

# §8 error codes. The host treats an unknown code like "internal".
ERROR_CODES = frozenset(
    {
        "bad_json",
        "line_too_long",
        "bad_request",
        "unknown_cmd",
        "not_available",
        "bad_args",
        "proto_mismatch",
        "wrong_mode",
        "id_conflict",
        "sensor_unavailable",
        "lock_unknown",
        "not_aligned",
        "interlock",
        "cause_persists",
        "confirm_required",
        "recovery_failed",
        "result_expired",
        "stopped",
        "fault",
        "internal",
    }
)

# §6.2
END_REASONS = frozenset(
    {
        "volume_reached",
        "clogged",
        "duration_reached",
        "timeout",
        "pressure_cap",
        "fault",
        "stopped",
    }
)
PHASE_NAMES = frozenset({"prime", "purge", "water", "air_push", "shield", "empty"})
STAGES = ("purge", "sample", "shield", "empty")  # RUN_CYCLE order (§5.8)


def ms_elapsed(earlier: int, later: int) -> int:
    """Milliseconds from `earlier` to `later` on the Due's uint32 `ms` clock (wraps at 2**32)."""
    return (later - earlier) & 0xFFFFFFFF
