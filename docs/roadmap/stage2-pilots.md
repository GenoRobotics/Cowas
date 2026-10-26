# CoWaS roadmap, stage 2: lake pilots

**Goal:** first eDNA data from Lake Geneva in two pilots.

- **Pilot A (supervised):** 1–2 days from a pier, pontoon or boat, on battery. Someone stays on site. Fixed intake depth. Blanks and manual comparison samples. Filters go to the lab.
- **Pilot B (unattended):** 1–2 weeks, 14 samples, on battery, with remote monitoring and alerts.

**Stages**
- Stage 1 = [stage1-FINISHED-prototype.md](stage1-FINISHED-prototype.md): the operator test, done apart from the field.
- **Stage 2 = this file:** the two pilots.
- Stage 3 = [stage3-full-system.md](stage3-full-system.md): work after the first data.

"ROADMAP §N" in the code and older notes means a section of this file. Findings (F…) are in [findings.md](../logs/findings.md) and decisions (D…) in [decisions.md](../logs/decisions.md). The old step prompts are in [the archive](../archive/roadmap-stage2-prompts-2026-10-23.md); the ideas that matter are in §7 below.

---

## 0. How to use this file

- **One step = one branch from `cowas-v2` = one PR**, with its unit tests and its hardware check.
- Before a step, read the sections it names and the findings it fixes. Ask before changing anything outside the step.
- **Priorities:** **P0** = needed for Pilot A. **P1** = needed for Pilot B. **P2** = after the first data (stage 3).
- **Owners:** Quentin does the firmware, host, hardware and validation. Felix does the calibrations (shield, DNA pump).
- The rules in §6 apply to every step.

---

## 1. Milestones and critical path

| # | Milestone | Done when |
|---|---|---|
| M1 | Bench-safe | Pressure reads correctly, safety fixes in, a full production cycle runs on the bench under supervision |
| M2 | Sample quality known | Filtered volume, shield volume, residual water, carry-over and cross-talk measured. SOPs written |
| M3 | Host-driven bench run | A 14-sample mission runs from the host and gives a sample CSV. STOP, E-stop and power cut recover correctly |
| M4 | **Pilot A** | Supervised 1–2 days on the lake. Filters and CSV go to the lab |
| M5 | **Pilot B** | Unattended 1–2 weeks, remote monitoring and alerts |

**Critical path:** FW3 (pressure) → FW4 → FW5 → FW6 → FW7. FW3 needs a pressure sensor on the line (HW4). The host runs against a simulator, so it does not wait for FW7. The laptop is the host for Pilot A; the Pi is optional (HO4).

**If the schedule slips, cut scope, not safety.** You can cut: replicates, the clog end condition, the Pi. You cannot cut: FW3, FW4, FW5, FW6, the relief valve, the E-stop, LA3, LA4.

---

## 2. Why this plan

1. **Built around the pilots,** not the full target system. The rest goes to stage 3.
2. **A lab track.** The main risks to usable data are biological: carry-over through the shared intake, residual water diluting the shield, shield draining from the filters, no blanks. They are cheap to measure now and expensive after a deployment.
3. **The pressure sensor is on the critical path.** No production cycle can run without a trustworthy reading.
4. **The protocol spec comes early (FW1)** so the host works in parallel.

---

## 3. Design decisions

### 3.1 Who does what

- **The Due:** every sensor and actuator, the safety checks, the procedures. No clock, no storage: it forgets everything on reset.
- **The host** (laptop, later a Pi): schedule, slot inventory, sample records (CSV), device logs, alerts.
- **Rule:** the Due never starts a sample on its own, and the host never switches a valve directly.

### 3.2 Safety layers and recovery

| # | Layer | Pilot | Covers |
|---|---|---|---|
| 0 | Mechanical pressure-relief valve, about 2.5–3 bar, to waste | A | Everything else failing, including a wrong sensor |
| 1 | Hardware E-stop cutting the 24 V actuator rail (latching switch, no relay). A second contact to a Due input | A / B | Firmware hang, link down |
| 2 | Due watchdog | A | Stuck loops |
| 3 | Firmware supervisor `service()`: cap, sensor plausibility, timeouts, rotation interlock | A | Normal faults |
| 4 | Software STOP (console `stop`, host `STOP`), always accepted | A | Operator sees a problem |
| 5 | Host checks and alerts | B | Unattended operation |

- **Safe state:** pump and micro pump off, valves closed, manifold stopped, **O-ring locked** if it is locked.
- **Rest state:** the O-ring is locked whenever the machine waits. It is unlocked only to rotate: unlock, rotate, lock. Every procedure ends locked ([protocol §5.1](../software/protocol.md)).
- **Recovery:** after any fault, STOP or boot, procedures are refused until `RECOVER` passes: cause gone, encoder OK, O-ring state known (home it if unknown), valves idle. The host marks an interrupted slot `SUSPECT`.

### 3.3 Pressure cap

The only safety value is the cap: **3.6 bar** (`STX_MAX_PRESSURE`, user decision 2026-10-19). It acts on `P_in`, the pressure before the filter. An implausible reading stops the pump. The sensor is calibrated to 2.0 bar only (F136), so the upper range is not yet measured.

### 3.4 Two serial ports

| Port | Used by | Carries |
|---|---|---|
| Native USB | The host, always connected | Protocol v1 (JSON lines), a copy of every log line |
| Programming port | A laptop console | The test menu |

Opening the Programming port **resets the Due**, so a host restart would reboot it mid-sample. The Native port does not reset, except on the 1200-baud "touch", which the host must never use.

### 3.5 Controller modes

```
BOOT -> self-check -> RECOVERY_REQUIRED -> (RECOVER ok) -> IDLE
IDLE <-> BUSY      (a procedure runs; only STOP, STATUS, PING, HELLO, GET_INFO are accepted)
IDLE <-> SERVICE   (console on the Programming port; `service` to enter, `exit` to leave)
any  -> FAULT (latched) or ESTOP -> clear + RECOVER -> IDLE
```

Boot goes to RECOVERY_REQUIRED with the O-ring UNKNOWN. The boot default is SERVICE until FW7; then IDLE (host mode). Today `setup()` ends in the blocking test menu (F23).

### 3.6 Slots and sample records

- **Slot states (host):** `EMPTY`, `READY`, `IN_PROGRESS`, `SAMPLED`, `FAILED`, `DISABLED`, `SUSPECT`.
- **Filter roles:** `sample` (with a replicate number), `equipment_blank` (DNA-free water through the whole machine). Later `negative_control` and `positive_control` (P2).
- **One CSV row per filter:** sample ID, deployment, slot, role, replicate, start and end time (UTC), filtered volume, duration, `end_reason`, pressure max and mean, flow, purge volume, shield time, time from filtration end to shield, water temperature, fault codes, warnings, firmware and host versions, notes. Column names to agree with the lab.
- No code hard-codes 14 or 15 slots. The count comes from the Due (`GET_INFO`).

### 3.7 eDNA quality rules

These are requirements, not options.

1. **Purge large enough for the dead volume.** The rule was 3 × the dead volume (F110 suggests 5 for a mixed volume). Stage 1 uses the user's values: **500 mL without the spool, 3 L with it** (2026-10-26, D50). The dead volume with the spool is 2.57 L (30 mL intake + 2.54 L spool), so 3 L is about 1.2 × it, below the 3 × rule. The user accepted this for stage 1. The stage 2 rule is still to decide (§7b).
2. **Filtered volume** from the calibrated flowmeter, checked by weighing the outlet (VA1). Every filter gets an `end_reason`: `volume_reached`, `clogged`, `timeout`, `pressure_cap`, `fault` or `stopped`.
3. **Residual water and shield volume** measured by weighing (LA3).
4. **Blanks in every deployment:** one equipment blank at the start and one at the end, plus one manual field blank.
5. **Manual comparison samples** in Pilot A: 2–3 manual filtrations at the same place, time and volume. This is the key result of Pilot A.
6. **Handling:** gloves, the cleaning SOP, the time from sampling to retrieval recorded, caps on, cool and dark.
7. **Cross-talk between slots** measured with a dye (LA4), including while other filters sit in the manifold.
8. **Biosecurity:** quagga mussels are in Lake Geneva. Clean and dry all wetted parts before use in another water body.

### 3.8 Power (battery only)

- **Known:** the lead screw self-locks, so the stepper can be off while the O-ring is locked (F34).
- **Not yet measured:** the pump driver's input range (F25) and every power figure (HW5).
- **Estimates:** pump ≤ 12 W, 20–25 min per sample (about 5 Wh per sample). Due and logic 1–1.5 W always on. Pi and modem 3–7 W always on unless duty-cycled. The always-on load matters more than the pump.
- **Pilot A:** about 0.4–0.5 kWh; a 12 V 50 Ah LiFePO4 (about 640 Wh) has margin. **Pilot B:** 1.5–2.5 kWh always on, or 0.6–0.9 kWh with the Pi duty-cycled. Decide after HW5.
- **Voltage:** a 24 V LiFePO4 pack charges to 29.2 V. Use a regulated 12 V → 24 V boost for the actuators and a 5 V buck for the Pi. **Never power the Due from raw 24 V** (F32).

### 3.9 Not in the pilots (stage 3)

Second pressure sensor and flowmeter, PID variants, runtime parameters, the firmware restructure, the web app and API, full notifications, environmental sensors beyond water temperature, the spool and depth, automated controls, firmware updates from the Pi, the Jetson.

---

## 4. Findings the steps must fix

Only the findings a step still has to fix. Their status is in [findings.md](../logs/findings.md). The full F1–F36 texts are in [the archive](../archive/roadmap-stage2-details-2026-10-23.md).

| Finding | What | Step |
|---|---|---|
| F1 | Slot availability is RAM-only; after a reset, used filters look free | HO2 |
| F2 | Panel Pump mode: the cap is checked at start only | FW4 |
| F4 | Shield time is `FILL_STERIVEX_TIME / 10` (25 s), a test value | FW4, LA3 |
| F5 | Some rotation commands ignored the O-ring lock; each rotation command must be checked | FW2, FW4 |
| F8 | The legacy Pi link blocks for a whole sample; no acknowledgements | FW7, HO1 |
| F10 | Console and Pi share one port at 9600 baud | FW7 |
| F16 | Hard-coded slot counts in old code | FW2 (new code), P2 (rest) |
| F23 | `setup()` waits in the test menu after any reset | FW6, FW7 |
| F24 | The legacy Pi link treats the first log line as "done" | FW7, HO1 |
| F25 | The DAC outputs about 0.55–2.75 V, not 0–3.3 V; the relay is the real stop | HW1 |
| F28 | The air push cannot empty a wet Sterivex; residual water is unknown | LA3, FW4 |
| F29 | After a cycle the O-ring unlocks and used filters stay open for days | LA3, LA4 |
| F30 | The purge is smaller than the dead volume of a realistic intake (§3.7) | FW4, LA4 |
| F32 | No raw 24 V into the Due; cold-boot reliability not tested | HW1, HW5 |
| F33 | 9600 baud limits telemetry; Native USB removes the limit | FW7 |
| F34 | The stepper stays enabled while locked (holding current) | FW4 |
| F35 | The field intake has no strainer; debris and mussels can jam the flowmeter | HW6 |
| F36 | SPI bus shared by the ABP and the encoder: long cables give bad frames | HW1, FW3 |

---

## 5. Test strategy

**Every step ships tests. A step without tests is not done.**

| Level | What | When |
|---|---|---|
| 1. Firmware unit tests | Pure logic in `src/Core/` (`pio test -e native`) | Every commit |
| 2. Host unit tests | Domain logic, storage, scheduler with a fake clock (`pytest`) | Every commit |
| 3. Contract tests | The same `protocol/vectors.json` checked by C++ and Python | Every commit |
| 4. Simulation tests | The host against `FakeController` with injected faults | Every commit |
| 5. On-device checks | Console commands print `TEST\|<name>\|<id>\|PASS/FAIL/SKIP\|k=v`; `tools/session_log.py` saves them | After each flash |
| 6. Acceptance campaigns | Scripted runs with numeric pass criteria (VA1, VA3) | Before each pilot |
| 7. Field checklists | Pilot procedures on paper | On site |

Conventions: a safety decision is a pure function with a truth-table test. Every threshold has boundary tests (just below, at, just above, NaN, sensor absent). Before moving math out of a hardware file, record its outputs (golden values) and assert them exactly. Checklists give PASS in numbers.

---

## 6. Rules every step follows

These are copied into [CLAUDE.md](../../CLAUDE.md).

1. Behaviour changes only in steps that say so, each listed in the PR description.
2. Nothing is deleted without evidence and the user's approval. Git tag `baseline-v0` keeps everything.
3. Pure logic in `src/Core/`, unit-tested. Hardware code stays thin.
4. Safety is never weakened:
   - the pump never runs above the cap; an implausible pressure stops it;
   - no rotation while locked or while the lock state is unknown;
   - every procedure has a timeout;
   - STOP and E-stop are always accepted;
   - safe state = pumps off, valves closed, O-ring locked;
   - recovery is required after every stop and boot (from FW6);
   - placeholders stay marked `PLACEHOLDER`;
   - the one exception is the sensorless pump test mode: explicit, test-only, low power, time-limited, never reachable from production.
5. A failing non-safety sensor: keep working, warn, flag the record. Never fail silently.
6. No hard-coded slot counts in new code.
7. Every step ends with: `pio run -e due` and `pio test -e native` passing, a hardware checklist with numeric pass criteria, and the checks in §7a updated.
8. The eDNA rules (§3.7) are requirements.
9. Claude does not flash, commit, push or delete files without asking.

---

## 7. Steps

Status: ☑ done, ☐ open. Stage 1 already covers parts of FW2–FW4 (see [stage 1](stage1-FINISHED-prototype.md)).

| ID | Step | Pri | Needs | Status |
|---|---|---|---|---|
| FW0 | Baseline: tests, CI, pin map, command list | P0 | — | ☑ |
| FW1 | Protocol v1 spec and test vectors | P0 | FW0 | ☐ |
| FW2 | Mechanical test suite, lock tri-state, `TEST\|` logging | P0 | FW0 | ☐ (partly stage 1) |
| FW3 | Pressure sensor on `P_in`: driver, safety and PID | P0 | FW0, HW1, HW4 | ☐ (core in stage 1) |
| FW4 | Safety and sample-quality fixes | P0 | FW2, FW3 | ☐ (part in stage 1) |
| FW4a | One implementation per physical action, used by tests and the run | P0 | FW4 | ☐ |
| FW5 | Supervisor `service()`, `wait_ms()`, watchdog | P0 | FW4 | ☐ |
| FW6 | STOP, safe state, fault latch, RECOVER, boot, modes | P0 | FW5 | ☐ |
| FW7 | Protocol v1 in firmware on Native USB; boot into host mode | P0 | FW1, FW6 | ☐ |
| FW8 | Sample record and minimal clog end condition | P0 | FW7 | ☐ |
| FW9 | Unattended hardening: E-stop input, leak sensor, home switch, water temperature | P1 | FW8, HW7, HW8 | ☐ |
| HO1 | Host skeleton: client, transport, simulator, CLI | P0 | FW1 | ☐ |
| HO2 | Slots, deployments, sample records, CSV export | P0 | HO1 | ☐ |
| HO3 | Mission runner, scheduler, cold-boot resume | P0 | HO2 | ☐ |
| HO4 | Deploy on the Pi | P1 | HO3 | ☐ |
| HO5 | Remote access and alerts | P1 | HO4 | ☐ |
| LA1 | Lab meeting | — | — | dropped 2026-10-23 (values set with Felix) |
| LA2 | SOPs: loading, cleaning, labels, chain of custody | P0 | — | ☐ draft [filter-sop.md](../lab/filter-sop.md) |
| LA3 | Shield dose, residual water, shield retention | P0 | FW4 | ☐ |
| LA4 | Carry-over and slot cross-talk | P0 | FW4, HW6 | ☐ |
| HW0 | Order the parts (§11.1) | P0 | — | ☐ |
| HW1 | Electrical audit | P0 | FW0 | ☐ part 1 done 2026-09-25 |
| HW2 | Relief valve, E-stop in the 24 V line, fuses | P0 | HW1 | ☐ |
| HW3 | Flowmeter: safe input, calibration, real flow | P0 | HW1 | ☐ (factor done, F6 closed) |
| HW4 | Install and plumb the pressure sensor | P0 | HW0 | ☐ |
| HW5 | Power: battery, rails, measured budget, cold boots | P0 | HW1 | ☐ |
| HW6 | Field fluidics and splash-proof box | P0 | HW3 | ☐ |
| HW7 | O-ring lock endurance; home switch (P1) | P0/P1 | FW2 | ☐ |
| HW8 | Pilot B enclosure, leak sensor, temperature probe | P1 | HW6 | ☐ |
| HW9 | Manifold encoder coupling: mechanical lock (F187, F191, F203) | P0 | HW6 | ☐ |
| VA1 | Bench acceptance campaign (go or no-go for Pilot A) | P0 | FW8, HO3, LA3, LA4, HW2–HW6 | ☐ |
| VA2 | **Pilot A** | P0 | VA1 | ☐ |
| VA3 | 72 h soak on battery, Pilot B readiness | P1 | all P1 | ☐ |
| VA4 | **Pilot B** | P1 | VA3 | ☐ |

### 7.1 Firmware

**FW0 · Baseline (P0, done)**
- Native test environment, CI (`pio run -e due`, `pio test -e native`), manifold maths extracted to `src/Core/` with golden values.
- Pin map and command list ([pinout.md](../hardware/pinout.md), [commands.md](../software/commands.md)).
- Done when: both builds pass, and the firmware diff is only the maths extraction and comments.

**FW1 · Protocol v1 spec (P0)** · needs FW0 · [protocol.md](../software/protocol.md)
- One JSON object per line on the Native port. Every human log line is also sent as `{"t":"log"}`.
- Messages: `req` (with an id), `ack` (at once for long commands), `res`, `evt` (mode, fault, step), `tel` (about 1 Hz while busy).
- Commands: `HELLO` (version, git hash, mode, recovery needed, capabilities), `STATUS`, `PING`, `STOP` (always accepted), `GET_INFO` (slots, purge slot, fill order: the host never assumes 14), `ROTATE`, `LOCK`, `UNLOCK`, `ACTUATOR_HOME`, `CLEAR_FAULT`, `RECOVER`, and the main one, **`RUN_CYCLE {slot, volume, stages}`** = purge → sample → shield → empty → unlock for one filter.
- Result fields are nullable when a sensor is absent. Reserved for P2: `p_out`, `q_out`, `clog_index`.
- Rules: the Due remembers the last 16 request ids and returns the stored result for a repeat (a cycle never runs twice). If the link drops, the Due finishes or aborts safely and starts nothing new.
- `protocol/vectors.json`: 30+ examples, including malformed lines, wrong types, commands refused in a mode, a repeated id.
- Done when: the C++ and Python tests both pass every vector. Open questions are listed for the user, not guessed.

**FW2 · Mechanical test suite and test logging (P0)** · needs FW0
- The lock is tri-state: LOCKED, UNLOCKED or UNKNOWN (UNKNOWN at boot). `ACTUATOR_HOME` runs only when the manifold is within tolerance of a slot.
- Every test prints `TEST|…` lines. Reuse the production functions; do not copy them.
- Test commands (each with `x` to abort, `auto` for no pauses, `from=`/`count=`): `lock_cycle`, `lock_endurance` (for HW7), `rotate_all dry`, `encoder_repeat`, `valve_check`, `leak_hold`, `multi_sample`, `full_cycle`, `full_all`, `shield_only`.
- Sensorless pump mode (F17): only inside a helper, after the operator types `NOSENSOR`, time-limited and low power. Proven by grep to be unreachable from production.
- Pure logic with native tests: slot iteration, the open-loop stop decision, the "may rotate" and "may home" rules.
- Done when: the bench checks in §7a for FW2 pass.

**FW3 · Pressure sensor on `P_in` (P0, critical path)** · needs FW0, HW1, HW4 · [F27](../logs/LOG.md)
- Range from the part number (060PG = 0–60 psi, 150PG = 0–150 psi), counts 1638–14745 (10–90 % of 2^14), psi → bar. Remove the fixed 10-bar constants.
- Status bits: `00` ok; `10` stale = keep the last value and fail only after a timeout; `01` / `11` implausible.
- Before blaming the sensor for "invalid SPI response": log a histogram of status bits over 1000 reads, at 100, 400 and 800 kHz, and check the CS timing and the encoder transactions.
- Delete the analogue A2 path (`pressure2`, its calibration command) after listing its uses (F44, F50). `pressure` shows `P_in` only.
- Cap and PID read `P_in`. An implausible reading (below −0.3 bar, above range + 10 %, NaN) stops the pump.
- Done when: native tests give the datasheet counts → bar for both ranges, the status handling and the boundaries. Bench: with a temporary cap of 1.0 bar and an outlet clamp, the pump trips at 1.0 ± 0.1 bar.

**FW4 · Safety and sample quality (P0)** · needs FW2, FW3
- The cap decision (reading, cap, bounds → OK / OVER_CAP / IMPLAUSIBLE) is a pure function with boundary tests. The pump control reads `P_in` on every loop (F2).
- Automatic unlock before rotation, as a pure rule: ROTATE / UNLOCK_THEN_ROTATE / WAIT / REFUSE. "Already at the target" must not unlock (the shield step relies on it). Procedures end **locked** (§3.2).
- Volume and timeout: the sample timeout is volume ÷ expected flow × 1.5, bounded. `CtrlPump` returns **why** it stopped (volume, timeout, cap, sensor, aborted) so a timeout never looks like a normal end (F15).
- Purge volume and dead volume as tested functions (§3.7). Log the pressure and duration of each emptying phase so LA3 can decide the air push (F28). Add the flow-factor float fix (F31).
- Shield time: replace `/ 10` with the value from LA3 (F4). The stepper coils go off while locked, and the stepper is re-enabled before a move (F34).
- Remove the V23 valve calls (F51, approved 2026-09-24) after listing every call site.
- Done when: native tests pass; bench: `calibrate_flow`, a lock then a rotation auto-unlocks, a rotation with the pump running is refused, and a reboot makes `slot5` ask for `actuator_home`.

**FW4a · One implementation per action (P0)** · needs FW4
- Write each physical action once, as a brick: `pump_through_filter`, `purge_line`, `empty_line`, `empty_filter`, `shield`. Each returns a result: ok, end reason, measured volume, seconds, max pressure.
- An options struct chooses the behaviour: `log` (every window and table line), `pause` (off in production until decided), `interactive` (wait between steps). The test menu and the run call the same bricks.
- `fill_test` and `full_cycle` become callers. Check: `fill_test volume=200` on the bench before and after gives the same end reason and mL within 5 %.
- Open question for the user: should the production run pause at the limit like `fill_test`?

**FW5 · Supervisor and watchdog (P0)** · needs FW4
- A `service()` call at least every 50 ms, from every wait: the cap check, the deadline check, the watchdog kick, and a `should_abort()` flag. `wait_ms()` returns false on abort.
- Enable the hardware watchdog once, at 4 s. Measure and print the longest gap between `service()` calls. At boot, print the reset cause.
- Done when: a hidden `wdt_test` resets the board and the boot prints `watchdog`; a cap trip during a cycle stops the pump within 200 ms; the maximum gap is below 50 ms.

**FW6 · Stop, safe state, recovery, modes (P0)** · needs FW5 · F71
- `enter_safe_state()` (pumps and valves off, O-ring stays locked, snapshot of the step). One fault enum, a latched fault and a 16-entry ring buffer. Warnings never latch.
- Modes as in §3.5: the console test menu is `SERVICE`, left with `exit` (the old `stop` becomes the emergency stop). The button panel works only in SERVICE.
- E-stop input (if installed): debounced; a broken wire counts as a stop.
- Boot always goes to RECOVERY_REQUIRED. `recover` runs the checklist step by step and stops at the first failure. Every moving command is refused until recovery passes.
- Open question (F71): an unknown lock with the manifold between slots can deadlock. Decide the manual procedure.
- Done when: every mode transition is tested; the console STOP and E-stop, in each phase (pump, rotation, shield, emptying), leave the valves closed and the O-ring locked; 20 cold boots from the battery reach the console.

**FW7 · Protocol in firmware, host mode (P0)** · needs FW1, FW6 · F8, F10, F24, F33, F53
- The parser and dispatch are in `src/Core/`, tested against the vectors and against garbage input (random bytes, 10 kB lines: never a crash, never a command run).
- Host channel: Native USB. Console: Programming port. Output to the host must never block; drop and count lines if the host stops reading.
- `RUN_CYCLE` takes the slot from the host. Long commands: an ack, then events, then the result with the same id. Link watchdog: no `PING` for the timeout → warning; the current procedure finishes safely.
- Boot into IDLE (host mode). Legacy text commands stay behind a flag, off by default.
- Done when: `tools/protocol_smoke.py` passes all commands on the Native port; restarting the host during a purge does not reset the Due; the console still works on the Programming port.

**FW8 · Sample record and clog end condition (P0)** · needs FW7 · F137, F141
- A metrics accumulator fed by `service()`: volume, duration, `P_in` (max, mean, final), flow, per-phase durations, shield time, time to shield. Volume and flow count **only in the water phase** (the flowmeter spins on air).
- Minimal clog rule (water phase only): flow below a floor for 30 s while the pump is at or near full power → `clogged`. The first end condition met sets `end_reason`.
- Done when: a 500 mL clean run gives `volume_reached` within 5 % of the weighed water; a slowly closed clamp gives `clogged`; the air push adds less than 5 mL.

**FW9 · Unattended hardening (P1)** · needs FW8, HW7, HW8
- Enclosure leak sensor (a fault that stops everything). Home switch for the O-ring at "unlocked": each unlock must reach it within a set number of steps. Water temperature probe, logged per sample and every 30 min between samples.
- Done when: each sensor unplugged gives the expected warning or fault; 50 lock cycles with the home switch are clean; the water temperature is within 0.5 °C of a reference thermometer.

### 7.2 Host

**HO1 · Host skeleton (P0)** · needs FW1 · [protocol.md](../software/protocol.md)
- Python 3.11+ package `cowas`, cross-platform. Finds the Native port by VID/PID; never uses 1200 baud. Reconnects.
- Request/response by id, timeouts per command, retry with the same id, a heartbeat.
- `FakeController` (simulator) with injected faults: pressure trip, clog, link drop, reboot mid-cycle, dead flowmeter, watchdog reset.
- CLI: `ports`, `hello`, `status`, `stop`, `recover`, `run-cycle`, `monitor`, `logs`. `--sim` everywhere.
- Done when: the tests pass against the simulator; on the bench `hello` and `monitor` work with the real Due.

**HO2 · Slots, deployments, records, export (P0)** · needs HO1
- Slot states with a fixed transition table. The inventory is sized from `GET_INFO`.
- Deployment: site, coordinates entered by hand, intake depth, hose size and length, operator, filter lot, shield lot. Sampling event: planned time, volume, replicates, role.
- `sample_id` format agreed with the lab. Labels printed from the same ids.
- SQLite with migrations; one transaction per change. At start-up, `IN_PROGRESS` → `SUSPECT`.
- Commands used by the SOP: `cowas slots` (list), `slots load <slots> --filter-lot`, `slots unload` (retrieval time per filter), `cowas clean` (bleach, rinse, DNA-free water).
- Export: one CSV, one row per filter, UTC times plus a local column, a schema version in the header.
- Done when: every legal and illegal transition is tested; data survives a restart; an export matches a golden file.

**HO3 · Mission runner (P0)** · needs HO2
- Plan file or a repeating form. Import the legacy formats with a warning that depth is ignored.
- Before each event: `HELLO` (expected version), `STATUS` idle, no recovery pending.
- Policies: a late event runs only within a set delay. A sample fault (`fault`, `timeout`) marks the slot `FAILED` and retries on the next slot, up to N times. A system fault (STOP, E-stop, cap, leak, recovery needed) pauses the mission; it never retries by itself. A clogged sample is kept, not a fault.
- Equipment blanks: pause before and after, and resume only when the operator confirms.
- Cold-boot resume from the database.
- Done when: a 14-sample plan runs in accelerated simulated time with a power cut, a link drop, a clog, a STOP and a reboot mid-cycle, each with the expected result. Bench: a three-event plan on a bucket.

**HO4 · Pi deployment (P1)** · needs HO3
- Service that restarts after a fault, a stable device name for the Due, the database on the SD card with a nightly backup, a safe power-cut behaviour.
- The clock must be trusted (synced, or a valid RTC) before a mission starts.
- A Wi-Fi fallback for on-site access.
- Done when: a power cut during a mission resumes correctly on the Pi; the Pi's idle and busy current is measured for HW5.

**HO5 · Alerts and remote access (P1)** · needs HO4
- Alert levels: critical (stop, E-stop, cap, leak, recovery needed, link lost for 10 min), warning (clogged or timeout, few READY slots, low disk), info (sample done, mission done).
- Critical alerts repeat every 15 min until acknowledged. Every alert is stored with its acknowledgement.
- Channels behind one interface (for example ntfy with a private topic). A daily status message.
- Remote access over a VPN; telemetry is not pushed over LTE.
- Done when: a simulated critical alert reaches the phone and repeats until acknowledged; a delivery failure is retried.

**HO6 · Sample timing from field records (P1, spec only: not started)** · needs HO3 (planner) and HO4 (trusted clock); needs two record fields first (see Prerequisites); field records from stage 1 and Pilot A.

- **Purpose.** Estimate how long one `run_sample` takes from real field runs, keep that estimate in a table the host owns, check it against each new run, and warn when a planned mission does not fit it. The estimate comes only from measured times. Nothing is typed in as a guess, and the firmware never reads the table.
- **Origin of "longest (while repeatable)".** The request asked for the longest repeatable timing. Confirmed by the user (2026-10-26): the 90th percentile of the repeatable group, which keeps one outlier from setting the value.

**Prerequisites** (before the first estimate counts)
1. **Sample record gets `spool` and `depth_m`.** Today `spool` is only on the TEST line (`spool=installed|absent`) and the record has no depth. Owner: the step that adds the record fields (firmware and session tool). Until then, `spool` is read from the TEST line and depth is not a key.
2. **Trusted PC clock** (HO4). Timings are PC times (`run_start_pc`, `run_finish_pc`).

**Inputs (per session folder `docs/logs/test-logs/<date>/<NN>_<tag>/samples/`)**
- Fields used: `purpose`, `run_start_pc`, `run_finish_pc`, `end_reason`, `target_ml`, `fw`, `calib`, and the prerequisite fields above.
- The step times `t_purge_s`, `t_sample_s`, `t_shield_s`, `t_unlock_s`, `t_total_s` (added to the firmware on 2026-10-23) are kept for a breakdown. No bench record has them yet (checked 2026-10-26), so the total `run_finish_pc − run_start_pc` is the timing used.

**Definitions**
- **Timed run:** a record with `purpose = sample`. The run is the whole `run_sample` (purge, sample, shield, unlock). Its duration is `run_finish_pc − run_start_pc`.
- **Complete sample:** `end_reason = volume`. Other ends (`clogged`, `stalled`, `pressure`, `timeout`, `stop_key`, `skipped`, `refused`, `sensor`) are not timed. They are counted next to the value, so the report shows how often a run stopped early.
- **Key:** the string `spool=<installed|absent>;depth_m=<m or ->;target_ml=<mL>;fw=<fw>;calib=<calib>`. Samples with the same key form one group. A change of `fw` or `calib` starts a new group, so old and new timings are never mixed.
- **Not keys today:** pump power in the purge (fixed at 100 %, D51; add it as a key once it varies) and shield time (a placeholder, F4, constant in every record).
- **Window:** the last **10 session folders** that hold timed samples, in folder-name order (date, then `NN`). All of them if there are fewer. Older folders are not read, so the start stays fast.
- **Repeatable group:** at least **3 complete samples** in the window (PLACEHOLDER), and each within **±15 %** of the group median (PLACEHOLDER). A key without a repeatable group has no value. The estimator says "no timing for this key"; it does not guess.
- **Candidate value per key (`window_s`):** the **90th percentile** of the group's durations in the window (linear interpolation), multiplied by **1.05** (the 5 % margin, applied once), rounded up to the second. Outliers are not in the group (the ±15 % rule), so they never set it.
- **Stored value per key (`value_s`):** the **largest candidate value ever seen** for that key: `value_s = max(value_s, window_s)`. It never goes down. A slow run that leaves the window keeps its effect on the value. Confirmed by the user (2026-10-26): "never decreases, except the outliers". The outliers are excluded by the repeatability rule, not by a separate threshold.

**Stored table: `timings.json`** in the host data folder. One file for all sessions. The run folders are not changed by it. Example (the numbers are illustrative only):
```json
{
  "schema": 1,
  "updated": "2026-11-02T08:15:00+01:00",
  "keys": {
    "spool=absent;depth_m=-;target_ml=100;fw=1.4.0;calib=77.0": {
      "value_s": 1234, "window_s": 1190, "group_size": 5, "margin_pct": 5,
      "window": ["2026-10-23/02_field-test-20261023"]
    }
  },
  "history": [ { "updated": "2026-11-01T08:10:00+01:00", "keys": { "...": "previous values" } } ]
}
```
An update replaces `keys` and appends the old `keys` to `history`. A reset (by hand) is logged in `history` with its reason.

**At each host start**
1. Read `timings.json` (missing: start with no values).
2. Read the window. For each key, compare each complete sample with the stored `value_s`. A sample above it is **reported** ("does not fit the table"). Note that about 10 % of a window lies above its 90th percentile by definition, so these are information, not errors.
3. Compute the candidate `window_s` per key from the window. Set `value_s = max(value_s, window_s)` and write `timings.json` (the values, the inputs used, and a history entry when `value_s` changes).
4. Print a short report: keys updated, keys without a value, samples above the stored value, and the number of early stops per key.

**Use in planning (HO3)**
- A planned event whose time slot is shorter than the value of its key gets a **warning**, never a block.
- An event that starts before the previous event's value has elapsed gets a warning.
- An event whose key has no value gets the warning "no timing for this key".
- The operator decides. The mission is not stopped by these warnings.

**Structure for the developer**
- The estimator is a pure function: `estimate(records, window, margin=0.05) -> table`. No file access inside it (rule §6.3 for the host: logic apart from I/O).
- Reading the records and reading or writing `timings.json` are a thin layer around it.
- The moving-math rule applies: the 2026-10-23 records and a synthetic fixture are the golden inputs, and the outputs are compared bit for bit.
- A CLI command (proposed): `cowas timing` shows the table and runs the start check, with `--sim` as everywhere.

**Done when**
1. On the 2026-10-23 records (8 records: 4 purge, 4 sample; of the sample records 1 complete, 3 `clogged`) the estimator gives **no value** for any key. The one complete time is listed as it is.
2. On a synthetic fixture with known durations, `value_s` equals the 90th percentile × 1.05, rounded up (golden, bit for bit).
3. A key with 2 samples gives no value.
4. With 12 folders on disk, only the last 10 are read.
5. A record with another `fw` or `calib` is not mixed into a group.
6. A planned slot one minute shorter than the value gives a warning in the planner.
7. A sample above the stored value is reported at the next start, and the table is updated.
8. Two starts on the same data give the same table (the update is repeatable).
9. **Never decreases:** after a window in which the slow runs have left (the window moves on), `value_s` is unchanged, and `window_s` shows the lower candidate.
10. **Outliers excluded:** a single sample outside ±15 % of its group's median does not change `value_s`, and it is listed as an outlier in the report.

**Open questions for the user**
- (a) **Decided (2026-10-26): the stored value never decreases; outliers are excluded** (see "Stored value per key"). The window still bounds what is read, and `window_s` is kept in the table so the current window stays visible. The "reel" check is the per-sample report at each start.
- (b) The 3-sample and ±15 % numbers are placeholders until the first field data.

**Depends on:** HO3 (planner warnings), HO4 (trusted clock), the record fields in Prerequisites (owner: firmware and session tool, in the step that enables the spool). Reduces the open item of F208.

### 7.3 Lab and eDNA

**LA1 · Lab meeting (dropped 2026-10-23).** The values are set with Felix ([shield-protocol.md](../lab/shield-protocol.md) §8). The questions below stay useful for any lab meeting later: the extraction kit, the shield volume per filter, the residual-water limit, storage and transport, labels, blank policy, the metadata columns, the chain of custody, the markers.

**LA2 · SOPs (P0)** · draft [filter-sop.md](../lab/filter-sop.md)
- Still to do: check the Sterivex orientation text on the real filter; check the rotor turns with a filter screwed on and the O-ring locked; remove "until HO2" notes once the commands exist.
- Done when: the dry run in §7a passes (loading ≤ 30 min, unloading ≤ 20 min, no leftover caps, no drips, no label mismatch).

**LA3 · Shield dose and residual water (P0)** · needs FW4
- A console `shield_calibrate`: dispense for a few durations, three repeats each, weighed on the 0.01 g scale. It prints the time for the target volume.
- Per filter: dry mass → after the water cycle (residual water) → after the shield (shield volume) → after 1, 3 and 7 days in the manifold (retention). Three filters per condition. Test the air-push settings too.
- `tools/analyze_la3.py` computes the numbers and gives PASS/FAIL against the lab's thresholds, with pytest on synthetic data.
- Done when: the shield volume is within 10 % of the target and the thresholds in [shield-protocol.md](../lab/shield-protocol.md) §8 are met. Results give the production shield time (`DNA_SHIELD_FILL_MS`).

**LA4 · Carry-over and cross-talk (P0)** · needs FW4, HW6
- Tracer: food dye read by spectrophotometer, or conductivity.
- Dead volume: wash a dyed line with clear water and measure the washout curve. This gives the internal dead volume and the purge needed for less than 0.1 % carry-over.
- Carry-over: dyed sample on one slot, then a clear cycle on the next slot. Cross-talk: a dyed slot while the rotor passes the others.
- `tools/analyze_la4.py` fits the curve (pytest on synthetic data). Recommend the purge factor and whether a bleach flush is needed before Pilot B.

### 7.4 Hardware and field

Each step is a short card. These are the sign-off checks; tick them in the log when done.

- **HW0** Order the parts in §11.1 first. Lead times set the critical path.
- **HW1** Electrical audit. Part 1 and the checklist are done ([power.md](../hardware/power.md), 2026-09-25). Part 2: enter the measured results, list the fixes (buffer, divider, fuse, pin move) and add their parts to §11.
- **HW2** Safety hardware: relief valve, E-stop with a DC rating above the 24 V current, fuses rated about 1.5 × the measured current. Test: a running pump stops within 0.5 s of pressing the E-stop.
- **HW3** Flowmeter: a 3.3 V-safe input. Calibrate by weighing (three runs of 500 mL, spread under 3 %). Measure the real sampling flow with a clean filter at 2 bar.
- **HW4** Pressure sensor: find pin 1 before powering it, power it from 3.3 V, mount it on a tee with no trapped air, clamp the barbs. The bench test is in §7a.
- **HW5** Power: battery, regulated 24 V and 5 V rails, the measured budget (idle W and Wh per cycle), 20 cold boots from the battery with 20 of 20 reaching the console, and a pump start that does not reset the Due.
- **HW6** Field fluidics: strainer and weight on the intake at a fixed depth, the outlet at least 5 m away, quick-disconnects, IP54 box above the fluidics with a shade. The pump must prime from the real water level.
- **HW7** O-ring: check the A4988 current setting is unchanged since the last calibration. `lock_endurance 200` with no encoder drift. Pressurise to 2.5 bar with the outlet clamped for 10 min with the coils off. Keep the O-ring locked for 7 days, then check for leaks.
- **HW8** Pilot B hardware: IP65 enclosure, cable glands, desiccant, a leak sensor at the lowest point, a water probe on the intake, a mooring plan and a label with a contact.
- **HW9** Manifold encoder coupling: a mechanical lock, since it slipped three times in two days (F187, F191). The cable and connector were the cause of F203, to be checked first.

### 7.5 Validation

**VA1 · Bench acceptance (P0)** · needs FW8, HO3, LA3, LA4, HW2–HW6
1. A full 14-slot mission from the host, on the battery: every record complete, volumes within 5 % of the weighed water.
2. A 24 h unattended run (about six events) with the machine alone.
3. A power cut in each phase, then recovery: the interrupted slot is `SUSPECT`, the mission resumes.
4. STOP and E-stop in each phase: valves closed, O-ring locked, nothing new starts.
5. A host restart mid-cycle: the Due is not reset, and the result arrives by request id.
6. A clogged filter: end `clogged`, and the cap is never exceeded (maximum `P_in` printed).
7. A cap trip with the relief valve isolated: trips at 2.5 ± 0.1 bar.
8. The USB cable pulled mid-cycle: the cycle ends safely and nothing new starts.
9. The export opened by the lab: columns accepted.
10. Battery: measured Wh per cycle and idle W give a margin of at least 50 % for Pilot A.

Go / no-go: tests 1–8 all pass. A failure in 9 or 10 needs a written workaround.

**VA2 · Pilot A (P0)** · see §8.1. **VA3 · Soak (P1)**: 72 h on battery with the Pi, remote alerts, including a power cut and a forced fault. **VA4 · Pilot B (P1)**: see §8.2.

---

## 7a. Bench checks still to run

| Step | Check | Pass when |
|---|---|---|
| FW2 | `leak_hold`, `multi_sample`, `full_cycle`, `full_all` in sensor mode | Each prints `TEST\|…\|PASS`, no SAFETY line, O-ring unlocked at the end |
| FW2 / FW5 | After a reset, `sample1m` is refused while the lock is UNKNOWN | `REFUSED`, nothing moves; after `actuator_home` it runs |
| FW3 / FW4 | Every pump command runs under the cap | Commands that do not poll the cap are not used: `cal_DNA` (F202), panel Pump (F2); see [commands.md](../software/commands.md) §7 |
| HW1 | Electrical audit M1–M44 | Every line of [power.md](../hardware/power.md) §4 is "Acceptable" |
| HW3 | Flowmeter D13 peak voltage (F26) | ≤ 3.3 V under pulses |
| HW4 | Spare 150 psi sensor alive (optional) | Steady 0.0–0.1 bar at rest, a clear rise when blown into. Check pin 1 first (datasheet table 8) |
| HW4 | Manifold alignment by airflow (F131) | Maximum flow within about 1° of the table value |
| Maintenance | Ethanol manifold prime and slot flush | With no filters installed, at least 50 mL is delivered through slot 0 before slot 1 starts; the flowmeter indicates at least 50 mL; each sample slot runs for 3 s after the pump's soft start; timeout or pressure-cap stop prevents later slots |
| HO1 | Host against the real Due | Native port found; reconnect after a cable pull with the same boot id; opening the port does not reset the Due |
| LA2 | SOP dry run, two people, 14 Sterivex | Loading ≤ 30 min, unloading ≤ 20 min, no leftover caps, no drips, no label mismatch |
| LA3 | Shield calibration (Felix) | CV ≤ 5 % per duration, R² ≥ 0.995 (shield protocol) |

Dropped: PID tuning by measurement. The gains stay at 200 / 100 / 100 (user decision 2026-10-19).

---

## 7b. Carried from the stage 1 operator test

Left out of stage 1. Nothing here blocks the field.

| Item | Ref | Owner |
|---|---|---|
| Vent: the O-ring unlocks even if the line stays above 0.3 bar (user decision). A venting step with a real stop, if it shows up on the bench | F192 | FW4 |
| Stop rules read the flowmeter, which reads almost nothing below about 25 mL/min: a slow filter can be stopped early | F150, F194 | FW8 |
| Confident-volume threshold at 50 mL/min: the flowmeter reads about 32 % low there | F150 | HW3 |
| Manifold encoder coupling: mechanical lock | F187, F191 | HW9 |
| Manifold sometimes does not turn; a cable move made it turn once | F203 | HW9 |
| Water drips from the filter after the O-ring unlocks: decide the drain step | F205 | FW4 |
| Sterivex emptying of a clogged filter prints about 940 `End cond` lines (kept as is, user) | F204 | FW8 |
| Purge: stage 1 uses 500 mL / 3 L (D50); the stage 2 rule and the 13 min spool timeout need a bench check. The 3 L purge at full power (D51) needs a manifold pressure check (F207) | F30, F110, F130, F207 | FW4 |
| Ethanol cleaning: prime through the purge slot to 50 mL indicated, then flush each sample slot for 3 s; confirm actual volume and timing on the bench before use | — | Felix |
| Filter ID, site, weights and operator are on the paper sheet, not in the machine record | HO2 | HO2 |
| `cal_DNA` pump run needs a cap check, or the routine is removed | F202 | Felix |
| Code clean-up: two sample paths, `fill_test`'s own pump loop, legacy `maintenance.cpp`, `step_dive`. Removed code is in [the archive](../archive/code/README.md) | — | FW4 |
| Timed sample list (`--schedule`) does not check that the samples fit the timeline: the cycle time is not measured yet. Add a check once it is known | F208 | HO6 |
| `run_sample` typed in `pio device monitor` skips the session tool: no record, no loading order, no timed list. One console for the operator (the host channel on native USB) | F210 | FW7 |
| Relief valve, E-stop and leak at 3 bar (F144): **accepted by the user 2026-10-23, not raised again** | HW2, F144 | — |

---

## 8. Pilot procedures

### 8.1 Pilot A (supervised, 1–2 days): VA2

Sampling plan (14 slots):

| Use | Count | When |
|---|---|---|
| Equipment blank (DNA-free water through the machine) | 1 | Start |
| Lake samples: 4 timepoints × 3 replicates | 12 | For example every 3–4 h |
| Equipment blank | 1 | End |
| Manual comparison samples (same time and place, same volume) | 2–3 timepoints | Beside the machine |
| Manual field blank | 1 | Any time |

Before the run: VA1 passed; site access, weather and life jackets; filters loaded per the SOP and marked READY; battery charged; DNA-free water (about 6 L), cooler, gloves, luer caps, a paper log; intake at the planned depth; clock synced; the equipment blank run is clean.

During and after: log events on paper (time, leaks, noises, weather, water temperature). Stop, unload per the SOP, chain of custody. Export the CSV, the logs and `docs/logs/test-logs` to `data/pilots/A_<date>/`. Debrief within 48 h.

**Success:** at least 12 of 14 cycles complete without a fault; blanks clean; machine and manual samples comparable. "Clean" and "comparable" are defined by Quentin and Felix ([lab/README.md](../lab/README.md)).

### 8.2 Pilot B (unattended, 1–2 weeks): VA3, VA4

Before: all P1 steps done and VA3 passed (72 h soak with alerts, a power cut and a forced fault). Plan: a blank at the start, 12 samples, a blank at the end, a visit on day 3–4 (leaks, battery, strainer).

---

## 9. Future work

Stage 3 ([stage3-full-system.md](stage3-full-system.md)) holds the P2 steps.

---

## 10. Old step IDs

The old roadmap used other step IDs. The mapping is in [the archive](../archive/README.md).

---

## 11. Parts and pins

### 11.1 Order first (HW0)

Specifications, not brands. Check each part against your fittings and measured currents. Lead times set the critical path.

| Part | Spec | For |
|---|---|---|
| Pressure sensor ABPDANV060PGSA3 (×3) | SPI, 3.3 V, 0–4.1 bar. `P_in` now, spares. **FW3 merged before the swap** (F27) | FW3 |
| Mechanical relief valve | About 2.5–3 bar, water-compatible, outlet to waste | HW2 |
| E-stop | Latching mushroom switch, 2 NC contacts: one DC-rated for the 24 V current in series, one to the Due | HW2 |
| Fuses and holders | Per branch: pump, VNH5019, A4988, valves, main. Plus spares | HW2 |
| Battery 12 V LiFePO4 ≥ 50 Ah with BMS | Pilot A. Pilot B sized after HW5 | HW5 |
| DC-DC boost 12 → 24 V and buck 5 V ≥ 3 A | Regulated, current ≥ 1.5 × measured peak | HW5 |
| Main switch, main fuse, DC energy meter (shunt) | Measures Wh per cycle | HW5 |
| Intake kit | Strainer (200–500 µm), weight, 10 m food-grade hose, rope | HW6 |
| Quick-disconnect fittings (×4) | Intake, outlet, purge | HW6 |
| Splash-proof box IP54+ and shade | Pilot A | HW6 |
| Sterivex ≥ 40; DNA/RNA Shield; DNA-free water ≥ 10 L; luer caps; labels; gloves; cooler | Lab consumables | LA2–LA4, VA2 |
| Test bench: scale 0.01 g, 1 L cylinder, bucket, clamp, food dye | Shield and flow measurements | LA3, LA4, HW3 |
| Spare Arduino Due; O-ring spare sets (×3) | Spares | all |

For Pilot B (P1, before the pilot): a Hall switch and magnet for the O-ring home, an enclosure leak sensor, waterproof temperature probes, current monitors, an IP65 enclosure, a SIM and LTE antenna, an RTC battery, the Pilot B battery after HW5.

### 11.2 Pin plan

The code uses about 31 digital pins and a few analog ones. The real limit is what the PCB shield brings out ([pinout.md](../hardware/pinout.md)).

| Item | Pin | Pri |
|---|---|---|
| Pressure `P_in` (ABP, SPI) | SPI CS D8 (unchanged) | P0 |
| Flow `Q_in` (turbine) | D13 if a pulse test passes at 3.3 V; otherwise D36 | P0 |
| E-stop state | D37 (NC to GND, `INPUT_PULLUP`; a broken wire reads as stop) | P0/P1 |
| O-ring home switch | D39 | P1 |
| Enclosure leak sensor | D41 or A6 | P1 |
| Water temperature (DS18B20) | D43 (4.7 kΩ pull-up to 3.3 V) | P1 |
| Battery voltage | A0 (divider 47 k / 10 k) or INA226 | P1 |
| Current monitors (INA226) | SDA 20 / SCL 21 | P1 |
| `P_out`, `Q_out`, turbidity | SPI D45; D12 or D47; A5 | P2 |

### 11.3 Electrical rules

- The Due is **3.3 V only**, not 5 V tolerant. Check every module's output level before connecting it.
- The DAC outputs about 0.55–2.75 V and is fragile: buffer it. The relay stops the pump, not the zero setting.
- Never feed the Due raw 24 V or a charged 24 V pack (above 16 V).
- One ground point for the power return. Keep motor current out of the sensor grounds.
- Flyback diodes on every coil and relay.
- Short SPI and I²C cables (under 30 cm). List every I²C address in [pinout.md](../hardware/pinout.md) before ordering.

---
