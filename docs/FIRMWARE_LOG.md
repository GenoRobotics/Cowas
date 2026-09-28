# CoWaS firmware log

One entry per ROADMAP step, newest at the bottom. Each entry says what changed, which **behavior** changed, how it was tested, and what the hardware checklist gave. It also records new findings. Every Claude session reads this file after CLAUDE.md, and adds its entry before the step is merged. It is also the source for the final report.

New findings get the next free ID after ROADMAP §4 (F37, F38, ...). **When steps run in parallel sessions**, each one uses the ID range written in its prompt or given by the user (e.g. FW2: F60–F69, HO1: F70–F79), so two branches never claim the same number; the gaps are fine. The "Step" column says where each one should be resolved. When a finding is fixed, write the step and the date in the "Status" column; don't delete the row.

---

## Findings log (new since ROADMAP §4)

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F37 | Console commands are exact string matches: a trailing `\r` (CRLF line ending) makes every command silently do nothing. Prefix commands (`slotN`, `clockMM`) still work. Worked around in the monitor settings (`monitor_eol = LF`), not in the firmware. | `Tests.cpp:87`, `main.cpp:254` | Medium | FW7 (the protocol parser must strip CR) | open |
| F38 | A bare `clock` or `clockxx` parses as minute 0 and **rotates to slot 7** instead of printing an error. | `Tests.cpp:277-290` | Low | FW2 | open |
| F39 | Legacy Pi link: **any line starting with `sample`** runs a full sample; a malformed depth only prints "Error depth" first. | `main.cpp:257-298` | High (legacy) | FW7 (legacy protocol retired) | open |
| F40 | `step_purge()` (`purgeSterivexFunction`, `purgeContainerFunction`) **leaves the O-ring locked** at the purge slot; the next bare `rotateMotor()` then rotates while locked. | `Step_functions.cpp:220-259` | High | FW4 (auto-unlock before rotation) | open |
| F41 | `calibrate_flow` sets no valves before pumping, and its pump loop doesn't poll the pressure cap (only `pump.start()` checks it). | `Tests.cpp:1384-1474` | High | FW4 / FW5 | open |
| F42 | `step_fill_container()` runs the pump at 100 % for up to 11 min (the container switch D27 is gone), without polling the cap. Reached by `fillContainerFunction` and `purgePipesFunction`. | `Step_functions.cpp:126-213` | Medium | FW5 (cap polling); P2 (remove container code) | open |
| F43 | `zero` (`go_to_zero()`) has no timeout and no encoder-failure check: it spins forever if the encoder fails. It also ignores the lock. | `Tests.cpp:1841-1873` | Medium | FW2 | open |
| F44 | `calibrate_pressure2` computes the new offset from `OLD_OFFSET = 0.483`, but the sensor code uses `OffSet = 3.105`, so the printed value is wrong. (The A2 sensor is retired in FW3 anyway.) | `Tests.cpp:744`, `Pressure_sensor.cpp:10` | Low | FW3 | open |
| F45 | `Flow_sensor.h` declares `flow_small = 12` and `flow_big = 13`, the opposite of Settings.h (`FLOW_SMALL_PIN = 13`, `FLOW_BIG_PIN = 12`). Unused, but misleading. | `Flow_sensor.h:11-12` | Low | FW4 (flow sensor work) | open |
| F46 | `scale_angle()`'s loop guard (`loop_nb`) is never incremented: ±inf would loop forever. Not reachable from the encoder path. Kept as is in FW0 (no behavior change). | `src/Core/manifold_geometry.cpp` | Low | FW2 | open |
| F47 | VNH5019 diagnostics are never read: EN/DIAG D5/D30 and current sense A10/A11 (`Motor::stopIfFault()` has no caller). Motor faults and stalls go unseen. | `Motor.cpp:218` | Medium | FW5 | open |
| F49 | `Serial.begin(9600)` is called twice: `C_output.cpp:30`, then again at `main.cpp:202`. The second call resets the UART while the last boot lines are still in the TX buffer, so `system initalized` / `Programm started` and the last "Button ... initiated" lines are lost and a garbage byte appears. Seen on the FW0 smoke test. Cosmetic today, but it hides boot messages that FW6 will rely on (reset cause, self-check). | `main.cpp:202` | Low | FW6 (boot rework) | open |
| F50 | **No pressure sensor is installed** (FW0 bench check, 2026-09-24). The analog sensor on A2 (`pressure2`) was physically removed, yet it still backs the pump cap and every PID loop: `pressure` reads 0.11 bar from the floating pin, **so the cap can never trip**. Nothing answers on the ABP chip select D8 (`pressure1`): `ERROR \| P1: invalid SPI response (bad status bits)`, 0.00 bar. Until FW3 + HW4: no pumping against a closed or clogged outlet, and none unattended. FW3 renames `pressure1` → `p_in` and deletes the A2 code (user-approved). | `main.cpp:116-139`, `Step_functions.cpp`, `Pressure_sensor.cpp` | **Blocking** | FW3 + HW4 | open |
| F51 | **The 3/2 valve V23 was physically removed** (user, 2026-09-24). Its relay on D40 still clicks (`valves` test), and the code still switches `valve_23` in `step_fill_container`, `sample_process`, `demo_sample_process`, `purge_pipes_manifold`, `calibrate_DNA_pump`, `test_valves` and the panel mode V23. Harmless, but dead. Removal is user-approved and planned in FW4 item 11. | `main.cpp:79,136`, several | Low | FW4 | open |
| F52 | `enc_rot` (`getRotationSPI()`) prints a meaningless turn count (16382 = 0x3FFE: the single-turn AMT22 doesn't answer the turns command) and an angle above 360° (637.38°): it scales the 14-bit value by the 12-bit factor without the `>> 2`. Display only; `rotateMotor()` uses `getPositionSPI()`, which is correct. | `Manifold.cpp:449-474` | Low | FW2 | open |
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
