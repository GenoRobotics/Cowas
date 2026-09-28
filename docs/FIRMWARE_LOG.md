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
| F44 | `calibrate_pressure2` computes the new offset from `OLD_OFFSET = 0.483`, but the sensor code uses `OffSet = 3.105`, so the printed value is wrong. (The A2 sensor is retired in FW3 anyway.) | `Tests.cpp:744`, `Pressure_sensor.cpp:10` | Low | FW3 | open |
| F45 | `Flow_sensor.h` declares `flow_small = 12` and `flow_big = 13`, the opposite of Settings.h (`FLOW_SMALL_PIN = 13`, `FLOW_BIG_PIN = 12`). Unused, but misleading. | `Flow_sensor.h:11-12` | Low | FW4 (flow sensor work) | open |
| F46 | `scale_angle()`'s loop guard (`loop_nb`) is never incremented: ±inf would loop forever. Not reachable from the encoder path. Kept as is in FW0 (no behavior change). | `src/Core/manifold_geometry.cpp` | Low | FW2 | fixed FW2 (2026-09-28): bounded, bit-exact within 1024 turns (goldens pass), fmod beyond, NaN for ±inf |
| F47 | VNH5019 diagnostics are never read: EN/DIAG D5/D30 and current sense A10/A11 (`Motor::stopIfFault()` has no caller). Motor faults and stalls go unseen. | `Motor.cpp:218` | Medium | FW5 | open |
| F49 | `Serial.begin(9600)` is called twice: `C_output.cpp:30`, then again at `main.cpp:202`. The second call resets the UART while the last boot lines are still in the TX buffer, so `system initalized` / `Programm started` and the last "Button ... initiated" lines are lost and a garbage byte appears. Seen on the FW0 smoke test. Cosmetic today, but it hides boot messages that FW6 will rely on (reset cause, self-check). | `main.cpp:202` | Low | FW6 (boot rework) | open |
| F50 | **No pressure sensor is installed** (FW0 bench check, 2026-09-24). The analog sensor on A2 (`pressure2`) was physically removed, yet it still backs the pump cap and every PID loop: `pressure` reads 0.11 bar from the floating pin, **so the cap can never trip**. Nothing answers on the ABP chip select D8 (`pressure1`): `ERROR \| P1: invalid SPI response (bad status bits)`, 0.00 bar. Until FW3 + HW4: no pumping against a closed or clogged outlet, and none unattended. FW3 renames `pressure1` → `p_in` and deletes the A2 code (user-approved). | `main.cpp:116-139`, `Step_functions.cpp`, `Pressure_sensor.cpp` | **Blocking** | FW3 + HW4 | open |
| F51 | **The 3/2 valve V23 was physically removed** (user, 2026-09-24). Its relay on D40 still clicks (`valves` test), and the code still switches `valve_23` in `step_fill_container`, `sample_process`, `demo_sample_process`, `purge_pipes_manifold`, `calibrate_DNA_pump`, `test_valves` and the panel mode V23. Harmless, but dead. Removal is user-approved and planned in FW4 item 11. | `main.cpp:79,136`, several | Low | FW4 | open |
| F52 | `enc_rot` (`getRotationSPI()`) prints a meaningless turn count (16382 = 0x3FFE: the single-turn AMT22 doesn't answer the turns command) and an angle above 360° (637.38°): it scales the 14-bit value by the 12-bit factor without the `>> 2`. Display only; `rotateMotor()` uses `getPositionSPI()`, which is correct. | `Manifold.cpp:449-474` | Low | FW2 | fixed FW2 (2026-09-28): angle fixed (`>> 2`); the turn count is still meaningless on a single-turn AMT22 |
| F48 | F12 correction: `pio run -e uno` fails because of Due-only code (`DAC1`, `A10`, `A11`, `Serial1`, SAM3X registers in `Encoder_atmel.cpp`), **not** because `include/interfaces` is missing (it doesn't exist, but `due` builds with the same flag). | `platformio.ini`, several | Low | P2 (drop `uno` or guard the code) | open |

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
| 1 | (manifold at a slot) `actuator_home` | `TEST\|actuator_home\|home\|PASS`, \|err_deg\| ≤ 3, audible stall then unlock, state UNLOCKED | |
| 2 | `lock_cycle 5` | 5 × PASS; lock_ms and unlock_ms each within ±10 % of their mean (≈ 7 s: 231 / 210 steps at 33 steps/s) | |
| 3 | `rotate_all dry auto` | every slot PASS: \|err_deg\| < 1.0 (purge + 14) | |
| 4 | `encoder_repeat 5 10 auto` | 10 × \|err\| < 1.0 and `spread` PASS: spread < 0.5° | |
| 5 | `valve_check` | V1 and Vman PASS (click heard on open and close) | |
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

