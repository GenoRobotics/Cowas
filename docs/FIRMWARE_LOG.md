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
| F52 | `enc_rot` (`getRotationSPI()`) prints a meaningless turn count (16382 = 0x3FFE: the single-turn AMT22 doesn't answer the turns command) and an angle above 360° (637.38°): it scales the 14-bit value by the 12-bit factor without the `>> 2`. Display only; `rotateMotor()` uses `getPositionSPI()`, which is correct. | `Manifold.cpp:449-474` | Low | FW2 | open |
| F70 | The Due's dedupe table (last 16 ids) outlives a **host** restart, but PROTOCOL.md §3.1 only asks ids to be unique "for the lifetime of a host session". A restarted host that counts from `h-000001` again could get an old stored result instead of an action (e.g. a `ROTATE` that silently doesn't move). The host now uses a random per-session prefix (`h<6 hex>-<n>`). §3.1 should say "unique while the Due is up (use a per-session prefix)". | PROTOCOL.md §3.1, `host/src/cowas/client.py` | Medium | FW7 (doc wording) | mitigated in HO1 (2026-09-28) |
| F71 | **RECOVER can deadlock**: with the lock state `unknown` (every boot) and the manifold between slots, the `align` step needs a rotation (forbidden while the lock is unknown), and `ACTUATOR_HOME` refuses `not_aligned`. PROTOCOL.md §5.6 orders `align` before `oring`. The simulator fails RECOVER at `align` (`recovery_failed`). FW6 must choose: a manual procedure (free the O-ring by hand, `service`), or homing the actuator off-slot if that is mechanically safe. | PROTOCOL.md §5.6, §5.10 | Medium | FW6 (+ FW4) | open |
| F72 | The request vectors' `expect` lists contain protocol messages only, but §3.6 says every console line is also sent as `{"t":"log"}`. A Due that logs while accepting a command would fail a literal match. The host tests leave out `t:"log"` when they compare; FW7's C++ tests should do the same, or §13 should say so. | `protocol/vectors.json`, PROTOCOL.md §13 | Low | FW7 | open |
| F73 | The element format of `STATUS.data.warnings` isn't specified (every vector has `[]`). The host and the simulator assume `{code, msg, phase}` like the record's `warnings`. | PROTOCOL.md §5.5 | Low | FW7 | open |
| F74 | "Every pumping command needs `P_in`" (§5.8) doesn't list the commands. The simulator refuses `RUN_CYCLE`, `PURGE`, `SAMPLE`, `DNA_SHIELD` and `EMPTY` (air push / empty use the diaphragm pump) without `caps.p_in`, but not `PRIME_SHIELD` (micro pump only). | PROTOCOL.md §5.8, `host/src/cowas/protocol/spec.py` | Low | FW7 | open |
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
- `docs/ROADMAP.md`: HO2 prompt item 4 names the host commands the SOP uses (`cowas slots`, `slots load`, `slots unload` with the retrieval time, `cowas clean`); HO3 prompt: equipment-blank events pause before and after for the operator; §9.1 checklist points to the SOP and packs ≈ 6 L of DNA-free water (resolves LA1's first open point, pending F2).
- `docs/TESTS_PENDING.md`: the SOP dry run, with numeric pass criteria.
- Follow-up (same day): the ROADMAP LA2 prompt gets a "Status / left to do" block and the §7 row says drafted, not done; HW0 #15 DNA-free water ≥ 5 L → ≥ 10 L (the SOP's blanks + rinses), new caps for loading and retrieval.

**Tests:** docs only. Run on `la2-sops` anyway (2026-09-25): `pio run -e due` SUCCESS, `pio test -e native` 15/15 pass.

**Hardware checklist:** the SOP dry run in docs/TESTS_PENDING.md (loading ≤ 30 min, unloading ≤ 20 min, 0 caps left, 0 thread drips, 0 label mismatches). Not run.

**Open points (no F-number: documentation):**
- To verify at the first loading: whether the rotor can turn when a filter is screwed on with the O-ring locked, and whether the Sterivex model's inlet/outlet match the SOP's orientation text (inlet = female luer-lock).
- The bleach flush (§4.2) has no one-shot command before HO2's `cowas clean`; until then only someone who knows the console runs it (F5/F40 cautions). Its volumes (50 mL per slot) are a proposal; LA4 measures the dead volume and may change them.
- Shield line cleaning is left to LA3 (`PRIME_SHIELD`); the SOP forbids bleach there.
## HW1: Electrical audit (2026-09-25)

**Branch:** `hw1-electrical-audit` (from `bed1b77`, the FW1 tip; worktree `../Cowas-hw1`) · **Status:** part 1 done (POWER.md + checklist); waiting for the measurements.

**Behavior changes:** none (documentation only, no code).

**Added:** `docs/POWER.md`: today's power flow (ASCII, with `___` for the unknowns) and a per-load table; the rules from ROADMAP §11.3; the proposed pilot topology (§3.8: 12 V LiFePO4 → regulated 24 V boost behind the E-stop, 5 V buck for the Pi, a separate fused Due supply that the E-stop does not cut, one ground star); the measurement checklist M1–M44 with commands and acceptance values; the HW5 energy table. `docs/TESTS_PENDING.md`: one row pointing at the checklist.

**Points to watch in the results:** the VNH5019 shield's ARDVIN=VOUT jumper (it would put the shield's 24 V on the Due's Vin, limit 16 V); the shield's logic VDD (5 V would put 5 V pull-ups on D5/D30); the pump driver's input range vs the DAC's 2.75 V maximum (F25); the A4988 Vref: don't change it without redoing `calibrate_actuator`, since the stall point sets `LINEAR_ACT_LOCKED_STEPS`.

**Tests:** no code. `pio run -e due` / `pio test -e native`: tree unchanged (not affected).

**Hardware checklist:** docs/POWER.md §4, to fill in. Part 2 (after the results): fill in PINOUT.md (reachable, measured), POWER.md, the fixes and their parts in ROADMAP §11, and any new findings.
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
1. **Commit HO1?** Everything is uncommitted in the working tree of branch `ho1-host-skeleton` (branched from `fw1-protocol-spec`): `host/`, `.github/workflows/ci.yml`, `.gitignore`, `CLAUDE.md`, this log, `docs/TESTS_PENDING.md`, and the F70–F74 pointers in the ROADMAP FW6/FW7 prompts. The user's own LA2 note in `docs/ROADMAP.md` is in the same working tree (not HO1 work).
2. **Merge order:** FW1 (`fw1-protocol-spec`) and HO1 are not merged into `cowas-v2`; merge FW1 first (HO1 builds on it).
3. **F71** (RECOVER deadlock) needs a user decision in FW6; **F70, F72–F74** are PROTOCOL.md wording/choices for FW7. Pointers added to both ROADMAP prompts.
4. **ROADMAP §7:** HO1 not ticked; tick it after the merge and the bench check in TESTS_PENDING (needs FW7).
5. **CA bundle:** the PEM used for pip was built in a session scratch folder and is gone. Rebuild it (certifi + Windows root store) the next time pip/pio downloads fail with `CERTIFICATE_VERIFY_FAILED`; consider keeping one outside the repo, e.g. `%USERPROFILE%\.certs\`.
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
## LA4: Carry-over and slot cross-talk (IN PROGRESS, paused 2026-09-28)

**Branch:** `la4-carryover` in the worktree `../Cowas-la4`, from `18db617` (FW1 + LA1; `cowas-v2` doesn't have them yet) · **Finding IDs for this step:** F80–F89 · **Status:** analysis tool done; the protocol document is **not written yet**; nothing is committed (the files are untracked in the worktree: **commit or copy them before removing the worktree or merging folders**).

**Done:**
- `tools/analyze_la4.py` (stdlib only, Python ≥ 3.8): calibration from a dilution series (piecewise-linear, blanks → LOD 3σ / LOQ 10σ), `washout` (test a: line volume = area under the curve, exponential fit, early-vs-late tailing check, conservative purge for the target, model-free purge, `INTERNAL_DEAD_VOLUME_ML` and `PURGE_DEAD_VOLUME_FACTOR` recommendation = max needed factor × 1.2 rounded up to 0.5), `carryover` (test b: outlet cups + filter eluate, PASS/FAIL/INCONCLUSIVE with an LOD upper bound), `crosstalk` (test c: fraction of the source found per slot, neighbours from the `clocklist` minutes, no hard-coded slot count, `passed_over`, controls). Optional `dilution` columns for out-of-range cups.
- `tools/tests/test_analyze_la4.py`: 47 pytest tests on synthetic data, all passing (run with `python -m pytest tools/tests`; used HO1's `host/.venv` interpreter because this laptop has no system Python). Ruff: only `B905` (needs 3.10) and a few long lines left.
- `tools/la4_example/*.csv`: **synthetic** example files (the CSV formats for the team).

**Still to do (the ROADMAP LA4 prompt):**
1. `docs/LA4_PROTOCOL.md` (not started). Design decided so far:
   - Tracer: **20 g/L NaCl + blue food dye (E133)** in demineralised water. Conductivity (a pen, ATC) gives the numbers, and the dye makes leaks and stained membranes visible. A spectrophotometer (630 nm) is optional if the lab lends one. Standards are made by mass on the 0.01 g scale (1, 0.1, 0.01, 1e-3, 1e-4 + 3 blanks) at every session. Rinse the machine with ≥ 2 L of fresh water afterwards.
   - Test a (washout) **can run before FW4 and should**, so FW4 gets a measured `INTERNAL_DEAD_VOLUME_ML`. It pumps through the purge slot 0 with an open outlet (no filter), supervised (F50). Lock by hand (`linear_actuator`) before FW4, then `pump` at low power. Stop-and-go is allowed. Cups: 25 mL up to 500 mL, then 50, then 100 mL, until 3 cups are below the LOQ. Run 3 times, with the HW6 hose if it's ready (else a bench hose whose volume is known by weighing).
   - Tests b and c pump through Sterivex filters: **blocked by FW3 + HW4 (the cap) and FW4** (purge formula; FW2 `full_cycle`). Test b: dyed cycle on A, then a clear cycle on B with the FW4 purge, 500 mL (smaller = conservative). B's outlet goes in cups (4 × 25 mL, 100 mL, rest pooled), plus a 10 mL syringe eluate of B's filter; 3 A/B pairs. Optional b2: tracer in the micro-pump reservoir (shield path, F81).
   - Test c: load the source (≈ 100 g/L NaCl + dye, its own standards) into A by syringe, with the production shield volume. Hold ≥ 24 h locked at rest, then run short clear cycles (≈ 100 mL) on the other slots. **Sensitivity rule: (measured volume per slot × LOD) < target × source volume.** A 500 mL outlet with a 2 mL source can't prove 0.1 % (a pytest pins this). The first slot after A must be far from A, so that common-path carry-over is told apart from neighbour leakage. Record `passed_over`. Finish with photos of the membranes and a wipe of the manifold face.
   - Test d (qPCR spike) only with the lab (LAB_INTERFACE G3).
2. Recommendations (§3 of the prompt): **`PURGE_DEAD_VOLUME_FACTOR`: provisional 5, not 3 (F80)**, final value from test a. `INTERNAL_DEAD_VOLUME_ML`: the test a result (PLACEHOLDER until then). **Bleach flush:** not for Pilot A. For Pilot B, decide from the data: b/c pass, test d (if done), and a clean end-of-Pilot-A equipment blank → no flush (P2 stays P2); otherwise promote it (thiosulfate neutralisation + rinse needed, or it destroys the next sample's DNA). Consider a mid-deployment equipment blank at the day 3–4 visit.
3. `docs/TESTS_PENDING.md`: rows for LA4 a (blocked by buying the tracer and meter; the pump may refuse to start, F17), b/c (FW3 + HW4, FW4, HW6), d (the lab).
4. ROADMAP §11.1 #16: add a conductivity pen, NaCl, demineralised water ≈ 10 L, ≈ 40 numbered cups, 10 mL syringes.
5. CI: `tools/tests` is not run by CI yet (only `pytest host/`). Add a job, or include it in the host job.

**Open questions for the user (unanswered on 2026-09-28):**
- Which instrument is available (conductivity pen / spectrophotometer / phone)? The user didn't know what a tracer is; it was explained (a harmless substance, salt or dye, added to the test water, not firmware). The plan above assumes buying a cheap conductivity pen.
- Update the FW4 prompt's `PURGE_DEAD_VOLUME_FACTOR = 3` to 5 provisionally (F80)? Not edited, as it waits for the user.
- The field hose (HW6): placeholders 6 mm ID × 5 m (ROADMAP §3.7 example) until HW6.
- Commit LA4 locally? (Nothing is committed; see [[git-local-only]] in memory.)

**Findings (proposed, F80–F89 range):**

| # | Finding | Where | Severity | Step | Status |
|---|---|---|---|---|---|
| F80 | "Purge ≥ 3 × dead volume" (§3.7) only works for near plug flow. A well-mixed volume (pump chamber, sensor tee) needs ln(1/0.001) ≈ **6.9 volumes** for 0.1 % carry-over; 3 × leaves up to 5 %. The factor must come from the LA4 washout; provisional 5. A larger purge also outgrows `step_purge`'s fixed 3 min timeout (FW4 item 6 must size it). | `Settings.h` (FW4), ROADMAP §3.7 | High (data quality) | FW4, LA4 | open |
| F81 | Per the `step_DNA_shield()` code, the shield is pushed with air through the common line and the rotor channel into the slot. Shield and residue stay in the common path until the next purge, which flushes only through slot 0. This carry-over path is not measured yet (LA4 test b2). | `Step_functions.cpp:659-695` | Medium | LA4 | open |
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

