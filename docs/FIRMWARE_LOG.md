# CoWaS firmware log

One entry per ROADMAP step, newest at the bottom. Each entry says what changed, which **behavior** changed, how it was tested, and what the hardware checklist gave. It also records new findings. Every Claude session reads this file after CLAUDE.md, and adds its entry before the step is merged. It is also the source for the final report.

New findings get the next free ID after ROADMAP §4 (F37, F38, ...). **When steps run in parallel sessions**, each one uses the ID range written in its prompt or given by the user (e.g. FW2: F60–F69, HO1: F70–F79), so two branches never claim the same number; the gaps are fine. The "Step" column says where each one should be resolved. When a finding is fixed, write the step and the date in the "Status" column; don't delete the row.

---

## Findings log (new since ROADMAP §4)

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F37 | Console commands are exact string matches: a trailing `\r` (CRLF line ending) makes every command silently do nothing. Prefix commands (`slotN`, `clockMM`) still work. Worked around in the monitor settings (`monitor_eol = LF`), not in the firmware. | `Tests.cpp:87`, `main.cpp:254` | Medium | FW7 (the protocol parser must strip CR) | open |
| F38 | A bare `clock` or `clockxx` parses as minute 0 and **rotates to slot 7** instead of printing an error. | `Tests.cpp:277-290` | Low | FW2 | fixed FW2 (2026-09-28) |
| F39 | Legacy Pi link: **any line starting with `sample`** runs a full sample; a malformed depth only prints "Error depth" first. | `main.cpp:257-298` | High (legacy) | FW7 (legacy protocol retired) | open |
| F40 | `step_purge()` (`purgeSterivexFunction`, `purgeContainerFunction`) **leaves the O-ring locked** at the purge slot; the next bare `rotateMotor()` then rotates while locked. | `Step_functions.cpp:220-259` | High | FW4 (auto-unlock before rotation) | open |
| F41 | `calibrate_flow` sets no valves before pumping, and its pump loop doesn't poll the pressure cap (only `pump.start()` checks it). | `Tests.cpp:1384-1474` | High | FW4 / FW5 | open |
| F42 | `step_fill_container()` runs the pump at 100 % for up to 11 min (the container switch D27 is gone), without polling the cap. Reached by `fillContainerFunction` and `purgePipesFunction`. | `Step_functions.cpp:126-213` | Medium | FW5 (cap polling); P2 (remove container code) | open |
| F43 | `zero` (`go_to_zero()`) has no timeout and no encoder-failure check: it spins forever if the encoder fails. It also ignores the lock. | `Tests.cpp:1841-1873` | Medium | FW2 | fixed FW2 (2026-09-28): timeout, encoder-failure stop, lock guard |
| F44 | `calibrate_pressure2` computed the new offset from `OLD_OFFSET = 0.483`, but the sensor code used `OffSet = 3.105`. | `Tests.cpp` | Low | FW3 | **closed 2026-10-12:** the A2 code and `calibrate_pressure2` were deleted (user request) |
| F45 | `Flow_sensor.h` declares `flow_small = 12` and `flow_big = 13`, the opposite of Settings.h (`FLOW_SMALL_PIN = 13`, `FLOW_BIG_PIN = 12`). Unused, but misleading. | `Flow_sensor.h:11-12` | Low | FW4 (flow sensor work) | open |
| F46 | `scale_angle()`'s loop guard (`loop_nb`) is never incremented: ±inf would loop forever. Not reachable from the encoder path. Kept as is in FW0 (no behavior change). | `src/Core/manifold_geometry.cpp` | Low | FW2 | fixed FW2 (2026-09-28): bounded, bit-exact within 1024 turns (goldens pass), fmod beyond, NaN for ±inf |
| F47 | VNH5019 diagnostics are never read: EN/DIAG D5/D30 and current sense A10/A11 (`Motor::stopIfFault()` has no caller). Motor faults and stalls go unseen. | `Motor.cpp:218` | Medium | FW5 | open |
| F49 | `Serial.begin(9600)` is called twice: `C_output.cpp:30`, then again at `main.cpp:202`. The second call resets the UART while the last boot lines are still in the TX buffer, so `system initalized` / `Programm started` and the last "Button ... initiated" lines are lost and a garbage byte appears. Seen on the FW0 smoke test. Cosmetic today, but it hides boot messages that FW6 will rely on (reset cause, self-check). | `main.cpp:202` | Low | FW6 (boot rework) | open |
| F50 | **No pressure sensor is installed** (FW0 bench check, 2026-09-24). The analog sensor on A2 (`pressure2`) was physically removed, yet it still backs the pump cap and every PID loop: `pressure` reads 0.11 bar from the floating pin, **so the cap can never trip**. Nothing answers on the ABP chip select D8 (`pressure1`): `ERROR \| P1: invalid SPI response (bad status bits)`, 0.00 bar. Until FW3 + HW4: no pumping against a closed or clogged outlet, and none unattended. FW3 renames `pressure1` → `p_in` and deletes the A2 code (user-approved). | `main.cpp:116-139`, `Step_functions.cpp`, `Pressure_sensor.cpp` | **Blocking** | FW3 + HW4 | **half open (2026-10-06):** an ABPDANV150PGSA3 is installed on D8 and answers (user). The cap and the PID loops still read A2 until stage 1 S1-1. **Code fixed S1-1 (2026-10-09):** cap and PID read the ABP (`read_p_in`), `PRESSURE_SENSOR_AVAILABLE = true`; closed when the S1-1 bench checks pass (dead-sensor stop, cap trip) |
| F51 | **The 3/2 valve V23 was physically removed** (user, 2026-09-24). Its relay on D40 still clicks (`valves` test), and the code still switches `valve_23` in `step_fill_container`, `sample_process`, `demo_sample_process`, `purge_pipes_manifold`, `calibrate_DNA_pump`, `test_valves` and the panel mode V23. Harmless, but dead. Removal is user-approved and planned in FW4 item 11. | `main.cpp:79,136`, several | Low | FW4 | open |
| F70 | The Due's dedupe table (last 16 ids) outlives a **host** restart, but PROTOCOL.md §3.1 only asks ids to be unique "for the lifetime of a host session". A restarted host that counts from `h-000001` again could get an old stored result instead of an action (e.g. a `ROTATE` that silently doesn't move). The host now uses a random per-session prefix (`h<6 hex>-<n>`). §3.1 should say "unique while the Due is up (use a per-session prefix)". | PROTOCOL.md §3.1, `host/src/cowas/client.py` | Medium | FW7 (doc wording) | mitigated in HO1 (2026-09-28) |
| F71 | **RECOVER can deadlock**: with the lock state `unknown` (every boot) and the manifold between slots, the `align` step needs a rotation (forbidden while the lock is unknown), and `ACTUATOR_HOME` refuses `not_aligned`. PROTOCOL.md §5.6 orders `align` before `oring`. The simulator fails RECOVER at `align` (`recovery_failed`). FW6 must choose: a manual procedure (free the O-ring by hand, `service`), or homing the actuator off-slot if that is mechanically safe. | PROTOCOL.md §5.6, §5.10 | Medium | FW6 (+ FW4) | open |
| F72 | The request vectors' `expect` lists contain protocol messages only, but §3.6 says every console line is also sent as `{"t":"log"}`. A Due that logs while accepting a command would fail a literal match. The host tests leave out `t:"log"` when they compare; FW7's C++ tests should do the same, or §13 should say so. | `protocol/vectors.json`, PROTOCOL.md §13 | Low | FW7 | open |
| F73 | The element format of `STATUS.data.warnings` isn't specified (every vector has `[]`). The host and the simulator assume `{code, msg, phase}` like the record's `warnings`. | PROTOCOL.md §5.5 | Low | FW7 | open |
| F74 | "Every pumping command needs `P_in`" (§5.8) doesn't list the commands. The simulator refuses `RUN_CYCLE`, `PURGE`, `SAMPLE`, `DNA_SHIELD` and `EMPTY` (air push / empty use the diaphragm pump) without `caps.p_in`, but not `PRIME_SHIELD` (micro pump only). | PROTOCOL.md §5.8, `host/src/cowas/protocol/spec.py` | Low | FW7 | open |
| F52 | `enc_rot` (`getRotationSPI()`) prints a meaningless turn count (16382 = 0x3FFE: the single-turn AMT22 doesn't answer the turns command) and an angle above 360° (637.38°): it scales the 14-bit value by the 12-bit factor without the `>> 2`. Display only; `rotateMotor()` uses `getPositionSPI()`, which is correct. | `Manifold.cpp:449-474` | Low | FW2 | fixed FW2 (2026-09-28): angle fixed (`>> 2`); the turn count is still meaningless on a single-turn AMT22 |
| F48 | F12 correction: `pio run -e uno` fails because of Due-only code (`DAC1`, `A10`, `A11`, `Serial1`, SAM3X registers in `Encoder_atmel.cpp`), **not** because `include/interfaces` is missing (it doesn't exist, but `due` builds with the same flag). | `platformio.ini`, several | Low | P2 (drop `uno` or guard the code) | open |
| F120 | **One bad SPI frame stops the pump.** Since S1-1 a single fault frame from the ABP (status 01 or 11, FF FF FF, 00 00 00) makes the reading implausible at once, and the cap stops the pump; only stale frames (status 10) get a 500 ms grace. Fail-safe, but electrical noise from the 24 V pump on the SPI lines could cause nuisance stops in the middle of a sample. If the bench shows them: require 2–3 consecutive fault frames (a few ms) before distrusting. | `src/Core/abp_convert` (`abp_trust_update`), `Trustability_ABP_Gage.cpp` | Medium | S1-1 bench, else FW3 | open (watch for `ERROR \| P1: pressure reading not trusted` during pumping) |
| F121 | The reference manometer is not identified: a 0–2.5 bar dial gauge that "looks like a Riegler 107-KD" (user, 2026-10-09), accuracy class not read. `CAL_PRESSURE_MAX_RESIDUAL_BAR = 0.06` assumes class 2.5 (±0.0625 bar) and is a PLACEHOLDER. The calibrated cap is only as good as this gauge and the reading of its needle. Stage 2's bench acceptance (VA1) may want a better reference. | `Settings.h` | Low | S1-0 item 7 | **answered 2026-10-12 (S1-0 item 7):** class 1.6, 0–2.5 bar, 0.1 bar divisions (user). 0.06 bar kept = 0.04 bar of class + about 0.02 bar of needle reading; no longer a placeholder |
| F122 | With `PRESSURE_SENSOR_AVAILABLE = true`, the commands that run the pump **without polling the cap** are reachable again: the panel Pump mode (F2), `calibrate_flow` (F41), `fillContainerFunction` / `purgePipesFunction` (F42). Each still checks the cap once at `pump.start()`. Stage 1 rule (§4): don't use them, use `full_cycle` / `leak_hold` / `pump`. | `main.cpp`, `Tests.cpp`, `Step_functions.cpp` | High | FW4 / FW5 (as F2, F41, F42) | open |
| F123 | The driver's "Pressure to high on sensor" print (above the `begin()` limit of 3 bar) is not throttled: it prints at every read. Pre-existing; with the cap at 2.5 bar it should not be reached. | `Trustability_ABP_Gage.cpp` | Low | FW3 | open |
| F124 | **The ABP returned garbage frames with a "valid" status** (bench, 2026-10-09, firmware before S1-1). Three `abp_raw` runs: 20 × `FF FF FF` (no answer); then counts 13377–16321 (9–11.6 bar); then counts 64–3520, all with status `00`. The counts did not follow the syringe pressure, the low 7 bits of byte 2 were almost constant (`40` / `C0`), and byte 3, which should be the temperature, was a copy of the pressure bits shifted by 13 bits. Not a pressure reading: wiring, supply, ground or SPI timing. **The status bits alone can't catch this**, so the S1-1 trust rule would accept these frames; the cap would then mostly stop the pump (readings above 2.5 or below −0.5 bar), but a garbage value between the two looks normal. **No pumping and no calibration until `abp_raw` is steady at rest.** S1-1 lowers the SPI clock from 800 kHz (the datasheet maximum) to 100 kHz and `abp_raw` prints the spread and the temperature. If frames can still be corrupted after the wiring is fixed: add a consistency check (temperature byte range, jump limit between reads) in FW3. | `Trustability_ABP_Gage.cpp`, wiring of P1 | **Blocking** (S1-1 bench) | S1-0 / S1-1 bench, FW3 | **fixed S1-1 (2026-10-09): the SPI clock.** With `ABP_SPI_HZ = 100000` and no hardware change (user), `abp_raw` is clean at rest (spread 5 counts) and under about 1 and 2 bar (smooth, 20 °C on every line). 800 kHz was too fast for this wiring. The consistency check stays a FW3 idea |
| F125 | **USB link dropped and the Due reset when `actuator_home` was typed** (bench, 2026-10-09): the monitor printed `Disconnected (ClearCommError failed ...)`, reconnected, and the boot messages followed. Not known whether the actuator had started to move (the manifold was 9.84° off slot 4 afterwards, so a second `actuator_home` refused). Candidates: a supply dip or ground bounce when the A4988 enables, a loose USB cable, the Due powered from USB only. May share a cause with F124 (supply / ground). Reproduce it before trusting any long run: a reset in the middle of a sample leaves the pump relay state to the boot code and the lock state UNKNOWN. | power / grounding (HW1 audit M-items) | High | S1-0 bench, HW1 | open |
| F126 | **The ABP and the manometer disagree by a factor of 2.4, unresolved.** The part was ordered and supplied as `ABPDANV150PGSA3` (user, 2026-10-09: "clearly", the supplier is trusted). Measured, syringe + manometer on one tee (2026-10-09): about 1 bar gave about 4545 counts and about 2 bar about 7700 (rest: 1634). A 150 psi part would give 2905 and 4173; a 60 psi part (`ABPDANV060PGSA3`, 4.14 bar, the part ordered in HW0) gives 4806 and 7975. So either the sensor does not follow the 150 psi transfer function, or the manometer reads 2.4 times too low; the firmware can't tell which. **Decision (user: "do what's best for it to work"): the manometer is the reference**, so `ABP_FULL_SCALE_PSI = 60` (PLACEHOLDER), the value that agrees with it, and `calibrate_pressure` then fits a gain near 1. One argument for the manometer: 4.8 bar from a hand-held 12 mL syringe needs about 10 kg on the plunger and less than 2.5 mL of dead volume, while 1.9 bar needs about 4 kg. **Risk if the manometer is the wrong one:** the real pressure would be 2.4 times the displayed one, and the 2.5 bar cap would sit at 6 bar. **User decision (2026-10-09): trust the manometer and move on; no independent check is planned.** If one is wanted later: the marking on the sensor body, or a water column (1.00 m of water = 0.098 bar = 124 counts on a 150 psi part, 311 counts on a 60 psi one). | `Settings.h` | **High** | S1-1 bench (read the marking, calibrate) | open |
| F127 | `jog_until_keypress()` (stage 2 of `calibrate_actuator`) restarted its step count at zero each time the operator answered `n` ("not stalled yet"), so the printed `LINEAR_ACT_LOCKED_STEPS` only counted the steps after the last `n`: too small. The safety bound also restarted. | `Tests.cpp` | Medium | S1-0 | **fixed 2026-10-12:** the count and the bound now cover the whole call |
| F128 | The manifold stop offsets (`angle_offset_pos` 5.2 CW, `angle_offset_neg` -1.05 CCW) were never checked on this machine. `rotate_all dry` (2026-10-12) gave errors against the slot table of -2.11 to -2.46 deg turning CCW and -3.78 to -4.13 deg turning CW; `encoder_repeat 5 10` gave -3.95, spread 0.09: a repeatable pure offset, and the coast after the motor is cut is about 1.2 deg both ways (speed 40). With the offsets set to 1.2 / +1.2 the same tests give -0.35 to +0.18 deg and a spread of 0.18 (the results match that build; the flashed build was not read back). **Open:** these errors are against the table. One manifold aligned by hand read -2.81 deg against it, so `purge_angle` (342.07) may itself be about 2.8 deg off the holes | `Manifold.cpp`, `Manifold.h` | Medium | S1-0 | offsets **changed 2026-10-12**, physical alignment **not verified** (TESTS_PENDING): align by hand 5 times and read the encoder; if they agree within about 1 deg, run `cal` and paste `purge_angle`, then redo the offsets |
| F129 | A rotation between slot 0 and slot 1 took the long way round (about 338 deg instead of 22.5). `rotateMotor()` picks the direction from the sign of the difference of two `scale_angle()` values, and the wrap point of that function (+/-180 deg) sat exactly halfway between slot 0 and slot 1. A move never crosses the wrap point. Reported by the user (2026-10-12), who wants the wrap point between slots 3 and 4 (the no-hole position). | `Manifold.cpp:496`, `Manifold.h` | Medium | S1-0 | **flashed and reported working by the user 2026-10-12** (slot0 to slot1 direct; no numbers kept; the rotate_all runs of the same day were on the build before it): the wrapper `scale_angle()` uses `scale_reference_angle` (the wrap on the no-hole position); `core::scale_angle()` and the goldens are untouched. Scaled order 3, 2, 1, 0, 14 ... 5, 4. Check: `slot0` then `slot1` turns 22.5 deg, `rotate_all dry auto` still passes |
| F130 | The purge volume depends on the whole water path, and the spool module adds most of it. Measured 2026-10-12 (user): the 2.40 m path of 4 mm ID is the whole water path before the exit, including the pump, sensors and tubing: about 30 mL (12.6 mL/m). So the dead volume without the spool is about 30 mL, and 3 × it is about 90 mL: the current 100 mL meets the rule, with little margin. The spool path (9 mm ID × 40 m, about 2.5 L, 63.6 mm² × 40 m) is connected by default. With it, the dead volume is about 2.57 L and 3 × it is about 7.7 L. For calibration the user wants a smaller, quick purge: allowed only if the calibration run is labelled as not a sample (CLAUDE.md rule 8, §3.7), and the purge value is written in the record. (An earlier note said the spool was 40 mm and about 2.5 mL: wrong, corrected.) | `Settings.h` (`PURGE_MILLILITERS`) | High (data quality) | S1-2 | partly fixed 2026-10-12: `calibrate_all` asks whether the spool is connected (default yes) and prints the purge the rule needs (`ask_spool_and_purge_check()`, Tests.cpp). Standalone `calibrate_flow` does not ask; the production purge (`PURGE_MILLILITERS`) is not changed yet; the calibration purge value and its label; the sample purge (100 mL meets the rule without the spool, not with it) |
| F131 | The hand alignment and the table disagree by about 3.5°. Five hand alignments read with `oring_state` (manifold moved away and back by hand between readings): -3.69, -4.31, -3.52, -3.69, -2.99 deg (mean -3.6, range 1.3, sd about 0.5). An earlier hand reading was -2.81. The motor stops land within ±0.35° of the table (`rotate_all dry`, offsets 1.2 / +1.2). The offsets were tuned against the table, so they hide the gap. Either `purge_angle` (342.07) is about 3.5° off the real holes, or the hand alignment is biased or scattered by about ±0.7°. Not resolved: this decides whether the O-ring seals on the port. | `Manifold.h` (`purge_angle`), `Manifold.cpp` (offsets) | Medium | S1-0 | deferred to pilot (user decision 2026-10-12): the air-flow check (angle of maximum flow through the port, V1 open) before `cal` is repeated. Risk until then: if the gap is real, the O-ring may not seal the port fully |
| F132 | **The PID loops are blind for up to 4 s after every big step.** Since the soft start of 2026-10-12, `Pump::set_power()` on a running pump calls `move_output_to()`, which ramps over `PUMP_RAMP_MS` (4 s, 20 ms steps) for any change of `PUMP_RAMP_MIN_STEP_PERCENT` (10 %) or more. The loops use `pump_pid` with Kp = 200 % per bar (`main.cpp:201`), so an error of 0.05 bar already asks for 10 %. While the ramp runs, `CtrlPump::run()` serves nothing else: no PID update, no flow stop, no key press (only the cap, every 20 ms). A clogged filter (pressure up fast) gets its power cut over 4 s instead of at once. | `Pump.cpp` (`set_power`, `move_output_to`), `main.cpp:201` | **High** | S1-1b | open. `pressure_ramp` avoids it (output steps ≤ 3 %, `PRESSURE_RAMP_MAX_STEP_PERCENT`); `CtrlPump` is fixed in S1-1b |
| F133 | **Every `CtrlPump::run()` starts open-loop at 50 %.** `set_power(50); start();` runs the 4 s soft start up to 50 % before the PID is consulted. On a clogged or nearly clogged filter 50 % is more than the line can take: the pressure crosses `STX_MAX_PRESSURE` during the start ramp and the cap stops the pump "almost at once" (reported 2026-10-15, S1-1 row 7). **Hypothesis, not proven:** no pressure curve was logged. | `Pump.cpp` (`CtrlPump::run`) | **High** | S1-1b | open |
| F134 | **`pump_pid` is untuned for a changing filter and never reset.** The same instance serves purge, sampling and emptying; `run()` never calls `reset()`, so the integral and the last output of the previous phase carry over. Gains 200 / 100 / 100 (setup) against 200 / 100 / 0 (constructor); the D term acts on a pulsating diaphragm-pump signal; no output slew limit; no anti-windup beyond the MiniPID clamp. The plant gain (bar per % of power) grows as the filter clogs, so gains that are right for a clean filter oscillate or overshoot on a clogged one. | `main.cpp:199-201`, `Pump.cpp` | Medium | S1-1b | open |
| F135 | **The cap is polled every 100 ms inside `CtrlPump::run()`** (once per PID update; between updates only the flowmeter is polled), 50 Hz in the soft start. The ABP can be read every ms. A spike shorter than 100 ms can be missed, and the SAFETY line prints the one reading that tripped, not the peak. The periodic print is throttled to 3 s, so a spike is invisible in the log. | `Pump.cpp` (`CtrlPump::run`) | Medium | S1-1b | open. `pressure_ramp` checks the cap on every pass (about 2 ms) and prints `p_max` and when it happened |
| F136 | **The cap is 3.0 bar, the calibration stops at 2.0 and the manometer at 2.5.** Between 2.0 and 3.0 bar the sensor is extrapolated along the fitted line (linear part, small risk), and nothing on the bench can confirm a trip at 3.0. User decision 2026-10-12 (3 bar seal test). Cap-trip check (row 6) was done at a temporary 1.0 bar. | `Settings.h` | Medium | S1-1b | open: check the reading at 2.5 bar against the manometer end of scale during `pressure_ramp` |

---

## FW0: Baseline, test harness, CI, pin map (2026-09-24)

**Branch:** `Quentin-sealingMotor-stepperMotor` · **Base commit:** `d508f41` · **Status:** smoke test passed; waiting for the commit + tag `baseline-v0`.

**Decisions taken with the user (2026-09-24):** delete the A2 analog pressure code and rename `pressure1` → `p_in` in FW3 (not FW0: FW0 must not change behavior); delete the V23 code in FW4. Both are written into the ROADMAP prompts.

**Behavior changes:** none. The firmware diff is the manifold-math extraction plus comments:
- `src/Core/manifold_geometry.{h,cpp}` (new): the slot-angle table, clock labels and `scale_angle`, moved out of `Manifold.cpp` unchanged.
- `src/Hardware/Manifold.cpp`: the four functions now call `core::`. It adds a `static_assert` that `NB_SLOT` matches the geometry.
- `include/Manifold.h`: `MANIFOLD_RAW_POSITIONS = 16` (replaces the literal 16s).
- `include/Settings.h`: comments only (pointers to PINOUT.md).
- `platformio.ini`: the `native` test env, `test_ignore = *` on `due`, and serial-monitor settings (LF line ending, send on ENTER, local echo).
- Size: flash 85,848 → 86,112 B (+264); RAM unchanged at 4,564 B.

**Added:** `CLAUDE.md`, `.github/workflows/ci.yml`, `docs/PINOUT.md`, `docs/COMMANDS.md`, `docs/TESTS_PENDING.md`, this log, `test/test_manifold_geometry/` (15 tests), `test/stubs/`, and `tools/golden/` (regenerates the goldens from `d508f41`).

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 15/15 pass. The goldens (the table, the clock labels in both directions, 1,335 `scale_angle` inputs) are compared bit for bit. Mutation check: breaking the clock rounding or the ±180° boundary makes the tests fail.
- The goldens were computed on the PC (g++), not on the Due. The smoke test step `clocklist` checks the labels on the board.

**Hardware smoke test:** to fill in:

| Check | Pass criterion | Result |
|---|---|---|
| Boot prints `system initalized` then `Programm started` | both lines (or at least: the menu answers) | partial: output stops after `Button B_start initiated` + a garbage byte (F49, pre-existing); the menu answers → pass |
| `clocklist` | 0=30, 1=34, 2=38, 3=41, 4=49, 5=53, 6=56, 7=0, 8=4, 9=8, 10=11, 11=15, 12=19, 13=23, 14=26 | **pass** (2026-09-24) |
| (precondition) O-ring unlocked | found locked after the reboot (F18): unlocked by hand with `linear_actuator` → `e` → `f` ... → `x` | |
| `slot0`, `slot5`, `clock00` (→ slot 7), `slot0` | each < 15 s, no `ERROR`, aligned within ~1° | **pass** (visual) |
| `enc_rot`, `pressure` | print values, no hang | **pass**, with findings: `enc_rot` = turns 16382, angle 637.38 (F52, display bug); `pressure` = P1 SPI error + 0.00 bar, P2 = 0.11 bar from a floating pin: **no pressure sensor installed** (F50) |
| `valves` | each valve clicks twice in 30 s | **pass**: 3 relays heard twice; the V23 valve itself is physically removed (F51) |

**Smoke test verdict: PASS** (2026-09-24). FW0 behaves like `d508f41`; the findings above are pre-existing hardware/code facts, not regressions.

**Environment notes (this laptop):** WinLibs GCC 16.1 installed with winget, for `pio test -e native`. PlatformIO downloads need a CA bundle that includes the Windows root store (TLS inspection); see CLAUDE.md.

**Open questions for later steps:** The team has a spare 150 psi ABP (ABPDANV150PGSA3), possibly damaged. Its check (pin identification + `pressure_sensors`) is pending in docs/TESTS_PENDING.md, and the FW3 prompt asks for it first. The 060 psi sensors are being ordered (HW0 #1). See findings F37–F52.

---

## FW1: Protocol v1 specification (2026-09-25)

**Branch:** `fw1-protocol-spec` (from `cowas-v2`) · **Status:** done (docs only); open questions answered 2026-09-25.

**Behavior changes:** none (documentation only, no code). ROADMAP prompts FW2, FW4, FW7, LA3 and HW7 were updated with the decisions; §3.2 gained the rest state.

**Added:**
- `docs/PROTOCOL.md`: transport (Native USB, JSON lines, 2048-byte max line, never 1200 baud), the six message types, modes and the mode × command table, all P0 commands with args and results, the sample record (`RUN_CYCLE` result), request-id dedupe (last 16) and the check order, error codes, heartbeat (PING 5 s, link lost after 30 s: the running procedure finishes, nothing new starts), timing and host timeouts (`ack.max_s` + 30 s), the legacy `main_program()` mapping, versioning rules.
- `protocol/vectors.json`: 95 request vectors (malformed lines, framing, unknown/reserved commands, wrong arg types, wrong modes, dedupe, check order), 30 Due → host message examples (results with null fields, forward-compatible unknown fields), 3 sequences (RUN_CYCLE happy path, STOP mid-purge + recovery, reboot mid-cycle). Generated once with a throwaway script and hand-checked; edit the JSON directly from now on.

**Decisions (user, 2026-09-25; PROTOCOL.md §14):** `max_duration_s` = water pumping phase only; new `end_reason` `duration_reached` (no-cap run reaching its duration = normal end); `GET_INFO` accepted while BUSY; STOP from IDLE latches `USER_STOP` (CLEAR_FAULT + RECOVER); **rest state = O-ring locked, unlocked only to rotate** (every rotation = unlock → rotate → lock, every procedure ends locked; written into ROADMAP §3.2 and the FW4 prompt, which implements it); a purge below the §3.7 minimum is raised to it with the warning `PURGE_CLAMPED`; dedupe keeps 16 ids with short answers/summaries + the full records of the last 4 procedures (≈ 13 kB RAM), older ones answer `result_expired` and never re-run; `device_id` = `cowas-V1`. Also decided in the spec: procedures return `ok:true` + a partial record once started (`ok:false` = nothing moved); `end_reason` = why the water phase ended; STOP is never deduplicated; refusals are not remembered; `boot_id` (hardware RNG) reveals a Due reboot.

**Deferred to later steps (written into their ROADMAP prompts):** `PRIME_SHIELD` destination and duration (LA3, then FW7); compression set of an O-ring locked for days (HW7: 7-day locked hold + `leak_hold`); filter-outlet caps with the O-ring locked at rest (LA3).

**Tests:** no code. `vectors.json` parses; names are unique; every Due → host example parses; the generated long lines are 2100 and 2047 bytes. `pio run -e due` / `pio test -e native`: unchanged tree (not affected).

**Hardware checklist:** none (docs only).

**New findings:**

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F53 | To verify: on the Arduino SAM core, `SerialUSB.write()` can block when the host has the port open but does not read. The protocol forbids blocking output (PROTOCOL.md §2); FW7 must check `availableForWrite()` and drop + count lines, and test it with a host that stops reading mid-cycle. | `SerialUSB` (FW7) | Medium (safety loop stall) | FW7 | open |

---

## LA1: Lab alignment, meeting template (2026-09-25)

**Branch:** `la1-lab-interface` (from `fw1-protocol-spec`, so the template can use the PROTOCOL.md field names; FW1 is not merged into `cowas-v2` yet) · **Status:** template drafted; **the meeting with the lab is pending**. LA1 is done (ROADMAP §7) once §4 "Decisions" of LAB_INTERFACE.md is filled in.

**Behavior changes:** none (documentation only, no code).

**Reminders placed in ROADMAP** (2026-09-25), so the steps that use the answers find them: §7 LA1 row (done = §4 filled), FW4 5a (target volume C1), HO2 items 1–2 (roles, `sample_id`), LA3 (B1–B5, D1), LA4 d (G3), §9.1 sampling plan (K2, F1–F4) and success criteria (F6, K1), §11.1 #15 (labels E2, Sterivex model A3). The DNA-free water packing line of §9.1 (F2) is updated by LA2.

**Added:** `docs/LAB_INTERFACE.md`:
- §2: 48 questions with stable IDs (A1–L3) in 12 groups: filters and extraction kit, Shield volume and residual water, filtered volume, storage and transport, labels and `sample_id`, blanks, positive controls, metadata, chain of custody, markers, Pilot A plan and success criteria, cleaning and biosecurity. Each row: our starting proposal, an empty Answer column, and the step/setting it feeds; the answers that block a step are marked (B1, B3, A2, D1, E1, H1, K1, L1).
- §3: the metadata column table (ROADMAP §3.6 + the PROTOCOL.md §6 record + retrieval time, lots, operator), with empty columns for the lab's names and units.
- §4: the decisions table later steps read (LA2, LA3, FW4, HO2, HO3, VA2); §5: actions.
- §6: the one-page "Sterivex handling in CoWaS" summary for the lab, including what is not measured yet (residual water, retention, carry-over, cross-talk, volume accuracy) and which step measures it.

**Tests:** none (docs only). `pio run -e due` / `pio test -e native`: tree unchanged (not affected).

**Hardware checklist:** none. docs/TESTS_PENDING.md unchanged (no hardware test added).

**Open points (no F-number: documentation, not code/hardware):**
- ROADMAP §9.1 packs "DNA-free water ≥ 3 L for blanks + purge". If an equipment blank filters the same 2 L as a sample, 2 blanks + their purges need ~5 L. LAB_INTERFACE F2 asks the lab; fix §9.1 with the answer.
- Proposed `sample_id` scheme for blanks and manual samples (`EB1`, `FB`, `MANn` in place of the slot) and the roles `field_blank` / `manual_sample` go beyond ROADMAP §3.6's roles; HO2 decides after E1/H1.

---

## LA2: SOPs for filters, cleaning, biosecurity, chain of custody (2026-09-25)

**Branch:** `la2-sops` (from `18db617`, the LA1 commit, so LAB_INTERFACE.md is present; worktree `../Cowas-la2`, because the main folder holds HO1's uncommitted `host/`) · **Status:** drafted **before the lab meeting**: LAB_INTERFACE.md §4 is still empty, so every lab value in the SOP is our proposal, marked `⟨LAB X⟩` with the question ID. LA2 is done once those markers are replaced by the lab's answers and the dry run (TESTS_PENDING) has passed.

**Behavior changes:** none (documentation only, no code).

**Added:** `docs/SOP_FILTERS.md`, a printable checklist (a ☐, a record field and a "Who" field on every step): §0 deployment header, the pending lab values, general rules (gloves, E-stop before hands in the manifold, never pump against a capped outlet (F50), O-ring stays locked and the rotor is never turned by hand, no bleach with Shield or lysis buffers); §1 cleaning before loading; §2 loading (slots in `FILL_ORDER`, orientation, finger-tight + ¼ turn, inlet cap off → screw on → all outlet caps off last with a count and a second-person check, photo, `cowas slots load`); §3 unloading (blanks first, outlet cap while mounted → unscrew → inlet cap, labels, retrieval time, cool box, photos, count); §4 cleaning + biosecurity (drain at the site, bleach / water / DNA-free water flush of the purge path and every slot, dry ≥ 5 days before another water body); §5 chain-of-custody form (header, 24 filter rows, transfers, lab reception); §6 equipment blank (water budget ≈ 3 L per blank, hose tip handling for the start and end blanks, role `equipment_blank`, purge with blank water first); §7 sign-off.

**Changed:**
- `docs/LAB_INTERFACE.md`: new questions D6 (unused filters), D7 (caps at retrieval), F7 (hose tip before the end blank), and a §4 row for them.
- `docs/ROADMAP_STAGE2_PILOTS.md`: HO2 prompt item 4 names the host commands the SOP uses (`cowas slots`, `slots load`, `slots unload` with the retrieval time, `cowas clean`); HO3 prompt: equipment-blank events pause before and after for the operator; §9.1 checklist points to the SOP and packs ≈ 6 L of DNA-free water (resolves LA1's first open point, pending F2).
- `docs/TESTS_PENDING.md`: the SOP dry run, with numeric pass criteria.
- Follow-up (same day): the ROADMAP LA2 prompt gets a "Status / left to do" block and the §7 row says drafted, not done; HW0 #15 DNA-free water ≥ 5 L → ≥ 10 L (the SOP's blanks + rinses), new caps for loading and retrieval.

**Tests:** docs only. Run on `la2-sops` anyway (2026-09-25): `pio run -e due` SUCCESS, `pio test -e native` 15/15 pass.

**Hardware checklist:** the SOP dry run in docs/TESTS_PENDING.md (loading ≤ 30 min, unloading ≤ 20 min, 0 caps left, 0 thread drips, 0 label mismatches). Not run.

**Open points (no F-number: documentation):**
- To verify at the first loading: whether the rotor can turn when a filter is screwed on with the O-ring locked, and whether the Sterivex model's inlet/outlet match the SOP's orientation text (inlet = female luer-lock).
- The bleach flush (§4.2) has no one-shot command before HO2's `cowas clean`; until then only someone who knows the console runs it (F5/F40 cautions). Its volumes (50 mL per slot) are a proposal; LA4 measures the dead volume and may change them.
- Shield line cleaning is left to LA3 (`PRIME_SHIELD`); the SOP forbids bleach there.

---

## HW1: Electrical audit (2026-09-25)

**Branch:** `hw1-electrical-audit` (from `bed1b77`, the FW1 tip; worktree `../Cowas-hw1`) · **Status:** part 1 done (POWER.md + checklist); waiting for the measurements.

**Behavior changes:** none (documentation only, no code).

**Added:** `docs/POWER.md`: today's power flow (ASCII, with `___` for the unknowns) and a per-load table; the rules from ROADMAP §11.3; the proposed pilot topology (§3.8: 12 V LiFePO4 → regulated 24 V boost behind the E-stop, 5 V buck for the Pi, a separate fused Due supply that the E-stop does not cut, one ground star); the measurement checklist M1–M44 with commands and acceptance values; the HW5 energy table. `docs/TESTS_PENDING.md`: one row pointing at the checklist.

**Points to watch in the results:** the VNH5019 shield's ARDVIN=VOUT jumper (it would put the shield's 24 V on the Due's Vin, limit 16 V); the shield's logic VDD (5 V would put 5 V pull-ups on D5/D30); the pump driver's input range vs the DAC's 2.75 V maximum (F25); the A4988 Vref: don't change it without redoing `calibrate_actuator`, since the stall point sets `LINEAR_ACT_LOCKED_STEPS`.

**Tests:** no code. `pio run -e due` / `pio test -e native`: tree unchanged (not affected).

**Hardware checklist:** docs/POWER.md §4, to fill in. Part 2 (after the results): fill in PINOUT.md (reachable, measured), POWER.md, the fixes and their parts in ROADMAP §11, and any new findings.

---

## HO1: Host skeleton, protocol client, simulator, CLI (2026-09-28)

**Branch:** `ho1-host-skeleton` (from `fw1-protocol-spec`, because FW1 is not merged into `cowas-v2` yet) · **Status:** code and tests done; bench check waits for FW7.

**Behavior changes:** none on the Due (no firmware change). `computer_interface/` (legacy) untouched. CI: the `host-tests` job now installs `host[dev]` and runs `ruff check`, `ruff format --check` and `pytest host/` on Ubuntu **and Windows**.

**Added:** `host/` (Python 3.11+, package `cowas`, src layout; deps: pydantic 2, pyserial):
- `cowas.protocol`: the §4/§5 tables (`spec`), pydantic models for every message and result (strict types, unknown fields kept), the codec (NaN refused), the Due's request checks as pure functions (`validate`: §7 steps 1, 2, 4, 5), the vector matcher.
- `cowas.transport`: `SerialTransport` (finds the Native USB port by VID:PID 2341:003E; **refuses 1200 baud**; refuses the Programming port unless `--bench`; reconnects, re-detecting the port), routing (`{` lines → protocol; other lines and `{"t":"log"}` → the device log), `DeviceLog` (UTC timestamps, rotated files under `data/logs/`), `MemoryTransport`.
- `cowas.client.CowasClient`: single-threaded, timeouts on the transport's clock (so the same code runs in virtual time). Session-unique ids (F70); acks; §10 timeouts (2 s short, `ack.max_s` + 30 s long); same-id resends (3, then link recovery); heartbeat PING every `ping_period_s`; reconnect → HELLO → `boot_id` check → resend of pending ids; reboot → `DeviceRebooted`, ids never resent; subscriptions (a subscriber may send STOP).
- `cowas.sim.FakeController` + `SimTransport`: protocol v1 in virtual time (check order, mode table, dedupe 16 ids + last 4 records, ack/events/1 Hz telemetry/result, STOP/CLEAR_FAULT/RECOVER, link loss, boot_id, log copies, bench-mode text lines). Faults: `PressureTrip`, `Clog`, `SlowProcedure`, `LinkDrop` (silent per direction, or port vanishing), `Reboot` (lost boot event while the USB re-enumerates), `WatchdogReset` (hang, then reset), `FlowmeterDead`. Geometry from GET_INFO (V1 = 15 slots; tests also use 16 and 24).
- CLI `cowas`: ports, hello, status, info, stop, clear-fault, recover, rotate, lock, unlock, run-cycle, monitor, logs; `--sim` everywhere (sim files go to `data/sim/`). Ctrl-C during a long command → STOP + the partial result. RUN_CYCLE results are saved to `data/results/*.json` until HO2's store exists. `clear-fault` and `info` were not in the prompt: needed for the STOP → CLEAR_FAULT → RECOVER path and to see the slots.

**Tests:** `pytest host/`: **364 passed** (8 s). They include all 95 request vectors against the simulator; the 30 message vectors and 3 sequences through the models; the client consuming the happy-path and reboot sequences; the client against the simulator (heartbeat over a 30-min cycle, a lost ack, retried RUN_CYCLE runs once, a stored result on resend, LinkLost, CommandTimeout after `max_s` + 30 s, reconnect after the port vanishes, STOP during RUN_CYCLE from a subscriber, the log separation and garbage lines, reboot detected via HELLO and via the boot event, watchdog reset); arg boundaries; port selection and the 1200-baud guard; CLI runs; no PyQt/ZMQ/Pi imports outside `adapters/`. `ruff`: clean. `pio run -e due`: pass (firmware unchanged). `pio test -e native`: 15/15 pass.

**Hardware checklist:** none possible yet: the Due speaks the legacy protocol until FW7. The bench check (`cowas hello`, `cowas monitor` on the real Due) is in docs/TESTS_PENDING.md.

**New findings:** F70–F74 (table above).

**Unanswered when the HO1 session closed (2026-09-28)** — check these when merging folders/branches:
1. **Commit HO1?** Everything is uncommitted in the working tree of branch `ho1-host-skeleton` (branched from `fw1-protocol-spec`): `host/`, `.github/workflows/ci.yml`, `.gitignore`, `CLAUDE.md`, this log, `docs/TESTS_PENDING.md`, and the F70–F74 pointers in the ROADMAP FW6/FW7 prompts. The user's own LA2 note in `docs/ROADMAP_STAGE2_PILOTS.md` is in the same working tree (not HO1 work).
2. **Merge order:** FW1 (`fw1-protocol-spec`) and HO1 are not merged into `cowas-v2`; merge FW1 first (HO1 builds on it).
3. **F71** (RECOVER deadlock) needs a user decision in FW6; **F70, F72–F74** are PROTOCOL.md wording/choices for FW7. Pointers added to both ROADMAP prompts.
4. **ROADMAP §7:** HO1 not ticked; tick it after the merge and the bench check in TESTS_PENDING (needs FW7).
5. **CA bundle:** the PEM used for pip was built in a session scratch folder and is gone. Rebuild it (certifi + Windows root store) the next time pip/pio downloads fail with `CERTIFICATE_VERIFY_FAILED`; consider keeping one outside the repo, e.g. `%USERPROFILE%\.certs\`.

---

## LA3: Shield volume, residual water, shield retention: protocol and tools (2026-09-25)

**Branch:** `la3-shield-retention` (from `18db617`, the LA1 commit: it has FW1's PROTOCOL.md and LAB_INTERFACE.md; neither is merged into `cowas-v2` yet), worktree `../Cowas-la3` · **Finding IDs:** F80–F89 · **Status:** tools and protocol done; **no measurement yet**. The dependencies are not met (FW4 not done, LAB_INTERFACE §4 empty), so this step delivers everything that doesn't need them. Part A can run now.

**Inputs from the user (2026-09-25):** the Shield has **its own tube straight to the manifold** (not a tee into the water line); Shield density unknown (1.00 PLACEHOLDER + a pipette check, LA3_PROTOCOL A1); the lab thresholds are unknown ("do what you can and note what's left").

**Behavior changes:** one **new console command**, `shield_calibrate` (micro pump only: no main pump, no valve, no rotation). No existing command or procedure changed. Settings.h gains `SHIELD_*` constants that only this command reads (density and target are PLACEHOLDER).

**Added:**
- `src/Core/shield_calibration.{h,cpp}`: mass → mL, tube volume, least-squares line (rate, offset, R²) over the runs, per-duration mean/SD/CV, fill time for a target (bounded), pass/fail verdict.
- `src/Tests.cpp` `calibrate_shield_pump()` (console `shield_calibrate`): optional empty-line fill time (→ `PRIME_SHIELD`), prime (≤ 120 s), 3 repeats × 2–3 durations interleaved, masses typed in; prints `TEST|shield_calibrate|...` lines and `DNA_SHIELD_FILL_MS`. Every pump run is time-limited; any key stops the pump and aborts.
- `test/test_shield_calibration/` (14 native tests: exact line, density, noise → lower R², < 2 durations, bad inputs, stats, capacity, fill-time bounds and round trip, verdict order and boundaries, the real Settings.h values).
- `docs/LA3_PROTOCOL.md`: Part A (density, rate, line volume, drain-back), Part C (12 filters: A-today, B-long, [B-high], F-warm, S-room, H-handling; paired weighings; holds at 1, 3, 7 days; in-machine hold at room temperature with 2 rotations a day, off-machine surrogate at 30 °C), Part D (analysis), Part E (decision rules + recommendations), results tables, open items.
- `tools/analyze_la3.py` (stdlib only), `tools/la3_thresholds.json` (all `null` = OPEN), `tools/la3_masses_template.csv`, `tools/tests/` (22 pytest tests on a synthetic dataset).
- CI: a `pytest tools/` job. CLAUDE.md, COMMANDS.md, TESTS_PENDING.md, PROTOCOL.md §14 (pointers to the recommendations), LAB_INTERFACE.md §4 (copy B1–B5 into the thresholds file) updated.

**Recommendations (LA3_PROTOCOL §5; interim, until the data):**
- `DNA_SHIELD_FILL_MS`: none yet (rate unknown); run Part A, then check the dose reaching the filter in Part C (±10 %).
- Air push: keep today's unless the longer/higher push cuts the residual water by ≥ 0.1 mL and ≥ 20 %; if the ratio is still too low, raise the Shield dose instead (F28 physics).
- Caps/check valves: caps in Pilot A (the operator is there); order ~16 luer check valves as a hedge for Pilot B; the warm-hold retention decides.
- `PRIME_SHIELD`: to the **purge slot**; `PRIME_SHIELD_MS` = the empty-line fill time × 1.25 (from `shield_calibrate`); before each deployment and after a bottle change; if the line drains back between samples (A4), a check valve at the manifold end of the Shield tube.

**Code changes proposed, not made (ask first; one small follow-up PR):** (1) a console command that runs **the Shield stage alone** (`step_DNA_shield`: micro pump + air push), needed by the paired design (FW2's `shield_only` has no air push; FW2 could instead add `sample=n` to `full_cycle`); (2) an emptying-phase setpoint parameter (`EMPTY_STX_SETPOINT_BAR`, default 0.4) for the optional B-high variant (FW4 only adds the min/max times).

**Unanswered when the LA3 session closed (2026-09-28): ask the user before acting on them.**
1. The follow-up PR above (Shield-stage-alone command; emptying setpoint parameter): not yet approved.
2. The LA3 work was **left uncommitted** in the worktree `../Cowas-la3` (branch `la3-shield-retention`); the user will reorganise the folders and merge. When merging into `cowas-v2`, expect conflicts in CLAUDE.md, docs/COMMANDS.md, docs/FIRMWARE_LOG.md and docs/TESTS_PENDING.md (parallel steps edit them too): keep both sides.
3. Part A (`shield_calibrate`) has not been run on the machine (needs this branch flashed).
4. The lab thresholds (LAB_INTERFACE B1–B5, D1) are still unknown: `tools/la3_thresholds.json` stays all `null` until LA1's meeting.

**Tests:**
- `pio run -e due`: pass (flash 93,216 B; RAM 4,564 B, unchanged).
- `pio test -e native`: 29/29 pass (15 manifold + 14 shield).
- `pytest tools/`: 22/22 pass (Python 3.11 venv; the `python` on PATH is the Microsoft Store stub on this laptop, use PlatformIO's `penv\Scripts\python.exe` to create a venv).

**Hardware checklist** (not run: flashing is the user's call). Part A of LA3_PROTOCOL, in order: `shield_calibrate` with the defaults → `TEST|shield_calibrate|fit|PASS`: every duration CV ≤ 5 %, R² ≥ 0.995, |offset| < 0.1 mL; `line_fill` line_ml within ±20 % of π(ID/2)²L of the measured tubes; drain-back after ≥ 12 h ≥ 95 % of the calibrated dose; any key during a run stops the micro pump at once (check it once). Listed in docs/TESTS_PENDING.md.

**New findings:**

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F80 | The Shield's air push is `PUMP_SHIELD_POWER = 13 %` for `PUMP_SHIELD_TIME = 300 ms`. With the DAC offset (F25) and the pump's spin-up, it's unverified that this moves any air at all, so the Shield may stay in the manifold passage (and then go to the purge on the next rotation, not into the filter). | `Settings.h:32-33`, `step_DNA_shield()` | Medium (data quality) | LA3 Part C measures the dose reaching the filter; FW4 | open |
| F81 | There is no Shield prime anywhere: the first sample of a deployment (and the first after a bottle change) loses the volume of the Shield tube, which can be as large as the whole dose (1 m of 1.6 mm ID = 2 mL). | `step_DNA_shield()`, `sample_process()` | High (data quality) | FW7 (`PRIME_SHIELD`, LA3_PROTOCOL §5.4) | open |
| F82 | `micro_pump` (10 s) and `calibrate_dna_volume` (operator-typed duration, not bounded) run the micro pump with `delay()`: no key stops them. `shield_calibrate` doesn't have this problem. | `Tests.cpp:300`, `Tests.cpp` `calibrate_dna_pump_volume()` | Low | FW5 (`wait_ms()`) | open |

---

## LA4: Carry-over and slot cross-talk (IN PROGRESS, paused 2026-09-28)

**Branch:** `la4-carryover` in the worktree `../Cowas-la4`, from `18db617` (FW1 + LA1; `cowas-v2` doesn't have them yet) · **Finding IDs for this step:** F110–F119 · **Status:** analysis tool done; the protocol document is **not written yet**; nothing is committed (the files are untracked in the worktree: **commit or copy them before removing the worktree or merging folders**).

**Done:**
- `tools/analyze_la4.py` (stdlib only, Python ≥ 3.8): calibration from a dilution series (piecewise-linear, blanks → LOD 3σ / LOQ 10σ), `washout` (test a: line volume = area under the curve, exponential fit, early-vs-late tailing check, conservative purge for the target, model-free purge, `INTERNAL_DEAD_VOLUME_ML` and `PURGE_DEAD_VOLUME_FACTOR` recommendation = max needed factor × 1.2 rounded up to 0.5), `carryover` (test b: outlet cups + filter eluate, PASS/FAIL/INCONCLUSIVE with an LOD upper bound), `crosstalk` (test c: fraction of the source found per slot, neighbours from the `clocklist` minutes, no hard-coded slot count, `passed_over`, controls). Optional `dilution` columns for out-of-range cups.
- `tools/tests/test_analyze_la4.py`: 47 pytest tests on synthetic data, all passing (run with `python -m pytest tools/tests`; used HO1's `host/.venv` interpreter because this laptop has no system Python). Ruff: only `B905` (needs 3.10) and a few long lines left.
- `tools/la4_example/*.csv`: **synthetic** example files (the CSV formats for the team).

**Still to do (the ROADMAP LA4 prompt):**
1. `docs/LA4_PROTOCOL.md` (not started). Design decided so far:
   - Tracer: **20 g/L NaCl + blue food dye (E133)** in demineralised water. Conductivity (a pen, ATC) gives the numbers, and the dye makes leaks and stained membranes visible. A spectrophotometer (630 nm) is optional if the lab lends one. Standards are made by mass on the 0.01 g scale (1, 0.1, 0.01, 1e-3, 1e-4 + 3 blanks) at every session. Rinse the machine with ≥ 2 L of fresh water afterwards.
   - Test a (washout) **can run before FW4 and should**, so FW4 gets a measured `INTERNAL_DEAD_VOLUME_ML`. It pumps through the purge slot 0 with an open outlet (no filter), supervised (F50). Lock by hand (`linear_actuator`) before FW4, then `pump` at low power. Stop-and-go is allowed. Cups: 25 mL up to 500 mL, then 50, then 100 mL, until 3 cups are below the LOQ. Run 3 times, with the HW6 hose if it's ready (else a bench hose whose volume is known by weighing).
   - Tests b and c pump through Sterivex filters: **blocked by FW3 + HW4 (the cap) and FW4** (purge formula; FW2 `full_cycle`). Test b: dyed cycle on A, then a clear cycle on B with the FW4 purge, 500 mL (smaller = conservative). B's outlet goes in cups (4 × 25 mL, 100 mL, rest pooled), plus a 10 mL syringe eluate of B's filter; 3 A/B pairs. Optional b2: tracer in the micro-pump reservoir (shield path, F111).
   - Test c: load the source (≈ 100 g/L NaCl + dye, its own standards) into A by syringe, with the production shield volume. Hold ≥ 24 h locked at rest, then run short clear cycles (≈ 100 mL) on the other slots. **Sensitivity rule: (measured volume per slot × LOD) < target × source volume.** A 500 mL outlet with a 2 mL source can't prove 0.1 % (a pytest pins this). The first slot after A must be far from A, so that common-path carry-over is told apart from neighbour leakage. Record `passed_over`. Finish with photos of the membranes and a wipe of the manifold face.
   - Test d (qPCR spike) only with the lab (LAB_INTERFACE G3).
2. Recommendations (§3 of the prompt): **`PURGE_DEAD_VOLUME_FACTOR`: provisional 5, not 3 (F110)**, final value from test a. `INTERNAL_DEAD_VOLUME_ML`: the test a result (PLACEHOLDER until then). **Bleach flush:** not for Pilot A. For Pilot B, decide from the data: b/c pass, test d (if done), and a clean end-of-Pilot-A equipment blank → no flush (P2 stays P2); otherwise promote it (thiosulfate neutralisation + rinse needed, or it destroys the next sample's DNA). Consider a mid-deployment equipment blank at the day 3–4 visit.
3. `docs/TESTS_PENDING.md`: rows for LA4 a (blocked by buying the tracer and meter; the pump may refuse to start, F17), b/c (FW3 + HW4, FW4, HW6), d (the lab).
4. ROADMAP §11.1 #16: add a conductivity pen, NaCl, demineralised water ≈ 10 L, ≈ 40 numbered cups, 10 mL syringes.
5. CI: `tools/tests` is not run by CI yet (only `pytest host/`). Add a job, or include it in the host job.

**Open questions for the user (unanswered on 2026-09-28):**
- Which instrument is available (conductivity pen / spectrophotometer / phone)? The user didn't know what a tracer is; it was explained (a harmless substance, salt or dye, added to the test water, not firmware). The plan above assumes buying a cheap conductivity pen.
- Update the FW4 prompt's `PURGE_DEAD_VOLUME_FACTOR = 3` to 5 provisionally (F110)? Not edited, as it waits for the user.
- The field hose (HW6): placeholders 6 mm ID × 5 m (ROADMAP §3.7 example) until HW6.
- Commit LA4 locally? (Nothing is committed; see [[git-local-only]] in memory.)

**Findings (proposed, F110–F119 range; renumbered from F80/F81 at the merge, 2026-09-28: F80–F82 were already taken by LA3):**

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F110 | "Purge ≥ 3 × dead volume" (§3.7) only works for near plug flow. A well-mixed volume (pump chamber, sensor tee) needs ln(1/0.001) ≈ **6.9 volumes** for 0.1 % carry-over; 3 × leaves up to 5 %. The factor must come from the LA4 washout; provisional 5. A larger purge also outgrows `step_purge`'s fixed 3 min timeout (FW4 item 6 must size it). | `Settings.h` (FW4), ROADMAP §3.7 | High (data quality) | FW4, LA4 | open |
| F111 | Per the `step_DNA_shield()` code, the shield is pushed with air through the common line and the rotor channel into the slot. Shield and residue stay in the common path until the next purge, which flushes only through slot 0. This carry-over path is not measured yet (LA4 test b2). | `Step_functions.cpp:659-695` | Medium | LA4 | open |

---

## FW2: Mechanical test suite, lock tri-state, test logging (2026-09-28)

**Branch:** `fw2-mech-tests` (from the FW1 tip `bed1b77`, like HO1/HW1; worktree `../Cowas-fw2`) · **Status:** code done, builds and tests pass; **hardware checklist not run yet** (nothing flashed).

**Hardware at the start:** no pressure sensor installed (F50), flowmeter uncalibrated (factor 77, F6).

**Decisions (user, 2026-09-25):** a failed or refused rotation **stops the procedure** (`goto_slot_locked()` / `step_*` return `bool`; `sample_process()` refuses when the lock is UNKNOWN or there's no sensor, and stops at the first failed step); `TEST_OPENLOOP_MAX_POWER = 30 %` (PLACEHOLDER); `ACTUATOR_HOME_SLOT_TOLERANCE_DEG = 3°` (PLACEHOLDER); **no escape** when the lock is UNKNOWN and the manifold is not aligned (F60).

**Audit (step 0):** lock/rotation tests were partial (`linear_actuator` = manual jog; `test_manifold` had no abort and a hard-coded 15; `test_valves` was timed with no confirmation); `leak_hold`, `lock_endurance`, `encoder_repeat`, `actuator_home` and the `TEST|` logging were missing; `multi_sample`, `sample_cycle_full` and `micro_pump` existed. Everything new reuses `lock_oring`/`unlock_oring`, `goto_slot_locked`, `rotateMotor`, `load_slot*` and `step_*`.

**Behavior changes** (for the PR description):
1. O-ring lock tri-state LOCKED / UNLOCKED / UNKNOWN (was `bool`, false at boot). UNKNOWN at boot and after `linear_actuator` jogs / `calibrate_actuator`. `lock_oring`/`unlock_oring` refuse while UNKNOWN; `unlock_oring()` now returns `bool`. New `actuator_home` (aligned within 3° → overdrive lock → unlock → UNLOCKED).
2. `rotateMotor()` refuses unless UNLOCKED, and refuses an out-of-range slot index. `cal`, `zero`, `temp`, `encoder_manifold`, `manifold_direction` and `manifold` refuse too (F5).
3. `PRESSURE_SENSOR_AVAILABLE = false` (Settings.h): every procedure that needs pressure refuses (list in COMMANDS.md §4), including the legacy Pi sample/purge commands and the panel pump mode.
4. `goto_slot_locked()` and `step_purge`/`step_sampling`/`step_empty`/`step_DNA_shield` return `bool` and stop (pumps off, valves closed, O-ring as is) at the first refused/failed move. `sample_process`, `demo_sample_process`, `abort_sample`, `load_slot_full`, `purge_pipes_manifold`, `calibrate_DNA_pump` and `DNA_shield_test` stop too. Before, a failed rotation locked and pumped at whatever slot was in front.
5. `step_DNA_shield()` no longer calls a bare `rotateMotor()`: no motion if already locked at the slot (encoder within 3°); otherwise unlock → rotate → lock.
6. `step_sampling()` takes an optional `volume_ml` (default `STX_SAMPLE_MILLILITERS`, unchanged). `load_slot()` is split into `load_slot_water()` + `load_slot_air()` (same code, same order).
7. The menu's `multi_sample` is now the extended FW2 command (same defaults; sensorless variant when no sensor). The old `test_multi_sample_loading()` stays in Tests.cpp, unreachable (not deleted, F61).
8. Fixes: F38 (`clock` parse), F43 (`zero` timeout), F46 (`scale_angle` guard), F52 (`enc_rot` angle).
9. Size: flash 122,880 B, RAM 8,328 B (FW0: 86,112 / 4,564).

**Added:**
- `src/Core/`: `lock_rules` (tri-state, may_rotate/lock/unlock/home), `slot_iteration` (FILL_ORDER check, from/count/order list), `openloop` (stop decision, clamps), `test_args` (command options), `test_line` (the `TEST|` formatter); `angle_diff_deg` + `nearest_slot` in `manifold_geometry`.
- `src/Test_report.cpp` (TEST lines + summary table), `src/Test_openloop.cpp` (sensorless mode: NOSENSOR per test run, 30 % clamp, 120 s limit, 5 s no-flow stop, any key stops, the cap re-attached before it returns), `src/Tests_mech.cpp` (the commands in COMMANDS.md §4 + `help`).
- Settings.h: `PRESSURE_SENSOR_AVAILABLE`, `TEST_OPENLOOP_*`, `ACTUATOR_HOME_SLOT_TOLERANCE_DEG`, `TEST_ROTATE_MAX_ERROR_DEG = 1.0`, `TEST_ENCODER_MAX_SPREAD_DEG = 0.5`, ...
- `tools/hw_test_log.py` (pyserial; mirrors the console, appends TEST lines to `test_logs/<date>_<tag>.csv`, transcript in `.log`) + `tools/tests/` (pytest) + a CI job `pytest tools/`; `test_logs/README.md`.

**Grep proof (sensorless mode unreachable from production):** `set_pressure_safety(` is called only in `main.cpp:139` (setup) and `Test_openloop.cpp` (detach + re-attach). `pump_open_loop_test` / `OpenLoopSession` are used only in `Tests_mech.cpp`; `Test_openloop.h` is included only by `Tests_mech.cpp` and `Test_openloop.cpp`. `Tests_mech.cpp` is reached only through `mech_tests_dispatch()` (called only from `Tests.cpp:94`, inside `test_all_components()`), which is called only from `setup()` (`main.cpp:219`). `loop()` (`main_program`, `button_control`) and `Step_functions.cpp`/`maintenance.cpp` never reach it. The helper also refuses unless armed and while `PRESSURE_SENSOR_AVAILABLE` is true.

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 68/68 pass (lock_rules 9, manifold_geometry 20 incl. the unchanged goldens, openloop 12, slot_iteration 11 incl. every from/count/order/purge combination for 15 and 24 slots, test_args 16). On this laptop, Windows Application Control sometimes blocks a freshly built test exe (`WinError 4551`, F64): rerun that suite (`-f <suite>`).
- `pytest tools/`: 9/9 pass (run from a scratch venv: pip needed a PEM bundle exported from the Windows root store, see CLAUDE.md).

**Hardware checklist** (in order, safe first; log with `python tools/hw_test_log.py --port COMx --tag fw2`): to fill in.

| # | Command | PASS criterion | Result |
|---|---|---|---|
| 0 | reset, then `slot3` | `REFUSED \| rotateMotor: ... UNKNOWN`, nothing moves | |
| 1 | (manifold at a slot) `actuator_home` | `TEST\|actuator_home\|home\|PASS`, \|err_deg\| ≤ 3, audible stall then unlock, state UNLOCKED | **PASS** 2026-10-12 (user report: no numbers kept; `LINEAR_ACT_LOCKED_STEPS = 160`) |
| 2 | `lock_cycle 5` | 5 × PASS; lock_ms and unlock_ms each within ±10 % of their mean (≈ 7 s: 231 / 210 steps at 33 steps/s) | **PASS** 2026-10-12 (user report: 5 × PASS, the times were not kept) |
| 3 | `rotate_all dry auto` | every slot PASS: \|err_deg\| < 1.0 (purge + 14) | **PASS** 2026-10-12: 15/15, worst err -0.35 deg (slot 1), range -0.35 to +0.18. First run, offsets 5.2 / -1.05: 15/15 FAIL, -2.11 to -4.13 (F128). Log `test_logs/2026-10-12_s1-0.log` |
| 4 | `encoder_repeat 5 10 auto` | 10 × \|err\| < 1.0 and `spread` PASS: spread < 0.5° | **PASS** 2026-10-12: 10 × err 0.09 to 0.26, spread 0.18 deg. First run, offsets 5.2 / -1.05: err -3.95, spread 0.09, FAIL on the 1.0 limit (F128) |
| 5 | `valve_check` | V1 and Vman PASS (click heard on open and close) | **PASS** 2026-10-12: V1 and Vman, click on open and close |
| 6 | (outlet open, filter in slot 5, water at the intake) `slot5`, `leak_hold 30` + NOSENSOR | water_reason=volume, 30 ± 10 mL on the flowmeter (uncalibrated), leak=no, air push 10 s, unlock ok → PASS | |
| 7 | `multi_sample auto count=3` + NOSENSOR | 3 × PASS (slots 1, 2, 3), no leak seen, O-ring UNLOCKED at the end | |
| 8 | `shield_only 1 2000` | PASS, ms_run = 2000 ± 50, shield liquid seen at the filter inlet | |
| 9 | `rotate_all` (with lock/hold/unlock) | every slot PASS, err_after_unlock_deg within ±1.0 | |

**New findings (FW2 range F60–F69):**

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F60 | **No escape from "UNKNOWN + not aligned"** (user decision, 2026-09-25): rotation and `actuator_home` both refuse, and so does `cal`. After a rotation abort mid-way (encoder loss, timeout), or when `purge_angle` is off by more than 3° (e.g. before recalibration), the machine can only be recovered by hand: power off, turn the rotor to a slot by hand with the O-ring released, reboot, `actuator_home`. Commissioning `cal` therefore needs a roughly right `purge_angle`. Revisit when the "unlocked" home switch arrives (HW7, P1). | `Oring_lock.cpp`, `Manifold.cpp` | Medium (bench) | HW7 / FW6 (RECOVER) | open |
| F61 | `test_multi_sample_loading()` (pre-FW2, hard-coded 14) is no longer reachable from the menu: `multi_sample` now runs the extended FW2 command with the same defaults. Kept, not deleted (rule 2); delete with approval. | `Tests.cpp` | Low | P2 | open |
| F62 | Sensorless mode assumes the flowmeter (D13) sits on the water path of the water phase; if it doesn't see water within 5 s the run stops with `no_flow` (safe). The air push is time-only (10 s, PLACEHOLDER); its effect on a wet membrane is F28. | `Test_openloop.cpp` | Low | HW3 | open |
| F63 | `purge_pipes_manifold()` rotates without unlocking and pumps without locking (pre-existing). Since FW2 it refuses unless UNLOCKED and without the sensor, but with a sensor it would pump through an unlocked O-ring. | `maintenance.cpp:54` | Low | FW4 (auto-unlock/lock) | open |
| F64 | Dev laptop: Windows Application Control intermittently blocks the freshly built native test executables (`[WinError 4551]`), so `pio test -e native` shows random ERRORED suites. Not a code failure; rerun the suite. CI (Linux) is unaffected. | environment | Low | — | open |

---

## Merge of the parallel steps into `cowas-v2` (2026-09-28)

**Done in the worktree `../Cowas-merge`, local only (nothing pushed).** Merge order: FW1 → LA1 → LA2 → HW1 → HO1 → LA3 → LA4 → FW2 (`--no-ff`, one merge commit each).

**Committed first** (they were only in their working trees when their chats closed): HO1 (`72284cc`), the user's run-status notes in ROADMAP §8 that sat in HO1's folder (separate commit `b351b27`), LA3 (`b18c47b`), LA4 (`8797014`, partial: see its entry), FW2 (`64f34fc`).

**Conflicts resolved (both sides kept):** ROADMAP LA2 heading (user note + LA2 status); `ci.yml` (HO1's ruff + host matrix job, and one `tools-tests` job, which LA3 and FW2 had both added); CLAUDE.md (host and tools test lines, one CI line); `src/Tests.cpp` includes (LA3's `shield_calibration.h` + FW2's `Tests_mech.h`). LA3's `shield_calibrate` drives the micro pump only, so FW2's lock and pressure gates don't affect it; FW2's `help` now lists it.

**Log clean-up:** the union merge kept two F52 rows (the old "open" one is removed); LA4's proposed F80/F81 clashed with LA3's F80–F82 and are renumbered **F110/F111** (LA4's assigned range F110–F119); the `---` separators before HW1, HO1, LA3, LA4 and FW2 are restored.

**Checks on the merged tree:** `pio run -e due` pass (flash 129,904 B, RAM 8,328 B); `pio test -e native` 82/82 (6 suites; F64: rerun a suite if Windows blocks its exe); `pytest host/` 364 pass + `ruff check` / `ruff format --check` clean (HO1's venv, `PYTHONPATH=host/src`); `pytest tools/` 78 pass (FW2 9, LA3 + LA4 69).

**Still open after the merge (not lost, just pointed to):** the HO1, LA3 and LA4 entries above ("Unanswered when ... closed" / LA4 to-do list); LA4 is still in progress (LA4_PROTOCOL.md, TESTS_PENDING rows, order list); the FW2 hardware checklist; ROADMAP §7 ticks (HO1 after its bench check, which needs FW7); the user note `mock community?HW1 )` in the LA1 prompt (ROADMAP §8.3) looks like a stray edit, kept as written.


---

## Planning: three stages, stage 1 = functional prototype (2026-10-06)

**Branch:** `cowas-v2` working tree, nothing committed · **Status:** documents only, no code change.

**Why (user, 2026-10-06):** the roadmap was making the code more complete but slower to reach a working machine. Priority now: a functional prototype for early water tests in the week of 2026-10-12, even if not every safety layer exists yet.

**Hardware facts reported by the user:** the pressure sensor works. It is an **ABPDANV150PGSA3** (150 psi, SPI, CS D8), in the place of the old "Trustability" sensor. The other (analog, A2) sensor no longer exists. Not calibrated against a manometer yet. The FW2 hardware checklist was not completed (the sensor was off at the time).

**Changed:**
- The three roadmap files are now stages: `docs/ROADMAP_STAGE1_PROTOTYPE.md` (new), `docs/ROADMAP_STAGE2_PILOTS.md` (was `ROADMAP.md`), `docs/ROADMAP_STAGE3_FULL_SYSTEM.md` (was `ROADMAP_LATER.md`). Links updated in CLAUDE.md, this log and LAB_INTERFACE.md; `docs/archive/ROADMAP_v1.md` is left as it was (its links to `ROADMAP.md` are historical). "ROADMAP §N" still means the stage 2 file.
- Stage 1 steps: S1-0 bench facts, S1-1 safety and PID on the ABP (slice of FW3), S1-2 a real cycle (slice of FW4), S1-3 timed series (optional), S1-4 pressure calibration against a manometer. Each has a prompt and a Notes block; the notes rule is stage 1 §3.
- Stage 2 §7: the FW3 and FW4 rows say what stage 1 covers.

**Decisions:** the manometer calibration closes stage 1 (S1-4) and does not block the first water tests, because S1-1 fixes the conversion for the 150 psi part and tares the zero; it can move earlier if the manometer is available. What stage 1 knowingly leaves out, with the rule for each, is in stage 1 §4 (never unattended; filters are engineering samples).

**Revision 2026-10-09 (documents only):** the user added a manual manometer (0–2.5 bar) and is ready to calibrate. Stage 1 revised: S1-4 is merged into S1-1 (part A = cap, dead-sensor stop and PID on the ABP with the nominal conversion; part B = `calibrate_pressure` against the manometer, points ≤ 2.2 bar); `STX_MAX_PRESSURE = 2.5` moves from S1-2 to S1-1 because 3.0 is above the gauge's range; S1-0 gains an item for the manometer's model, class and position. CLAUDE.md and stage 2 (intro, §7 FW3 row) follow.

**Uncommitted code in the same working tree (not part of this entry's changes):** the `abp_raw` diagnostics command (`Tests.cpp`, `Tests.h`, `Tests_mech.cpp`, `Trustability_ABP_Gage.{h,cpp}`); S1-1 commits it.

**Tests:** none run (documents only).

---

## S1-1: Pressure on the ABP, calibration command (2026-10-09)

**Branch:** `s1-1-pressure-abp` (from `cowas-v2` at `58a1fa4`, carrying the uncommitted three-stage documents and the `abp_raw` work) · **Status:** code done, builds and tests pass; **nothing committed, nothing flashed; hardware checklist not run.**

**Goal:** stage 1 step S1-1 (a slice of FW3 items 1–3 and 5, plus the manometer calibration that was S1-4): the pump cap and the PID loops read the installed ABP instead of a floating pin, a dead sensor stops the pump, and the sensor can be calibrated against the bench manometer.

**Decisions (user, 2026-10-09):** the calibration uses a **syringe of air** on a tee with the sensor and the manometer, off the pump line, and not the pump against a restriction (no valve on the machine; a pinched hose fluctuates). `calibrate_pressure` is also a stage of `calibrate_all`. The manometer is a 0–2.5 bar dial gauge that looks like a Riegler 107-KD (F121).

**Behavior changes** (for the PR description):
1. **The pump cap and every PID loop read the ABP on SPI D8** (`read_p_in()` in main.cpp) instead of the analog A2 pin (`read_pressure2()`). 7 call sites: `main.cpp` `set_pressure_safety`; `Step_functions.cpp` `step_purge` (CtrlPumpFlow), `step_sampling` (CtrlPumpFlow + CtrlPumpNoWater), `load_slot_water` (CtrlPumpFlow), `load_slot_air` (CtrlPumpNoWater), `step_empty` (CtrlPumpNoWater). The A2 code and `read_pressure2()` are kept, unused by safety (FW3 deletes them).
2. **ABP conversion** (F27): counts → bar from the datasheet transfer function for `ABP_FULL_SCALE_PSI = 150` (10–90 % of 2^14 counts), then `pressure = PRESSURE_GAIN × (datasheet − PRESSURE_ZERO_OFFSET_BAR)`. Gain 1.0 and offset 0.0 are PLACEHOLDERs until the calibration. Readings are 3.4 % higher than before at the same counts.
3. **Dead or frozen sensor** : stale frames (status 10) keep the last value for at most `ABP_STALE_TIMEOUT_MS = 500`; a fault frame (status 01 / 11, `FF FF FF`, `00 00 00`) distrusts the reading at once, and it takes a new valid frame to trust it again. While not trusted, `getPressure()` returns `PRESSURE_SENSOR_INVALID_BAR = −99`, so the existing plausibility checks stop the pump and refuse to start it. Before, a stale frame printed "invalid SPI response" and any bad frame left the last value in place for ever. The message is now `ERROR | P1: pressure reading not trusted (<why>, last frame <status>)`, at most every 2 s.
4. **`PRESSURE_SENSOR_AVAILABLE`: written as `true`, set back to `false` the same day** (F124: the ABP gave garbage frames on the bench; see "Added after the first bench session" below). It becomes true at checklist row 4b. Once true: `sample_process`, `step_*`, `load_slot*`, `pump`, the panel Pump mode, `calibrate_flow`, `cal_DNA`, `purge_pipes`, `abort`, `full_cycle`, `full_all` and the legacy Pi sample/purge commands no longer refuse. `leak_hold` and `multi_sample` run in sensor mode; the sensorless mode (NOSENSOR) refuses, as designed. See F122 for the commands that still don't poll the cap.
5. **`STX_MAX_PRESSURE` 3.0 → 2.5 bar** (was planned for S1-2; 3.0 is above the manometer's range). The PID setpoint is still the hard-coded 2 bar: an overshoot of 0.5 bar now trips the cap.
6. New console commands `pressure_zero` and `calibrate_pressure` (below). `calibrate_all` asks for the pressure calibration first. `pressure` prints the ABP reading with 3 decimals, the datasheet value, the gain and the offset, or `NOT TRUSTED`. `abp_raw` prints the datasheet conversion for the configured full scale. The `pump` test prints the ABP reading (it printed A2).
7. Size: flash 136,176 B, RAM 8,344 B (before: 130,848 / 8,328).

**Added:**
- `src/Core/abp_convert` (frame classification, counts → bar, gain / zero, the trust rule) and `src/Core/pressure_calibration` (least-squares gain + offset, R², largest residual, verdict FIT / POINTS / GAIN / RESIDUAL).
- `pressure_zero`: pump off, open to air, 5 s average of the datasheet reading, prints the `PRESSURE_ZERO_OFFSET_BAR` line. Refuses a zero beyond ±0.3 bar or an unsteady reading. Read-only.
- `calibrate_pressure`: point 0 open to air, then 0.5 / 1.0 / 1.5 / 2.0 bar with the syringe. Live ABP reading once a second while the operator sets the needle; ENTER starts a 3 s average; the manometer value is typed after releasing (ENTER alone = the target, comma accepted). `s` skips a point, `x` aborts. Prints `TEST|calibrate_pressure|p<i>|...` (manometer, ABP datasheet, before, after, residual, spread), `TEST|calibrate_pressure|fit|PASS/FAIL` and the two Settings.h lines. PASS: at least 3 points, gain within 0.9–1.1, every residual ≤ 0.06 bar (PLACEHOLDER, F121). The pump is never started.
- Settings.h: `ABP_FULL_SCALE_PSI`, `ABP_STALE_TIMEOUT_MS`, `PRESSURE_SENSOR_INVALID_BAR`, `PRESSURE_GAIN`, `PRESSURE_ZERO_OFFSET_BAR`, `PRESSURE_ZERO_*`, `CAL_PRESSURE_*`.
- Driver: `getNominalPressure()`, `isTrusted()`; the `7.63e-4` / `1.25` constants are gone.

**Formula chosen:** `pressure = gain × (datasheet − zero_offset)`, with `zero_offset` = what the datasheet conversion reads at 0 bar. A later `pressure_zero` then changes only the offset and leaves the gain alone. The fit gives `reference = gain × datasheet + intercept`, so `zero_offset = −intercept / gain`.

**A2 code deleted (2026-10-12, user request: "the sensor is just absent"):** `BigPressure` (`pressure2`, `pressure_2_pin`), `read_pressure2()`, the `pressure_sensors` command (`test_pressure_sensor()`), `calibrate_pressure2` (`calibrate_pressure2_zero()`, F44), the `test_2_remplissage_container_*` functions (they used the A2 timer `timer_control_pressure2`, never called), and the commented A2 lines in `Flow_sensor.cpp`. `SmallPressure` (the unused A1 analog class in `Pressure_sensor.h/.cpp`) and `pressure_3_pin` were kept: not A2, and removing them needs a separate OK. `pressure1` is unchanged. `pio run -e due` and `pio test -e native` pass after the removal; the `pressure` command no longer prints a line for A2.

**Not done (stays in FW3):** renaming `pressure1` → `p_in`, SPI diagnostics, the second sensor.

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 109/109 pass (8 suites). New: `test_abp_convert` 18 (each status pair, the two dead-line patterns, 1638 → 0 and 14745 → full scale and the midpoint for 60 and 150 psi, the old constants reading 3.3 % low, the trust rule: stale grace and timeout, a slow poll, never valid, immediate faults, `millis()` wrap, Settings.h consistency) and `test_pressure_calibration` 9 (exact line, round trip through the driver formula, noise, one misread point, < 2 points, bad inputs, gain bounds on both sides, Settings.h consistency).
- `pytest host/`, `pytest tools/`: not run (nothing in `host/` or `tools/` changed).

**Hardware checklist** (in order; log with `python tools/hw_test_log.py --port COM6 --tag s1-1`): to fill in.

| # | Command / action | PASS criterion | Result |
|---|---|---|---|
| 1 | `abp_raw`, then `pressure`, sensor open to air | status `00 valid`; counts near 1638; `pressure` within ±0.16 bar of 0 before the zero is pasted | |
| 2 | Syringe + manometer + sensor on a tee: `calibrate_pressure` | `TEST\|calibrate_pressure\|fit\|PASS`: ≥ 3 points, gain 0.9–1.1, every residual ≤ 0.06 bar | |
| 3 | Paste `PRESSURE_GAIN` and `PRESSURE_ZERO_OFFSET_BAR`, rebuild, reflash; syringe again, `pressure` at about 1.0 and 2.0 bar | within 0.06 bar of the manometer at both | |
| 4 | Sensor back on the pump line, at rest: `pressure` | 0 ± 0.05 bar | |
| 4b | Rows 1–4 passed: set `PRESSURE_SENSOR_AVAILABLE = true` in Settings.h, rebuild, reflash | `oring_state` prints `PRESSURE_SENSOR_AVAILABLE = true` | |
| 5 | `pump` at 20 %, open outlet (water or air), then unplug the sensor's connector | `ERROR \| P1: pressure reading not trusted` and `SAFETY \| implausible pressure reading`; pump off within 1 s; `pump` refuses to start until the sensor is plugged back | |
| 6 | `STX_MAX_PRESSURE` temporarily 1.0, reflash; `pump` at 20 %, pinch the outlet shut | `SAFETY \| pressure exceeded cap`; the pump stops when the manometer shows 1.0 ± 0.1 bar. Then put 2.5 back and reflash | |
| 7 | `actuator_home`, `slot5`, `leak_hold 100` (sensor mode, water at the intake, a filter in slot 5) | water phase ends on volume, no SAFETY line, no leak, PASS | |

**Added after the first bench session (same day, see the next entry):** `PRESSURE_SENSOR_AVAILABLE` is back to `false` until the sensor is steady and calibrated, so no pump command can run on a garbage reading (the status quo of FW2; the cap itself is attached to the ABP either way). `ABP_SPI_HZ = 100000` (was a hard-coded 800 kHz, the datasheet maximum; F124); `abp_raw` prints the temperature of each frame and a min / max / spread line with a NOT STEADY warning (reworded later the same day, below); `calibrate_actuator` says which jog direction releases and that `c` is pressed with the O-ring released, and reminds that the lock state stays UNKNOWN. Builds and tests rerun: pass.

**Changed on the user's request (2026-10-09, after the 1 and 2 bar readings):** `calibrate_pressure` no longer judges a point "not steady". After each average it prints how much the reading moved and asks "Good readout?": type the manometer value to keep the point (ENTER alone = the target) or `r` to redo it. `abp_raw` prints the spread in counts and bar without a verdict. `pressure_zero` keeps its steadiness check (nobody holds anything there).

**Added on the user's request (2026-10-09, after a first `calibrate_pressure` try):**
- **First fix attempt in `calibrate_pressure` (superseded by the final flow below):** the first pressure point was measured before the operator could apply it (the transcript shows `averaging` right after the prompt, and a reading of −0.003 bar). A key typed as a character + ENTER left its line feed in the serial buffer, and the next prompt took it for the ENTER that starts the average. The console input is now drained before every prompt of the command (`drain_serial_input()`). The prompts are rewritten as numbered steps, and say that nothing is measured before ENTER.
- A pump-driven `water_column` check of the sensor's scale was written the same day and **removed on the user's decision** ("I think things will go wrong ... let's trust the manometer and move on"): no trace of it is left in the code.
- **Second bench report (user, 2026-10-09):** a command had to be typed twice before it ran, and `calibrate_pressure` again measured the 0.5 bar point before the pressure was applied. Cause not identified from the transcript (stray or late bytes on the console are suspected; input typed during the boot is also lost, F49). Changes:
  - A version that measured by itself on a steady reading was written and **dropped: not what the user asked**.
  - **Final flow (user's requirement, quoted in stage 1 §0):** for each of 0 / 0.5 / 1.0 / 1.5 / 2.0 bar, ask, wait for ENTER, only then measure 3 s. `wait_for_enter()` discards everything received before the prompt (until 100 ms of silence), waits for one whole new line (CR, LF or both) and discards what follows, so no byte is left for the next question. The manometer value of a point is its target; the value prompt, the redo and the skip are gone. `x` + ENTER aborts. Not tried on the machine yet.
  - **Test menu (behavior change, F37):** each command line is trimmed (CR, spaces) and leading non-printable bytes are removed before matching; a lone ENTER is ignored; the echo is now `> command`. The menu clears the input received during the boot and prints `=== Test menu READY ... ===` when it starts listening.
- Tests after these changes: `pio run -e due` pass (flash 139,088 B); `pio test -e native` 109/109.
- **Handover for the next session: stage 1 §0.**

**New findings:** F120 (one bad frame stops the pump), F121 (manometer not identified), F122 (commands without cap polling reachable again), F123 (unthrottled over-range print). **Status of older ones:** F27 fixed for the installed part in code (2026-10-09; confirmed by checklist rows 2–3); F50 and F7/F22 fixed in code, closed when rows 5–6 pass; F3: the cap is 2.5 bar, the hard-coded setpoint remains (S1-2).

---

## S1-0: first bench session (2026-10-09, incomplete)

**Firmware on the Due:** `cowas-v2` working tree with the `abp_raw` work (before S1-1), flashed by the user. **Console:** `pio device monitor` (the logger command was pasted into the monitor instead of a PowerShell prompt, so `tools/hw_test_log.py` did not run and `test_logs/` has no file for this session). The results below come from the transcript the user pasted.

| S1-0 item | Result |
|---|---|
| 2. `abp_raw` | **FAIL (F124).** Run 1: 20 × `FF FF FF`. Run 2: status `00`, counts 13377–16321. Run 3: status `00`, counts 64–3520. No steady reading at rest was captured; the values do not follow the pressure applied with the syringe. Resting counts: not obtained |
| 3, row 0. reset, `slot3` | **PASS** (twice): `REFUSED \| rotateMotor: O-ring lock state is UNKNOWN - run actuator_home first, manifold not moved` |
| 3, row 1. `actuator_home` | First try: the USB link dropped and the Due reset (F125). Second try: `REFUSED ... not aligned to a slot (nearest slot 4, error -9.84 deg, tolerance 3.0 deg)`, `TEST\|actuator_home\|home\|FAIL`. Correct refusal: the manifold was not on a slot |
| 3, rows 2–5 | not run |
| 4. `calibrate_actuator` | **Not completed.** Two starts: one aborted at once (`x`), one jogged `b` (towards lock) and was then aborted (`x`). No UNLOCKED mark, no stall point, no value printed. `LINEAR_ACT_LOCKED_STEPS` stays 210 (PLACEHOLDER). The actuator is now at an unknown position, moved towards lock, with the manifold 9.84° off slot 4 |
| 5, 6, 7 | not done |

**Notes for the operator (confusions seen in the session):** the lock state stays UNKNOWN after `calibrate_actuator`, even a completed one: only `actuator_home` makes it known, because the firmware still holds the old step count until the printed value is pasted and reflashed. `c` marks the RELEASED position, the stall is found in the stage after it. A mistyped command (`calibrate actuator`, `calibration_actuator`) prints nothing at all (F37 family: exact string match, no "unknown command" message).

**Second session, same day, S1-1 firmware flashed by the user (SPI clock 100 kHz):** `abp_raw` with the sensor open to air gave 20 valid frames, `06 62 5A` / `06 64 5A` (one `06 5F 5A`): counts 1631–1636, spread 5, −0.003 bar with the datasheet conversion, 20 °C on every line. Healthy at rest (S1-1 checklist row 1, first half). Not yet known whether the lower SPI clock or a reseated connector cured F124, nor whether it holds under pressure: the 1 and 2 bar readings come next.

**Third reading, same session, syringe + manometer on the tee** (only the firmware was reflashed since the garbage frames, no hardware change: F124 was the SPI clock). About 1 bar on the manometer: counts 4477–4608, mean about 4545. About 2 bar: counts 7632–7884, mean about 7700 (the user was pushing against a leak). Frames clean, 20 °C throughout. **With the 150 psi constant these read 2.30 and 4.78 bar: 2.3–2.4 times the manometer. With a 60 psi full scale they read 0.92 and 1.91 bar (F126).** `ABP_FULL_SCALE_PSI` is set to 60 (PLACEHOLDER until the marking is read).

**Consequences (updated after the second session):** S1-0 item 2 is done (steady at rest, gain check at about 1 and 2 bar; log `test_logs/2026-10-09_s1-0.log`), so F124 no longer blocks S1-1's bench checklist. S1-0 items 3 (rows 1–5), 4, 5, 6 and 7 are still to do.

---

## S1-0: bench session of 2026-10-12 (values reported by the user, no log)

**Evidence:** the user's report in the chat. The logger did not work, so `test_logs/` has no file for this session.

| S1-0 item | Result |
|---|---|
| 4. `calibrate_actuator` | Completed: `LINEAR_ACT_LOCKED_STEPS = 160` (was 210, PLACEHOLDER). Pasted into Settings.h. **To confirm:** whether `n` was answered at the stall prompt; if so the value is too small (F127) and the calibration must be redone with the fixed firmware |
| 5. D13 voltage | **Not measured.** The user believes the flowmeter in place is a 3.3 V one. F26 stays open |
| 6. Intake hose | Reported as "~0.4 mm diameter", length not given. 0.4 mm is not plausible for the intake (4 mm?): **to confirm**, with the length, before the purge volume of S1-2 |
| 7. Manometer | Class 1.6, 0–2.5 bar, divisions of 0.1 bar (F121 answered) |
| 3, rows 1–5 | Not run yet: the firmware with 160 steps must be flashed first |

**Changes:** `LINEAR_ACT_LOCKED_STEPS` 210 → 160 (Settings.h). F127 fixed in `jog_until_keypress()` (Tests.cpp). Comment of `CAL_PRESSURE_MAX_RESIDUAL_BAR` updated, value unchanged (0.06).

**Behavior changes:** (1) `lock_oring` / `unlock_oring` / `actuator_home` move 176 / 160 steps instead of 231 / 210. (2) In `calibrate_actuator`, answering `n` at the stall prompt keeps the step count, and the safety bound applies to the total travel.

**Tests:** `pio run -e due` and `pio test -e native` (109 cases) pass.

**Second part of the session (same day):**

- FW2 rows 1–5 all PASS (table above; `test_logs/2026-10-12_s1-0.log` holds the transcripts the user pasted: the logger did not work).
- F128: the manifold stop offsets differ by direction and were off; set to 1.2 / +1.2 (`Manifold.cpp:17`, `Manifold.h`). F129: the wrap point of `scale_angle()` moved to the no-hole position (between slots 3 and 4). Both are behavior changes of `rotateMotor()`.
- The rows 3–4 results were obtained **before** the F129 change (so with the old wrap point): reflash and re-run `rotate_all dry auto` once.
- S1-0 still open: item 5 (D13 voltage, not measured), item 6 (hose: "~0.4 mm" reported, length missing), the physical alignment of F128, and the `n` question on the 160 (the lock tests passed with it).
- The lock state is UNKNOWN after every reset by design (F18, rule 4); `actuator_home` with the manifold on a slot is the one command that clears it. Opening the serial port resets the Due.

**Third part of the session (same day), S1-0 items 5 and 6:**

- D13 (F26): deferred by the user (2026-10-12) to later, if needed. Last reading: 3.3 V steady at rest and under a gentle blow; the peak under pulses was not checked.
- Intake hose and spool (F130): the 4 mm × 2.40 m path is the whole water path before the exit (pump, sensors, tubing included), about 30 mL. Spool path 9 mm ID × 40 m (about 2.5 L), connected by default. An earlier entry said the spool was 40 mm, 2.5 mL: wrong.
- Hand alignments (F131): -3.69, -4.31, -3.52, -3.69, -2.99 (`oring_state`), and -2.81 earlier.
- Still open: the purge decision with the spool (F130), and whether the spool is in the calibration procedure (to ask the user). The F131 alignment check is deferred to pilot.
- F129 bench: the wrap point was flashed by the user and reported as working (slot0 to slot1 is a direct move). No numbers kept.

**Behavior change (S1-0 follow-up, 2026-10-12, F130):** `calibrate_all` (commissioning) now asks "Is the spool module connected to the water path?" (Enter = yes) and prints the dead volume (30 mL path, + 2.54 L if the spool is connected) and the purge the rule needs (3 ×). It warns if `PURGE_MILLILITERS` (100 mL) is below it. Prints only: no purge value changes. New constants in Settings.h: `INTAKE_PATH_ML`, `SPOOL_PATH_ML`, `PURGE_DEAD_VOLUME_FACTOR`. Tests: `pio run -e due` and `pio test -e native` (109) pass. Bench check pending: run `calibrate_all`, answer the spool question, read the printed numbers.

---

## S1-1: bench results (2026-10-12, reported by the user on 2026-10-15) and S1-1b: `pressure_ramp`

**Evidence:** the user's report in the chat. No log file (the logger did not work on 2026-10-12), so the numbers below are only those the user gave or those pasted into Settings.h. Numbers not reported are marked "not reported": ask before filling them.

**S1-1 checklist, as reported:**

| # | Check | Result |
|---|---|---|
| 1 | `abp_raw`, `pressure` at rest | PASS (2026-10-09: counts 1631–1636, spread 5, 20 °C; reported "done" on 2026-10-12) |
| 2 | `calibrate_pressure` with the syringe | **PASS.** 5 points, R² 0.99897, largest residual 0.032 bar (comment in Settings.h). **`PRESSURE_GAIN = 1.0413`, `PRESSURE_ZERO_OFFSET_BAR = -0.0311`.** Table of points (manometer, ABP before, after): not reported |
| 3 | Two syringe pressures after the paste | reported done, numbers not reported |
| 4 | Sensor on the pump line, `pressure` at rest | reported done, number not reported |
| 4b | `PRESSURE_SENSOR_AVAILABLE = true` | set (Settings.h) |
| 5 | Dead-sensor stop | **PASS** (user). Time to stop: not reported |
| 6 | Cap trip at a temporary 1.0 bar | **done** (user). Manometer reading at the trip: not reported |
| 7 | `actuator_home`, `slot5`, `leak_hold 100` in sensor mode | **FAIL with clogged filters.** The pressure spiked and the cap was reached almost at once, and water leaked. The pressure reached is unknown (possibly well above 3 bar): the SAFETY line prints one reading, and the periodic print is throttled (F135). The leak position was not reported |

**Changes made on the bench side (Settings.h, user decisions):** `STX_MAX_PRESSURE = 3.0` (2026-10-12, for a 3 bar seal test; the plan said 2.5, F136), `PRESSURE_GAIN` and `PRESSURE_ZERO_OFFSET_BAR` pasted, `PRESSURE_SENSOR_AVAILABLE = true`, `ABP_FULL_SCALE_PSI = 60` (PLACEHOLDER, F126: the fit absorbed the 2.4 factor).

**Status of older findings:** F27 fixed (conversion + calibration, row 2). F21: fixed for the installed sensor. F50: the cap reads the ABP, dead-sensor stop PASS and cap trip done: closed except for what row 7 showed (F132–F135: the cap works, but the control loop in front of it does not keep a clogged filter below it). F126: not resolved, but the calibration makes the reading agree with the manometer up to 2.0 bar.

**New findings:** F132 (the PID loops are blind for up to 4 s after a big step), F133 (every `CtrlPump::run()` starts at 50 % open-loop), F134 (`pump_pid` untuned and never reset), F135 (the cap is polled every 100 ms and the peak is not logged), F136 (cap above the calibrated range). F132 and F133 are the most likely reason for the instant cap trips on clogged filters; this is a hypothesis until a pressure curve is logged.

### S1-1b (code, 2026-10-15): the `pressure_ramp` test

**What it is.** A console test that regulates the pump **by pressure**, not by a fixed power: a PID follows a setpoint that rises from 0 to a target at a set rate, holds it, then the pump stops and the pressure decay is read. Its purpose: show whether the locked manifold (O-ring, joints) holds about 3 bar without a pressure spike, which validates the actuator work (lock steps 160).

`pressure_ramp [target=bar] [rate=bar/s, 0=step] [hold=s] [decay=s] [maxpower=%] [start=%] [slew=%] [kp= ki= kd=] [label=text] [quick]`. Water only (valves as in `load_slot_water`). Defaults for the seal check: 2.8 bar, 0.05 bar/s (56 s), hold 20 s, decay 30 s, output at most 60 %, start 15 %, output steps at most 3 % per 100 ms, the production PID gains (200 / 100 / 100, now `PUMP_PID_KP/KI/KD` in Settings.h, same values as before). `quick` skips the leak question; `label=` tags the run for `tools/compare_tune.py`. The O-ring is locked first (`lock_oring`) and **left locked** at the end (`oring_unlock` when the pressure is gone). It refuses to start if the line is not empty (P above 0.3 bar), so two runs start from the same state.

**Safety.**
- Refuses unless the pressure sensor is available, a cap is registered on the pump, the manifold is within ±3° of a slot, and `target ≤ cap − 0.2 bar` (so a target of 3.0 is refused while the cap is 3.0: use 2.8, or raise the cap on purpose).
- The cap is checked **on every pass** (about every 2 ms), not every 100 ms (F135).
- The pump output changes by at most 3 % per update (100 ms), below the 10 % that triggers the 4 s ramp (F132). The test starts at 15 % through the normal soft start, then the PID takes over. Unit test: `PRESSURE_RAMP_MAX_STEP_PERCENT < PUMP_RAMP_MIN_STEP_PERCENT`.
- The test stops the pump **itself** at `min(target + 0.15, cap − 0.1)` = 2.9 bar by default (`end=soft_limit`, FAIL), before the hard cap.
- Any key stops it (`end=aborted`). A dead sensor or the cap stops it through `Pump` as always.
- It is a test command: it is not reachable from `sample_process`, the panel or the legacy Pi protocol.

**Output.** One line per second (`t`, phase, setpoint, P, power), then `TEST|pressure_ramp|<slot>|PASS/FAIL|...` (kept under 480 characters: the TEST line buffer was 200 and would have cut it). The numbers that compare two runs: `end`, `pmax`, `ovs` (overshoot over the target), `rise` (s to 90 % of the target, -1 = never), `err` (mean P minus the target in the second half of the hold), `rip` (max minus min of P there), `pw` and `pwp2p` (mean and swing of the power), `rev` (power reversals: chatter), `sat` (% of the time at max power), `reached`, and with a decay `pstop`, `decay`. PASS = completed, `err` ≥ -0.1 bar and no leak seen. **The decay is reported, not judged** (no threshold until there is data: PLACEHOLDER).

**What it does and does not tell.** Pressure only builds if the path is blocked or restrictive (a clogged filter, a closed outlet). On slot 7 (a bare tube) the PID can't build pressure: `sat` is high and `reached=no`, which is the expected reading of "no resistance", not a fault.

**Gains and the tuning loop.** The default gains are the production ones (200 / 100 / 100: tuned by hand, they work, user 2026-10-15), so the first run is the baseline. `kp= ki= kd=` try others without reflashing; `slew=0 rate=0 start=50 maxpower=100` reproduces what `CtrlPump` does today. `tools/compare_tune.py` compares a baseline and a candidate label slot by slot (the guide is in docs/ROADMAP_STAGE1_PROTOTYPE.md, S1-1b part 1b).

**Not changed (S1-1b proper, the next code step):** `CtrlPump::run()` and `pump_pid` (F132–F135). `pressure_ramp` does not use them. The design for their fix is in docs/ROADMAP_STAGE1_PROTOTYPE.md, S1-1b.

**Files:** `src/Core/pressure_ramp.{h,cpp}` (setpoint profile, plan check, slew limit, soft limit), `test/test_pressure_ramp/` (9 tests), `include/Settings.h` (`PRESSURE_RAMP_*`), `src/Tests_mech.cpp` (`cmd_pressure_ramp`, help, dispatch).

**Behavior changes:** none to existing commands. One new console command.

**Tests:** `pio run -e due` pass (flash 141,744 B, RAM 8,348 B). `pio test -e native` 135/135 (11 suites; new: `test_pressure_ramp` 9, `test_pid_metrics` 9). `tools/compare_tune.py`: 8 tests, run with a plain runner because pytest is not installed in the PlatformIO Python (`pytest tools/` still to run in CI or the host venv). `pytest host/`, `pytest tools/`: not run (nothing changed there). **The command has not been run on the machine.**

**Hardware checklist (to run, in this order):**

| # | Command / action | PASS criterion | Result |
|---|---|---|---|
| R1 | Slot 12 (the fully clogged filter), the manifold aligned. `pressure_ramp target=1.0 hold=10 decay=20` | `end=completed`, `reached=yes`, no SAFETY line, `p_max` ≤ 1.15 bar, manometer within 0.1 bar of the printed P | |
| R2 | Same, `target=2.0` | same, `p_max` ≤ 2.15 bar, no overshoot beyond 0.15 bar | |
| R3 | `pressure_ramp` (2.8 bar, defaults), the manometer watched at the end of its scale | `end=completed`, `reached=yes`, `p_max` ≤ 2.95 bar; **note the manometer reading when P says 2.5** (F136) | |
| R4 | Read the decay of R3 | `decay` recorded (no threshold yet). Compare a run with the O-ring locked and one unlocked to see what a leak looks like | |
| R5 | `leak=` answer after R3 | `no`; if `yes`, where (kept in the TEST line) | |

Capture the session: `python tools/hw_test_log.py --port COM6 --tag s1-1b`.
