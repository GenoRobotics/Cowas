# CoWaS protocol v1 (host ↔ controller)

**Status:** v1.0, FW1, 2026-09-25 (open questions answered by the user the same day, §14). Firmware implementation: FW7 (+ FW8 for the full sample record). Host implementation: HO1. Test vectors: [`protocol/vectors.json`](../protocol/vectors.json), loaded by both the C++ (`pio test -e native`) and the Python (`pytest`) tests.

This document is the contract. If the firmware or the host disagrees with it, the code is wrong, or this file changes first (in a PR that also updates the vectors). Decisions are taken from ROADMAP §3; the decisions taken in FW1 and the questions deferred to later steps are in §14.

Words: **Due** = the controller (Arduino Due). **Host** = the Python program on the laptop (Pilot A) or the Pi (Pilot B). "MUST", "MUST NOT", "SHOULD" have their usual meaning.

---

## 1. Principles

1. **The host says *what*, the Due knows *how*** (ROADMAP §3.1). The host never toggles a valve or a pump; it asks for procedures. The Due never starts a procedure on its own.
2. **Safety never depends on the link.** Caps, timeouts, the rotation interlock and the watchdog run on the Due whatever the host does. A lost link never stops a running procedure halfway (§9).
3. **One request at a time.** The Due runs at most one command that moves hardware. While it runs (mode `BUSY`), it still answers `STOP`, `STATUS`, `PING`, `HELLO` and `GET_INFO` immediately.
4. **Everything is explicit.** Unknown commands, wrong argument types, unknown arguments and out-of-range values are refused, never guessed. A refusal never moves anything.
5. **The host never assumes a slot count.** Slot count, purge slot, sample slots and fill order come from `GET_INFO`.
6. **Forward compatible.** Receivers ignore unknown fields and unknown event types; §12 says what may change inside v1.

## 2. Transport

| Item | Value |
|---|---|
| Host port | **Native USB** (`SerialUSB`, USB CDC). USB VID:PID `2341:003E` (Arduino Due native). The Programming port is `2341:003D`: the host MUST NOT use it in production. |
| Console port | Programming port (`Serial`), human text only, for a laptop. Opening it resets the Due (auto-reset), which is why the host is not on it (ROADMAP §3.4). |
| Baud rate | Ignored by USB CDC. **The host MUST NEVER open the Native port at 1200 baud**: that "touch" erases the Due's flash (bootloader entry). Use 115200. |
| DTR | The host opens the port with DTR asserted (pyserial default). |
| Framing | One JSON object per line, UTF-8 (the Due only emits ASCII), terminated by `\n`. The Due accepts `\n` or `\r\n` (it strips one trailing `\r`, F37). Empty or whitespace-only lines are ignored silently. |
| Max line length | **2048 bytes including the `\n`**, both directions. The Due discards a longer line up to the next `\n` and answers `line_too_long` (§8). The Due never sends a longer line: it truncates `log` messages (ending them with `...`) and bounds every array in a result (§6.4). |
| Numbers | JSON numbers. Floats with at most 3 decimals. **NaN and ±inf are never sent: they become `null`.** Integers for counts and `ms`. |
| Time | The Due has no clock. `ms` = milliseconds since boot (`millis()`, uint32, wraps after 49.7 days: the host MUST handle the wrap). The host timestamps every received line in UTC. |
| Direction of JSON | Host → Due: only `req`. Due → host: `ack`, `res`, `evt`, `tel`, `log`. |
| Non-blocking output | The Due MUST NOT block on the host port. If the host does not read and the USB TX buffer is full, the Due drops the line and counts it (`STATUS.data.tx_dropped`, warning `OUTPUT_DROPPED`). Safety loops never wait for the host. |
| Single-cable bench mode | FW7 setting `HOST_PORT = Serial`: protocol and console share the Programming port. JSON lines start with `{`; the host treats every other line as a device log line. Bench only (the port resets the Due when opened). |

## 3. Messages

Every message is a JSON object with a string field `t`. Field order is not significant. Receivers ignore fields they do not know.

### 3.1 `req`: host → Due

```json
{"t":"req","id":"h-000123","cmd":"ROTATE","args":{"slot":3}}
```

| Field | Type | Rules |
|---|---|---|
| `t` | `"req"` | required |
| `id` | string | required, 1–32 characters from `A–Z a–z 0–9 . _ : -`. Unique per request for the lifetime of a host session: a new request gets a new id; a **retry of the same request reuses the same id** (§7). Integers are refused. |
| `cmd` | string | required, upper case, case-sensitive (`ping` is `unknown_cmd`). |
| `args` | object | optional; missing = `{}`. Must be an object if present. Arguments are **strict**: unknown keys, wrong types and out-of-range values give `bad_args`. `true`/`false` are not numbers and `1`/`0` are not booleans. |

Unknown top-level fields in the envelope are ignored (room for later additions).

### 3.2 `ack`: Due → host, "accepted and started"

```json
{"t":"ack","id":"h-000124","cmd":"RUN_CYCLE","max_s":2950}
```

Sent **at once** (< 100 ms) for every *long* command (column "ack" in §5), only after all checks passed and the command has started. `max_s` = the Due's own worst-case duration for this command with these arguments (the sum of its internal timeouts); the host uses it for its timeout (§10). A retried id of a command that is still running gets the ack again with `"dup":true`.

Refused commands get no ack: just a `res` with `ok:false`.

### 3.3 `res`: Due → host, the final answer

```json
{"t":"res","id":"h-000124","ok":true,"mode":"IDLE","data":{ ... }}
{"t":"res","id":"h-000125","ok":false,"mode":"BUSY","err":{"code":"wrong_mode","msg":"busy with RUN_CYCLE h-000124","mode":"BUSY"}}
```

| Field | Type | Rules |
|---|---|---|
| `id` | string or `null` | the request id; `null` only when the id could not be read (`bad_json`, `line_too_long`, bad id). |
| `ok` | bool | see below |
| `mode` | string | the mode **after** the command (§4) |
| `data` | object | present when `ok:true` (may be `{}`) |
| `err` | object | present when `ok:false`: `{code, msg, ...}`. `code` is stable (§8); `msg` is for humans and may change. Extra fields per code are listed in §8. |

**Exactly one `res` per accepted-or-refused request id** (a retried id may get its stored `res` again, §7). Read-only requests are answered in < 50 ms.

**`ok` for procedures.** For the procedure commands (`RUN_CYCLE`, `PURGE`, `SAMPLE`, `DNA_SHIELD`, `EMPTY`, `PRIME_SHIELD`), `ok:true` means *the procedure started and here is its record*, whatever happened next: read `data.completed`, `data.end_reason` and `data.abort`. `ok:false` means it was refused and **nothing moved**. This keeps the partial record of an interrupted sample. For the motion commands (`ROTATE`, `LOCK`, `UNLOCK`, `ACTUATOR_HOME`, `RECOVER`), `ok:false` can also mean "started, then failed" (`err.code` = `stopped`, `fault` or `recovery_failed`).

### 3.4 `evt`: Due → host, something happened

```json
{"t":"evt","ms":81234,"type":"mode","from":"BUSY","to":"FAULT","cause":"USER_STOP"}
```

Every event has `ms` and `type`. Events caused by a request are sent **before** that request's `res`. The host MUST ignore event types it does not know.

| `type` | Fields | When |
|---|---|---|
| `boot` | `boot_id`, `reset_cause`, `proto`, `fw` | once, when the host port is ready after a reset (may be lost if no host is connected; `HELLO` repeats it) |
| `mode` | `from`, `to`, `cause` (a fault code, a reset cause, or the command that caused it) | every mode change |
| `fault` | `code`, `msg`, `phase` | a fault latches (§4.2) |
| `fault_cleared` | `code` | `CLEAR_FAULT` succeeded |
| `warning` | `code`, `msg`, `phase` | a non-safety problem (rule 5, §4.3). Never changes the mode. |
| `step_start` | `id`, `cmd`, `step` | a procedure step starts (the stages of §6.1; a single-step command has one step named after it in lower case) |
| `step_end` | `id`, `cmd`, `step`, `ok`, `duration_s` | it ends |
| `phase` | `id`, `from`, `to` | the pumping phase changes (§6.2) |
| `link` | `state` (`lost` / `restored`), `silent_s` | host heartbeat missing / back (§9) |

### 3.5 `tel`: Due → host, telemetry at ~1 Hz while `BUSY`

```json
{"t":"tel","ms":90210,"id":"h-000124","cmd":"RUN_CYCLE","step":"sample","phase":"water","elapsed_s":412.0,"slot":3,"lock":"locked","p_in":1.87,"flow_ml_min":98.4,"volume_ml":671.2,"pump_pct":74}
```

Sent every 1000 ± 200 ms while a long command runs, never when `IDLE`. Every value is `null` when unknown or when its sensor is absent. `volume_ml` = the volume of the current pumping phase (`purge` or `water`), `null` in the other phases. `pump_pct` = the commanded diaphragm pump power 0–100 (not measured flow, F25). Units in §6.5.

### 3.6 `log`: Due → host, a copy of every human log line

```json
{"t":"log","ms":90222,"lvl":"info","msg":"Step sample: pumping water through sterivex"}
```

`lvl` ∈ `debug`, `info`, `warn`, `error`. Every line the Due prints on the console is also sent here (ROADMAP §3.4), except the service-menu echo. The host writes them to the device log; it MUST NOT parse `msg` for control.

## 4. Modes, faults, warnings

### 4.1 Modes (ROADMAP §3.5)

| Mode | Meaning | Left by |
|---|---|---|
| `BOOT` | self-check after a reset (a few s) | → `RECOVERY_REQUIRED` (or `ESTOP`) |
| `RECOVERY_REQUIRED` | after every boot and every cleared fault. Procedures and moves are refused. | `RECOVER` ok → `IDLE` |
| `IDLE` | ready for host commands | a long command → `BUSY`; `STOP` → `FAULT`; console `service` → `SERVICE` |
| `BUSY` | one long command runs | its end → `IDLE`; `STOP` or a fault → `FAULT` |
| `SERVICE` | a person uses the console on the Programming port (tests, calibrations). The host may only observe and STOP. | console `exit` → `IDLE` |
| `FAULT` | a fault is latched; safe state | `CLEAR_FAULT` → `RECOVERY_REQUIRED` |
| `ESTOP` | the E-stop input is active (from FW6/FW9, if installed) | released + `CLEAR_FAULT` → `RECOVERY_REQUIRED` |

**Safe state** (ROADMAP §3.2): pumps off, all valves closed, manifold motor stopped, the O-ring stays as it is (locked stays locked).

**Rest state:** the O-ring is **locked** whenever no command runs. It is unlocked only to rotate (ROADMAP §3.2, decided in FW1).

`STOP` is a fault on purpose: after any STOP, even from `IDLE`, the path back is `CLEAR_FAULT` → `RECOVER` → `IDLE` (§3.2 "recovery after every stop"). `RECOVER` moves nothing when nothing needs it (§5.6).

### 4.2 Fault codes (latched, FW6)

`OVER_PRESSURE`, `IMPLAUSIBLE_PRESSURE`, `ESTOP`, `USER_STOP`, `TIMEOUT` (a procedure's own deadline), `ENCODER`, `ROTATION_REFUSED`, `WATCHDOG_RESET`, `SENSOR_DEAD`, `MOTOR` (VNH5019 diagnostic, F47), `ACTUATOR` (FW9 home switch), `LEAK_ENCLOSURE` (FW9). The list may grow within v1; the host treats an unknown code as a system fault.

### 4.3 Warning codes (never latched)

`FLOW_SILENT` (no flowmeter pulses while pumping water), `FLOW_LOW`, `SENSOR_ABSENT` (an optional sensor configured but not answering, e.g. water temperature), `PLACEHOLDER` (a `PLACEHOLDER` setting was used; `msg` names it), `LINK_LOST`, `OUTPUT_DROPPED`, `AIR_PUSH_SHORT`, `PURGE_CLAMPED` (the requested purge was below the §3.7 minimum and was raised to it), `UNLOCKED_AT_REST` (the O-ring was left unlocked by `UNLOCK`). The list may grow; the host stores unknown codes as they are.

### 4.4 Recovery causes

`HELLO`/`STATUS` report `recovery_cause` when the mode is `RECOVERY_REQUIRED`: a reset cause (`POWER_ON`, `WATCHDOG`, `SOFTWARE`, `USER_RESET`, `BROWNOUT`), or the fault code that was cleared.

## 5. Commands

### 5.1 Summary

"ack" = long command (an `ack` first, then `res` later, and `tel` while it runs). "dedupe" = its id is remembered (§7).

| Command | Args | ack | dedupe | Purpose |
|---|---|---|---|---|
| `HELLO` | `proto?`, `client?` | | | versions, identity, mode, capabilities |
| `PING` | — | | | heartbeat |
| `STATUS` | — | | | the live state |
| `GET_INFO` | — | | | geometry, fill order, limits, placeholders |
| `STOP` | — | | never | safe state + `FAULT(USER_STOP)`, any mode |
| `CLEAR_FAULT` | — | | ✓ | `FAULT`/`ESTOP` → `RECOVERY_REQUIRED` |
| `RECOVER` | `confirm_moving` | ✓ | ✓ | the §3.2 recovery checklist → `IDLE` |
| `ACTUATOR_HOME` | — | ✓ | ✓ | O-ring state unknown → known (unlocked) |
| `ROTATE` | `slot` | ✓ | ✓ | unlock → rotate to a slot → lock |
| `LOCK` | — | ✓ | ✓ | lock the O-ring at the current slot |
| `UNLOCK` | — | ✓ | ✓ | service: unlock the O-ring (leaves the machine off its rest state) |
| `PRIME_SHIELD` | `duration_ms?` | ✓ | ✓ | fill the DNA/RNA Shield line before a deployment |
| `RUN_CYCLE` | see §5.8 | ✓ | ✓ | **the production sequence for one filter** |
| `PURGE` | `volume_ml?` | ✓ | ✓ | service: purge through the purge slot |
| `SAMPLE` | `slot`, `volume_ml`, `max_duration_s` | ✓ | ✓ | service: water + air push through one filter |
| `DNA_SHIELD` | `slot`, `duration_ms?` | ✓ | ✓ | service: shield into one filter |
| `EMPTY` | — | ✓ | ✓ | service: empty the deployment line through purge |
| `SPOOL_*`, `NEG_CONTROL`, `POS_CONTROL`, `LEAK_TEST`, `GET_PARAMS`, `SET_PARAM` | any | | | **reserved**: always `not_available` (P2) |

**Rest state = locked.** Every command that moves hardware ends with the O-ring **locked** at the slot where it ends, except `UNLOCK`. A command that needs to rotate unlocks first, rotates, then locks at the target (FW4). A stop or a fault leaves the O-ring as it is (safe state). The legacy purge already ended locked (F40); v1 makes that the rule, and FW4 makes every rotation unlock first.

### 5.2 Which mode accepts what

✓ = accepted; ✗ = `wrong_mode`; "—" = never reached (reserved commands answer `not_available` in every mode).

| Command | BOOT | RECOVERY_REQUIRED | IDLE | BUSY | SERVICE | FAULT | ESTOP |
|---|---|---|---|---|---|---|---|
| `HELLO`, `PING`, `STATUS`, `GET_INFO` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `STOP` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `CLEAR_FAULT` | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ (`cause_persists` while pressed) |
| `RECOVER` | ✗ | ✓ | ✗ | ✗ | ✗ | ✗ | ✗ |
| `ACTUATOR_HOME`, `ROTATE`, `LOCK`, `UNLOCK`, `PRIME_SHIELD`, `RUN_CYCLE`, `PURGE`, `SAMPLE`, `DNA_SHIELD`, `EMPTY` | ✗ | ✗ | ✓ | ✗ | ✗ | ✗ | ✗ |
| reserved | — | — | — | — | — | — | — |

`STOP` in `FAULT` or `ESTOP` re-applies the safe state and changes nothing else (no `mode` event). `STOP` in `SERVICE` also stops whatever the console is running.

### 5.3 `HELLO`

Args (both optional): `proto` (int, the host's major protocol version), `client` (string ≤ 40, e.g. `"cowas-host 0.1.0"`, logged by the Due). If `proto` ≠ 1: `proto_mismatch`.

```json
{"t":"res","id":"h-1","ok":true,"mode":"RECOVERY_REQUIRED","data":{
  "proto":"1.0","device_id":"cowas-V1","boot_id":"9f3c21a7","uptime_s":12.4,
  "fw":{"version":"fw7","git":"62a222d","dirty":false,"built":"2026-10-20T10:00:00Z"},
  "mode":"RECOVERY_REQUIRED","recovery_required":true,"recovery_cause":"POWER_ON",
  "fault":null,
  "caps":{"p_in":true,"p_out":false,"q_in":true,"q_out":false,"estop_input":false,
          "spool":false,"water_temp":false,"leak":false,"oring_home_switch":false}}}
```

- `boot_id`: 8 hex characters from the SAM3X hardware RNG at boot. **A new `boot_id` means the Due rebooted**: the dedupe table is empty, the lock state is unknown, any procedure that was running was interrupted (the host marks its slot `SUSPECT`, ROADMAP §3.6). The host MUST call `HELLO` after every (re)connection and compare.
- `fw.git`: the short commit hash, injected at build time (FW7); `dirty` = uncommitted changes.
- `device_id`: a Settings.h constant (`DEVICE_ID = "cowas-V1"`), written into every sample record.
- `caps`: what is **installed and configured** (compile-time). Runtime health is in `STATUS.sensors`. A capability the host does not know is ignored.

### 5.4 `PING`

No args. `data: {"ms": <uptime ms>}`. Any valid request counts as a heartbeat; `PING` is the one that has no side effect (§9).

### 5.5 `STATUS`

```json
{"t":"res","id":"h-7","ok":true,"mode":"BUSY","data":{
  "mode":"BUSY","boot_id":"9f3c21a7","uptime_s":5230.1,
  "recovery_required":false,"recovery_cause":null,"fault":null,
  "lock":"locked","slot":3,"angle_deg":117.1,
  "busy":{"id":"h-5","cmd":"RUN_CYCLE","step":"sample","phase":"water","elapsed_s":412.0,"max_s":2950},
  "sensors":[{"name":"p_in","value":1.87,"unit":"bar","stat":"ok"},
             {"name":"flow_ml_min","value":98.4,"unit":"mL/min","stat":"ok"}],
  "warnings":[],"link_ok":true,"tx_dropped":0,
  "last":{"id":"h-4","cmd":"ROTATE","ok":true,"end_reason":null}}}
```

- `lock`: `locked` | `unlocked` | `unknown` (tri-state from FW2; `unknown` after every boot, F18).
- `slot`: the slot the manifold is aligned with (encoder within tolerance), or `null` if between slots or the encoder fails.
- `fault`: `null` or `{code, msg}` (the latched fault).
- `busy`: `null` unless `BUSY`.
- `sensors[].stat` here is the health: `ok`, `stale`, `implausible`, `absent`.
- `last`: the last finished long command, or `null` (lost on reboot).

### 5.6 `STOP`, `CLEAR_FAULT`, `RECOVER`

**`STOP`** (no args): always accepted, never deduplicated (a repeated STOP always acts). The Due enters the safe state within 100 ms (FW6 target), latches `USER_STOP` (unless already in `FAULT`/`ESTOP`), then answers `data: {"interrupted": {"id","cmd"} | null}`. If a long command was running, its own `res` follows within 2 s (§3.3: a procedure returns its partial record with `abort.reason = "stopped"`; a motion command returns `err.code = "stopped"`).

**`CLEAR_FAULT`** (no args): `FAULT`/`ESTOP` → `RECOVERY_REQUIRED`. Refused with `cause_persists` while the cause is still there (E-stop pressed, P_in implausible, ...). `data: {"cleared": "<code>"}`.

**`RECOVER`** `{confirm_moving: bool}` (required). Runs ROADMAP §3.2 in order and stops at the first failure: `cause_gone` → `encoder` (reads, plausible) → `align` (manifold on a slot; if not, re-home to the nearest slot: **moving**) → `oring` (if `unknown`, `ACTUATOR_HOME`; if `unlocked`, `LOCK`: **moving**; it ends locked, the rest state) → `valves` (to idle) → `IDLE`.
- If a moving step is needed and `confirm_moving` is `false`: **nothing moves**, `ok:false`, `err.code = "confirm_required"`, `err.needs = ["align","actuator_home"]` or `["lock"]` (the moving steps that would run). The host shows this to the operator and resends with `true` (a new id).
- Success: `data: {"steps":[{"name":"cause_gone","ok":true}, ...]}`, mode `IDLE`.
- Failure: `ok:false`, `err.code = "recovery_failed"`, `err.step = "<name>"`, `err.steps = [...]`; mode stays `RECOVERY_REQUIRED` (or `FAULT` if the step latched a fault).

### 5.7 `GET_INFO`

```json
{"t":"res","id":"h-2","ok":true,"mode":"IDLE","data":{
  "slot_count":15,"purge_slot":0,
  "sample_slots":[1,2,3,4,5,6,7,8,9,10,11,12,13,14],
  "fill_order":[1,2,3,14,13,12,11,10,9,8,4,5,6,7],
  "raw_positions":16,"clock_dir":"cw",
  "positions":[{"slot":0,"angle_deg":162.07,"clock":30},{"slot":1,"angle_deg":139.57,"clock":34}, "..."],
  "limits":{"volume_ml_min":10,"volume_ml_max":5000,"max_duration_s_max":3600,
            "purge_ml_min":424,"purge_ml_default":424,"shield_ms_default":250000,
            "duration_ms_max":600000,"pressure_cap_bar":2.5,
            "ping_period_s":5,"link_timeout_s":30,"max_line":2048},
  "placeholders":["LINEAR_ACT_LOCKED_STEPS","calibrationFactor_small","DNA_SHIELD_FILL_MS"]}}
```

- `positions`: one entry per slot, `angle_deg` = the calibrated encoder angle, `clock` = the clock-face label (`clocklist`). Informative: the host uses slot numbers only.
- `fill_order`: the order in which the host SHOULD allocate filters (leak containment, Settings.h). It lists every sample slot once.
- `limits`: the ranges the Due will accept in args (the host validates with them too). Values above are examples; the real ones come from Settings.h (FW4 values; `purge_ml_min` = `purge_volume_ml()`, the §3.7 rule 1 minimum).
- `placeholders`: every setting still marked `PLACEHOLDER` in the firmware. The host copies the list into each deployment record. Empty list = everything calibrated.

### 5.8 `RUN_CYCLE`: the production sequence for one filter

```json
{"t":"req","id":"h-000124","cmd":"RUN_CYCLE","args":{
  "slot":3,"purge_ml":null,"volume_ml":2000,"max_duration_s":1800,
  "stages":{"purge":true,"sample":true,"shield":true,"empty":true},
  "sample_id":"CW01-D03-03-20261015T0930"}}
```

| Arg | Type | Rules |
|---|---|---|
| `slot` | int | required; one of `GET_INFO.sample_slots` (the purge slot is refused). The Due does **not** track which filters are used: the host owns the inventory (F1). |
| `purge_ml` | number or `null` | required key; `null` = the firmware default (`limits.purge_ml_default`). A value below `limits.purge_ml_min` is **raised to the minimum** with the warning `PURGE_CLAMPED` (an event before the `ack`, and in the record): §3.7 rule 1 is a requirement. Zero, negative, or above `volume_ml_max`: `bad_args`. |
| `volume_ml` | number or `null` | required key; the target filtered volume (`volume_ml_min`..`volume_ml_max`). `null` = no volume cap: filter until clogged or `max_duration_s` (then `end_reason = duration_reached`). Needs `caps.q_in` when not null. |
| `max_duration_s` | int | required; the limit of the **water pumping phase** (filtration) only, 1..`max_duration_s_max`. The other stages have their own firmware timeouts; `ack.max_s` gives the whole-cycle bound. |
| `stages` | object | optional; keys `purge`, `sample`, `shield`, `empty` (bools); a missing key = `true`. All four `false` = `bad_args`. |
| `sample_id` | string ≤ 40 | optional; echoed in the result and in the log lines, for traceability. |

Sequence (each stage is a `step_start`/`step_end` pair): **`purge`** (rotate to the purge slot, lock, pump `purge_ml`) → **`sample`** (rotate to `slot`, lock, water until `end_reason`, air push through purge and through the filter) → **`shield`** (DNA/RNA Shield into the filter, time-based, then the air push of the shield) → **`empty`** (empty the deployment line through purge). Every rotation between stages is unlock → rotate → lock. The cycle **ends locked**, normally at the purge slot where the last stage runs. Stages set to `false` are skipped (no events).

Refusals before anything moves: `bad_args`, `wrong_mode`, `sensor_unavailable` (no trusted `P_in`: every pumping command needs it, F50; or `volume_ml` set without `q_in`), `lock_unknown`.

### 5.9 Service procedures

Same rules as `RUN_CYCLE`, one stage each. Results use the record of §6 with only the fields of the stages that ran (the others `null` or empty).

| Command | Args | Does |
|---|---|---|
| `PURGE` | `volume_ml`: number or `null`/missing (default); below `purge_ml_min` → raised, `PURGE_CLAMPED` | the `purge` stage; ends locked at the purge slot |
| `SAMPLE` | `slot` (sample slot), `volume_ml` (number or `null`), `max_duration_s` | the `sample` stage |
| `DNA_SHIELD` | `slot` (sample slot), `duration_ms` (optional, 1..`duration_ms_max`, default `shield_ms_default`) | the `shield` stage: rotate, lock, shield, air push; ends locked at the purge slot |
| `EMPTY` | — | the `empty` stage |
| `PRIME_SHIELD` | `duration_ms` (optional, 1..`duration_ms_max`, default `PRIME_SHIELD_MS`, PLACEHOLDER) | rotate to the purge slot, lock, micro pump for `duration_ms`; ends locked. Fills the shield line before a deployment so the first sample gets its full dose. **Destination and duration are deferred to LA3/FW7 (§14).** |

### 5.10 Motion commands

| Command | Args | Does | Specific refusals |
|---|---|---|---|
| `ROTATE` | `slot`: any slot 0..`slot_count`-1 | unlock (if locked and safe, FW4) → rotate → lock at the target. Already at the target and locked: nothing moves. | `lock_unknown`, `interlock` (locked and not safe to unlock, after the FW4 wait) |
| `LOCK` | — | lock at the current slot; already locked: `ok`, nothing moves | `lock_unknown`, `not_aligned` |
| `UNLOCK` | — | service only: unlock and stay unlocked (warning `UNLOCKED_AT_REST`); the next `LOCK`, `ROTATE` or procedure locks again. Already unlocked: `ok`, nothing moves | `lock_unknown`, `interlock` |
| `ACTUATOR_HOME` | — | overdrive lock against the physical stop (a known reference) → `locked` (FW2 homes to unlocked; from FW4 it ends locked) | `not_aligned` |

`data` on success: `{"slot": <int|null>, "lock": "<state>", "angle_deg": <num|null>}`. A failure after the start: `ok:false` with `err.code` `stopped` or `fault` (and `err.fault`); the mode is then `FAULT`.

## 6. The sample record (`RUN_CYCLE` result)

### 6.1 Example

```json
{"t":"res","id":"h-000124","ok":true,"mode":"IDLE","data":{
  "slot":3,"sample_id":"CW01-D03-03-20261015T0930",
  "completed":true,"abort":null,"end_reason":"volume_reached",
  "volume_ml":2003.5,"duration_s":1184.2,"cycle_s":1702.4,
  "purge_ml_target":424,"purge_ml_measured":431.0,
  "p_in":{"max":2.06,"mean":1.41,"final":1.98},
  "flow_ml_min":{"mean":101.5,"min":62.0,"final":64.3},
  "phases":[{"name":"purge","duration_s":251.3},{"name":"water","duration_s":1184.2},
            {"name":"air_push","duration_s":71.8},{"name":"shield","duration_s":251.1},
            {"name":"empty","duration_s":95.0}],
  "shield_ms":250000,"t_end_to_shield_s":118.5,
  "faults":[],
  "warnings":[{"code":"PLACEHOLDER","msg":"calibrationFactor_small","phase":null}],
  "sensors":[{"name":"water_temp","value":11.8,"unit":"degC","stat":"mean"}],
  "p_out":null,"q_out":null,"dp":null,"r_filter":null,"clog_index":null}}
```

### 6.2 Fields

**Every field is present; any value can be `null`** when its sensor is absent, the stage did not run, or the value is not measured yet (FW7 sends the record, FW8 fills the statistics).

| Field | Type | Meaning |
|---|---|---|
| `slot`, `sample_id` | int, string\|null | from the request |
| `completed` | bool | every requested stage finished normally |
| `abort` | null \| `{step, reason, fault}` | where and why the cycle stopped early: `step` = the stage, `reason` = `stopped` \| `fault` \| `timeout`, `fault` = the fault code or `null` |
| `end_reason` | string \| null | **why the water phase ended**: `volume_reached`, `clogged`, `duration_reached` (`max_duration_s` reached on a run **without** a volume cap: a normal end), `timeout` (`max_duration_s` reached **before** the target volume: a sample problem), `pressure_cap`, `fault`, `stopped`. If the cycle was interrupted *before the water phase ended* (including during the purge), `end_reason` = `stopped`/`fault`/`pressure_cap`. If it was interrupted *after*, `end_reason` keeps the water result and `abort` says what failed (e.g. the shield). `null` only when `stages.sample` was `false`. |
| `volume_ml` | number | filtered volume, water phase only (turbine meters spin on air, FW8) |
| `duration_s` | number | water phase duration (the filtration time) |
| `cycle_s` | number | the whole command, moves included |
| `purge_ml_target`, `purge_ml_measured` | number | purge volume asked / measured |
| `p_in` | `{max, mean, final}` | bar gauge, water phase |
| `flow_ml_min` | `{mean, min, final}` | water phase, after the settling window (FW8) |
| `phases` | `[{name, duration_s}]` | pumping phases in order (≤ 12 entries). Names: `prime`, `purge`, `water`, `air_push`, `shield`, `empty`. Moves are not phases. |
| `shield_ms` | int | micro pump on-time actually run |
| `t_end_to_shield_s` | number | end of the water phase → start of the shield |
| `faults` | `[{code, msg, phase}]` | faults during the command (≤ 8) |
| `warnings` | `[{code, msg, phase}]` | warnings during the command (≤ 8; one more than 8 → the last entry is `{"code":"MORE","msg":"<n> more"}`) |
| `sensors` | `[{name, value, unit, stat}]` | extra sensors (≤ 8): `stat` ∈ `mean`, `min`, `max`, `final`. The host turns each into a column `<name>_<stat>_<unit>` (HO2), so a new sensor needs no protocol change. |
| `p_out`, `q_out`, `dp`, `r_filter`, `clog_index` | null | **reserved for P2** (second pressure sensor, second flowmeter, clog analytics). Always `null` in v1. |

### 6.3 `PURGE`, `SAMPLE`, `DNA_SHIELD`, `EMPTY`, `PRIME_SHIELD` results

The same object. `slot` is the slot used (0 for purge/empty). Fields of stages that did not run are `null` (or `[]`). `end_reason` is set by `SAMPLE` only.

### 6.4 Size

With the bounds above the record stays under ~1.7 kB, below `max_line`. `msg` strings in results are ≤ 60 characters.

### 6.5 Units

| Quantity | Unit | Field names |
|---|---|---|
| pressure | bar, gauge | `p_in`, `p_out`, `dp`, `pressure_cap_bar` |
| volume | mL | `*_ml` |
| flow | mL/min | `flow_ml_min`, `q_out` |
| time | s (float) unless the name ends in `_ms` (int) | |
| angle | degrees 0–360 | `angle_deg` |
| pump power | % of the commanded range | `pump_pct` |
| temperature | °C (`degC` in `sensors[].unit`) | |

## 7. Request ids and retries

- The Due remembers the **last 16 ids** of *accepted* commands marked "dedupe" in §5.1, with their `cmd` and their final `res` (or "running").
- Checks happen in this order, so that the answers are deterministic:
  1. line too long → `line_too_long`; not JSON / not an object → `bad_json`;
  2. envelope (`t`, `id`, `cmd`, `args` types) → `bad_request`;
  3. **dedupe**: the id is remembered →
     - same `cmd`, finished: the **stored `res` is sent again, unchanged**; nothing runs (this holds in any mode);
     - same `cmd`, still running: the `ack` again with `"dup":true`; the single `res` comes when it ends;
     - different `cmd`: `id_conflict`;
  4. reserved → `not_available`; unknown → `unknown_cmd`;
  5. args → `bad_args` (`err.arg` names the first bad argument);
  6. mode → `wrong_mode` (`err.mode`; plus `err.cause` in `RECOVERY_REQUIRED`/`FAULT`);
  7. preconditions → `sensor_unavailable`, `lock_unknown`, `not_aligned`, `interlock`, `cause_persists`, `confirm_required`;
  8. accepted: remembered (if "dedupe"), `ack` (if long), run.
- **Refusals are not remembered**: nothing happened, so a retry with the same id is evaluated again. Read-only commands and `STOP` are never remembered.
- **So a retried `RUN_CYCLE` never runs twice** as long as the Due has not rebooted. The table is RAM only: after a reboot (new `boot_id`) the Due is in `RECOVERY_REQUIRED`, and the host MUST NOT resend any id from before the reboot; it marks the interrupted slot `SUSPECT`.
- The host MUST NOT reuse an id for a different request. The Due checks `cmd` only, not the args.
- **Storage (RAM budget).** The 16 are *request ids*, not samples: `ROTATE`, `LOCK`, `CLEAR_FAULT`, `RECOVER`, ... each use one. Most answers are short (< 256 bytes); only procedure records are big (~1.7 kB). So the Due keeps:
  - for each of the 16 ids: `id`, `cmd`, and either the full `res` (when ≤ 256 bytes) or a **summary** (`ok`, `end_reason`, `completed`);
  - the **full `res` of the last 4 procedures** (`RUN_CYCLE`, `PURGE`, `SAMPLE`, `DNA_SHIELD`, `EMPTY`, `PRIME_SHIELD`).

  About 16 × 300 B + 4 × 2 kB ≈ 13 kB of the Due's 96 kB (FW0 uses 4.6 kB). The host retries within seconds or minutes, so the full record is always there in practice. If a retried id's full record was already evicted, the Due answers `result_expired` with the summary: it still **never runs the command again**, and the host marks the record incomplete (it still has the `tel` and events it logged).

## 8. Error codes

| `code` | When | Extra `err` fields |
|---|---|---|
| `bad_json` | not valid JSON, or not a JSON object (id `null`) | |
| `line_too_long` | > 2048 bytes (id `null`) | `max` |
| `bad_request` | envelope wrong: `t` ≠ `"req"`, `id` missing / not a string / bad characters or length, `cmd` missing or not a string, `args` not an object. `id` is echoed only if it is a valid id. | `field` |
| `unknown_cmd` | `cmd` not in §5.1 | |
| `not_available` | reserved command, or a command that needs a capability this Due does not have | |
| `bad_args` | missing required arg, unknown arg, wrong type, out of range, purge below the minimum, `slot` not allowed | `arg` |
| `proto_mismatch` | `HELLO` with another major version | `proto` |
| `wrong_mode` | not accepted in the current mode (§5.2) | `mode`, `cause?` |
| `id_conflict` | id remembered for another `cmd` | `cmd` (the remembered one) |
| `sensor_unavailable` | the command needs a sensor that is absent or not trusted (`P_in` for every pumping command; `q_in` for a volume target) | `sensor` |
| `lock_unknown` | O-ring state unknown: run `ACTUATOR_HOME` (or `RECOVER`) | |
| `not_aligned` | the manifold is not on a slot | `angle_deg` |
| `interlock` | locked and not safe to unlock (pump running or `P_in` above the unlock limit), after the FW4 wait | |
| `cause_persists` | `CLEAR_FAULT` while the cause is still there | `fault` |
| `confirm_required` | `RECOVER` needs to move and `confirm_moving` is false | `needs` |
| `recovery_failed` | a `RECOVER` step failed | `step`, `steps` |
| `result_expired` | a retried id whose full result was evicted (§7); the command is **not** run again | `cmd`, `summary` `{ok, end_reason, completed}` |
| `stopped` | a motion command was interrupted by `STOP` | |
| `fault` | a motion command was interrupted by a fault | `fault` |
| `internal` | a firmware bug (should never happen): logged, safe state | |

The host MUST treat an unknown `code` like `internal`.

## 9. Heartbeat and link loss

- The host sends `PING` every **5 s** (`limits.ping_period_s`) while connected, also while `BUSY`. Any valid request resets the Due's link timer.
- **Link lost** = no valid request for **30 s** (`limits.link_timeout_s`). The Due then:
  - raises the warning `LINK_LOST` (an `evt link lost`, a console log line, `STATUS.link_ok = false`, and a warning in the running command's record);
  - **finishes the running command normally**, including the shield and the empty stages. It never aborts a sample because of the link: stopping mid-sample would lose the filter and leave water in the lines, and every stage has its own timeout. Faults still stop it, as always;
  - keeps the result (dedupe table + `STATUS.last`);
  - **never starts anything new**. The Due has no queue and no schedule: a request is executed when it arrives or refused.
- **Link restored** = the next valid request: `evt link restored`. The host then calls `HELLO` (reboot check), `STATUS`, and, if it was waiting for a result, resends the same request id to get the stored `res` (§7).
- The Due never reboots, stops or changes mode because of the link.

## 10. Timing and host timeouts

Rule: for a long command the host timeout is **`ack.max_s` + 30 s**, counted from the `ack`. No `ack` within **2 s** → resend the same id (safe, §7). Short commands: timeout **2 s**, then resend (up to 3 times, then treat the link as lost).

Expected durations with the FW0 values (open-loop lock = 210 × 1.1 steps at 33 steps/s ≈ 7.0 s; unlock ≈ 6.4 s; rotation ≤ 15 s timeout). "Bound" = what the firmware enforces today; FW4 changes the purge, sample and shield values.

| Command | Typical | Bound (FW0 → after FW4) |
|---|---|---|
| `HELLO`, `PING`, `STATUS`, `GET_INFO`, `CLEAR_FAULT` | < 50 ms | 2 s |
| `STOP` | safe state < 100 ms, `res` < 200 ms | 2 s |
| `LOCK` / `UNLOCK` | 7 s / 6.5 s | 20 s |
| `ROTATE` | 6.5 s unlock + 3–10 s + 7 s lock ≈ 17–24 s | 6.5 + 15 + 7 s (+ the FW4 unlock wait) |
| `ACTUATOR_HOME` | ~7 s | 20 s |
| `RECOVER` | < 1 s with nothing to move; ~30 s with align + home | 60 s |
| `PRIME_SHIELD` | `duration_ms` + ~30 s | `duration_ms` + 60 s |
| `PURGE` | ~30 s of moves + purge volume / flow (424 mL at ~100 mL/min ≈ 4 min) | 30 s + 180 s → 30 s + FW4 purge timeout |
| `SAMPLE` | moves ~90 s + water + air push ≤ 120 s | 90 + 180 + 120 s → 90 s + `max_duration_s` + 120 s |
| `DNA_SHIELD` | moves ~60 s + shield (25 s today, F4 → 250 s) | 60 s + `duration_ms` |
| `EMPTY` | 30 s + ≤ 5 min | 30 + 300 s |
| `RUN_CYCLE` 2 L | **~25–40 min** (flow unknown until HW3) | the sum of the stages: ≈ 17 min + `max_duration_s` |

These are budgets, not promises: FW7 computes `ack.max_s` from the real Settings.h values, and HW3/VA1 replace the typical values with measurements.

## 11. Legacy `main_program()` commands → v1

The legacy text protocol (docs/COMMANDS.md §3) stays behind `LEGACY_PROTOCOL_ENABLED` (default `false`) from FW7 and is removed in P2.

| Legacy command | v1 | Notes |
|---|---|---|
| `reloadManifoldFunction` | **dropped** | The host owns the slot inventory (F1, HO2 reload flow); the Due forgets it at every reset anyway. |
| `purgeSterivexFunction` | `PURGE {volume_ml: null}` | Same `step_purge()`: ends locked at purge, as before; from FW4 the next rotation unlocks first (F40). Uses the §3.7 purge volume (FW4). |
| `purgeContainerFunction` | `PURGE` | The same function; the container no longer exists. |
| `purgePipesFunction` | **dropped** | Waits for the START button, uses the removed V23 valve and the container (F42, F51). A person-at-the-machine maintenance flush: a console command if still wanted (FW4 asks; P2). |
| `fillContainerFunction` | **dropped** | The container and its switch D27 are gone; it runs the pump 11 min at 100 % without cap polling (F42). |
| `sampleFunction<depth>` | `RUN_CYCLE {slot, purge_ml, volume_ml, max_duration_s}` | The host chooses the slot (the Due's `FILL_ORDER` search is legacy only). Depth dropped: fixed intake, spool disabled. Fixes F8, F24, F39. |
| `rollingFunctionUp`, `rollingFunctionDown`, `stopRolling` | `SPOOL_*` → `not_available` | Spool broken (`SPOOL_USE = false`); reserved for P2. |
| `startDNA` / `stopDNA` | `PRIME_SHIELD {duration_ms}` / `STOP` | An open-ended micro pump run has no timeout (rule 4): v1 only has timed runs. |
| (console) `abort` | `STOP`, then `CLEAR_FAULT`, `RECOVER`, `EMPTY` | The legacy abort ran a purge and an empty; v1 leaves that choice to the operator. |

## 12. Versioning and extension

- `HELLO.data.proto` = `"1.<minor>"`. The host refuses a Due with another major version.
- **Allowed within v1 (minor bump):** new commands; new optional args; new fields in any message; new `evt` types; new fault, warning and error codes; new `caps` keys; new `sensors[]` names; filling a reserved field with a value.
- **Needs v2:** removing or renaming anything; changing a type, a unit or the meaning of a field; making an optional arg required; changing the check order of §7.
- Receivers ignore unknown fields. The Due refuses unknown *args* (safety: a misspelled `volume_ml` must not be silently ignored), so the host sends a new optional arg only when `proto` minor says it is supported.

## 13. Test vectors (`protocol/vectors.json`)

The file has three parts:

- **`requests`**: a host line (`line`, sent as is, so it can be malformed) + the Due `state` before it (defaults in `defaults.state`) → `expect`: the messages the Due sends **synchronously**, in order. For a long command that is the `ack` only; its result is a `messages` example. `line_gen` (instead of `line`) builds long lines: `{"prefix", "fill", "length", "suffix"?}` = `prefix` + `fill` repeated + `suffix`, exactly `length` bytes, without the `\n`. Keys in `state` that a test does not model (e.g. `history`) are comments.
- **`messages`**: Due → host lines the host parser MUST accept (`valid: true`), including results with `null` fields and unknown fields/event types (forward compatibility).
- **`sequences`**: whole exchanges (RUN_CYCLE happy path, STOP mid-purge, reboot mid-cycle) for the host client and the simulator.

Matching rule for `expect`: every key in the expected object must be present in the actual message with an equal value; keys not listed are not checked. The special strings `"$num"`, `"$str"`, `"$bool"`, `"$any"` match any value of that type (`"$any"` includes `null`). Arrays compare element by element with the same rule and must have the same length. `expect: []` = the Due sends nothing.

The C++ side (FW7) drives the dispatcher with a fake machine set from `state`; the Python side (HO1) checks its models and its simulator against the same file.

## 14. Decisions taken in FW1, and questions deferred

Answered by the user on 2026-09-25:

1. `max_duration_s` limits the **water pumping phase** only (§5.8).
2. `end_reason` has **`duration_reached`** for a run without a volume cap that reaches `max_duration_s` (a normal end); `timeout` stays for a run that misses its target volume (§6.2).
3. `GET_INFO` is accepted while `BUSY` (read-only; ROADMAP FW7 updated).
4. `STOP` from `IDLE` latches `USER_STOP` and needs `CLEAR_FAULT` + `RECOVER` (strict §3.2).
5. **Rest state = O-ring locked.** Unlocked only to rotate; every rotation is unlock → rotate → lock; every command ends locked (§5.1). Implemented in FW4 (ROADMAP updated: §3.2, FW4 item 4, LA3, HW7 compression-set check).
6. A `purge_ml` below the §3.7 minimum is **raised to the minimum** with the warning `PURGE_CLAMPED`.
7. Dedupe storage: 16 ids with short answers or summaries + the full records of the last 4 procedures, ≈ 13 kB (§7).
8. `device_id` = `cowas-V1`. `sample_id` stays an optional pass-through arg.

Deferred (written into the ROADMAP prompts of the step that will answer them):

| Question | Answered in |
|---|---|
| `PRIME_SHIELD`: where the primed shield goes (the purge slot?) and how much/how long (`PRIME_SHIELD_MS`, PLACEHOLDER) | LA3 (volume), then FW7 (implementation). **LA3 recommendation (2026-09-25, docs/LA3_PROTOCOL.md §5.4):** the purge slot; `PRIME_SHIELD_MS` = the empty-line fill time × 1.25 from `shield_calibrate` (not measured yet, stays PLACEHOLDER) |
| Does an O-ring kept compressed for days between samples take a set and leak? | HW7 (7-day locked hold, then `leak_hold`) |
| Do filter outlets still need caps/check valves with the O-ring locked at rest (F29)? | LA3. **Decision rule and interim recommendation in docs/LA3_PROTOCOL.md §5.3** (caps in Pilot A; order luer check valves as a hedge for Pilot B); the retention data decide |
