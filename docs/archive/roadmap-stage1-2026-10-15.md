# ARCHIVED 2026-10-19: the Stage 1 roadmap as it was (S1-0 to S1-3, 430 lines). Replaced by the focused ../roadmap/stage1-FINISHED-prototype.md. Kept for the record and for the long prompts and checklists.

# CoWaS roadmap, stage 1: functional prototype (old version)

**Written 2026-10-06, revised 2026-10-09 (a manometer is available: the pressure calibration moved from the end, S1-4, into S1-1, and it is done with a syringe, without the pump). Target: first supervised water tests in the week of 2026-10-12.**

**Revised 2026-10-15: S1-1 passed on the bench except the clogged-filter `leak_hold` (row 7), which exposed the pressure control loop (new step S1-1b: a `pressure_ramp` test first, then a layered controller); the Raspberry Pi is replaced by the laptop as the master for stage 1 (new S1-3: a campaign runner).**

The roadmap has three stages, one file each:

| Stage | File | Goal | Ends when |
|---|---|---|---|
| **1. Functional prototype** | this file | A full sampling cycle runs from the console, supervised, with a pressure cap that works | S1-2 done (and S1-3 if it is used): several samples in a row on the bench or at the lake side, with the sensor calibrated in S1-1 |
| **2. Lake pilots** | [roadmap/stage2-pilots.md](../roadmap/stage2-pilots.md) | Pilot A (supervised, 1–2 days) then Pilot B (unattended, 1–2 weeks): host-driven missions, full safety layers, known sample quality | First eDNA data from Lake Geneva |
| **3. Full system** | [roadmap/stage3-full-system.md](../roadmap/stage3-full-system.md) | Depth (spool), clogging analytics, web app, runtime parameters, environmental sensors | — |

Stage 1 is a shortcut through stage 2, not a separate design. It takes the thinnest slice of the stage 2 steps FW3 and FW4 that makes a cycle run, and postpones the rest. Nothing done here has to be undone: each item is a subset of what FW3/FW4 specify. The design decisions, the findings (F-numbers) and the rules are in the stage 2 file (§3, §4, §6) and they still apply. "ROADMAP §N" anywhere in the repository means that file.

---

## 0. Handover (2026-10-09, end of the first S1 session): read this first

**Update 2026-10-15 (the state now; the rest of §0 is the 2026-10-09 handover, kept for the record).**

- **S1-1 bench (2026-10-12, user report):** calibration PASS (`PRESSURE_GAIN = 1.0413`, `PRESSURE_ZERO_OFFSET_BAR = -0.0311`, max residual 0.032 bar), dead-sensor stop PASS, cap trip done at a temporary 1.0 bar. `PRESSURE_SENSOR_AVAILABLE = true`, `STX_MAX_PRESSURE = 3.0` (user decision, for a 3 bar seal test). **Row 7 failed:** `leak_hold` on clogged filters: pressure spike, cap reached almost at once, water leaked. Details and findings F132–F136: ../LOG.md.
- **New code, not flashed:** `pressure_ramp` (the pump regulated by pressure: a setpoint rising slowly to a target, hold, decay). `pio run -e due` passes, `pio test -e native` 126/126. **It has not run on the machine.**
- **Next, in order:** (1) flash this tree; (2) `pressure_ramp` on a blocked path to 1.0, 2.0, then 2.8 bar, note the manometer at 2.5 bar (S1-1b part 1, LOG R1–R5): this confirms or not that the locked setup holds about 3 bar; (3) S1-1b part 2, the layered pressure controller (design below); (4) S1-2; (5) S1-3, the laptop runner; (6) commit when the user says so. The whole branch is still uncommitted.
- **Answered 2026-10-15:** cap stays 3.0 bar (it fits the Sterivex and the tubing); the row-7 leak was at the manifold; the dead-sensor stop was "fast enough", the manometer reading at the 1.0 bar cap trip and the calibration points table were not noted. Test slots: 7 bare tube, 10 broken open filter, 11 new filter, 12 fully clogged.

**Where the work is.** Branch `s1-1-pressure-abp`, **nothing committed** (it also carries the three-stage documents and the `abp_raw` work). `pio run -e due` passes, `pio test -e native` 109/109. The Due runs an **older build of this branch**: the tree as it is now has not been flashed.

**What the user wants from `calibrate_pressure` (user's words, 2026-10-09; this is the requirement, don't redesign it):**

> calibrate_pressure asks if we are at 0 bar, then press a key (NO BUG, that's the main point), then measure 0 bar. Asks BEFORE 0.5 bar, then press (NO BUG HERE EITHER), then measure, same for 1, 1.5 and 2 bars.

So, for each of 0 / 0.5 / 1.0 / 1.5 / 2.0 bar: **ask, wait for the key, only then measure.** Nothing may be measured before the key press of that point. Twice on the bench an earlier version measured the 0.5 bar point before the pressure was applied. The first time, the line feed of a key typed as "character + ENTER" was left in the serial buffer and taken as the next key press. The second time the cause was not found.

**State of the code for that requirement:** written, **not tried on the machine**. `calibrate_pressure()` in `src/Tests.cpp` is one loop over the five points: prompt, `wait_for_enter()`, 3 s average, "done". `wait_for_enter()` throws away everything received before the prompt (until the line has been quiet for 100 ms), waits for a whole new line (ENTER as CR, LF or both), and throws away what follows it. The manometer value of a point is its target; nothing else is typed. `x` + ENTER aborts.

**Things tried in this session and dropped, don't bring them back unasked:** an automatic measurement when the sensor is steady (not what the user asked), a pump-driven water column test (removed on the user's decision), a "not steady" verdict on a point, typing the manometer value after each point.

**Decisions of the user to keep:**
- The sensor ordered and supplied is **ABPDANV150PGSA3**. Don't state otherwise.
- Against the manometer the sensor read 2.4 times too high with the 150 psi conversion (F126). **The manometer is trusted**: `ABP_FULL_SCALE_PSI = 60` is the value that agrees with it, and the calibration fits the rest. No further check is planned.
- The calibration uses a syringe of air on a tee with the sensor and the manometer, off the pump line. No pump.
- Short, plain instructions on the console and in answers.

**Open bench problems:**
- A command sometimes had to be typed twice before it ran (user, 2026-10-09). Cause not found. The menu now trims each line, echoes it as `> command`, and prints `=== Test menu READY ===` when it starts listening (input typed before that is lost, F49). If it happens again, the echo line tells whether the first attempt arrived.
- The USB link dropped and the Due reset once when `actuator_home` was typed (F125).
- The SPI clock of the sensor had to go from 800 kHz to 100 kHz to get clean frames (F124, fixed).

**Next, in order:**
1. Flash this branch. Wait for `=== Test menu READY ===`.
2. `calibrate_pressure` (syringe + manometer). Expected: five "ask, ENTER, measure" steps, then `TEST|calibrate_pressure|fit|PASS` and two lines to paste.
3. Paste `PRESSURE_GAIN` and `PRESSURE_ZERO_OFFSET_BAR` into Settings.h, set `PRESSURE_SENSOR_AVAILABLE = true`, reflash.
4. Sensor back on the pump line: `pressure` at rest (0 ± 0.05 bar), the dead-sensor stop, the cap trip at a temporary 1.0 bar (../LOG.md, S1-1 checklist rows 4 to 7).
5. The rest of S1-0, which does not need the pressure sensor: release the O-ring (`linear_actuator`, `f`), put the manifold on a slot by hand (`oring_state` within 3°), `calibrate_actuator` (press `c` with the O-ring RELEASED, then a key at the stall), paste the value, reflash, `actuator_home`, `lock_cycle 5`, `rotate_all dry auto`, `encoder_repeat 5 10 auto`, `valve_check`; D13 voltage; hose inner diameter and length.
6. Commit S1-1 when the user says so, then S1-2.

**Prompt for the new session:**

```text
Read CLAUDE.md, then ../roadmap/stage1-FINISHED-prototype.md §0 (handover) and the S1-1 section, then the
S1-1 and "S1-0: first bench session" entries of ../LOG.md. Branch s1-1-pressure-abp,
nothing committed. Do not redesign calibrate_pressure: the requirement is quoted in §0.
I will flash and paste the console output of calibrate_pressure. From it: put PRESSURE_GAIN and
PRESSURE_ZERO_OFFSET_BAR in Settings.h, set PRESSURE_SENSOR_AVAILABLE = true, build, and walk me
through checklist rows 4 to 7. Keep answers short. Don't flash or commit without asking.
```

---

## 1. What "functional prototype" means

One person at the machine, laptop on the Programming port, types commands in the test menu:

```
actuator_home                          (after every reset)
full_cycle 1 volume=1000               one sample: purge -> filter -> shield -> empty
full_all count=5 volume=1000           several samples in FILL_ORDER, a key press between each
```

The machine purges through slot 0, filters the asked volume through a Sterivex under PID pressure control with a live cap, adds the shield, empties the lines, and prints one `TEST|full_cycle|<slot>|PASS|...` line per filter. `tools/hw_test_log.py` saves those lines to a CSV: that is the sample record for now.

**Not in stage 1:** the host (`cowas` CLI, protocol v1, Native USB), scheduled or unattended sampling, recovery after a reset, the watchdog, the supervisor, the sample-quality measurements (LA3, LA4).

**Laptop as the master (revised 2026-10-15).** The person at the machine does not have to type each command: from S1-3 on, a runner on the laptop (`tools/campaign.py`) sends them over the same console, in the order of a sample plan, and writes the sample record. The Due stays the slave that executes one filter at a time under its own safety. This is the Pi plan with the laptop in the Pi's place, so the stage 2 host is a replacement of the runner, not of the machine. See S1-3.

## 2. Where we are (2026-10-06)

The cycle already exists. `full_cycle` and `full_all` (FW2) call the real `step_purge` → `step_sampling` → `step_DNA_shield` → `step_empty`. They refuse today for one reason: `PRESSURE_SENSOR_AVAILABLE = false`.

| Piece | State | Blocks the prototype? |
|---|---|---|
| Manifold rotation, encoder, lock tri-state, `actuator_home` | Coded (FW2). **Hardware checklist not completed** (user, 2026-10-06; `docs/test-logs/` is empty) | Run it (S1-0) |
| ABP pressure sensor on SPI D8 | **Bench 2026-10-09: clean frames at a 100 kHz SPI clock. Against the manometer it reads 2.4 times too high with the 150 psi constant (F126): **the manometer is trusted (user decision)**, so the firmware follows it (`ABP_FULL_SCALE_PSI = 60`).** Ordered and supplied as ABPDANV150PGSA3 (user), in the place of the old "Trustability" one. The analog A2 sensor no longer exists. **A manual manometer (0–2.5 bar, looks like a Riegler 107-KD, class not read yet) is available since 2026-10-09** (user). Gain not checked against it yet | No: `calibrate_pressure` exists (S1-1), run it at the bench |
| Safety cap and all PID loops | Were reading the floating A2 pin (`read_pressure2`, 7 call sites). **Code done in S1-1 (2026-10-09):** they read the ABP (`read_p_in`). Bench checks pending | Bench → S1-1 checklist |
| Pressure control on a filter that clogs | The cap and the PID read the ABP, but on clogged filters the cap is reached almost at once and the loop is blind for 4 s after each step (F132–F135) | **Yes → S1-1b** |
| ABP conversion | Constants assumed a 10 bar part (F27): the 150 psi part read 3.3 % low. **Fixed in S1-1:** `src/Core/abp_convert`, full scale in Settings.h | No |
| ABP dead or stale | The driver kept the last value for ever. **Fixed in S1-1:** a fault frame, no answer, or 500 ms of stale frames gives an implausible reading, which stops the pump. Bench check pending | Bench → S1-1 checklist |
| Sample volume and timeout | 100 mL default; `step_sampling` gives up after a hard-coded 3 min and then carries on as if it had finished (F15) | **Yes → S1-2** |
| Purge | 100 mL, less than the intake hose (F30) | Yes for real water → S1-2 |
| Shield time | `FILL_STERIVEX_TIME / 10` = 25 s, a test shortcut (F4) | Value needed → S1-2 |
| Stop during pumping | None: `CtrlPump::run()` reads no key. Only the power switch stops a 20 min run | **Yes → S1-2** |
| Flowmeter | Factor 77 never calibrated on this machine (F6) | Calibrate by weighing (S1-2 bench) |
| O-ring lock steps | `LINEAR_ACT_LOCKED_STEPS = 160`, measured 2026-10-12 | FW2 rows 1–5 with this value (S1-0) |
| Uncommitted work | The `abp_raw` diagnostics, the three-stage documents and all of S1-1 sit uncommitted on the branch `s1-1-pressure-abp` | Commit when the user says so |

## 3. Steps

| ID | What | Who | Effort | Slice of (stage 2) | Done |
|---|---|---|---|---|---|
| **S1-0** | Bench facts, no code | person | 1 h | FW2 checklist, FW3 inputs | ◐ 2026-10-12: items 1–4, 6, 7 and FW2 rows 0–5 done; item 5 (D13) and the alignment (F131) deferred by the user; the purge with the spool (F130) awaits a decision |
| **S1-1** | Safety and PID on the ABP; sensor gate on; **calibration against the manometer (syringe)** | Claude + flash + bench | 1 day | FW3 items 1–3, 5 and the former FW3b | code ☑ 2026-10-09 · bench ☑ 2026-10-12 rows 1–6 (calibration, dead-sensor stop, cap trip) · **row 7 failed** on clogged filters, see S1-1b |
| **S1-1b** | **Pressure control that survives a clogging filter:** part 1 the `pressure_ramp` test (pump regulated by pressure; leak check of the locked setup at about 3 bar); part 2 one layered controller (setpoint ramp, soft limit, one PI, clog end), tested against a simulated clogging plant | Claude + flash + bench | 1–1.5 days | FW3 item 3 (PID), FW4 item 1 (end reason) | part 1 code ☑ 2026-10-15 (not run on the machine) · part 2 ☐ |
| **S1-2** | A real cycle: volume, timeout, purge, shield time, `full_cycle ... auto` and the record fields (the end reason, the key-press stop and the pressure target moved to S1-1b) | Claude + flash + bench | 1 day | FW4 items 5a/c/d, 6, 7 | ☐ |
| **S1-3** | **Laptop campaign runner:** the laptop is the master over the console (no Pi, no FW7): sample plan, slot inventory, timed series, sample CSV, stop rules | Claude + bench | 1 day | HO2/HO3 in miniature, FW8 record fields | ☐ |
| ~~S1-4~~ | ~~Pressure calibration against a manometer~~ | | | | merged into S1-1 (2026-10-09) |

Suggested days (revised 2026-10-15, a proposal): Thu 15: flash this tree, `pressure_ramp` R1–R3 (the seal check); Claude writes S1-1b part 2 (the pure controller and its simulated-plant tests). Fri 16: `CtrlPump` on the new controller, flash, retest the clogged filters. Mon 19: S1-2 and the flowmeter weighing; the S1-3 runner in parallel. Tue 20: a laptop-driven series on the bench with water. From Wed 21: a lake-side series with the laptop, a person at the machine.

**Where the calibration sits (changed 2026-10-09).** It was the last step (S1-4), to be done "the day the manometer is connected". A manometer is available now, so it is part of S1-1. **Method (user's idea, 2026-10-09): a syringe of air.** The sensor and the manometer are taken off the pump line and joined to a 12 mL syringe on a tee. Both then see the same static pressure, with no pump pulsation, and the pump is never started, so the calibration does not depend on the cap at all. The first plan (pump at low power against a partly closed outlet) was dropped: there is no valve on the machine, and a pinched hose gives a fluctuating pressure. The order at the bench is therefore: flash S1-1, `calibrate_pressure`, paste the two lines, reflash, and only then the checks that run the pump (dead-sensor stop, cap trip). The points go up to 2.0 bar; the cap (2.5 bar) is 0.5 bar above the last point, which is fine for a linear sensor. `STX_MAX_PRESSURE` went to 2.5 in S1-1 (it was planned for S1-2): 3.0 was above the gauge's range. `calibrate_pressure` is also the first stage of `calibrate_all` (user, 2026-10-09).

### Keeping notes (every step, including the ones with no code)

1. **[LOG.md](../logs/LOG.md):** one entry per step, written before the step is merged: what changed, every behavior change, test results, the hardware checklist table with the measured numbers, new findings (next free F-number), and the status of the findings it fixes.
2. **`docs/test-logs/`:** every bench session is captured with `python tools/hw_test_log.py --port COMx --tag s1-<n>` (CSV of the `TEST|` lines + the full transcript).
3. **the pending checks:** move passed rows to "Done" with the date and the log file; add what is still blocked.
4. **The "Notes" block under each step below:** date, who, the two or three numbers that later steps need, and anything surprising. Short; the detail goes in the log entry.
5. Tick the step in the table above and, when it completes a stage 2 item, note it in that file's §7.

Each step is one branch from `cowas-v2`, with `pio run -e due` and `pio test -e native` passing. What stage 1 cuts is scope, not the checks.

### S1-0: bench facts (no code, do first)

1. ~~Which ABP is installed?~~ Answered 2026-10-06: `ABPDANV150PGSA3` (150 psi).
2. Flash the current tree, then `abp_raw` at rest: expect status `00 valid` (some `10 stale` is fine) and counts near **1638**, **the same on all 20 lines** (within about ±20 counts; a spread of hundreds or thousands is a wiring fault, F124). Blow into the port: counts rise. Note the resting counts. Quick gain check with the syringe and the manometer on a tee: about 2905 counts at 1.0 bar and 4173 at 2.0 bar.
3. FW2 checklist rows 0–5 (../LOG.md, FW2 entry). These rows need no pressure sensor and no water: refusal after reset, `actuator_home`, `lock_cycle 5`, `rotate_all dry auto`, `encoder_repeat 5 10 auto`, `valve_check`.
4. `calibrate_actuator` → the real `LINEAR_ACT_LOCKED_STEPS`.
5. Multimeter on the flowmeter signal wire (D13) with the pump off and on: it must stay ≤ 3.3 V (F26). If it shows 5 V, power the flowmeter from 3.3 V or add a divider before going further.
6. Measure the intake hose: inner diameter and length (for the purge volume in S1-2).
7. Manometer (2026-10-09, 0–2.5 bar, looks like a Riegler 107-KD): note the accuracy class printed on the dial ("Kl. 1.6" or "Kl. 2.5") and the smallest division. They set `CAL_PRESSURE_MAX_RESIDUAL_BAR` (0.06 bar). Answered 2026-10-12: class 1.6, 0.1 bar divisions; 0.06 kept (0.04 class + 0.02 reading).

**Notes (fill in):**

| Date / who | ABP resting counts | FW2 rows 0–5 | `LINEAR_ACT_LOCKED_STEPS` | D13 max voltage | Hose ID × length | Manometer (class, division) | Log file | Surprises |
|---|---|---|---|---|---|---|---|---|
| 2026-10-09, Quentin (incomplete) | First session: not obtained, garbage frames (F124). **Second session, S1-1 firmware: 1631–1636 (spread 5), 20 °C; about 4545 counts at ~1 bar and 7700 at ~2 bar: 2.4 times the 150 psi expectation (F126)** | row 0 PASS; row 1 not passed (USB drop, then not aligned); rows 2–5 not run | not measured (calibration aborted twice) | — | — | — | none (monitor used, not the logger); summary in ../LOG.md "S1-0: first bench session" | F124: ABP frames valid by status but garbage. F125: USB drop + reset at `actuator_home` |
| 2026-10-12, Quentin (reported in chat) | (done 2026-10-09) | rows 1–5 all PASS (rotate_all worst -0.35 deg, encoder_repeat spread 0.18 deg, F128), see LOG | **160** (to confirm: no `n` answered at the stall prompt, F127) | deferred by the user (F26): 3.3 V steady at rest, blow test | 4 mm ID × 2.40 m, about 30 mL; spool path 9 mm ID × 40 m, about 2.5 L, connected by default (F130) | class 1.6, 0–2.5 bar, 0.1 bar | `docs/test-logs/2026-10-12_s1-0.log` (pasted transcripts; the logger did not work) | F127: step count restarted after `n` (fixed). F128: stop offsets off by direction. F129: wrap point between slots 0 and 1 (moved to 3/4) |
| | | | | | | | | |

### S1-1: pressure on the ABP, calibrated

Prompt for a new session on branch `s1-1-pressure-abp`:

```text
Read CLAUDE.md, ../roadmap/stage1-FINISHED-prototype.md (§3 "Keeping notes" and this step only),
../roadmap/stage2-pilots.md §3.3, ../LOG.md (F21, F27, F50),
src/Hardware/Trustability_ABP_Gage.cpp, src/main.cpp (read_pressure2, set_pressure_safety), Pump.cpp.
Installed sensor: ABPDANV150PGSA3 (150 psi, SPI, CS D8). The analog A2 sensor is physically gone.
Resting counts from abp_raw: <N>.
Reference manometer: 0-2.5 bar dial gauge (looks like a Riegler 107-KD), class not read yet.
Pressure source for the calibration: a 12 mL syringe of air on a tee with the sensor and the
manometer, off the pump line (user, 2026-10-09). The pump is not used.
Minimal slice of FW3, plus the calibration. Do only this, in two parts.

Part A: safety on the ABP (flash and check before part B)
1. src/Core/abp_convert: counts -> bar for a given full scale in psi (10-90 % of 2^14 counts,
   1 psi = 0.0689476 bar), and a frame classifier (valid / stale / fault). Native tests: 1638 -> 0,
   14745 -> full scale, midpoint, for 60 and 150 psi; each status-bit pair. The driver uses it;
   the 7.63e-4 / 1.25 constants go away. Full scale = ABP_FULL_SCALE_PSI in Settings.h.
2. Stale handling: status 10 keeps the last value; if no valid frame arrives for
   ABP_STALE_TIMEOUT_MS (500), or status is 01/11, or the frame is FF FF FF / 00 00 00,
   getPressure() returns a value below PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR so the existing
   plausibility check stops the pump. Print the reason (throttled).
3. Gain and zero in Settings.h: PRESSURE_GAIN (1.0 until part B) and PRESSURE_ZERO_OFFSET_BAR,
   applied in the driver (bar = PRESSURE_GAIN * nominal - PRESSURE_ZERO_OFFSET_BAR; say which form
   you chose). Console command `pressure_zero` (pump off, lines open, 5 s average, prints the line
   to paste): a quick re-tare that needs no manometer. The end conditions use 0.04 bar thresholds,
   so the zero matters.
4. read_p_in() in main.cpp replaces read_pressure2 in set_pressure_safety() and in every
   CtrlPumpFlow / CtrlPumpNoWater begin() call (list the call sites). Do NOT delete the A2 code
   or rename pressure1 (that stays in FW3).
5. STX_MAX_PRESSURE = 2.5 (was 3.0, above the manometer's range). PRESSURE_SENSOR_AVAILABLE = true.
   Commit the pending abp_raw work in the same branch.

Part B: calibration against the manometer
6. Console command `calibrate_pressure` (requirement in §0): for each of 0 / 0.5 / 1.0 / 1.5 / 2.0
   bar it asks, waits for ENTER, and only then measures for 3 s. The manometer value of a
   point is its target. 'x' + ENTER aborts. The pump is never started. It works on the
   datasheet reading, so it can be rerun with a gain already pasted. It is also the first
   stage of `calibrate_all`.
7. Pure src/Core/ function: least-squares gain + offset, R², the largest residual. Native tests:
   exact line, noise, < 2 points, a gain outside 0.9-1.1 is refused (wrong part number or a
   wiring fault, not a calibration).
8. Prints PRESSURE_GAIN and PRESSURE_ZERO_OFFSET_BAR as Settings.h lines and a
   `TEST|calibrate_pressure|fit|PASS/FAIL` line. PASS: every residual < the larger of 0.05 bar and
   the manometer's accuracy.
9. Notes: update CLAUDE.md "Known hazards", ../reference/commands.md, the pending checks, the ../LOG.md
   entry (F50, F27, F21 status; the table of points: manometer, ABP before, ABP after) and this
   step's Notes block.

Hardware checklist to walk me through, in this order (the calibration first: it needs no pump).
1. `abp_raw` and `pressure` at rest: status valid, 0 ± 0.05 bar (before the zero is pasted: the
   raw zero, expected within ± 0.16 bar).
2. `calibrate_pressure` with the syringe -> `TEST|calibrate_pressure|fit|PASS`; paste the two
   lines into Settings.h, reflash; with the syringe again, `pressure` agrees with the manometer
   at two pressures (within 0.06 bar).
3. Sensor and manometer back on the pump line. `pressure` reads 0 ± 0.05 bar at rest. Then set
   PRESSURE_SENSOR_AVAILABLE = true and reflash (it stays false until here, F124).
4. Dead sensor: `pump` at 20 % on an open outlet, unplug the sensor -> SAFETY line and pump off
   within 1 s; the pump refuses to start until the sensor is back.
5. Cap trip: STX_MAX_PRESSURE temporarily 1.0, `pump` at low power, pinch the outlet shut -> the
   pump stops when the manometer shows 1.0 ± 0.1 bar. Put 2.5 back.
6. `actuator_home`, `slot5`, `leak_hold 100` in sensor mode on one slot.
```

**Notes (fill in):**

| Date / who | Zero offset (bar) | Gain | Largest residual (bar) | Dead-sensor stop (s) | Cap trip at (bar, on the manometer) | `leak_hold` result | Log file | Surprises |
|---|---|---|---|---|---|---|---|---|
| 2026-10-12, Quentin (reported 2026-10-15) | -0.0311 | 1.0413 | 0.032 (R² 0.99897, 5 points) | PASS (time not reported) | done at a temporary 1.0 bar (reading not reported) | **FAIL on clogged filters:** cap reached almost at once, water leaked (F132–F135) | none (the logger did not work) | `ABP_FULL_SCALE_PSI = 60` is absorbed by the fit (F126). The user raised the cap to 3.0 (F136). Row 7 exposed the control loop, not the cap: S1-1b |

### S1-1b: pressure control that survives a clogging filter (added 2026-10-15)

**Why.** S1-1 row 7 failed: `leak_hold` on clogged filters saw a pressure spike, the cap was reached almost at once, and water leaked (peak pressure unknown, possibly well above 3 bar). Reading the code found four reasons the loop in front of the cap cannot cope with a filter that is close to clogging (LOG F132–F135):
1. every `CtrlPump::run()` first goes **open-loop to 50 %** (4 s soft start) before any PID action: on a clogged filter 50 % is already more than the line takes (F133);
2. a PID output change of 10 % or more triggers a **blocking 4 s ramp** inside `Pump::set_power()`, and with Kp = 200 % per bar almost every update is that big: the PID is blind for 4 s after each step (F132);
3. `pump_pid` is **never reset** between phases, has D = 100 on a pulsating signal, and was never tuned for a changing filter (F134);
4. the cap is read **every 100 ms** and the peak is not logged (F135).
The first two are the likely cause of the instant trips. That is a hypothesis until a pressure curve is logged.

#### Part 1: `pressure_ramp` (code done 2026-10-15; not yet run on the machine)

The pump is regulated **by pressure**, not by a fixed 20 % or 100 %: the PID follows a setpoint, holds it, then (optionally) the pump stops and the decay is read. Two uses: a **slow rise to about 3 bar** as a seal check of the locked setup (the defaults: 2.8 bar at 0.05 bar/s, hold 20 s, decay 30 s), and the **fine tuning of the production PID** (below). It is water only. The default gains are the production ones (200 / 100 / 100, tuned by hand: they are the baseline, not something to throw away). Description, safety rules and checklist R1–R5: ../LOG.md, "S1-1b". It refuses to start if the line is not empty (P above 0.3 bar), so runs are comparable; the cap is checked every pass, and the test stops the pump itself 0.15 bar above the target (never closer than 0.1 bar to the cap).

**The test slots (temporary, during the tests).** Slot 7: a bare tube, no filter (no resistance: the pressure can't build, expect `reached=no` and `sat` high: it only checks the sensor and pump chain). Slot 10: a broken, open filter (low resistance). Slot 11: a new, unused filter (medium). Slot 12: a fully clogged filter (high resistance; pressure builds fast).

#### Part 1a: the seal check (do this first)

On slot 12, `actuator_home` then `slot12`, then, one after the other, each with the manometer in view and someone at the 24 V switch:
1. `pressure_ramp target=1.0 hold=10 decay=20`
2. `pressure_ramp target=2.0 hold=10 decay=20`
3. `pressure_ramp` (2.8 bar, hold 20 s, decay 30 s). Note what the manometer shows when the console says 2.5 bar (F136).
Answer the leak question each time. **PASS: `reached=yes`, no SAFETY line, `pmax` ≤ target + 0.15, no leak, and the decay in the TEST line noted.** The earlier leak was at the manifold: watch it during the hold. Let the pressure out between runs (the next one refuses above 0.3 bar).

#### Part 1b: fine-tuning the PID, step by step

**The rule that makes it measurable:** every setting is a label, every label is run **twice on each of slots 10, 11 and 12**, and the script compares two labels. A difference smaller than the repeat-to-repeat spread counts as SAME, so noise is not mistaken for an improvement. Capture the session: `python tools/hw_test_log.py --port COM6 --tag tune`. Always target 2.0 bar (the production setpoint), `decay=0`, `quick` (no leak question).

1. **Baseline = what production does today, twice per slot.**
   - Slots 10 and 11: `pressure_ramp target=2.0 rate=0 start=50 slew=0 maxpower=100 hold=20 decay=0 quick label=base`
   - Slot 12: the same with `start=15` (starting at 50 % on a clogged filter is the spike that leaked; every run on slot 12, baseline or candidate, uses `start=15`).
   Look at the numbers before changing anything: `ovs` (overshoot), `rise` (s to 90 %), `rip` (pressure ripple), `rev` (power reversals = chatter), `sat` (time at max power). The two repeats of the baseline tell you the noise.
2. **First candidate: no gain change, only the two structural fixes** (`slew=3 start=15`: the pump output moves at most 3 % per update, so the blocking 4 s ramp never happens, F132, and the start is gentle, F133). Same command with `label=slew3`. This is the most likely win and it costs no tuning.
3. **Compare:** `python tools/compare_tune.py docs/test-logs/<date>_tune.csv --base base --cand slew3`. Per slot and metric it says BETTER / SAME / WORSE, and ends with "not worse on any slot" or "WORSE somewhere" (exit code 1).
4. **Then the gains, one at a time, factor 1.4 up or down, each its own label** (`kp=140 label=kp140`, `kp=280 label=kp280`, then `ki=`, then `kd=`), always on top of the best setting so far (keep `slew=3 start=15`). Reading the numbers:

   | You see | Try |
   |---|---|
   | `ovs` high, then oscillation (`rev` and `rip` high) | lower `kp`, or lower `ki` |
   | `rise` long, `sat` high, `err` negative (target not reached) | higher `kp` or `ki` (and check `maxpower`) |
   | `err` stays off the target but nothing oscillates | higher `ki` |
   | `rip` high while the power swings (`pwp2p`, `rev` high) with a quiet pressure | lower `kd` (the sensor ripple of the pump goes through D) |
5. **Acceptance: "it helps and it does not make anything worse".** A candidate is kept only if, over the repeats: the script says "not worse on any slot"; at least one metric is BETTER on slot 11 or 12; on slot 12 `pmax` stays under the soft limit with margin and no run ends `soft_limit` or `cap`. If a setting is better on slot 10 and worse on slot 12, it is **not** a better PID: it is the evidence that one fixed gain set does not cover the whole range (that is when gain scheduling gets added, part 2).
6. **Use the winner.** The gains go in `PUMP_PID_KP/KI/KD` in Settings.h and apply to production at once (`main.cpp` reads them). `slew` and `start` are only inside this test until part 2 rebuilds `CtrlPump` on them. Then recheck the case that failed: `leak_hold 100` on slot 11, then slot 12: no cap trip, no leak.

Stop rules while tuning: any SAFETY line, a leak, or a run that ends `soft_limit`/`cap` ends the session for that setting. Do not raise `STX_MAX_PRESSURE` to make a run complete.

#### Part 2: the controller (design, then code)

**Update 2026-10-15: pressure regulation findings.** Testing revealed the pump has a **deadband: 0-10% power produces no flow** (mechanical stiction). Setpoint-tracking PID on high-resistance systems (slots 11, 12) causes catastrophic instability: the PID tries low power (1-5%) to reduce overshoot, the pump doesn't respond (deadband), error grows, PID swings to 15-20%, pump overshoots. Cycle repeats. Lower gains (100/50/50) improve rise time 83s → 9s but oscillations persist. **Root cause: control architecture mismatch (setpoint-tracking PID) with deadband + lag at high system gain.** Simple minimum clamping (1% → 10%) is not enough; the PID logic must be redesigned. Soft cap raised from 2.15 to 2.40 bar (F137). **Solution: Part 2 must use a power-ramp controller, not setpoint-tracking PID during ramp phase.** (Findings F137–F138, logged in memory: s1-1b-deadband-findings.md.)

**Is the answer two PIDs and a switch? No: one controller, in layers.** A switch between a "clean filter" PID and a "nearly clogged" PID adds a boundary to tune, a bump when control changes hands, and does not fix the start (open-loop at 50 %) or the spike. What fails today is mostly structure, not tuning. The layers:

| Layer | What it does | Where |
|---|---|---|
| 0. Hard cap | Pump stops above `STX_MAX_PRESSURE` (3.2 bar since 2026-10-15) or on an implausible reading. Checked **on every pass** (about 2 ms), not every 100 ms | `Pump` (exists; poll rate fixed, F135) |
| 1. Soft limit and rise-rate limit | If `P ≥ PRESSURE_SOFT_LIMIT_BAR` (2.5), or `P + τ·dP/dt ≥ soft limit` (τ about 0.3 s, PLACEHOLDER), the power is cut fast (down-slew about 10 % per update, no blocking ramp). It is not an error: the sample goes on at lower power. This is the layer that catches the spike | `src/Core/pressure_control`, pure |
| 2. One pressure PI | Setpoint `PRESSURE_TARGET_BAR` (2.0), **reached by a setpoint ramp** (0.1–0.3 bar/s) from 0 at the start of every phase, never by a fixed first power. PI only (D = 0). Output slew limit (3 % per 100 ms up), integrator clamp (anti-windup), **reset at the start of each phase** with the integrator seeded at the present power (no bump). Output limits `[min, max]` power | `src/Core/pressure_control`, pure |
| 3. Clog state | Power at the maximum and flow below a floor for N s, or the target not reachable at max power: the sample ends with `end_reason=clogged` and the partial volume, flagged (§3.7 rule: recorded, never silent) | `src/Core`, with the end reasons of S1-2 / FW8 |

**Why one PI tuned for the worst case works.** Pressure is about `R · Q(power)`. As the filter clogs `R` rises, so the loop gain (bar per % of power) rises: a PI tuned on a clean filter becomes too aggressive when clogged. Tune the single PI for the **highest gain** (a clogged filter or a pinched line): it is then stable everywhere and only slower on a clean filter. Slowness costs nothing: a sample ends on volume or on the timeout, not on how fast 2 bar is reached. Only if the clean-filter response is too slow, add **gain scheduling** (scale Kp by the measured ΔP/Δpower, or a table of two or three points): that is the clean form of "two PIDs", and it is still one controller.

**How to get the gains without guessing, in this order.**
1. **A closed-loop simulation in the native tests** (`pio test -e native`): a plant `P = R · f(power)` with a first-order lag, a dead time and pulsation noise, with `R` from 1× to 10× and a sudden jump of `R` (a filter that clogs). Assertions: `P` never reaches the soft limit plus a margin, no sustained oscillation, settles within a time, any start state works, the integrator is reset between phases, no output step above the limit. The clogging behaviour becomes a unit test, not a bench surprise. The plant numbers are PLACEHOLDERs until step 2.
2. **Step data from the bench** with `pressure_ramp` and a step test (`pump_step`: +10 % power from a steady point), at three restriction levels (a clean filter, a half-pinched line, a blocked path): gives the gain K (bar per %), the lag and the dead time for each. Pick Kp, Ki from the highest K. The live print at 9600 baud is too slow to fit (1 line per second): record `t, setpoint, P, power` at 10 Hz **in RAM during the run and dump it afterwards** as `CSV|` lines, which the laptop logger saves.
3. **Bench check** with the real clogged filters that failed in row 7.

**Firmware changes of part 2** (each a behavior change to list in the PR):
1. `src/Core/pressure_control.{h,cpp}` (setpoint ramp, PI with clamp and reset, slew limit, rise-rate limiter, clog rule) and its tests, including the simulated plant. Reuses `src/Core/pressure_ramp` from part 1.
2. `CtrlPump::run()` is rebuilt on it: starts from 0 with a setpoint ramp (no fixed 50 %), reads the cap and the soft limit on every pass, a fresh controller per run, no blocking ramp in the loop (PID trims go through a path that never ramps, F132), prints and returns `p_max` and an **end reason** (condition met / timeout / clogged / soft limit / pressure cap / implausible / aborted: this is S1-2 item 1, delivered here), and stops on a key press (S1-2 item 2).
3. Settings.h: `PRESSURE_TARGET_BAR = 2.0` (replaces the hard-coded 2 at 6 call sites, F3), `PRESSURE_SOFT_LIMIT_BAR = 2.5`, the gains, `PRESSURE_FLOW_FLOOR` and the clog times (all PLACEHOLDER). `STX_MAX_PRESSURE` is 3.2 (user decision 2026-10-15), and **needs checking against the weakest part's rating** (Sterivex housing, tubing, valves, O-ring): not checked yet.
4. `pump_pid` and its `setPID(200, 100, 100)` in `setup()` are removed once nothing uses them.
Not now: a second pressure sensor, a relief valve (HW2: still the right answer for a spike nobody can predict: fit it if the part is there), the clog analytics of stage 3.

**Hardware checklist (part 2, after part 1 has passed):**

| # | Action | PASS criterion | Result |
|---|---|---|---|
| C1 | `pressure_ramp` R1–R3 (part 1) on a blocked path | LOG S1-1b R1–R5 | |
| C2 | `pressure_ramp target=0.5` on a clean filter, then a half-pinched line, then blocked, with the 10 Hz dump | K, lag, dead time written down for each | |
| C3 | `leak_hold 100` on a clean filter, then on the clogged filters that failed | no cap trip, `p_max` ≤ 2.5 bar, the water phase ends on volume or `end_reason=clogged`, no leak | |
| C4 | `full_cycle` with the outlet pinched during the water phase | power backs off at once, `p_max` below the soft limit, the cycle ends with a stated `end_reason` | |
| C5 | A key press during every phase | the pump stops within 1 s | |

**Prompt for a new session on branch `s1-1b-pressure-control`:**

```text
Read CLAUDE.md, ../roadmap/stage1-FINISHED-prototype.md (§3 "Keeping notes" and S1-1b only),
../LOG.md (F132-F136 and the "S1-1b" entry), src/Hardware/Pump.cpp, src/Core/pressure_ramp.*,
src/Tests_mech.cpp (cmd_pressure_ramp). Do part 2 only, in this order:
1. src/Core/pressure_control + native tests with a simulated plant (R from 1x to 10x and a sudden
   jump, lag, dead time, pulsation noise). Do not touch the firmware until these pass.
2. Rebuild CtrlPump::run() on it (setpoint ramp from 0, soft limit + rise-rate limit, cap on every pass,
   fresh controller per run, no blocking ramp, end reason, key-press stop). List every behavior change.
3. The 10 Hz RAM record and CSV dump for pressure_ramp (and a pump_step test).
Do not change the safety cap logic itself, do not weaken any rule 4 invariant. Results of part 1 (R1-R5,
the gains it suggests): <paste>. Do not flash or commit without asking.
```

**Notes (fill in):**

| Date / who | `pressure_ramp` runs (target, rate, `p_max`, `hold_mean`, `decay_drop`) | Manometer at 2.5 bar (F136) | Leak seen (where) | `leak_hold` on the clogged filters | K / lag / dead time (clean, pinched, blocked) | Log file | Surprises |
|---|---|---|---|---|---|---|---|
| | | | | | | | |

### S1-2: a real cycle

**Changed 2026-10-15.** The row-7 failure of S1-1 (clogged filters, cap reached almost at once, F132–F135) moved items 1 (the stop reason), 2 (the key press and the cap stop) and 6 (`PRESSURE_TARGET_BAR`) into **S1-1b**, because they belong to the new pressure controller. S1-2 keeps items 3, 4, 5, 7 and the flowmeter weighing, and adds `full_cycle ... auto` and the record fields that the laptop runner of S1-3 reads. The filling rule changes from "PID at a fixed 2 bar" to the pressure levels of S1-1b: the pump adapts its flow to hold the target pressure as the filter clogs, and a clogging filter ends the sample with `end_reason=clogged` or `timeout` and the partial volume, flagged.

Prompt for a new session on branch `s1-2-real-cycle`:

```text
Read CLAUDE.md, ../roadmap/stage1-FINISHED-prototype.md (§3 "Keeping notes" and this step only),
../roadmap/stage2-pilots.md FW4 prompt (for context: do only the items named here),
src/Step_functions.cpp, src/Hardware/Pump.cpp.
Hose: <ID> mm x <L> m. Shield time to use: <ms, or keep 25000 as PLACEHOLDER>.
1. CtrlPump::run() returns why it stopped: condition met / timeout / pressure cap / implausible /
   aborted. The decision is a pure src/Core/ function with tests.
2. Key-press stop: inside CtrlPump::run(), any byte on Serial stops the pump and returns
   "aborted". step_* then call stop_procedure() and return false. Same for cap and implausible:
   the cycle stops there, it does not go on to the next phase. A timeout in the water phase
   prints a loud WARNING and continues to the emptying (the filter probably clogged: still a sample).
3. Timeouts: SAMPLE_MAX_DURATION_MS (30 min, PLACEHOLDER) and PURGE_MAX_DURATION_MS in Settings.h
   replace the two hard-coded 3 min. STX_SAMPLE_MILLILITERS = 1000 (PLACEHOLDER until the lab
   answers).
4. Purge: INTAKE_HOSE_ID_MM, INTAKE_HOSE_LENGTH_M, INTERNAL_DEAD_VOLUME_ML (PLACEHOLDER),
   PURGE_DEAD_VOLUME_FACTOR = 3; purge_volume_ml() in src/Core/ with a test; step_purge prints it.
5. Shield: DNA_SHIELD_FILL_MS in Settings.h replaces FILL_STERIVEX_TIME / 10. (PLACEHOLDER).
6. The hard-coded setpoint 2 becomes one constant PRESSURE_TARGET_BAR = 2.0 (STX_MAX_PRESSURE is
   already 2.5 since S1-1).
7. full_cycle's TEST line gains end_reason, water_s and p_max.
8. Notes: the ../LOG.md entry (every behavior change listed), the pending checks, CLAUDE.md
   placeholders list, ../reference/commands.md, and this step's Notes block.
Not now: auto-unlock, locked rest state, coils off, V23 removal, float flow factor, panel pump mode.
Hardware checklist: flowmeter by weighing (full_cycle 1 volume=500 purge=n shield=n empty=n,
outlet into a tared jug: new factor = 77 x shown mL / weighed g; repeat until within ±5 %);
a key press during the water phase stops the pump within 1 s; full_cycle 1 volume=1000 ends PASS
with end_reason=volume; full_all count=3 volume=500; pinch the outlet mid-run -> the cycle stops
with end_reason=pressure_cap.
```

**Notes (fill in):**

| Date / who | Flow factor (was 77) | Flow at 2 bar (mL/min) | Time for 1 L | Purge volume used | Key-press stop (s) | Log file | Surprises |
|---|---|---|---|---|---|---|---|
| | | | | | | | |

### S1-3: laptop campaign runner (replaces the Raspberry Pi for stage 1; decided 2026-10-15)

**Decision (user, 2026-10-15).** The original plan was a Raspberry Pi (or a mini computer) as the master and the Due as the slave. There is no time for that this week, so **the laptop is the master**, on the Programming port, speaking the console text the machine already has. A person stays at the machine and brings the laptop to the field. This is the same master/slave split as the Pi plan (the laptop decides what to do and when; the Due executes one filter at a time under its own safety layers), so nothing here has to be undone: the Pi and protocol v1 (FW7, HO1) stay the stage 2 plan, and stage 2 already says "the laptop is the host for Pilot A" ([roadmap/stage2-pilots.md](../roadmap/stage2-pilots.md) §2).

**Two kinds of commands from now on.**
- **Test commands** (`pressure_ramp`, `leak_hold`, `rotate_all`, ...): explore and measure, may ask the operator questions.
- **Operation commands** (the ones the runner sends): `full_cycle <slot> ...` and `actuator_home`, `STOP`-by-key. Rules: no question to the operator in the `auto` form, one `TEST|...|PASS/FAIL|k=v;...` result line with the fields below, an `end_reason` in it, a `REFUSED |` line and no motion when a precondition is missing, and the O-ring state known at the end. Anything the runner needs that is not in that list is a firmware change of S1-2, not a runner workaround.

**The runner** (`tools/campaign.py`, Python stdlib + pyserial, built on the parser of `tools/hw_test_log.py`; tests in `tools/tests/`, `pytest tools/`):
1. Opens the port. **Opening it resets the Due**: it waits for `=== Test menu READY`, then asks the operator to confirm the manifold is on a slot, and sends `actuator_home`.
2. Reads a **plan**: a list of samples `(sample_id, slot, role = sample | blank | field_blank, volume_ml, optional start time)` or `--count N --interval-min M` for a timed series. The slot is always explicit (`full_cycle <slot>`), never "the next free one".
3. Owns the **slot inventory** (F1): a CSV in `data/` that survives a Due reset and refuses a slot already used.
4. For each sample sends `full_cycle <slot> volume=<mL> purge=y shield=y empty=y`, mirrors the whole transcript to `docs/test-logs/<date>_campaign.log`, parses the `TEST|full_cycle|...` line, and appends one row to `data/samples.csv`.
5. **Sample record = the stage 2 columns** that exist in stage 1 (stage 2 roadmap §3.x, FW8): `sample_id`, `slot`, `role`, start and end time (UTC, from the laptop clock), `volume_ml` (asked and measured), `duration_s`, `end_reason`, `p_max`, `p_mean`, `purge_ml`, `shield_ms`, firmware version, `operator_notes`. Same names as HO2, so the file is not thrown away later.
6. **Stops the campaign** (and does not resend anything) on: `SAFETY |`, `REFUSED |`, a FAIL result, a boot banner (the Due reset in the middle: lock state UNKNOWN, slot memory lost, F125), no byte for `--idle-timeout` seconds during a cycle (120 s, PLACEHOLDER), or Ctrl-C (it sends one key byte, which stops the pump loops, then waits for the result line).
7. After a stop it prints what the operator has to do (`actuator_home` with the rotor on a slot, check the filter, cap the used filter) and exits non-zero.

**Not in S1-3:** protocol v1, the `cowas` host, Native USB, the Pi, remote access, notifications, an E-stop, unattended running. The laptop must be on mains or a charged battery with sleep and USB power saving off: a sleeping laptop resets the Due (§4).

**Firmware side (small, in S1-2):** `full_cycle` takes `auto` (no pauses, no questions) and its `TEST` line carries the record fields above (`end_reason`, `water_s`, `p_max`, `p_mean`, `sampled_ml`, `purge_ml`, `shield_ms`). Check first whether `full_cycle` asks anything today.

**Hardware checklist:**
1. `python tools/campaign.py --port COM6 --plan plan_bench.json` with 3 samples (slots 1–3, 100 mL, bench water): 3 rows in `data/samples.csv`, a log file, no SAFETY line.
2. Pull the USB cable in the middle of a cycle: the runner stops with a clear message, keeps the log, the CSV has the partial row marked `link_lost`. Plug back in, `actuator_home`, run again with the same plan: the used slots are refused.
3. Ctrl-C during the water phase: the pump stops within 1 s, the runner prints the end reason.
4. `--count 3 --interval-min 2`: three cycles two minutes apart, the start times in the CSV agree with the clock within 5 s.

**Notes (fill in):**

| Date / who | Series run (count, interval, volume) | Result | Log file | Surprises |
|---|---|---|---|---|
| | | | | |

### S1-4: merged into S1-1 (2026-10-09)

The manometer calibration was the last step of the stage. A manometer is on the line since 2026-10-09, so it is now part B of S1-1, and its notes go in the S1-1 block.

## 4. What is knowingly missing, and how to live with it

The prototype adds safety compared with today (a cap that works, a dead-sensor stop, a key-press stop). It does not remove any existing check. But these layers of stage 2 (§3.2) do not exist yet:

| Missing | Consequence | Rule for the stage 1 tests |
|---|---|---|
| Relief valve, E-stop (HW2) | The software cap is the only pressure protection | Someone within reach of the 24 V switch for every run. Fit the relief valve if the part is there: it needs no code |
| A reference better than a 2.5 bar dial manometer | The calibration stops at 2.0 bar and the manometer at 2.5; the cap is 3.0 (user decision 2026-10-12, F136), so 2.0 to 3.0 bar is the fitted line extended. | A trip at 3.0 bar can't be read on the gauge. Check the sensor against the manometer at 2.5 bar during `pressure_ramp`; if the gauge can't take a short overshoot above 2.5 bar, close it off. The cap has not been checked against the rating of the weakest part (Sterivex housing, tubing, valves, O-ring) |
| Watchdog, supervisor (FW5) | A firmware hang with the pump on runs until someone cuts power | Never unattended |
| Stop outside the pump loops | No stop during a rotation, a lock move or the micro pump (bounded: 15 s, ~7 s, shield time) | Wait, or cut power |
| Recovery (FW6) | After any reset: lock state UNKNOWN, slot memory lost. If the rotor is between slots, manual recovery (F60) | `actuator_home` after every reset; keep a paper list of used slots |
| Cap not polled in the panel pump mode (F2), `calibrate_flow` (F41), `fillContainer` (F42); the `CtrlPump` loops are unreliable on a clogged filter (F132–F135) | The first three can pump without the cap; the others reach the cap almost at once on a clogged filter | Don't use the first three. Until S1-1b part 2: no `leak_hold`, `full_cycle` or `load_slot*` on clogged or nearly clogged filters; use `pressure_ramp` for pressure checks |
| O-ring unlocked at rest, used filters open (F29) | Shield may drain, cross-talk unknown | Cap filters by hand after each run; fine for short tests |
| Purge factor, shield dose, residual water (LA3, LA4) | **Filters from these tests are engineering samples**, not publishable eDNA data | Tell the lab; include a blank anyway |
| Flowmeter 5 V level (F26), DAC range (F25) | Possible damage / unknown pump speed | S1-0 item 5 |
| Laptop as the master (no Pi, no protocol v1) | The Due still boots into the console menu. Opening the serial port resets it, and so does a laptop sleep, a USB glitch (F125) or a pulled cable: the lock state becomes UNKNOWN, the slot memory is lost and the pump stops | The runner stops the campaign on any boot banner and never resends a command. The operator runs `actuator_home` (rotor on a slot) before going on. Laptop on mains or a charged battery, sleep and USB power saving off |
| Slot inventory is in the laptop file, not in the Due (F1) | After a Due reset the Due thinks every filter is available | The runner owns the inventory (a CSV in `data/`), passes the slot explicitly, and refuses a slot already used. Keep a paper list as well |

## 5. Handover to stage 2

1. Finish FW3 (delete the A2 code, rename to `p_in`, SPI diagnostics).
2. Finish FW4 (auto-unlock, locked rest state, coils off, V23, float flow factor).
3. Continue [roadmap/stage2-pilots.md](../roadmap/stage2-pilots.md) from FW5. The numbers in the Notes blocks above (flow at 2 bar, time for 1 L, zero offset, hose size) are inputs to FW4, HW3 and LA4.
4. Replace the laptop runner (S1-3) by the `cowas` host over protocol v1 (FW7, HO1–HO3); the sample CSV keeps the same column names, so the files from stage 1 stay usable. The Raspberry Pi (HO4) is only needed for Pilot B.
5. Finish S1-1b part 2 if it was cut: the controller in `CtrlPump` is the base of FW4's pressure behaviour and of the clog analytics of stage 3.
