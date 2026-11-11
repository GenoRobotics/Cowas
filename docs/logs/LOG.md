# CoWaS project log

> **Read first:** the [field log](field-lexplore-geneva-2026-11-03.md), section 12 (restart order and what not to do; the handover file was removed on 2026-11-03).

One log for everything: firmware, host, hardware, lab, mechanics. It records what was done, tried, failed, decided and learnt, so that a report can be written from it. Every entry of the earlier log is kept, unchanged except for the points in the change note below.

**Related files**
- [findings.md](findings.md): every finding (`F<n>`), with severity, the step that fixes it, status, and the entry where it is first recorded.
- Status lives in the two registers. Log entries record events and link to the register rows.
- [decisions.md](decisions.md): every decision (`D<n>`, `PH<n>`), with date, who decided, and the entry where it is recorded.
- [test-logs/](test-logs/): transcripts of bench runs.
- Roadmaps: [stage 1](../roadmap/stage1-FINISHED-prototype.md), [stage 2](../roadmap/stage2-pilots.md), [stage 3](../roadmap/stage3-full-system.md).
- [archive/](../archive/README.md): superseded documents and removed code.

**How to use it**
- Read the timeline first, then the entry you need. Entries are in date order, grouped by phase.
- Each step entry has: goal, status, decisions, changes, tests, checklist results, open items.
- Findings are `F<number>`. F1–F36 are in [stage 2 §4](../roadmap/stage2-pilots.md). F37 onward are in [findings.md](findings.md). Keep the numbers. Never delete a finding; change its status.
- Decisions from before FW0 are numbered PH1–PH10 (PH = prehistory). Decisions are in [decisions.md](decisions.md).
- To add an entry, write a new section at the bottom. Use the same headings.
- Write short sentences. One fact per sentence. Use tables for numbers and results.
- Parallel steps used ID ranges so that branches never clash: FW2 F60–F69, HO1 F70–F79, LA3 F80–F89, LA4 F110–F119.

**Status words:** *open* (not fixed), *fixed in code* (not yet confirmed on the bench), *fixed* (confirmed), *closed*, *deferred*, *dropped*.

**Area column (findings):** `firmware`, `host`, `hardware` (parts, wiring, sensors, power), `mechanical` (frame, manifold, seal, alignment).

**Change note (2026-10-23):** the findings table and the decisions table moved to [findings.md](findings.md) and [decisions.md](decisions.md). The bench-check rows of the old decisions table are in the 2026-10-23 part of this log ("Bench checks"). Names were replaced by roles ("the helper", "the user"). The word "team" was removed. The findings table has a "Recorded" column, and file references in the log are linked where the file exists.

---

## Where things stand (latest, 2026-10-23)

- Stage 1 is not finished. Done: R0 and T1. Next: step A, the manifold alignment (realign by hand, `cal` on slot 7, new `purge_angle`, flash, check with `slot7` and one `run_sample 7`). Then T2, the operator test: `run_sample` on slots 7, 8, 9 and 10, then the tubing runs. Commit only when asked.
- Firmware on the bench: the build flashed after 2026-10-23 with `purge_angle` 344.27 (D40), the vent (F192), and the record fields (F198, F199).
- The helper's tasks (see [for-felix.md](../for-felix.md)): DNA shield dose calibration, sterivex air drying time, the `cal_DNA` cap check (F202), and the lab values in [shield-protocol.md](../lab/shield-protocol.md) §8.
- Open risks: the vent can unlock the O-ring above 0.3 bar (F192); the flowmeter reads almost nothing at slow flow (F150); the manifold is misaligned again and leaks in the top slots (step A); the manifold encoder coupling loosens (F187, F191); the manifold sometimes does not turn (F203).

## Timeline

| Date | Step | Result | Branch / place |
|---|---|---|---|
| 2026-09, before FW0 | Linear actuator bring-up (A4988, NEMA17). DIR wire fault | Vibration session solved (F167) | prehistory |
| 2026-09, before FW0 | Force sensor contact detection added, then removed | Open-loop lock from a known unlocked reference | prehistory |
| 2026-09, before FW0 | Manifold purge moved to 180°, `FILL_ORDER`, `NB_SLOT` fix | Clock labels and fill order in code. Physical direction not verified (F169) | prehistory |
| 2026-09, before FW0 | Calibration commands, pressure cap in `Pump`, `load_slot`, `multi_sample` | Tools written. Most calibrations not yet run on the machine | prehistory |
| 2026-09, before FW0 | Mechanical fixes printed, fitted and implemented: O-ring seal for the PTFE, PLA inserts, concentric double O-ring, sensor adapter | Leak and pop-out closed (F175–F178) | prehistory |
| 2026-09-24 | FW0 baseline | Smoke test PASS. Found: no pressure sensor installed (F50), V23 removed (F51) | `<user>-sealingMotor-stepperMotor`, base `d508f41` |
| 2026-09-25 | FW1 protocol v1 spec | Done (docs and test vectors) | `fw1-protocol-spec` |
| 2026-09-25 | LA1 lab interface | Template drafted. Meeting with the lab still pending | `la1-lab-interface` |
| 2026-09-25 | LA2 filter SOP | Drafted before the meeting. Lab values are proposals | `la2-sops` |
| 2026-09-25 | HW1 electrical audit | Part 1 done (checklist M1–M44). Measurements pending | `hw1-electrical-audit` |
| 2026-09-25 | LA3 shield protocol and tools | Protocol and tools done. No measurement yet | `la3-shield-retention` |
| 2026-09-28 | HO1 host skeleton | Code and 364 tests pass. Bench check waits for FW7 | `ho1-host-skeleton` |
| 2026-09-28 | FW2 mechanical tests | Code and tests pass. Hardware checklist: rows 1–5 PASS on 2026-10-12 | `fw2-mech-tests` |
| 2026-09-28 | LA4 carry-over | Analysis tool done. Protocol not written. Paused | `la4-carryover` |
| 2026-09-28 | Merge of parallel steps | Done locally into `cowas-v2` (not pushed) | worktree `Cowas-merge` |
| 2026-10-06 | Plan | Three stages. Stage 1 = working prototype | `cowas-v2` |
| 2026-10-09 | S1-0 bench (first) | Incomplete. Garbage ABP frames (F124); fixed by SPI clock | flashed by user |
| 2026-10-09 | S1-1 code | Pressure on the ABP, calibration command. Builds, tests pass | `s1-1-pressure-abp` |
| 2026-10-12 | S1-0 and S1-1 bench (user report) | Lock tests PASS. Calibration PASS (gain 1.0413). Dead-sensor stop PASS. Cap trip done. Row 7 FAIL on clogged filters (F132–F135) | reported in chat |
| 2026-10-15 | S1-1b | `pressure_ramp`, `fill_test`, controller work. Gains and cap changed several times | code, bench runs |
| 2026-10-19 | S1-A | Water flows (PASS). Slot 7: 1000 mL vs 1006 g. Slot 11: 100 mL in 52 s | bench |
| 2026-10-19 | S1-B | NOT MET: dirty water clogged a new filter at 673 mL. Accepted, moved on | bench |
| 2026-10-19 | S1-C fixes | Skip and abort keys, emptying rule, purge volume, pause in operation, shield timing. Built, not flashed | code |
| 2026-10-19 | Stage 1 rewritten | One goal: water through a filter, then several in a row | [`roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) |
| 2026-10-20 | Realignment | Manifold realigned by hand (F164). `purge_angle` = 270.44 (F165). `actuator_home` abort (F166). Not flashed | code |
| 2026-10-21 | Session log | Sample records in firmware. Console tool merged. Built, not flashed | code, [`tools/`](../../tools) |
| 2026-10-21 | Docs restructure | Folders, links, concise wording | [`docs/`](..) |
| 2026-10-22 | Outlet reprint, frame realignment, outlet squared to the axis | Outlet fitted. Frame realigned again. Outlet orthogonality with the manifold axis realigned by hand (F186). `purge_angle` 345.06 from `cal` on slot 7 (encoder 3926.00, D33). Not flashed | user, [`Manifold.h`](../../include/Manifold.h) |
| 2026-10-23 | Bench session 1 (R0, T1 partly, first `run_sample`) | Flashed. `actuator_home` PASS, `abp_raw` steady, `pressure` 0.02 bar, `session` OK. `run_sample` without a slot ran the purge, then refused the sample (F197). Metadata check not done (tool not used). `purge_angle` 344.27 on slot 7 in use. | bench, see the entry at the end |

## Steps

### Prehistory: linear actuator, manifold layout and calibration tools (2026-09, before FW0)

**Source:** the project memo, archived verbatim in [archive/memo-prehistory.md](../archive/memo-prehistory.md). The memo has no day-level dates; the user gives September 2026. This entry keeps its order. Items already covered later in this log are cross-referenced, not repeated.

**Starting point.** Arduino Due with the serial test menu. `stop` starts the panel and the Pi loop. `DEBUG_MODE_PRINT` is a `const bool`, so its `#if` blocks never compile (F11). The manifold purge angle was calibrated with `cal`.

#### Linear actuator (O-ring lock): hardware

- NEMA17 42SHD0034-20B with an A4988 driver. STEP pin 9, DIR 10, ENABLE 11 (active LOW). MS1–MS3 tied to GND: full step, 200 steps per revolution.
- Motor power comes from the VNH5019 shield's 24 V rail, shared with the shield's motors. Logic VDD from the Due 3.3 V. RESET and SLEEP are jumpered together (otherwise the driver stays off).
- Coil pairs by continuity: blue–1B, red–1A, green–2A, black–2B (about 6 Ω within a pair).
- Filter capacitor: 6 × 10 µF/50 V in parallel (about 43 µF, healthy), plus a 7th unit that shows leakage (F173).
- Vref raised to maximise the motor torque (user; F172).
- **Vibration session (F167).** Noise and vibration came from a DIR wire that was off the breadboard. Coil wiring, Vref and MS pins had been checked and were correct. The wire was re-seated. The breadboard power wiring was later soldered (F171).
- **Reason for the actuator.** The manifold must be stiff enough to compress the O-ring and seal, and loose enough to rotate. The actuator cycles: loosen to rotate, tighten to seal, loosen again.

#### Mechanics (manifold, plate, seals)

- Orthogonality of the rotating manifold and the port plate: same issue as F164, first seen here.
- The primary leak dropped once the actuator compressed the O-ring only on an aligned manifold.
- The other mechanical items were fixed (user, 2026-09): aluminium–PTFE leak with O-rings (F175), sensor pop-out with a new sensor and a tighter flexible adapter (F176), PLA water routing with heat-set inserts (F177), concentric double O-ring (F178).

#### Force sensor: added, then removed

- Part: Yiwu GS02889 thin-film FSR, 10 kg, generic, no repeatability specification. Pin A1 (A2 is in use). Divider 3.3 V – FSR – A1 – 10 kΩ – GND, with 10 µF to GND.
- The 10 µF RC filter removed the cable noise (F179). The sensor was used only as a contact detector.
- Firmware: `Force_sensor` (averaged read), `jog_until_contact` (contact confirmed after 3 chunks of 10 steps), `jog_until_keypress`, `test_sample_cycle`, `calibrate_force_sensor`, `calibrate_unlock_cap`, `test_force_sensor`.
- Partial unlock by sensor was dropped: the signal was too close to the noise floor. Unlock uses a fixed step count (PH2).
- **Later removed:** `Force_sensor` deleted. The lock is open-loop from a known unlocked reference (PH3). Reason: the FSR was not accurate (F179).

#### Firmware: linear actuator and sample cycle

- `Linear_actuator` class: `begin`, `set_direction` (writes DIR once), `step_pulses` (does not touch DIR: this fixed the vibrate and stall bug), `move_steps`, and an open-loop `position_steps` that drifts if the motor stalls. `move_mm` is an unused placeholder.
- `linear_actuator` command: manual jog, one turn per press at most. `v<n>` sets the speed.
- `sample_cycle` (`test_sample_cycle`): lock (contact, then fixed extra steps), pump (volume, pressure, 3 min or a key), unlock (fixed steps, always run). The pump stop was hand-rolled because `CtrlPump::max_pressure_` was not enforced (F180).
- `load_slot(volume)`: lock, water, air, unlock, with the manifold already in place. Split later into `load_slot_water` and `load_slot_air` (FW2). The production `step_sampling` kept its own emptying (PH7).

#### Calibration commands

All of them print values for [`Settings.h`](../../include/Settings.h). The user edits the file and reflashes.

| Command | What it does | Status |
|---|---|---|
| `cal` | Encoder calibration of `purge_angle`. Direction choice (`c` clockwise, other key anticlockwise). Speed 20, the latest code value (F181) | Used. Value in [`Manifold.h`](../../include/Manifold.h) |
| `calibrate_actuator` | Lock and unlock step counts | Used. Later `LINEAR_ACT_LOCKED_STEPS` = 210 (see the FW2 conflict note) |
| `calibrate_flow` (`calibrate_flow_sensor`) | Pumps a target volume (2000 mL default). The operator types the measured volume. Factor = old × reported ÷ actual | Written, never run on the machine. F6 open |
| `calibrate_dna_volume` (`calibrate_dna_pump_volume`) | Timed run, measured volume, mL/s, suggested run time. Informs `PUMP_SHIELD_TIME` and `FILL_STERIVEX_TIME` | Written, optional |
| `calibrate_all` (`run_commissioning_calibration`) | Chains encoder, actuator, flow, DNA pump and direction, with y/n per stage. Later also the pressure calibration (S1-1) | Used (from 2026-10-09 at the latest) |
| `manifold_direction` (`identify_manifold_direction`) | A small rotation. The operator reports the observed direction | Result not recorded (F169) |

#### Manifold layout and fill order

- 16 raw positions, 22.5° apart. One has no hole (`omitted_angle_nb` = 12) and is never targeted.
- Purge moved from the calibration point (top) to 180° (`PURGE_RAW_OFFSET` = 8). The 14 remaining positions are samples. Reason: contamination. A leak drips down, so with the purge at the bottom and the lower samples filled first, a leak never reaches the upper samples (PH4, PH5).
- The angle formulas are relative to `purge_angle`, so the anti-tangle logic did not change. The move was a re-indexing: one list, index 0 = purge, 1–14 = samples.
- `FILL_ORDER` = {1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7}. Low to high by quadrant, highest slot last (PH5). Same reasoning as the purge position (PH4). The slot-search loops of `sample_process` and `demo_sample_process` walk this list.
- Purge is not reachable through `FILL_ORDER`, so two `change_state(PURGE_SLOT, unavailable)` calls in [`main.cpp`](../../src/main.cpp) were removed.
- **Bug (F168).** `NB_SLOT` was 14, while 15 positions are addressable. Slot 14 could never be searched, and `get_state(14)` was out of bounds. Fixed: `NB_SLOT` = 15.
- **Clock labels.** `clock_minutes_for_slot()` (top = 00), the `clockMM` and `clocklist` commands, and `MANIFOLD_RAW_INDEX_INCREASES_CW` (true). The labels match the golden table (smoke test PASS, 2026-09-24). The code works in use (user); the physical check result was not recorded (F169).
- The code's index direction is opposite to the clockwise labelling. No slot maths is affected: the purge is symmetric, and so are the height tiers. Only the words "CW" and "CCW" change.
- The maths was checked on paper only. Nothing was checked against the rig before FW0.

#### Pressure safety and test procedures

- **Hard cap centralised in `Pump` (PH6).** `set_pressure_safety` and `enforce_pressure_safety`, checked in `set_power`, `start`, the polling loop and `wait_for_keypress`. Wired once in [`main.cpp`](../../src/main.cpp), so every pump use, tests included, is covered. Cap at the time: 3 bar (later 2.5, then 3.6; see S1-1 and stage 1).
- `CtrlPump::max_pressure_` is now enforced (F180).
- `multi_sample` (`test_multi_sample_loading`): 50 mL per slot in `FILL_ORDER`, with a pause for leak inspection between slots. Later the extended FW2 command (F61).

#### Status of the memo's open items

| Item in the memo | Status now |
|---|---|
| Force and unlock calibration with final hardware | Obsolete: the sensor was removed |
| `LINEAR_ACT_LOCKED_STEPS` placeholder | 210 (S1-0 and FW2 entries) |
| Orthogonality, seals, pressure-sensor pop-out | F164 realigned 2026-10-20 (frame design open). F175–F178 closed (2026-09) |
| ENABLE pull-up, filter capacitor, Vref, shared rail, breadboard wiring | Open: F170, F172 (risk check), F173, F174. Closed: breadboard wiring (F171, soldered) |
| Clock direction on the rig | Closed: the code works in use (F169) |
| Encoder calibration speed | Closed: 20 kept, the latest code value (F181) |
| Force sensing | Closed for the prototype: sensor removed (F179). New sensor type open (stage 3) |
| Linear actuator direction naming | Open (F182) |
| Production emptying not switched to `load_slot` | Decided to keep (PH7); superseded by S1-C changes |

### FW0: baseline, test harness, CI, pin map (2026-09-24)

**Status:** smoke test PASS. Waiting for the commit and the tag `baseline-v0`.

**Decisions:** delete the A2 analog pressure code and rename `pressure1` → `p_in` in FW3. Delete the V23 code in FW4. Neither in FW0, which must not change behavior.

**Changes (no behavior change):**
- `src/Core/manifold_geometry.{h,cpp}` (new): slot-angle table, clock labels, `scale_angle`, moved out of [`Manifold.cpp`](../../src/Hardware/Manifold.cpp) unchanged.
- [`src/Hardware/Manifold.cpp`](../../src/Hardware/Manifold.cpp): calls `core::`. A `static_assert` checks `NB_SLOT`.
- [`include/Manifold.h`](../../include/Manifold.h): `MANIFOLD_RAW_POSITIONS = 16`.
- [`include/Settings.h`](../../include/Settings.h): comments only.
- [`platformio.ini`](../../platformio.ini): `native` test env, `test_ignore = *` on `due`, monitor settings (LF, send on ENTER, no echo).
- Size: flash 85,848 → 86,112 B (+264). RAM 4,564 B, unchanged.

**Added:** [`CLAUDE.md`](../../CLAUDE.md), [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml), [`hardware/pinout.md`](../hardware/pinout.md), [`software/commands.md`](../software/commands.md), this log, [`test/test_manifold_geometry/`](../../test/test_manifold_geometry) (15 tests), [`test/stubs/`](../../test/stubs), [`tools/golden/`](../../tools/golden).

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 15/15. Goldens (table, clock labels, 1,335 `scale_angle` inputs) compared bit for bit. A mutation (breaking the clock rounding or the ±180° boundary) makes them fail.
- Goldens computed on the PC (g++), not on the Due.

**Hardware smoke test:**

| Check | Pass criterion | Result |
|---|---|---|
| Boot messages | `system initalized`, `Programm started` | Partial: output stops after `Button B_start initiated` and a garbage byte (F49). Menu answers: pass |
| `clocklist` | 0=30, 1=34, 2=38, 3=41, 4=49, 5=53, 6=56, 7=0, 8=4, 9=8, 10=11, 11=15, 12=19, 13=23, 14=26 | **PASS** 2026-09-24 |
| O-ring unlocked (precondition) | Found locked after reboot (F18) | Unlocked by hand with `linear_actuator` |
| `slot0`, `slot5`, `clock00` (→ slot 7), `slot0` | Each < 15 s, no error, aligned within about 1° | **PASS** (visual) |
| `enc_rot`, `pressure` | Values print, no hang | **PASS** with findings: turns 16382 and angle 637.38 (F52); P1 SPI error, 0.00 bar; P2 0.11 bar from a floating pin (F50) |
| `valves` | Each valve clicks twice in 30 s | **PASS**: 3 relays heard twice. V23 physically removed (F51) |

**Verdict:** PASS (2026-09-24). FW0 behaves like `d508f41`. The findings are existing facts, not regressions.

**Environment:** WinLibs GCC 16.1 (winget) for `pio test -e native`. PlatformIO downloads need a CA bundle with the Windows root store (TLS inspection). See CLAUDE.md.

**Open:** the spare 150 psi ABP (ABPDANV150PGSA3) may be damaged. Its check is in stage 2 §7a (HW4).

---

### FW1: protocol v1 specification (2026-09-25)

**Branch:** `fw1-protocol-spec` (from `cowas-v2`). **Status:** done (docs only). Open questions answered by the user the same day.

**Behavior changes:** none. ROADMAP prompts FW2, FW4, FW7, LA3 and HW7 updated. §3.2 gained the rest state.

**Added:**
- [`software/protocol.md`](../software/protocol.md): transport (Native USB, JSON lines, 2048-byte max, never 1200 baud), six message types, modes and the mode × command table, P0 commands, the sample record, dedupe (last 16) and check order, error codes, heartbeat (PING 5 s; link lost after 30 s: the running procedure finishes, nothing new starts), timeouts (`ack.max_s` + 30 s), the legacy mapping, versioning.
- [`protocol/vectors.json`](../../protocol/vectors.json): 95 request vectors, 30 Due → host examples, 3 sequences (happy path, STOP and recovery, reboot). Generated once and hand-checked. Edit the JSON directly from now on.

**Decisions (user, 2026-09-25; protocol §14):** D5 in the decisions table. Also: rest state = O-ring locked (D4). Procedures return `ok:true` with a partial record once started (`ok:false` = nothing moved). STOP is never deduplicated. Refusals are not remembered. `boot_id` reveals a Due reboot.

**Deferred to later steps:** `PRIME_SHIELD` destination and duration (LA3, FW7). Compression set of a locked O-ring (HW7: 7-day hold). Filter-outlet caps with the lock at rest (LA3).

**Tests:** no code. [`vectors.json`](../../protocol/vectors.json) parses. Names unique. Every Due → host example parses. The generated long lines are 2100 and 2047 bytes. Firmware and native tests unchanged.

**Hardware checklist:** none (docs only).

**New finding:** F53 (see the findings table).

---

### LA1: lab alignment, meeting template (2026-09-25)

**Branch:** `la1-lab-interface` (from `fw1-protocol-spec`, so the template uses the protocol's field names). **Status:** template drafted. **The meeting with the lab is pending.** LA1 is done when §4 "Decisions" is filled in.

**Behavior changes:** none (docs only).

**Reminders placed in the roadmap** (2026-09-25): stage 2 §7 LA1 row, FW4 item 5a (target volume), HO2 items 1–2 (roles, `sample_id`), LA3 (B1–B5, D1), LA4 d (G3), §9.1 sampling plan (K2, F1–F4), §11.1 #15 (labels, Sterivex model).

**Added:** `lab/lab-interface.md`:
- §2: 48 questions with stable IDs (A1–L3) in 12 groups. Each has our proposal, an empty Answer column, and the step it feeds. Blocking answers: B1, B3, A2, D1, E1, H1, K1, L1.
- §3: metadata columns (roadmap §3.6, the protocol record, retrieval time, lots, operator).
- §4: decisions that later steps read (LA2, LA3, FW4, HO2, HO3, VA2). §5: actions.
- §6: one-page "Sterivex handling in CoWaS" for the lab. Lists what is not measured yet (residual water, retention, carry-over, cross-talk, volume accuracy) and which step measures it.

**Tests:** none (docs only).

**Hardware checklist:** none.

**Open points (documentation):**
- Roadmap §9.1 packs "DNA-free water ≥ 3 L for blanks and purge". Two blanks and their purges need about 5 L. Lab question F2 decides. Fix §9.1 with the answer.
- Proposed `sample_id` for blanks and manual samples (`EB1`, `FB`, `MANn`) and roles `field_blank`, `manual_sample` go beyond roadmap §3.6. HO2 decides after E1 and H1.

---

### LA2: SOPs for filters, cleaning, biosecurity, chain of custody (2026-09-25)

**Branch:** `la2-sops` (from the LA1 commit). **Status:** drafted **before the lab meeting**. Every lab value is our proposal, marked `⟨LAB X⟩` with the question ID. LA2 is done when those markers are replaced and the dry run passes.

**Behavior changes:** none (docs only).

**Added:** `lab/sop-filters.md`, a printable checklist with a box, a record field and a "Who" field on each step:
- §0 deployment header and general rules: gloves, E-stop before hands go into the manifold, never pump against a capped outlet (F50), O-ring stays locked, rotor never turned by hand, no bleach near Shield or lysis buffer.
- §1 cleaning before loading. §2 loading (slots in `FILL_ORDER`, orientation, finger-tight plus a quarter turn, caps off last with a count and a second-person check, photo, `cowas slots load`).
- §3 unloading (blanks first, retrieval time, cool box, photos, count).
- §4 cleaning and biosecurity (flush with bleach, then water, then DNA-free water; dry ≥ 5 days before another water body).
- §5 chain-of-custody form. §6 equipment blank (about 3 L per blank; role `equipment_blank`). §7 sign-off.

**Changed:**
- `lab/lab-interface.md`: new questions D6 (unused filters), D7 (caps at retrieval), F7 (hose tip before the end blank).
- Stage 2: HO2 names the host commands the SOP uses (`cowas slots`, `slots load`, `slots unload`, `cowas clean`). HO3: equipment-blank events pause for the operator. §9.1 packs about 6 L of DNA-free water (resolves LA1's first open point, pending F2).
- Follow-up: HW0 #15 DNA-free water ≥ 5 L → ≥ 10 L. New caps for loading and retrieval.

**Tests:** docs only. Run on `la2-sops` anyway (2026-09-25): `pio run -e due` SUCCESS, `pio test -e native` 15/15.

**Hardware checklist:** SOP dry run (stage 2 §7a, LA2): loading ≤ 30 min, unloading ≤ 20 min, 0 caps left, 0 thread drips, 0 label mismatches. **Not run.**

**Open points:**
- Verify at the first loading: can the rotor turn with a filter screwed on and the O-ring locked? Does the Sterivex inlet/outlet match the SOP text (inlet = female luer-lock)?
- The bleach flush (§4.2) has no one-shot command yet. Until `cowas clean` exists, only someone who knows the console can run it. Volumes (50 mL per slot) are a proposal. LA4 may change them.
- Shield line cleaning is left to LA3 (`PRIME_SHIELD`). The SOP forbids bleach there.

---

### HW1: electrical audit (2026-09-25)

**Branch:** `hw1-electrical-audit` (from the FW1 tip). **Status:** part 1 done ([`hardware/power.md`](../hardware/power.md) and checklist). Waiting for measurements.

**Behavior changes:** none (docs only).

**Added:** [`hardware/power.md`](../hardware/power.md):
- today's power flow (ASCII, `___` for unknowns) and a per-load table;
- the rules from stage 2 §11.3;
- the proposed pilot topology (stage 2 §3.8): 12 V LiFePO4 → regulated 24 V boost behind the E-stop; 5 V buck for the Pi; a separate fused Due supply that the E-stop does not cut; one ground star;
- checklist M1–M44 with commands and acceptance values;
- the HW5 energy table.

**Points to watch in the results:**
- The VNH5019 shield's ARDVIN=VOUT jumper. It would put the shield's 24 V on the Due's Vin (limit 16 V).
- The shield's logic supply. 5 V would put 5 V pull-ups on D5 and D30.
- The pump driver's input range against the DAC's 2.75 V maximum (F25).
- The A4988 Vref. Do not change it without redoing `calibrate_actuator`: the stall point sets `LINEAR_ACT_LOCKED_STEPS`.

**Tests:** no code. Tree unchanged.

**Hardware checklist:** [`hardware/power.md`](../hardware/power.md) §4, to fill in. Part 2 (after the measurements): fill in [`hardware/pinout.md`](../hardware/pinout.md), [`hardware/power.md`](../hardware/power.md), the fixes in stage 1 §2, and new findings.

---

### HO1: host skeleton, protocol client, simulator, CLI (2026-09-28)

**Branch:** `ho1-host-skeleton` (from `fw1-protocol-spec`). **Status:** code and tests done. Bench check waits for FW7.

**Behavior changes:** none on the Due. [`computer_interface/`](../../computer_interface) (legacy) untouched. CI `host-tests` job now runs `ruff check`, `ruff format --check` and `pytest host/` on Ubuntu **and Windows**.

**Added:** [`host/`](../../host) (Python 3.11+, package `cowas`, deps pydantic 2 and pyserial):
- `cowas.protocol`: the §4/§5 tables, pydantic models for every message (strict types, unknown fields kept), the codec (NaN refused), the Due's request checks as pure functions (§7 steps 1, 2, 4, 5), the vector matcher.
- `cowas.transport`: `SerialTransport` finds the Native USB port by VID:PID 2341:003E. Refuses 1200 baud. Refuses the Programming port unless `--bench`. Reconnects. Routing: `{` lines go to the protocol; other lines and `{"t":"log"}` go to the device log. `DeviceLog` (UTC, rotated files under `data/logs/`). `MemoryTransport`.
- `cowas.client.CowasClient`: single-threaded, timeouts on the transport's clock (so it runs in virtual time). Session-unique ids (F70). Acks. Timeouts (2 s short, `ack.max_s` + 30 s long). Resends (3, then link recovery). Heartbeat PING. Reconnect → HELLO → `boot_id` check → resend pending ids. Reboot → `DeviceRebooted`, ids not resent. Subscribers may send STOP.
- `cowas.sim.FakeController` + `SimTransport`: protocol v1 in virtual time (check order, mode table, dedupe, ack, events, 1 Hz telemetry, STOP, CLEAR_FAULT, RECOVER, link loss, `boot_id`, log copies, bench text lines). Faults: `PressureTrip`, `Clog`, `SlowProcedure`, `LinkDrop`, `Reboot`, `WatchdogReset`, `FlowmeterDead`. Geometry from GET_INFO (V1 = 15 slots; tests also use 16 and 24).
- CLI `cowas`: ports, hello, status, info, stop, clear-fault, recover, rotate, lock, unlock, run-cycle, monitor, logs. `--sim` everywhere. Ctrl-C during a long command sends STOP and prints the partial result. RUN_CYCLE results saved to `data/results/*.json`. `clear-fault` and `info` were not in the prompt: needed for the STOP → CLEAR_FAULT → RECOVER path and to see the slots.

**Tests:**
- `pytest host/`: **364 passed**. Includes all 95 request vectors against the simulator, the 30 message vectors and 3 sequences, and the client against the simulator (heartbeat over a 30-min cycle, lost ack, retried RUN_CYCLE runs once, stored result on resend, LinkLost, CommandTimeout, reconnect, STOP from a subscriber, log separation, reboot detection, watchdog reset). Port selection and the 1200-baud guard. CLI runs. No PyQt, ZMQ or Pi imports outside `adapters/`.
- `ruff`: clean. `pio run -e due`: pass (firmware unchanged). `pio test -e native`: 15/15.

**Hardware checklist:** none possible until FW7 (the Due speaks the legacy protocol). The bench check (`cowas hello`, `cowas monitor` on the real Due) is in stage 2 §7a (HO1).

**New findings:** F70–F74.

**Open when the session closed (2026-09-28)** (check when merging):
1. Commit HO1? Everything was uncommitted in the working tree.
2. Merge order: FW1 first, then HO1 (HO1 builds on FW1).
3. F71 (RECOVER deadlock) needs a user decision in FW6. F70 and F72–F74 are protocol wording for FW7. Pointers added to both roadmap prompts.
4. Roadmap §7: HO1 not ticked until the bench check (needs FW7).
5. CA bundle for pip: the PEM was in a scratch folder and is gone. Rebuild it (certifi plus the Windows root store) when pip or pio fails with `CERTIFICATE_VERIFY_FAILED`. Keep it outside the repo, e.g. `%USERPROFILE%\.certs\`.

---

### LA3: shield volume, residual water, shield retention: protocol and tools (2026-09-25)

**Branch:** `la3-shield-retention`. **Finding IDs:** F80–F89. **Status:** tools and protocol done. **No measurement yet.** FW4 and the lab interface §4 are not done, so this step delivers what does not need them. Part A can run now.

**Inputs from the user (2026-09-25):** the Shield has its own tube straight to the manifold (not a tee into the water line). Shield density unknown (1.00 placeholder, pipette check A1). Lab thresholds unknown.

**Behavior changes:** one new console command, `shield_calibrate` (micro pump only; no main pump, no valve, no rotation). No existing command changed. [`Settings.h`](../../include/Settings.h) gains `SHIELD_*` constants (density and target are placeholders).

**Added:**
- `src/Core/shield_calibration.{h,cpp}`: mass → mL, tube volume, least-squares line (rate, offset, R²), per-duration mean, SD and CV, fill time for a target, pass/fail verdict.
- `calibrate_shield_pump()` (console `shield_calibrate`): optional empty-line fill time (→ `PRIME_SHIELD`), prime (≤ 120 s), 3 repeats × 2–3 durations interleaved, masses typed in. Prints `TEST|shield_calibrate|...` and `DNA_SHIELD_FILL_MS`. Every run is time-limited. Any key stops and aborts.
- [`test/test_shield_calibration/`](../../test/test_shield_calibration): 14 native tests.
- `lab/la3-protocol.md`: Part A (density, rate, line volume, drain-back). Part C (12 filters: A-today, B-long, F-warm, S-room, H-handling; paired weighings; holds at 1, 3, 7 days). Part D (analysis). Part E (decision rules and recommendations). Results tables. Open items.
- [`tools/analyze_la3.py`](../../tools/analyze_la3.py) (stdlib only), [`tools/la3_thresholds.json`](../../tools/la3_thresholds.json) (all `null` = OPEN), [`tools/la3_masses_template.csv`](../../tools/la3_masses_template.csv), [`tools/tests/`](../../tools/tests) (22 pytest tests on synthetic data).
- CI: `pytest tools/` job.

**Recommendations (LA3 protocol §5, interim):**
- `DNA_SHIELD_FILL_MS`: none yet. Run Part A, then check the dose reaching the filter in Part C (±10 %).
- Air push: keep today's unless a longer or higher push cuts residual water by ≥ 0.1 mL and ≥ 20 %. If not, raise the Shield dose (F28 physics).
- Caps: caps in Pilot A (operator present). Order about 16 luer check valves as a hedge for Pilot B.
- `PRIME_SHIELD`: to the purge slot. `PRIME_SHIELD_MS` = empty-line fill time × 1.25. Before each deployment and after a bottle change. If the line drains back (A4), a check valve at the manifold end.

**Code changes proposed, not made (ask first):** (1) a console command for the Shield stage alone (`step_DNA_shield`), needed by the paired design; (2) an emptying setpoint parameter (`EMPTY_STX_SETPOINT_BAR`, default 0.4) for the optional B-high variant.

**Open when the session closed (2026-09-28):**
1. The follow-up PR above: not approved.
2. Left uncommitted in worktree `Cowas-la3`. Merge conflicts expected in CLAUDE.md, commands, LOG and the pending checks: keep both sides.
3. Part A (`shield_calibrate`) not run on the machine.
4. Lab thresholds (B1–B5, D1) unknown. [`la3_thresholds.json`](../../tools/la3_thresholds.json) stays all `null`.

**Tests:**
- `pio run -e due`: pass (flash 93,216 B; RAM 4,564 B).
- `pio test -e native`: 29/29 (15 manifold + 14 shield).
- `pytest tools/`: 22/22.

**Hardware checklist (Part A, in order; not run):** `shield_calibrate` with the defaults → `TEST|shield_calibrate|fit|PASS`: every duration CV ≤ 5 %, R² ≥ 0.995, |offset| < 0.1 mL. `line_fill` line_ml within ±20 % of π(ID/2)²L. Drain-back after ≥ 12 h ≥ 95 % of the dose. Any key stops the micro pump at once (check once).

**New findings:** F80–F82 (see the findings table).

---

### LA4: carry-over and slot cross-talk (paused 2026-09-28; protocol dropped 2026-10-21)

**Branch:** `la4-carryover`, worktree `Cowas-la4`. **Finding IDs:** F110–F119 (F110 and F111 used). **Status:** analysis tool done. **The protocol document was never written and is dropped** (2026-10-21, user). Nothing committed in the worktree.

**Done:**
- [`tools/analyze_la4.py`](../../tools/analyze_la4.py) (stdlib only, Python ≥ 3.8):
  - calibration from a dilution series (piecewise linear; LOD = 3σ, LOQ = 10σ);
  - `washout` (test a): line volume, exponential fit, early-vs-late tailing check, conservative purge, model-free purge, `INTERNAL_DEAD_VOLUME_ML` and `PURGE_DEAD_VOLUME_FACTOR` recommendation (max factor × 1.2, rounded up to 0.5);
  - `carryover` (test b): PASS, FAIL or INCONCLUSIVE with an LOD upper bound;
  - `crosstalk` (test c): fraction of the source per slot, neighbours from `clocklist`, no hard-coded slot count.
- [`tools/tests/test_analyze_la4.py`](../../tools/tests/test_analyze_la4.py): 47 tests on synthetic data.
- `tools/la4_example/*.csv`: synthetic example files.

**Design decided (not carried out):**
- Tracer: 20 g/L NaCl plus blue food dye (E133) in demineralised water. A conductivity pen gives the numbers. Standards by mass on the 0.01 g scale. Rinse with ≥ 2 L fresh water afterwards.
- Test a (washout) can run before FW4, and should: it gives a measured `INTERNAL_DEAD_VOLUME_ML`. Purge slot 0, open outlet, no filter, supervised. Cups 25 mL up to 500 mL, then 50, then 100 mL, until 3 cups are below LOQ. Three runs.
- Tests b and c need filters, so FW3 + HW4 (cap) and FW4. Test b: dyed cycle on A, clear cycle on B, 500 mL, 3 A/B pairs. Test c: source (about 100 g/L NaCl and dye) loaded into A, hold ≥ 24 h, then clear cycles on the other slots. **Sensitivity rule:** measured volume per slot × LOD < target × source volume.
- Test d (qPCR spike) only with the lab (lab interface G3).

**Recommendations:**
- `PURGE_DEAD_VOLUME_FACTOR`: provisional **5, not 3** (F110). Final value from test a.
- Bleach flush: not for Pilot A. For Pilot B, decide from the data.
- ROADMAP §11.1 #16: add a conductivity pen, NaCl, about 10 L demineralised water, about 40 cups, 10 mL syringes.
- CI: [`tools/tests`](../../tools/tests) was not run by CI at the time.

**Open:** which instrument is available (pen, spectrophotometer, phone)? Field hose (HW6): placeholders 6 mm × 5 m. Tests b and c are also blocked because the pump may refuse to start (F17).

**Status:** paused. Its tests need hardware that does not exist yet.

---

### FW2: mechanical test suite, lock tri-state, test logging (2026-09-28)

**Branch:** `fw2-mech-tests` (from the FW1 tip). **Status:** code done, builds, tests pass. **Most of the hardware checklist was run on 2026-10-12** (see the table).

**Hardware at start:** no pressure sensor (F50). Flowmeter uncalibrated (factor 77, F6).

**Decisions:** D1, D2, D3 (decisions table). Plus `ACTUATOR_HOME_SLOT_TOLERANCE_DEG = 3°` (placeholder).

**Audit (step 0):** lock and rotation tests were partial. `linear_actuator` was a manual jog. `test_manifold` had no abort and a hard-coded 15. `test_valves` was timed with no confirmation. `leak_hold`, `lock_endurance`, `encoder_repeat`, `actuator_home` and the `TEST|` logging were missing.

**Behavior changes:**
1. O-ring lock is tri-state: LOCKED / UNLOCKED / UNKNOWN (was a bool, false at boot). UNKNOWN at boot and after `linear_actuator` and `calibrate_actuator`. `lock_oring` and `unlock_oring` refuse while UNKNOWN. New `actuator_home` (aligned within 3° → overdrive lock → unlock → UNLOCKED).
2. `rotateMotor()` refuses unless UNLOCKED, and refuses a bad slot index. `cal`, `zero`, `temp`, `encoder_manifold`, `manifold_direction` and `manifold` refuse too (F5).
3. `PRESSURE_SENSOR_AVAILABLE = false`: every pressure procedure refuses (list in [`software/commands.md`](../software/commands.md) §4), including the legacy Pi sample and purge and the panel pump mode.
4. `goto_slot_locked()` and `step_*` return `bool` and stop at the first failed move (pumps off, valves closed, lock as is). `sample_process` and others stop too. Before, a failed rotation locked and pumped at the wrong slot.
5. `step_DNA_shield()` no longer calls a bare `rotateMotor()`: no motion if already locked at the slot; otherwise unlock → rotate → lock.
6. `step_sampling()` takes an optional `volume_ml` (default unchanged). `load_slot()` split into `load_slot_water()` and `load_slot_air()` (same order).
7. Menu `multi_sample` is the extended FW2 command. The old function stays, unreachable (F61).
8. Fixes: F38, F43, F46, F52.
9. Size: flash 122,880 B, RAM 8,328 B (FW0: 86,112 / 4,564).

**Added:**
- [`src/Core/`](../../src/Core): `lock_rules` (tri-state, may_rotate/lock/unlock/home), `slot_iteration`, `openloop` (stop decision, clamps), `test_args`, `test_line` (`TEST|` formatter). `angle_diff_deg` and `nearest_slot` in `manifold_geometry`.
- [`src/Test_report.cpp`](../../src/Test_report.cpp) (TEST lines and summary), [`src/Test_openloop.cpp`](../../src/Test_openloop.cpp) (sensorless mode: 30 % clamp, 120 s limit, 5 s no-flow stop, any key stops, cap re-attached), [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp) (commands in [`software/commands.md`](../software/commands.md) §4, and `help`).
- Settings: `PRESSURE_SENSOR_AVAILABLE`, `TEST_OPENLOOP_*`, `ACTUATOR_HOME_SLOT_TOLERANCE_DEG`, `TEST_ROTATE_MAX_ERROR_DEG = 1.0`, `TEST_ENCODER_MAX_SPREAD_DEG = 0.5`.
- [`tools/hw_test_log.py`](../../tools/hw_test_log.py) (later merged into [`session_log.py`](../../tools/session_log.py)), [`tools/tests/`](../../tools/tests), CI job, [`docs/logs/test-logs/README.md`](test-logs/README.md).

**Grep proof (sensorless mode unreachable from production):** `set_pressure_safety(` is called only in the [`main.cpp`](../../src/main.cpp) setup and [`Test_openloop.cpp`](../../src/Test_openloop.cpp). `pump_open_loop_test` and `OpenLoopSession` are used only in [`Tests_mech.cpp`](../../src/Tests_mech.cpp). [`Tests_mech.cpp`](../../src/Tests_mech.cpp) is reached only through `mech_tests_dispatch()`, called only from `test_all_components()`, called only from `setup()`. `loop()` and [`Step_functions.cpp`](../../src/Step_functions.cpp) never reach it. The helper also refuses unless armed and while `PRESSURE_SENSOR_AVAILABLE` is true.

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 68/68 (lock_rules 9, manifold_geometry 20, openloop 12, slot_iteration 11 incl. every from/count/order/purge for 15 and 24 slots, test_args 16). F64 can block a suite on Windows: rerun it.
- `pytest tools/`: 9/9.

**Hardware checklist** (`python tools/hw_test_log.py --port COMx --tag fw2`, now [`session_log.py`](../../tools/session_log.py)):

| # | Command | PASS criterion | Result |
|---|---|---|---|
| 0 | Reset, then `slot3` | `REFUSED \| rotateMotor: ... UNKNOWN`, nothing moves | not recorded |
| 1 | `actuator_home` | PASS, \|err\| ≤ 3°, audible stall then unlock, UNLOCKED | **PASS** 2026-10-12 (user report; `LINEAR_ACT_LOCKED_STEPS` 160 at the time, see the conflict note below) |
| 2 | `lock_cycle 5` | 5 × PASS, lock and unlock times each within ±10 % of mean (≈ 7 s) | **PASS** 2026-10-12 (times not kept) |
| 3 | `rotate_all dry auto` | Every slot \|err\| < 1.0° | **PASS** 2026-10-12: 15/15, worst −0.35° (slot 1). First run with offsets 5.2 / −1.05: 15/15 FAIL (F128). Log `docs/logs/test-logs/2026-10-12_s1-0.log` |
| 4 | `encoder_repeat 5 10 auto` | 10 × \|err\| < 1.0°, spread < 0.5° | **PASS** 2026-10-12: err 0.09 to 0.26°, spread 0.18°. First run: err −3.95°, FAIL (F128) |
| 5 | `valve_check` | V1 and Vman click on open and close | **PASS** 2026-10-12 |
| 6 | Outlet open, filter in slot 5, `leak_hold 30` + NOSENSOR | Water 30 ± 10 mL on the flowmeter, no leak, unlock OK | not recorded |
| 7 | `multi_sample auto count=3` + NOSENSOR | 3 × PASS, no leak, UNLOCKED | not recorded |
| 8 | `shield_only 1 2000` | PASS, 2000 ± 50 ms, shield liquid seen at the filter inlet | not recorded |
| 9 | `rotate_all` (with lock, hold, unlock) | Every slot PASS, error after unlock within ±1.0° | not recorded |

**Conflict to check (how to confirm on the board):** the Due runs the last flashed build, and no command reads the value back. After the next flash, run `lock_cycle 5`. The unlock should take about 6.4 s (210 steps at 33 steps/s). About 5.3 s would mean the 160 build. The 2026-10-12 entries say `LINEAR_ACT_LOCKED_STEPS` went 210 → 160. [`Settings.h`](../../include/Settings.h) and the 2026-10-20 entry say 210. The current value is **210**. Confirm which value the flashed build has.

**New findings:** F60–F64.

---

### Merge of the parallel steps into `cowas-v2` (2026-09-28)

**Local only** (worktree `Cowas-merge`, nothing pushed). Merge order: FW1 → LA1 → LA2 → HW1 → HO1 → LA3 → LA4 → FW2, one merge commit each.

**Committed first:** HO1, the user's run-status notes (separate commit), LA3, LA4 (partial), FW2.

**Conflicts resolved (both sides kept):** ROADMAP LA2 heading; [`ci.yml`](../../.github/workflows/ci.yml) (HO1's ruff and host job, plus one `tools-tests` job); CLAUDE.md; [`src/Tests.cpp`](../../src/Tests.cpp) includes. FW2's `help` lists `shield_calibrate`.

**Log clean-up:** a union merge had kept two F52 rows (one removed). LA4's F80/F81 clashed with LA3's F80–F82 and were renumbered **F110/F111**.

**Checks on the merged tree:** `pio run -e due` pass (129,904 B, RAM 8,328 B). `pio test -e native` 82/82. `pytest host/` 364 pass, ruff clean. `pytest tools/` 78 pass.

**Still open after the merge:** HO1, LA3 and LA4 open items; the FW2 hardware checklist; HO1's roadmap tick (needs FW7); a stray user note in the LA1 prompt (stage 2 prompts, §8.3), kept as written.

---

### Planning: three stages, stage 1 = working prototype (2026-10-06)

**Branch:** `cowas-v2` working tree, nothing committed. **Status:** documents only.

**Why (user):** the roadmap made the code more complete but slower to a working machine. Priority: a functional prototype for early water tests in the week of 2026-10-12, even without every safety layer.

**Hardware facts reported by the user:** the pressure sensor works. It is an ABPDANV150PGSA3 (150 psi, SPI, CS D8), in place of the old sensor. The analog A2 sensor no longer exists. Not yet calibrated against a manometer. The FW2 checklist was not completed (the sensor was off then).

**Changed:**
- The three roadmap files became stages: [`roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) (new), [`roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md) (was `ROADMAP.md`), [`roadmap/stage3-full-system.md`](../roadmap/stage3-full-system.md) (was `ROADMAP_LATER.md`). Links updated. [`archive/roadmap-v1.md`](../archive/roadmap-v1.md) left as it was. "ROADMAP §N" still means stage 2.
- Stage 1 steps: S1-0 bench facts, S1-1 safety and PID on the ABP, S1-2 a real cycle, S1-3 timed series (optional), S1-4 pressure calibration against a manometer.

**Decisions:** the manometer calibration closes stage 1 (S1-4) and does not block the first water tests. What stage 1 leaves out is in stage 1 §8.2 (never unattended; filters are engineering samples).

**Revision 2026-10-09 (documents only):** the user has a manual manometer (0–2.5 bar) and is ready to calibrate. S1-4 merged into S1-1 (part A: cap, dead-sensor stop and PID on the ABP; part B: `calibrate_pressure` against the manometer, points ≤ 2.2 bar). `STX_MAX_PRESSURE` = 2.5 moved from S1-2 to S1-1, because 3.0 is above the gauge's range. S1-0 gains a manometer item.

**Uncommitted code in the same working tree (not part of this entry):** the `abp_raw` diagnostics command. S1-1 commits it.

**Tests:** none run (documents only).

---

### S1-1: pressure on the ABP, calibration command (2026-10-09)

**Branch:** `s1-1-pressure-abp` (from `cowas-v2` at `58a1fa4`, with the stage documents and the `abp_raw` work). **Status:** code done, builds, tests pass. **Nothing committed, nothing flashed in this entry. Hardware checklist mostly run on 2026-10-12** (see the S1-1 results below).

**Goal:** the pump cap and the PID loops read the installed ABP instead of a floating pin. A dead sensor stops the pump. The sensor can be calibrated against the bench manometer.

**Decisions:** D8 (syringe of air on a tee, not the pump). D7 (manometer is the reference). `calibrate_pressure` is also a stage of `calibrate_all`. The manometer is a 0–2.5 bar dial gauge (F121).

**Behavior changes:**
1. **The pump cap and every PID loop read the ABP on SPI D8** (`read_p_in()`), not the analog A2 pin. Seven call sites: [`main.cpp`](../../src/main.cpp) `set_pressure_safety`; `step_purge`, `step_sampling` (both `CtrlPumpFlow` and `CtrlPumpNoWater`), `load_slot_water`, `load_slot_air`, `step_empty`. The A2 code is kept, unused by safety, until FW3.
2. **ABP conversion** (F27): counts → bar from the datasheet transfer function for `ABP_FULL_SCALE_PSI = 150` (10–90 % of 2^14 counts). Then `pressure = PRESSURE_GAIN × (datasheet − PRESSURE_ZERO_OFFSET_BAR)`. Gain 1.0 and offset 0.0 are placeholders until calibration. Readings are 3.4 % higher than before at the same counts.
3. **Dead or frozen sensor:** stale frames (status 10) hold the last value for at most `ABP_STALE_TIMEOUT_MS` = 500 ms. A fault frame (status 01 or 11, `FF FF FF`, `00 00 00`) distrusts the reading at once. A new valid frame restores trust. While untrusted, `getPressure()` returns −99 bar, so the existing checks stop the pump and refuse to start it. Message: `ERROR | P1: pressure reading not trusted (<why>, last frame <status>)`, at most every 2 s.
4. **`PRESSURE_SENSOR_AVAILABLE`:** written as `true`, set back to `false` the same day (F124, see below). Once true, the pressure procedures no longer refuse. The sensorless mode (NOSENSOR) still refuses. F122 lists commands that still do not poll the cap.
5. **`STX_MAX_PRESSURE` 3.0 → 2.5 bar** (planned for S1-2; 3.0 is above the manometer's range). The PID setpoint is still the hard-coded 2 bar: an overshoot of 0.5 bar trips the cap.
6. **New commands:** `pressure_zero` and `calibrate_pressure`. `calibrate_all` asks for the pressure calibration first. `pressure` prints the ABP reading with 3 decimals, the datasheet value, gain and offset, or `NOT TRUSTED`. `abp_raw` prints the datasheet conversion. The `pump` test prints the ABP reading (it printed A2).
7. Size: flash 136,176 B, RAM 8,344 B (before: 130,848 / 8,328).

**Added:**
- `src/Core/abp_convert` (frame classification, counts → bar, gain and zero, the trust rule) and `src/Core/pressure_calibration` (least-squares gain and offset, R², largest residual, verdict FIT / POINTS / GAIN / RESIDUAL).
- `pressure_zero`: pump off, open to air, 5 s average, prints the `PRESSURE_ZERO_OFFSET_BAR` line. Refuses a zero beyond ±0.3 bar or an unsteady reading. Read-only.
- `calibrate_pressure`: point 0 open to air, then 0.5, 1.0, 1.5, 2.0 bar with the syringe. Live reading once a second. ENTER starts a 3 s average. The manometer value is typed after release (ENTER alone = the target). `s` skips a point, `x` aborts. Prints `TEST|calibrate_pressure|p<i>|...`, `TEST|calibrate_pressure|fit|PASS/FAIL`, and two Settings.h lines. PASS: at least 3 points, gain 0.9–1.1, every residual ≤ 0.06 bar (placeholder, F121). The pump is never started.
- Settings: `ABP_FULL_SCALE_PSI`, `ABP_STALE_TIMEOUT_MS`, `PRESSURE_SENSOR_INVALID_BAR`, `PRESSURE_GAIN`, `PRESSURE_ZERO_OFFSET_BAR`, `PRESSURE_ZERO_*`, `CAL_PRESSURE_*`.
- Driver: `getNominalPressure()`, `isTrusted()`. The old `7.63e-4` and `1.25` constants are gone.

**Formula:** `pressure = gain × (datasheet − zero_offset)`. The zero offset is what the datasheet conversion reads at 0 bar. A later `pressure_zero` changes only the offset. The fit gives `reference = gain × datasheet + intercept`, so `zero_offset = −intercept / gain`.

**A2 code deleted (2026-10-12, user: "the sensor is just absent"):** `BigPressure` (`pressure2`), `read_pressure2()`, the `pressure_sensors` command, `calibrate_pressure2` (F44), the `test_2_remplissage_container_*` functions (never called), and commented A2 lines in [`Flow_sensor.cpp`](../../src/Hardware/Flow_sensor.cpp). `SmallPressure` (unused A1 class) and `pressure_3_pin` kept: they are not A2, and removing them needs a separate OK. `pressure1` unchanged. Builds and tests pass after the removal.

**Not done (stays in FW3):** renaming `pressure1` → `p_in`, SPI diagnostics, the second sensor.

**Tests:**
- `pio run -e due`: pass.
- `pio test -e native`: 109/109 (8 suites). New: `test_abp_convert` (18: each status pair, the dead-line patterns, counts → bar at both full scales, the old constants, the trust rule with grace and timeout, `millis()` wrap, Settings consistency) and `test_pressure_calibration` (9: exact line, round trip, noise, one misread point, fewer than 2 points, bad inputs, gain bounds).
- `pytest host/`, `pytest tools/`: not run (nothing changed there).

**Hardware checklist** (S1-1, in order):

| # | Command or action | PASS criterion | Result |
|---|---|---|---|
| 1 | `abp_raw` and `pressure`, sensor open to air | Status `00` valid; counts near 1638; `pressure` within ±0.16 bar of 0 | **PASS** 2026-10-09 (counts 1631–1636, spread 5, 20 °C), reported done 2026-10-12 |
| 2 | Syringe, manometer and sensor on a tee: `calibrate_pressure` | `fit PASS`: ≥ 3 points, gain 0.9–1.1, residuals ≤ 0.06 bar | **PASS** 2026-10-12 (5 points, R² 0.99897, max residual 0.032 bar) |
| 3 | Paste `PRESSURE_GAIN` and `PRESSURE_ZERO_OFFSET_BAR`, reflash; syringe at about 1.0 and 2.0 bar | Within 0.06 bar of the manometer | Reported done 2026-10-12 (numbers not reported) |
| 4 | Sensor on the pump line, at rest: `pressure` | 0 ± 0.05 bar | Reported done (number not reported) |
| 4b | Rows 1–4 passed: `PRESSURE_SENSOR_AVAILABLE = true`, reflash | `oring_state` prints `true` | Set 2026-10-12 |
| 5 | `pump` at 20 %, open outlet, unplug the sensor | `pressure reading not trusted` and `implausible pressure reading`; pump off within 1 s; refuses to start until reconnected | **PASS** (user) 2026-10-12. Time not reported |
| 6 | `STX_MAX_PRESSURE` temporarily 1.0, reflash; `pump` at 20 %, outlet pinched | `pressure exceeded cap`; pump stops at 1.0 ± 0.1 bar on the manometer | **Done** 2026-10-12 (user). Manometer reading not reported |
| 7 | `actuator_home`, `slot5`, `leak_hold 100` (sensor mode, filter in slot 5) | Water phase ends on volume, no SAFETY, no leak | **FAIL** 2026-10-12 with clogged filters (see S1-1 bench results) |

**Added after the first bench session (2026-10-09):** `PRESSURE_SENSOR_AVAILABLE` back to `false` until the sensor is steady and calibrated (D10). `ABP_SPI_HZ = 100000` (was 800 kHz, the datasheet maximum; F124). `abp_raw` prints each frame's temperature and a min, max and spread line. `calibrate_actuator` says which jog direction releases, and that `c` is pressed with the O-ring released.

**Changed on the user's request (2026-10-09):** `calibrate_pressure` no longer judges a point "not steady". After each average it prints how much the reading moved and asks "Good readout?": type the manometer value to keep the point, or `r` to redo it. `abp_raw` prints the spread without a verdict. `pressure_zero` keeps its steadiness check.

**Added on the user's request (2026-10-09), after a first `calibrate_pressure` try:**
- **First fix (superseded):** the first point was measured before the operator applied the pressure. A key typed as a character plus ENTER left its line feed in the buffer, and the next prompt took it as the start. The console input is now drained before every prompt (`drain_serial_input()`). Prompts are numbered steps. Nothing is measured before ENTER.
- A pump-driven water-column check was written the same day and **removed on the user's decision** (D8). No trace in the code.
- **Second bench report (2026-10-09):** a command had to be typed twice, and `calibrate_pressure` again measured 0.5 bar before the pressure was applied. Cause not identified (stray or late bytes are suspected; input typed during boot is lost, F49). Changes:
  - A version that measured on a steady reading was written and **dropped: not what the user asked**.
  - **Final flow (user's requirement, 2026-10-09):** for each point, ask, wait for ENTER, only then measure 3 s. `wait_for_enter()` discards everything received before the prompt, waits for one whole new line, and discards what follows. `x` + ENTER aborts. Not yet tried on the machine.
  - **Test menu (behavior change, F37):** each command line is trimmed of CR and spaces, and leading non-printable bytes are removed. A lone ENTER is ignored. The echo is `> command`. The menu clears input received during boot and prints `=== Test menu READY ===`.
- Tests after these changes: `pio run -e due` pass (139,088 B). `pio test -e native` 109/109.

**New findings:** F120 (one bad frame stops the pump), F121 (manometer not identified), F122 (commands without cap polling reachable again), F123 (unthrottled print). Older ones: F27 fixed for the installed part (2026-10-09). F50 and F7 and F22 fixed in code, closed later (see F50). F3: the setpoint is still hard-coded at 2 bar.

---

### S1-0: first bench session (2026-10-09, incomplete)

**Firmware:** `cowas-v2` working tree with the `abp_raw` work, flashed by the user. **Console:** `pio device monitor`. The logger was not run, so [`docs/logs/test-logs/`](test-logs) has no file. The results come from the transcript the user pasted.

| S1-0 item | Result |
|---|---|
| 2. `abp_raw` | **FAIL (F124).** Run 1: 20 × `FF FF FF`. Run 2: status `00`, counts 13377–16321. Run 3: status `00`, counts 64–3520. No steady reading at rest. Values did not follow the syringe pressure |
| 3, row 0. Reset, `slot3` | **PASS** (twice): `REFUSED \| rotateMotor: O-ring lock state is UNKNOWN - run actuator_home first` |
| 3, row 1. `actuator_home` | First try: USB link dropped and the Due reset (F125). Second try: `REFUSED ... not aligned (nearest slot 4, error −9.84°, tolerance 3.0°)`. Correct refusal: the manifold was not on a slot |
| 3, rows 2–5 | Not run |
| 4. `calibrate_actuator` | **Not completed.** Aborted twice (`x`). No UNLOCKED mark, no stall point. `LINEAR_ACT_LOCKED_STEPS` stays 210 (placeholder). The actuator ended at an unknown position, with the manifold 9.84° off slot 4 |
| 5, 6, 7 | Not done |

**Notes for the operator (confusions in the session):** the lock state stays UNKNOWN after `calibrate_actuator`, even when completed. Only `actuator_home` makes it known. `c` marks the RELEASED position, and the stall is found in the stage after it. A mistyped command prints nothing (F37 family).

**Second session, same day, S1-1 firmware (SPI clock 100 kHz):** `abp_raw` with the sensor open to air gave 20 valid frames: counts 1631–1636, spread 5, −0.003 bar, 20 °C on every line. Healthy at rest. Not yet known whether the lower clock or a reseated connector cured F124.

**Third reading, same session, syringe plus manometer on the tee** (only the firmware was reflashed, no hardware change):
- About 1 bar on the manometer: counts 4477–4608, mean about 4545.
- About 2 bar: counts 7632–7884, mean about 7700 (the user pushed against a leak).
- Frames clean, 20 °C throughout.
- With the 150 psi constant these read 2.30 and 4.78 bar: 2.3–2.4 times the manometer. With a 60 psi full scale they read 0.92 and 1.91 bar (F126). `ABP_FULL_SCALE_PSI` set to 60 (placeholder).

**Consequences:** S1-0 item 2 done (steady at rest, gain check at 1 and 2 bar). F124 no longer blocks S1-1's checklist. Items 3 (rows 1–5), 4, 5, 6 and 7 still to do.

---

### S1-0: bench session of 2026-10-12 (values reported by the user, no log)

**Evidence:** the user's report in the chat. The logger did not work, so there is no file in [`docs/logs/test-logs/`](test-logs).

| S1-0 item | Result |
|---|---|
| 4. `calibrate_actuator` | Completed: `LINEAR_ACT_LOCKED_STEPS = 160` (was 210), pasted into [`Settings.h`](../../include/Settings.h). **To confirm:** whether `n` was answered at the stall prompt. If so the value is too small (F127) |
| 5. D13 voltage | **Not measured.** The user believes the flowmeter is a 3.3 V one. F26 stays open |
| 6. Intake hose | Reported as "~0.4 mm diameter", length not given. 0.4 mm is not plausible for an intake (4 mm?). **To confirm** with the length, before the purge volume of S1-2 |
| 7. Manometer | Class 1.6, 0–2.5 bar, 0.1 bar divisions (F121 answered) |
| 3, rows 1–5 | Not run at that time: the firmware with 160 steps had to be flashed first |

**Changes:** `LINEAR_ACT_LOCKED_STEPS` 210 → 160 (see the conflict note in FW2). F127 fixed in `jog_until_keypress()`. Comment of `CAL_PRESSURE_MAX_RESIDUAL_BAR` updated, value unchanged (0.06).

**Behavior changes:** (1) `lock_oring`, `unlock_oring` and `actuator_home` move 176 and 160 steps instead of 231 and 210. (2) In `calibrate_actuator`, answering `n` at the stall prompt keeps the step count, and the safety bound applies to the total travel.

**Second part of the session (same day):**
- FW2 rows 1–5 all PASS (see the FW2 checklist). `docs/logs/test-logs/2026-10-12_s1-0.log` holds the transcripts the user pasted.
- F128: stop offsets set to 1.2 / +1.2 ([`Manifold.cpp:17`](../../src/Hardware/Manifold.cpp), [`Manifold.h`](../../include/Manifold.h)). F129: wrap point moved to the no-hole position (between slots 3 and 4). Both are behavior changes of `rotateMotor()`.
- The rows 3 and 4 results were obtained **before** the F129 change. Reflash and rerun `rotate_all dry auto` once.
- Still open: item 5 (D13 voltage), item 6 (hose length), the physical alignment (F128), and the `n` question on the 160 value.
- The lock state is UNKNOWN after every reset (F18, rule 4). `actuator_home` with the manifold on a slot is the one command that clears it. Opening the serial port resets the Due.

**Third part (same day), items 5 and 6:**
- D13 (F26): deferred (D13). Last reading: 3.3 V steady at rest and under a gentle blow. Peak under pulses not checked.
- Intake and spool (F130): the 4 mm × 2.40 m path is the whole water path before the exit, about 30 mL. Spool path 9 mm ID × 40 m (about 2.5 L), connected by default. An earlier entry said 40 mm and 2.5 mL: wrong.
- Hand alignments (F131): −3.69, −4.31, −3.52, −3.69, −2.99 (`oring_state`), and −2.81 earlier.
- Still open: the purge decision with the spool (F130). The alignment check (F131) is deferred to the pilot.
- F129 bench: the wrap point was flashed and reported as working (slot 0 to slot 1 is direct). No numbers kept.

**Behavior change (F130):** `calibrate_all` now asks "Is the spool module connected?" (Enter = yes). It prints the dead volume (30 mL, plus 2.54 L if connected) and the purge the rule needs (3 ×). It warns if `PURGE_MILLILITERS` (100 mL) is below it. It prints only: no purge value changes. New constants: `INTAKE_PATH_ML`, `SPOOL_PATH_ML`, `PURGE_DEAD_VOLUME_FACTOR`. Bench check pending: run `calibrate_all`.

---

### S1-1: bench results (2026-10-12, reported 2026-10-15) and S1-1b

**Evidence:** the user's report in the chat. No log file. Numbers not reported are marked "not reported".

**Settings changed on the bench side (user decisions):** `STX_MAX_PRESSURE` = 3.0 (2026-10-12, for a 3 bar seal test; F136). `PRESSURE_GAIN = 1.0413` and `PRESSURE_ZERO_OFFSET_BAR = -0.0311` pasted (from the 5-point fit, R² 0.99897, max residual 0.032 bar). `PRESSURE_SENSOR_AVAILABLE = true`. `ABP_FULL_SCALE_PSI = 60` (placeholder; F126: the fit absorbed the 2.4 factor).

**Status of older findings:** F27 fixed (conversion and calibration). F21 fixed for the installed sensor. F50: dead-sensor stop PASS and cap trip done. Closed except for row 7 (F132–F135: the cap works, but the control loop in front of it does not keep a clogged filter below it). F126 not resolved, but the calibration makes the reading agree with the manometer up to 2.0 bar.

**New findings:** F132–F136 (see the findings table). F132 and F133 are the most likely reason for the instant cap trips on clogged filters. Hypothesis until a pressure curve is logged.

#### S1-1b (code, 2026-10-15): the `pressure_ramp` test

**What it is.** A console test that regulates the pump **by pressure**, not by fixed power. A PID follows a setpoint that rises at a set rate, holds, then the pump stops and the decay is read. Purpose: show whether the locked manifold holds about 3 bar without a spike. This validates the actuator work.

Syntax: `pressure_ramp [target=bar] [rate=bar/s, 0=step] [hold=s] [decay=s] [maxpower=%] [start=%] [slew=%] [kp= ki= kd=] [label=text] [quick]`. Water only. Defaults: 2.8 bar, 0.05 bar/s, hold 20 s, decay 30 s, output at most 60 %, start 15 %, steps at most 3 % per 100 ms, the production gains 200 / 100 / 100. `quick` skips the leak question. `label=` tags the run for [`tools/compare_tune.py`](../../tools/compare_tune.py). The O-ring is locked first and left locked at the end. It refuses to start if the line is not empty (P above 0.3 bar).

**Safety:**
- Refuses unless the pressure sensor is available, a cap is registered, the manifold is within ±3° of a slot, and `target ≤ cap − 0.2 bar`.
- Cap checked on every pass (about every 2 ms), not every 100 ms (F135).
- Output changes by at most 3 % per update, below the 10 % that triggers the 4 s ramp (F132).
- Stops the pump itself at `min(target + 0.15, cap − 0.1)` (`end=soft_limit`, FAIL), before the hard cap.
- Any key stops it (`end=aborted`).
- Test command only: not reachable from `sample_process`, the panel or the legacy Pi protocol.

**Output:** one line per second, then `TEST|pressure_ramp|<slot>|PASS/FAIL|...`, kept under 480 characters. The comparison numbers: `end`, `pmax`, `ovs` (overshoot), `rise` (s to 90 % of target), `err` (mean P minus target in the second half of the hold), `rip` (max minus min of P there), `pw` and `pwp2p` (mean and swing of power), `rev` (power reversals), `sat` (% of time at max power), `reached`, `pstop` and `decay`. PASS = completed, `err` ≥ −0.1 bar, no leak seen. **The decay is reported, not judged.**

**What it does and does not tell.** Pressure only builds if the path is restrictive (a clogged filter, a closed outlet). On slot 7 (bare tube) the PID cannot build pressure: `sat` is high and `reached=no`. That is "no resistance", not a fault.

**Files:** `src/Core/pressure_ramp.{h,cpp}`, [`test/test_pressure_ramp/`](../../test/test_pressure_ramp) (9 tests), [`Settings.h`](../../include/Settings.h) (`PRESSURE_RAMP_*`), [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp).

**Tests:** `pio run -e due` pass (141,744 B). `pio test -e native` 135/135. The command had not been run on the machine.

**Hardware checklist (in order):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| R1 | Slot 12 (fully clogged), `pressure_ramp target=1.0 hold=10 decay=20` | `end=completed`, `reached=yes`, no SAFETY, `p_max` ≤ 1.15, manometer within 0.1 bar | not run |
| R2 | `target=2.0` | `p_max` ≤ 2.15, overshoot ≤ 0.15 | not run |
| R3 | `pressure_ramp` (2.8 bar), manometer watched | `p_max` ≤ 2.95; note the manometer when P shows 2.5 (F136) | not run |
| R4 | Read the decay of R3 | Recorded. Compare locked and unlocked runs | not run |
| R5 | `leak=` answer after R3 | `no`; if `yes`, where | not run |

#### S1-1b bench session (2026-10-15): gentle controller, `fill_test`, cap 3.5 bar

**What happened.** The setpoint-tracking PID oscillated on slot 11 (F138). A "Part 2" power-ramp controller failed the same day. The PID log showed 4 s blind gaps (F139).

**Changes:**
1. `pressure_ramp` stopped using MiniPID. New `Core/gentle_ctrl`: an incremental PI with a rate limit on power, a 10 % floor, `kp = 0`, and a 2 s low-pass on the pressure. The hard cap still reads the raw value. Default start 10 %.
2. New `Pump::set_power_live(float)`: a fine (12-bit) write to a running pump. Never ramps or blocks. Still checks the cap. `0` is a pause.
3. New command `fill_test [volume=mL] [target=bar] [maxpower=%] [timeout=s] [stall=s] [label=] [quick]`: pumps 1000 mL through the filter of the current slot at 2.5 bar. At `cap − 0.4` bar the pump **pauses**. Water keeps draining through the filter. It resumes under `pause − 0.5` bar at 85 % of the previous power. Ends on `volume`, `timeout` (30 min), `stalled` (under 3 mL in 60 s while not paused), `aborted`, or the hard cap. Flowmeter uncalibrated (F6): weigh the water.
4. **`STX_MAX_PRESSURE` 3.0 → 3.5 bar** (user). `test_abp_convert` failed its "cap under 80 % of the sensor range" line (F143). Not edited, waiting for the user's decision.
5. `FILL_*` constants in [`Settings.h`](../../include/Settings.h).

**Tests:** `pio run -e due` pass. `pio test -e native`: 161 cases, 160 pass, 1 fails (F143). New: `test_gentle_ctrl` (9).

**Hardware checklist (slot 11, new filter):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| G1 | `pressure_ramp target=2.5 rate=0.05 hold=60 decay=10 label=g3 quick` | One line per second, no gap; mean in the last 30 s within 0.15 bar of 2.5; no SAFETY | Run: limit cycle (see next section) |
| G2 | `fill_test volume=200 label=f1` | `end=volume`, measured water within 20 % | not recorded |
| G3 | `fill_test` 1000 mL at 2.5 bar | `end=volume`, `pmax` < 3.2, pauses resumed | not recorded |
| G4 | `pressure_ramp target=3.0 ... ` on slot 11 or 7, locked, manometer watched | `end=completed`, `leak=no` | not recorded |
| G5 | `fill_test` on slot 12 (clogged) | Ends `stalled` or `volume`, never the cap | not recorded |

#### Update, same day: the g3 bench run, cap 3.2, controller redone (F144, F145)

**What the g3 run showed** (slot 11, `ki = 0.5`): a limit cycle, P between 0.4 and 3.3 bar every ~15 s with power 10–13 % (F145). Droplets at the manifold seal near 3 bar (F144). A hard-coded 3 bar limit in [`main.cpp`](../../src/main.cpp) printed "Pressure to high" in a flood.

**Changes (these replace the numbers above):**
1. `STX_MAX_PRESSURE` **3.2** bar (user). `fill_test` pauses at 2.8 bar and resumes under 2.3 bar. Default target **2.2 bar**. `pressure_ramp` default target 2.5 bar, at most 3.0.
2. `pressure1.begin(...)` in [`main.cpp`](../../src/main.cpp) uses `STX_MAX_PRESSURE` (was `3`). The print is throttled to one per 2 s (F123).
3. `gentle_ctrl`: `kp` removed. `ki = 0.05` (was 0.5). `up = 0.1 %/s` (was 0.5). `down = 2 %/s`. `over_factor = 4` (stronger above the setpoint). `rise_hold = 0.3 bar/s` (no power increase while the pressure climbs faster than this).
4. `reached` also needs overshoot ≤ 0.4 bar and ripple ≤ 0.8 bar (placeholders).
5. The sensor is a 150 psi part (user). `ABP_FULL_SCALE_PSI = 60` stays (it matches the manometer, F126).

**Tests:** `pio run -e due` pass. `pio test -e native` 163/163. The model of the bench plant (`test_gentle_ctrl`) reproduces the limit cycle with the old gains and settles with the new ones. The model is from one bench log and is not a proof.

**Hardware checklist (replaces G1–G5; slot 11, new filter):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| H1 | `pressure_ramp target=2.2 rate=0.05 hold=60 decay=10 label=h1 quick` | No "Pressure to high" line; pressure under 2.8; last 30 s within 0.3 bar of 2.2; `reached=yes` | not recorded |
| H2 | `fill_test volume=200 label=h2` | `end=volume`, measured water within 20 % | not recorded |
| H3 | `fill_test` 1000 mL at 2.2 bar | `end=volume`, `pmax` < 2.9 | not recorded |
| H4 | `pressure_ramp target=2.5 ...`, locked, look at the seal | No droplet (F144) | not recorded |
| H5 | `fill_test` on slot 12 | Ends `stalled` or `volume`, never the cap | not recorded |

If H1 still oscillates, the first try is `ki=0.03` or `up=0.05`. If it never reaches the target, `ki=0.1`.

#### Update 2, same day: back to the PID (user decision)

The user found the gentle controller "does nothing for ages and suddenly overshoots". `pressure_ramp` and `fill_test` use `MiniPID` again, with `PUMP_PID_KP/KI/KD` (200 / 100 / 100), slew 3 % per 100 ms, start 15 %, `maxpower` 60, raw pressure. All arguments can be overridden: `kp= ki= kd= slew= start= maxpower= filter=` (`filter=` is a low-pass in seconds, 0 = raw).

One difference from the first version, kept on purpose (F139): the output goes through `Pump::set_power_live`, not `set_power`. 1–9 % become the 10 % floor. 0 means the pump idles.

`Core/gentle_ctrl` stays in the tree, unused by the commands. The cap stays 3.2 bar. The print fix (F145) and the stricter `reached` stay.

#### Update 3, same day: no soft start in the regulated tests

The 4 s soft start of `Pump::start()` runs with no PID, only the cap check. A steep filter can overshoot meanwhile (user). New `Pump::start_at(percent)` energizes and applies the start power **at once**, with the same refusals as `start()`. `pressure_ramp` and `fill_test` use it. `Pump::start()` and the production soft start are unchanged.

#### Update 4, same day: why the PID oscillates (F146, F147)

**F146: the gains 200 / 100 / 100 cannot work on this pump.** `MiniPID` has no time scale: `kp` multiplies the error, and `ki` and `kd` act per 100 ms update. The pump pulses the reading by about ±0.25 bar every 2–3 s. With `kp = 200` that is ±50 % of power. The output slams between 0 and 18 % and the loop is a relay oscillator. A relay's ripple does not depend on its gain. Five bench runs (200/100/100, 150/100/150, 100/100/200, 200/150/100, 200/50/150) all gave ripple 0.99–1.13 bar and overshoot 0.68–0.90 bar. The last hit the cap.

**Model.** `test/test_pid_plant/` runs the real `MiniPID` against a model of the pump and filter, fitted to the five bench runs. Each setting is run on eight plants with five noise seeds. Ranked by its **worst** case: selecting on one noise sequence was misleading.

| Gains (kp / ki / kd, filter) | Worst overshoot | Worst ripple | Cap trips (of 40 runs) |
|---|---|---|---|
| 200 / 100 / 100, raw (bench) | 1.11 | 1.33 | 7 |
| **5 / 0.1 / 0, raw (chosen as the new default)** | 0.25 | 0.66 | 0 |
| 5 / 0.05 / 50, filter 1 s | 0.23 | 0.61 | 0 |
| 3 / 0.1 / 100, filter 2 s | 0.30 | 0.59 | 0 |

The good region is a plateau (kp 3–8, ki 0.05–0.3). **The model is a screening tool: the bench decides.**

**Changes:** `PRESSURE_RAMP_PID_*` = 5 / 0.1 / 0 (placeholder). `PUMP_PID_*` (200 / 100 / 100) stays for production. `CtrlPump::run()` no longer starts with `set_power(50); start();` (50 % open loop, F133). It starts at once at 15 % with `Pump::start_at`, refuses if the pump did not start, and writes the PID output with `set_power_live` after a slew limit of 3 % per 100 ms. **Behavior change in production; not run on the machine.**

**F147:** production still had 200 / 100 / 100 ([`main.cpp`](../../src/main.cpp) `pump_pid`). Not changed yet.

**Hardware checklist (replaces H1; slot 11):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| I1 | `pressure_ramp target=2.2 rate=0.05 hold=60 decay=10 label=i1 quick` (defaults 5 / 0.1 / 0) | `reached=yes`: overshoot ≤ 0.4, ripple ≤ 0.8, no cap | not recorded |
| I2 | If it oscillates: `kp=3`. If too slow: `ki=0.2`. If ripple is high: `kd=50 filter=1` | One change per run | not recorded |
| I3 | `start=10` on slot 12 | No cap | not recorded |

#### Update 5, 2026-10-19: Stage 1 rewritten; the validated gains as defaults

The user: "no water is flowing so it does not make sense ... we are losing ourselves in the details (one day on the PID without a success) ... it needs to work, not be perfect." [`roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) rewritten (old version archived). Four steps: S1-A water flows, S1-B one filter 1 L, S1-C one real cycle, S1-D several in a row.

Code: `PRESSURE_RAMP_PID_*` back to **3 / 0.2 / 20**, the gains that passed on the bench (overshoot 0.23, ripple 0.38 bar). `pio test -e native` 166/166.

**New finding F148:** the pressure builds but no water comes out (2026-10-19). First task of S1-A.

#### Update 6, 2026-10-19: the original PID again; a `fill_test` bug

1. **`fill_test` ended `stalled` at 0 s; the pump never ran (my bug).** The stall window started from `win_start = 0` (boot time), so the first pass was already over 60 s. Fixed: `win_start = t0`. The slot 7 run of 2026-10-19 tested nothing.
2. **Gains back to 200 / 100 / 100** (D26). The user's reasoning: the calm 3 / 0.2 / 20 may run too low to move water (it held 2.2 bar at only 8–12 % of power, near the pump's flow threshold). The original gains burst to about 18 %. The ripple of about 1 bar (F146) is accepted for the prototype. F147 is moot: tests and production share the gains. Command-line overrides still work.

#### Update 7, 2026-10-19: dead code removed (D27)

Removed, with the user's approval:
- `src/Core/power_ramp.{h,cpp}` and `test/test_power_ramp/` (F138). In git history since `06218ff`.
- `src/Core/gentle_ctrl.{h,cpp}` and `test/test_gentle_ctrl/` (F145). **Never committed**: a copy is in the session scratchpad only.
- `test/test_pid_plant/` (F146). **Never committed**, same as above.
- `ramp_soft_limit` and its test; its constants; the `PRESSURE_CTRL_*` and `POWER_*` constants.
- `lowpass_step` moved from the gentle controller into `Core/pressure_ramp`, because `pressure_ramp` and `fill_test` use it.

Native tests: 166 → 135 (the deleted tests covered only the deleted code). Flash size unchanged.

Not changed: the production gains (200 / 100 / 100 in both `PUMP_PID_*` and `PRESSURE_RAMP_PID_*`), `Pump::set_power` (still blocks for other callers, F132), `calibrate_*`, the cap.

---

### S1-A: water flows (bench 2026-10-19), and the `fill_test` changes made on the way

**What happened.** The first S1-A checks showed the pressure building with no water (F148). The cap was raised from 3.2 to 3.6 bar (user). The fill test got its stop criteria rebuilt from the bench logs. **Result: PASS** (user), on slot 7 (bare tube) and slot 11 (new filter).

**Changes:**
1. **One place for the pressure settings** ([`Settings.h`](../../include/Settings.h)). `STX_MAX_PRESSURE` = **3.6** bar (user, 2026-10-19) is the only number to change by hand. The rest derives from it: pause = cap − 0.25 = **3.35** bar, resume = pause − 0.2 = **3.15** bar, highest target = cap − 0.6 = **3.0** bar. The default `fill_test` target is 3.0 bar. A higher `target=` is refused.
2. **`fill_test` stop criteria** (`src/Core/fill_stop.{h,cpp}`, pure, tested natively):
   - volume;
   - timeout (1800 s);
   - **stalled**: under 3 mL in a 60 s window of pumping time (F149);
   - **clogged**: the mean of the last two windows is under 30 % of the first window, from the third window on. The warning `CLOG?` for one window under 30 %;
   - **pressure**: paused and the pressure stays above the resume level for 20 s;
   - aborted (any key); cap.
   The reason is in the TEST line as `end=`. The warning count is `clog_warn=`. Thresholds are placeholders.
3. **Table header:** `ptarget` (was `setpoint`) and `mL_total` (was `mL`). The mL column was always the running total.
4. `test_abp_convert`: the bound becomes 90 % of the sensor range (F143). User decision.
5. `test_pressure_ramp` checks the derived pause, resume and max target against the cap.

**Not changed:** the PID (200 / 100 / 100), the calibration, the flow factor (77, uncalibrated, F6), `pressure_ramp`, the production procedures.

**Tests:** `pio run -e due` pass. `pio test -e native`: 149/149 (`test_fill_stop` 14 new).

**Hardware checklist (S1-A; runs in `docs/logs/test-logs/2026-10-19_s1a.log`):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| A1 | `slot7`, `fill_test volume=1000 label=a1 quick` (the step asked 100 mL; 1 L was run instead) | Printed mL within 20 % of the weight | **PASS**: printed 1000 mL, weighed 1006 g (0.6 %), 92 s, P max 0.37 bar. A bare tube does not build pressure, so this checks the water path and the flowmeter only |
| A2 | `slot11`, `fill_test volume=100 label=a2 quick`, at 2.2 bar, new filter | `end=volume`, ≤ 5 min, ≤ 2.2 bar, weight close to printed mL | **PASS** (user): `end=volume`, 100 mL in 52 s, P max 2.43 bar, 0 pauses. **The weight of this run is not in the notes: to record** |
| A3 | `slot11`, `fill_test volume=100 target=3.0` on the same filter after the 1 L runs (diagnostic, not a pass line) | The clog stop works | `end=stalled` after 383 s at 49 mL counted (windows 8, 28, 9, 3, 1 mL), 366 pauses, P max 3.51 bar, no cap trip. **Not a new filter** (already run to 517 mL and 847 g). Checks the stop, not the flow. F151, F152 |

**Earlier runs in the same session (for the record):** the 1 L run on slot 11 at 2.2 bar, aborted by hand at 517 mL counted (847 g weighed, 763 s; before the stall fix, F149). The 100 mL run with the 3.2 bar cap (pause 2.8 bar): 8 mL counted for 125 g weighed (F150).

**Notes for S1-B:** the slot 11 filter was used for all the failing runs. S1-B needs a **new** filter. Weigh every run (F150).

---

### S1-B and S1-C: one filter at 1 L, the shared filter stop, `full_cycle` end reasons (2026-10-19)

**What happened.** S1-B on a new filter (slot 11) ran with field water, which is not clean. The filter clogged at 673 mL counted (865 g weighed). The S1-B pass line (`end=volume`, 900–1100 g) is **not met** (F153). **The user accepted this and moved on to S1-C** (D20). Bench log: `docs/logs/test-logs/2026-10-19_s1b.log`.

**Changes:**
1. **One set of stop criteria for every filter run** ([`include/Filter_watch.h`](../../include/Filter_watch.h), [`src/Hardware/Filter_watch.cpp`](../../src/Hardware/Filter_watch.cpp)). `FilterWatch` holds the window state and prints the windows. The rules are the pure `src/Core/fill_stop`. `fill_test` uses it. The production filter runs use it too: `CtrlPumpFlow` in `step_sampling` and `load_slot_water`. Settings: 60 s windows, 3 mL, 30 %.
2. **Behavior change:** a production filter run now stops early with `stalled` or `clogged`. Before, it ran to the volume or the time limit. The pressure rule is **off** in production: it needs a pause, and production does not pause. Adding pauses would be a separate behavior change.
3. **Behavior change:** the sampling time limit is `SAMPLE_MAX_DURATION_MS` = 30 min ([`Settings.h`](../../include/Settings.h)), replacing the silent 3 min (F15).
4. `CtrlPump::end_reason()`: the stop reason of the last pump run (`cond`, `timeout`, `cap`, `sensor`, `stalled`, `clogged`). `step_sampling` prints `Step sample ended: …`. The `full_cycle` TEST line carries `sample_end=` and `placeholders=purge_ml,shield_time,flow_factor`.
5. A `stalled` or `clogged` sample is still a sample: shield and emptying run. `full_cycle` still reads PASS with `sample_end=` saying why. **Open question for the user: should a partial sample read PASS?**
6. **Behavior change (F154):** after `stop`, an unknown line prints `UNKNOWN command` and a hint. A CR or space at the end is trimmed. An empty line is ignored.
7. `full_cycle` TEST line: `sample_end=clogged` for both clog criteria, with `rule=stalled` or `rule=clogged` (user: one reason, two criteria).

**Not changed:** the PID (200 / 100 / 100), the flow factor (77, F6), the purge and shield values (placeholders), the cap (3.6 bar).

**Tests:** `pio run -e due` pass. `pio test -e native`: 149/149. **Not flashed, not committed** (the user flashes).

**Hardware checklist (S1-B and S1-C):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| B1 | `slot11` (new filter), `fill_test volume=1000 label=b1` | `end=volume`, 900–1100 g weighed | **NOT MET**: `end=clogged`, 673 mL counted, 865 g, 487 s, 465 pauses, P max 3.56 bar. Diagnostic (dirty water, F153), accepted |
| C0 | `actuator_home` after the flash | PASS, UNLOCKED | pending |
| C1 | `full_cycle 1 volume=1000 purge=y shield=y empty=y` on a new filter, slot 11 | PASS, 1 L weighed (or a flagged partial sample), shield added | pending |
| C2 | The TEST line of that run | `sample_end=` matches what happened; `placeholders=` present | pending |

**Notes for S1-C:** production sampling has no pause. A clogged filter stops by `stalled` (after one window under 3 mL) or `clogged` (from the third window on). That is after about 1 to 3 minutes of pumping, not 30. The weighed partial volume is the sample.

---

### S1-C fixes: skip and abort keys, emptying rule, purge volume, pause in operation, shield timing (2026-10-19)

**Why.** The first `full_cycle` of the day (user log, slot 11, no filter) showed: the emptying steps ran to their maximum (F155); nothing proved the DNA shield delivered (F156); `x` could not stop an operation run (F158); the purge volume ignored the spool setting; operation filter runs had no pause at the cap (F157). The user asked that no step can run without an end.

**Changes (all behavior changes):**
1. **Skip and abort keys during a pump run** ([`include/Run_keys.h`](../../include/Run_keys.h), [`src/Hardware/Run_keys.cpp`](../../src/Hardware/Run_keys.cpp); rules pure in [`src/Core/run_keys.cpp`](../../src/Core/run_keys.cpp)):
   - `s` + ENTER (or `skip`) ends the **current pump run only**. The step goes on with its next phase. A skipped purge goes on to the sample. A skipped sample goes on to emptying, and the shield still runs.
   - `x` + ENTER (or `abort`, `stop`) aborts the **command**: pump stops, step stops at once (valves closed, O-ring stays LOCKED), and the next pump runs of the command refuse.
   - Every operation pump loop reads the keys: `CtrlPump::run`, `Pump::start(ms)` (air push), `Micro_Pump::start(ms)` (shield).
   - Any other typed line during a run is discarded with a message.
2. **Emptying rule (F155):** `EMPTY_WATER_PRESSURE_PURGE_THRESHOLD` 0.04 → **0.4 bar** (placeholder). Air-only lines read 0.02 to 0.09 bar. Water through the open tube read 0.75 to 0.83 bar. The rule: below threshold for 5 s = empty. A timeout prints the last pressure.
3. **Purge volume and spool (addendum item 3):** `SPOOL_INSTALLED = false`. `purge_volume_ml()` = max(`PURGE_MILLILITERS`, `PURGE_DEAD_VOLUME_FACTOR` × (intake + spool if installed)). Spool absent: 100 mL. Spool installed: 3 × (30 + 2540) = **7710 mL**. The user's "6 L" was the spool (2.54 L, F130). The TEST line carries `purge_ml=` and `spool=`.
4. **Pause at the pressure limit in operation (F157):** `CtrlPump::set_pause(FILL_PAUSE_BAR, FILL_RESUME_BAR)` with the `fill_test` rules. Output 0 at 3.35 bar, back at 3.15 bar with the power backed off. Pressure stop on (20 s paused and not coming back down). Used by `step_sampling` and `load_slot_water`. Not used by purge or emptying (no filter). The run prints `PAUSE at`, `RESUME at` and `pauses=N paused_s=S`.
5. **Pressure read on every pass** in `CtrlPump::run` (F135). The PID still updates every 100 ms.
6. **Shield timing (F156):** `step_DNA_shield` prints `micro pump ran N ms (time-based, volume not measured)` and `air push ran N ms at 13 %`. The times are unchanged (25 s, 300 ms: placeholders).
7. `full_cycle` TEST line: `purge_ml=`, `spool=`, `aborted=x` after an abort, `skipped=y` with result **SKIP** (a skipped step is not a PASS).
8. Each step checks the abort latch at its boundaries (`aborted_stop()`). `load_slot_water` and `load_slot_air` refuse when it is set.

**Decisions (2026-10-19 answers):** a skip ends only the current pump run (D25). Spool volume 2.5 L. The pause runs in operation. "No filter" on the purge is intentional, so the purge does not pause.

**Not changed:** PID gains, flow factor (77, F6), shield times, the 60 s and 5 min maxima, the cap (3.6 bar), `SPOOL_USE` (still false).

**Tests:** `pio run -e due` pass. `pio test -e native`: 154/154 (new: [`test/test_run_keys/`](../../test/test_run_keys), 5 cases). **Not flashed, not committed.**

**Hardware checklist (after the flash):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| K1 | `full_cycle 11 purge=y shield=n empty=n volume=1000`; `s` + ENTER 3 s into the purge | `SKIP \| pump run skipped by s` within 2 s; `end=skipped`; sampling starts; `skipped=y`, result SKIP | pending |
| K2 | Same, `x` + ENTER during sampling | `ABORT` within 2 s; pump off; valves closed; O-ring LOCKED; `aborted=x`; no further pumping | pending |
| K3 | `full_cycle 11 purge=n shield=y empty=n`; `s` + ENTER during the 25 s micro pump | `shield pump skipped by s`; the air push still runs | pending |
| E1 | `full_cycle 11 purge=y shield=n empty=y volume=1000` on a bare tube | `Step empty: end=cond` within 10 s of air. If `end=timeout`, send the pressure log: 0.4 bar is wrong | pending |
| E2 | `full_cycle 11 purge=y shield=n empty=n volume=100` on the bare tube, then `purge=y ... empty=y` | Emptying through purge ends with `cond` in under 60 s | pending |
| P1 | `full_cycle 11 purge=n shield=n empty=n volume=1000` on a new filter | `PAUSE at ...` lines with P between 3.35 and 3.6 bar; never above 3.6 bar; `pauses=N` printed | pending |
| P2 | Build with `SPOOL_INSTALLED = true`, `full_cycle 11 purge=y ...` (no flash needed to read the number) | `Step purge: 7710 mL, spool installed`; TEST line `purge_ml=7710;spool=installed`. Then set it back to `false` | pending |
| S1 | `full_cycle 11 purge=n shield=y empty=n volume=100`, bottle weighed before and after | Grams recorded here; micro pump time in the log (`ran 25000 ms`). F156 closes when the grams are recorded | pending |

**Open after these fixes:** the emptying threshold (0.4 bar) is a placeholder until E1 and E2 pass. The shield volume (F156) is unmeasured. `calibrate_all` still asks about the spool, while the purge reads `SPOOL_INSTALLED`: they can disagree until the question is removed.

#### S1-C fixes, second round (2026-10-19, user decisions after the trapped emptying)

**Why.** The `empty=y` run trapped on flowing water (100 s at 15 %, then aborted). The user's rules: with water the operation pump never goes above 60 %, whatever the pressure. With air it may go to 100 %. The operation target is 3 bar, the same as the test. An emptying lasts 5 s.

**Changes (behavior changes):**
1. **Power limit by medium** (`CtrlPump::run`): `PUMP_MAX_POWER_PERCENT` = 60 with water, `PUMP_AIR_MAX_POWER_PERCENT` = 100 with air. Air is known only in emptying: `CtrlPumpNoWater` sets `air_` when the filtered pressure is under its threshold. Purge and sampling are always water (60 %). (Before, the 60 % limit existed only in `pressure_ramp`.)
2. **Operation target 3 bar** (`OPERATION_TARGET_BAR`): purge, sampling, both emptyings, `load_slot_water`, `load_slot_air`, `step_empty`. The emptying phases used 2 bar and 0.4 bar before. The 0.4 bar target was wrong for an air line.
3. **Emptying time:** `EMPTY_MAX_TIME_MS` = 30 s (the user's choice, replacing an earlier 5 s), for every emptying phase, replacing 60 s and 5 min. The air hold (`EMPTY_AIR_HOLD_MS`) is 1 s, so an air line ends early. **Interpretation to confirm:** the user said "empty can be 5 s if relevant". The 5 s maximum was taken as the emptying time, and the air hold was shortened from 5 s to 1 s so a 5 s emptying can end early.
4. **Printing on in `step_empty`**, so the next run records the pressure.

**Not changed:** the shield air push (`PUMP_SHIELD_TIME`, still starts with the 4 s ramp: 4.8 s total, run 2026-10-19), the sterivex emptying threshold (0.8 bar), the 0.4 bar purge threshold, the cap, the PID gains.

**Open:** the air flag starts as "air" for a few milliseconds before the filtered pressure has risen. The slew limit keeps the power from jumping. Check on the bench (E1).

**Tests:** `pio run -e due` pass. `pio test -e native`: 154 pass. **Not flashed, not committed.**

---

### Session close (2026-10-19): what was learnt, and where it stands

**Learnt (keep for the next session):**
1. **`Pump::start(ms)` includes the 4 s soft ramp.** The shield air push `PUMP_SHIELD_TIME` = 300 ms measured 4.8 s in total (4 s ramp, 300 ms, and the 500 ms relay stop in `stop()`). The user accepted 4.8 s. Do not set the value to 4800: that would double it.
2. **The "started" and "stopped" lines of the micro pump are software, not liquid.** `Micro_Pump::start()` prints when the pin is set, not when liquid moves. Only a meter on D48, the transistor and a bottle weight show the pump works (F159).
3. **Water is not air.** An emptying line full of flowing water reads 0.75 to 0.9 bar. Air reads 0.02 to 0.09 bar. A pressure-only air rule trapped on water (F160). The time maximum is the safety net. The power limit (60 % with water) is the second.
4. **The operation pressure target is 3 bar.** A 2 bar target was used in S1-C until 2026-10-19 and was wrong for emptying.
5. **The abort works:** `x` + ENTER stopped the pump, closed the valves and kept the O-ring locked (log of 2026-10-19).
6. **Deployment-line emptying removed from the cycle** (D23). It was 5 min, the steps before it already empty the lines, and it was the trap. `step_empty()` stays for `abort_sample()`.
7. **Typed characters during a run** (`*`, a stray `s`) are ignored with a message. The operator must type s or x with ENTER on purpose.

**Where it stands:** the S1-C code is written and builds. S1-B and S1-C have not passed on the bench. Nothing is committed or flashed since S1-A. The open items are in stage 1 §2 and §7.2.

---

### Recalibration after the manual realignment, and the `actuator_home` abort (2026-10-20)

**Why.** The manifold axis drifted about 1.15° out of square with the cartridge plates (F164). The user realigned it by hand. The angle reference had to be measured again (F165).

**Measured (user, 2026-10-20):**
- `cal`: encoder **3077.00** → purge angle **270.44°** (3077.00 × 360 / 4096). Before: 341.37° (encoder 3884.00, 2026-10-12).
- `calibrate_actuator`: not run again. The user confirmed `LINEAR_ACT_LOCKED_STEPS` = 210 is good (D29).
- `encoder_repeat`: not run.

**Changes (behavior change for `actuator_home` only):**
1. [`include/Manifold.h`](../../include/Manifold.h): `purge_angle = 270.44`.
2. `actuator_home` has an abort key (F166). `x` + ENTER during the overdrive lock or the unlock stops it within one step pulse (about 30 ms) and prints `ABORT | actuator_home: ...`. The O-ring may be part way, so the lock state is set to UNKNOWN until `actuator_home` runs again. The test line says `reason=aborted`. `lock_oring` and `unlock_oring` are unchanged: they run in operation, with no key.
3. Documentation: `LINEAR_ACT_LOCKED_STEPS` was written as 160 in several docs. The code has 210. The docs now say 210. See the conflict note in FW2.

**Not changed:** 210 and the overdrive factor 1.10, the PID, the cap, the flow factor, the shield values, the emptying time.

**Tests:** `pio run -e due` pass. `pio test -e native`: 154/154. The golden mirror of `purge_angle` in [`test/test_manifold_geometry`](../../test/test_manifold_geometry) still has 342.07, so the test does not see the new value. Update it when the goldens are regenerated on purpose.

**Not flashed, not committed.** The flash is stage 1 §2 step 1.

**Hardware checklist (after the flash):**

| # | Command | PASS criterion | Result |
|---|---|---|---|
| A1 | `actuator_home` with the rotor on a slot; `x` + ENTER about 2 s in | `ABORT \| actuator_home: stopped` within 2 s; `reason=aborted`; `oring_state` UNKNOWN; `slot1` refused (no rotation) | pending |
| A2 | `actuator_home` with no key | `done, O-ring UNLOCKED (known)`; PASS | pending |
| A3 | `slot1` after A2 | The rotor moves to slot 1 | pending |

---

### Session log: sample records and the console tool (2026-10-21)

**Why.** Every sample must carry its own stop reason, volumes and pressure, written by the code that decided them (stage 1 §6.1). Pressure mean and peak were not logged anywhere (F135).

**Changes (log output only; no behavior change):**
1. `src/Core/sample_record.h/.cpp` (new, pure): `PressureStats` (mean and peak), `sample_end_name()` (`cond` → `volume`, `cap` → `pressure`, `aborted` → `stop_key`), `format_sample_record()` writes one `{"t":"sample",...}` JSON line. Numbers formatted by hand (no `%f`). NaN is `null`.
2. [`include/Pump.h`](../../include/Pump.h), [`src/Hardware/Pump.cpp`](../../src/Hardware/Pump.cpp): `CtrlPump` keeps the pressure mean and peak of its run. Accessors `pressure_mean()` and `pressure_max()`.
3. [`src/Step_functions.cpp`](../../src/Step_functions.cpp): `print_sample_record()`, called at the end of the purge run (`purpose=purge`) and the sample run (`purpose=sample`). Printed even when the run ends on a stop key. The id is `auto-N` (counts from boot). The `flags` list: `flow_uncal`, `placeholder:ABP_FULL_SCALE_PSI`, `placeholder:STX_MAX_PRESSURE`. `fw` is `S1-1b-dev`.
4. [`tools/session_log.py`](../../tools/session_log.py) (new, **merged with [`tools/hw_test_log.py`](../../tools/hw_test_log.py)**, D30). Shows the device lines, sends typed lines (LF), and writes to [`docs/logs/test-logs/`](test-logs): `<date>_<tag>.log` (transcript), `<date>_<tag>.csv` (TEST rows, as before) and `<date>_<tag>_samples/` (one JSON per sample, `samples.csv`, `pc_received` from the PC clock). A write error prints `LOG ERROR`; the next line written starts with `LOG GAP`. A disconnect prints `DISCONNECTED`. The port is opened with DTR and RTS low. 1200 baud is refused.
5. [`tools/hw_test_log.py`](../../tools/hw_test_log.py) is a shim over [`session_log.py`](../../tools/session_log.py) (same CLI and imports).
6. [`tools/tests/test_session_log.py`](../../tools/tests/test_session_log.py) (11 tests).

**Port.** The Programming port resets the Due on open (stage 2 §3.4). The tool sets DTR and RTS low before the port opens. **Not verified:** whether this stops the reset. Check 2 of stage 1 §6.2.

**Note for FW7:** the sample line starts with `{`, which [`host/`](../../host) routes as a protocol line. FW7 must route `{"t":"sample"` as a log line.

**Tests:** `pio run -e due` pass. `pio test -e native` 163/163 (new `test_sample_record`, 9 tests, with a golden line that the Python tool parses). `pytest tools/` 101 cases. **Not flashed, not committed.**

**Operator tutorial:** [software/session-log.md](../software/session-log.md).

**Hardware checklist:** stage 1 §6.2, items 1–10. Results pending. The serial parts have not been run on hardware.

---

### Docs restructure (2026-10-21)

**What changed**
- Folders: [`roadmap/`](../roadmap), `reference/`, [`hardware/`](../hardware), [`lab/`](../lab), [`archive/`](../archive). This file was renamed from `FIRMWARE_LOG.md` to [`LOG.md`](LOG.md). All links were checked.
- `test_logs/` moved to [`docs/logs/test-logs/`](test-logs). The tools' default folder changed to match.
- Stage 2 and stage 3: the step prompts moved verbatim to `stage2-prompts.md` and `stage3-prompts.md`. The roadmaps keep the plan and point to the prompts.
- Deleted: `TESTS_PENDING.md` and `SESSION_LOG_PLAN.md` (their content is in stage 1 §2, §6 and §7.3, and stage 2 §7a), and the three HTML sensor notes (condensed into [`archive/sensor-notes-2026-09.md`](../archive/sensor-notes-2026-09.md)).
- LA4 protocol dropped: never written.
- Archive kept, including the two old roadmaps. Every rename is listed in [`archive/renames-2026-10-21.md`](../archive/renames-2026-10-21.md).
- Rule 7 wording changed in [`CLAUDE.md`](../../CLAUDE.md) and in stage 2 §6: "the pending checks in the roadmaps updated" replaces "`docs/TESTS_PENDING.md` updated", because that file no longer exists. The rules otherwise match.
- [`CLAUDE.md`](../../CLAUDE.md) points to the stage 1 and stage 2 pending checks.
- Code comments and host docstrings that named the old paths were updated (no behaviour change): [`Settings.h`](../../include/Settings.h) (emptying time), [`src/Core/sample_record.h`](../../src/Core/sample_record.h), [`src/Step_functions.cpp`](../../src/Step_functions.cpp), [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp), [`tools/`](../../tools), [`host/`](../../host).

**Status of the docs (not condensed further, at the user's request)**
- Condensed: [`LOG.md`](LOG.md), [`roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md), [`roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md), [`roadmap/stage3-full-system.md`](../roadmap/stage3-full-system.md).
- Left as written, except links and stale facts: [`software/protocol.md`](../software/protocol.md), [`software/commands.md`](../software/commands.md), [`hardware/pinout.md`](../hardware/pinout.md), [`hardware/power.md`](../hardware/power.md), `lab/lab-interface.md`, `lab/sop-filters.md`, `lab/la3-protocol.md`. These are specifications, checklists or a printable form.
- Archive: kept as is.

---

### Outlet reprint, frame realignment and the reference (2026-10-22)

**Why.** The outlet part was reprinted and refitted (stage 1 §2 step 2). Fitting it showed that the outlet was out of square with the manifold axis (F186). The user also realigned the frame again by hand (F164). The calibration reference was measured again afterwards (F165).

**Changes made (in the working tree, not flashed):**
1. [`include/Manifold.h`](../../include/Manifold.h): `purge_angle` = **345.06**. The user's `cal` was run with the rotor on **slot 7**, which is raw 0 in the slot table (`build_slot_angle_table` walks from the purge at raw 8). So the value is the reference. 3926.00 × 360 / 4096 = 345.06. Was 270.44 (encoder 3077.00, 2026-10-20). The comment in the file now says this.
2. Build and tests: `pio run -e due` passes, `pio test -e native` 163/163. The golden in [`test/test_manifold_geometry`](../../test/test_manifold_geometry) still holds 342.07 and is not regenerated (only on purpose).

**Note for the reference.** The reference moved by 75° between 2026-10-20 (encoder 3077 → 270.44) and today (encoder 3926 → 345.06). The 1.15° realignment does not explain a 75° change. The log does not record which slot the 2026-10-20 `cal` was run on. Future `cal` runs: always record the slot the rotor is on.

**Effect on the homing check.** The `actuator_home` run of today (slot 6, error 2.81°, PASS, and the abort A1) was done with the flashed 270.44 reference. A new reference moves every slot by the same amount, so those results do not carry over. Repeat A2 and A3 after the flash.

**Not flashed.** Flash needed (user go-ahead).

**Checks still to do (stage 1 §2 step 1):** `actuator_home` (A2), `slot1` (A3), `encoder_repeat` (F165), `lock_cycle 5`, session log checks 1–10. Then the seal check at 3.0 bar (step 3) with the outlet as refitted.

**Findings:** F164 (frame drift, realigned again 2026-10-22), F165 (calibration after realignment), F186 (outlet out of square with the axis, new).

**Later the same day (user decisions and bench results):**
- **Direction confirmed on the rig.** `manifold_direction`: commanding `down` gives clockwise (user answered `c`, 2026-10-22). So `angle_offset_pos` is the CW allowance and `angle_offset_neg` the CCW one. The CCW overshoot is therefore `angle_offset_neg` (Manifold.h), not a direction swap.
- **Encoder reversal: not pursued.** User: a rotation, not a reversal. Not needed for operation, since the reference and the slot table come from the same encoder. The clock labels depend on `MANIFOLD_RAW_INDEX_INCREASES_CW`, and slot 0 reads clock 30 (bottom), which matches the purge port at the bottom.
- **`rotate_all` bug (fixed in code, not flashed).** `rotateMotor()` locks the O-ring on return (`OringScope`, Manifold.cpp:118). `rotate_all dry` checked for an unlocked O-ring only once, so it stopped after slot 0. Now it unlocks before each move, and the stop rule is "rotation failed". Test tool only. Build passes.
- **First `rotate_all dry order=fill` (before the fix):** slot 0, clock 30, err −1.67° (TEST limit 1.00°, actuator tolerance 3.0°). Fill order: 0, 1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7. Rerun after the flash.
- **Overshoot (CCW) is open.** Measure with `rotate_all dry order=fill` after the flash, then set `angle_offset_neg` = 1.2 + overshoot (F128).
- **Homing (2026-10-22, user):** A2 PASS (slot 0, err −1.76°, UNLOCKED). A3 `slot1` worked.
- **`rotate_all dry order=fill`, run 2 (user, with a key press per slot):** 2 PASS, 13 FAIL (limit 1.00°). Errors by move direction, from the slot table (CW = `down` = positive error):
  - CW moves: slot 0 +0.97, 14 +1.05, 13 +0.70, 12 +1.14, 11 +1.05, 10 +1.05, 9 +1.05, 8 +1.05, 4 +1.76. Mean +1.09°.
  - CCW moves: slot 1 −1.06, 2 −1.23, 3 −1.32, 5 −1.06, 6 −1.58, 7 −1.41. Mean −1.28°.
  - Both directions overshoot. The code has one offset per direction, not per slot; per-slot would need a table. The slot spread (±0.4°) is within the 3.0° homing tolerance.
  - Applied in code (user decision): `angle_offset_pos` = 2.3 ([`Manifold.cpp`](../../src/Hardware/Manifold.cpp)), `angle_offset_neg` = 2.5 ([`Manifold.h`](../../include/Manifold.h)). `pio run -e due` and `pio test -e native` 163/163 pass. User flashes. Check with `rotate_all dry order=fill auto`; pass: every |err| ≤ 1.00°.
  - **Result (user, flashed build, 2026-10-23): PASS, 15/15.** `rotate_all dry order=fill auto`, worst −0.97° (slot 0), others within ±0.79°. Means by direction: CW −0.26° (9 moves), CCW +0.40° (6 moves). No overshoot left. CCW slots 5, 6, 7 read +0.53 to +0.79°, a small stop-short in that direction, inside the 1.00° test limit and well inside the 3.0° homing tolerance. Not corrected. Autonomous run (`auto`), no key press.
  - **Homing after the flash (user, 2026-10-23):** `actuator_home`: slot 0 aligned, err −0.97°, PASS, UNLOCKED.
  - **Open:** the seal at 3.0 bar (stage 1 §2 step 3); `lock_cycle 5` and the session log checks 1–10 are not reported yet; `encoder_repeat` (F165) not run.
- **Reference changed again (2026-10-23, D34):** `cal` on slot 7 gives encoder 3181.00, so `purge_angle` = 279.58 ([`Manifold.h`](../../include/Manifold.h)). The user saw slot 7 misplaced by eye while the other slots looked aligned. Build and native tests pass. Not flashed. A reference change moves every slot by the same amount (here 65.48°). The `rotate_all` 15/15 result (2026-10-23) was for the 345.06 table and is void for 279.58. The offsets (2.3 CW, 2.5 CCW) are relative and should still hold. Rerun `rotate_all dry order=fill auto` and `actuator_home` after the flash. If slot 7 still reads off by eye while the others read aligned, the table spacing or the cal point is at fault, not the reference, and that needs checking before the seal test.
- **Pressure ramp on an empty slot (user):** no pressure rise, as expected with no filter. Small leak at the outlet, to fix with PTFE tape. Not urgent (open, low).

---

### Bench checks, 2026-10-23 (from the old decisions table)

These rows were in the decisions table of the earlier log. They are bench results, not decisions, so they are kept here.

| Check | Date | Result | By | Ref |
|---|---|---|---|---|
| Lock cycle | 2026-10-23 | `lock_cycle 5` (user, with key presses): 5 of 5 PASS, lock 7008 ms, unlock 6372 ms each cycle (roadmap: unlock about 6.4 s, 210 steps, PASS). Lock 7.0 s = 231 overdrive steps. Run again with `lock_cycle 5 auto` (no key presses) | user | D29 |
| Pressure path (roadmap step 3) | 2026-10-23 | Slot 12 (not 11, as the roadmap says for these targets). Leak answered `n` for each run. Firmware TEST line: **PASS**, then the roadmap's own rule fails (p_max ≤ target + 0.15). See the table below | user | stage 1 §2 step 3 |
| Session log checks 1–10 | 2026-10-23 | **Not run.** [`tools/session_log.py`](../../tools/session_log.py) was started with the placeholder `COMx`, not the Due's port, so it did not open. Checks 2 and 6–10 need the port. Open item | user | stage 1 §6.2 |
| K1 | 2026-10-23 | **PASS** (user, `full_cycle 11 purge=y shield=n empty=n volume=100`, `s` + ENTER in the purge, then `s` in the sample). `SKIP \| pump run skipped by s` within 1.1 s; `end=skipped`; sampling starts; TEST `skipped=y`, result SKIP; the next phases run. The first `s` skipped the purge, the second skipped the sample. The run used `volume=100`, not the roadmap's 1000; the skip rule does not depend on it | user | S1-C K1 |
| K1 (no skip, reference run) | 2026-10-23 | `full_cycle 11 purge=y shield=n empty=n volume=1000` on slot 11: PASS, `sampled_ml=1001`, `sample_end=volume`, 128 s, pressure 0.56–0.59 bar all along, `pauses=0`. Not weighed (S1-B). The emptying through the purge and the filter each ended after about 1 s (F188) | user | F188 |
| Table check | 2026-10-23 | `rotate_all dry order=fill auto` (user) on the 276.42 reference: 15/15 PASS, worst 0.35° (slot 4). The slot table fits the rig; the offsets (2.3 CW, 2.5 CCW) still hold | user | D37 |
| F165 closed | 2026-10-23 | `encoder_repeat 7 10 auto` (user): spread 0.09° (min 0.17, max 0.26; limit 0.50), 10 of 10 PASS, slot 7 approached from slots 14 and 0. The encoder is stable at 3145 (276.42). The earlier readings were 3077, 3181, 3134 and 3145, and 3926 was an offset when the encoder was loose | user | F165, F187 |

**Pressure path table, 2026-10-23 (slot 12)**

| Target | P max | Overshoot | Roadmap rule (p_max ≤ target + 0.15) | Firmware TEST | Note |
|---|---|---|---|---|---|
| 1.0 bar (p1.0) | 1.37 bar at 23.4 s | 0.33 | **not met** (limit 1.15) | PASS (`reached=yes`, `leak=no`) | |
| 2.5 bar (p2.5) | 3.04 bar at 57.3 s | 0.39 | **not met** (limit 2.65) | PASS (`reached=yes`, `leak=no`) | 2.5 was run, not 2.0 |
| 3.0 bar (p3.0), hold 10 s | 3.45 bar at 64.2 s | 0.44 | **not met** (limit 3.15) | FAIL (`reached=no`) | Hold was 10 s, not the 30 s seal check. Peak 3.45 is under the 3.6 cap |

- **2.0 and 2.8 bar were not run.** Only 1.0, 2.5 and 3.0.
- **Controller:** the overshoot is about 0.3–0.4 bar at all three targets, as the 200 / 100 / 100 gains give (F146). Accepted for the prototype by the roadmap (§5). The cap was never close (3.45 vs 3.6).
- **Seal (3.0 bar, 30 s hold):** not run as written. The 3.0 run above had a 10 s hold, and the leak question was answered `n` for the ramp, hold and decay. Still needed: `pressure_ramp target=3.0 hold=30 label=seal3.0` on slot 12.
- **Open decision:** does stage 1 accept the 0.3–0.4 bar overshoot, or lower the targets?
- **DECIDED (user, 2026-10-23): accept the overshoot.** 1.0 and 2.5 bar pass with the overshoot noted. 2.0 and 2.8 bar are not run (bracketed by 1.0, 2.5 and 3.0).
- **Seal check, 3.0 bar, 30 s hold (slot 12, user, 2026-10-23):** `pressure_ramp target=3.0 hold=30 label=seal3.0`. Peak 3.449 bar at 80.6 s, decay 2.43 bar in 30 s. The operator answered **no leak** for the ramp, the hold and the decay. The firmware TEST line is FAIL only because `reached=no` (the 3.0 target was not held, the same overshoot as above). **Seal: no leak at 3.0 bar (roadmap §2 step 3 pass line met).** The FAIL is the overshoot, not a leak.

### Session 2026-10-23: bench summary (stage 1 step 1 to step 7, part)

**Code changes today** (all in the working tree; the branch is uncommitted):
1. [`include/Manifold.h`](../../include/Manifold.h): `purge_angle` 345.06 → 279.58 → 275.45 → **276.42** (encoder 3145.00, slot 7, `cal`). D33–D37.
2. [`src/Hardware/Manifold.cpp`](../../src/Hardware/Manifold.cpp): `angle_offset_pos` 1.2 → **2.3** (CW). [`include/Manifold.h`](../../include/Manifold.h): `angle_offset_neg` 1.2 → **2.5** (CCW). From the first `rotate_all` run (CW +1.09°, CCW −1.28°). Result after the flash: 15/15, worst 0.35°.
3. [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp) `rotate_all`: unlocks before each move (`rotateMotor` locks on return). The stop rule is "rotation failed". Test tool only.
4. [`src/Hardware/Oring_lock.cpp`](../../src/Hardware/Oring_lock.cpp) `actuator_home`: runs at **any angle** (D36, overrides D3 / F60). It moves only the O-ring. Off-slot it prints `WARN`. [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp): the homing test PASSES only when aligned.
5. [`include/Settings.h`](../../include/Settings.h): `EMPTY_AIR_HOLD_MS` 1000 → **20000** (D38, F188).

**Bench results (user, 2026-10-23):**
- Homing: A1 abort PASS; A2 PASS (slot 0, −0.97°); A3 `slot1` PASS. The rotor was off-slot (6.6°) after the loose encoder, and `actuator_home` refused until the change in 4.
- `encoder_repeat 7 10 auto`: PASS, spread 0.09° (F165 closed). Earlier calibration readings 3077 / 3181 / 3134 / 3145 (3926 was an offset when the encoder was loose).
- `rotate_all dry order=fill auto` (276.42): 15/15 PASS, worst 0.35°.
- `lock_cycle 5`: 5/5 PASS, lock 7008 ms, unlock 6372 ms (roadmap about 6.4 s).
- Pressure path (slot 12, clogged filter): 1.0 bar PASS (peak 1.37, overshoot 0.33). 2.5 bar PASS (peak 3.04, overshoot 0.39). 3.0 bar, 30 s hold (seal): no leak; firmware FAIL only because `reached=no`. **Overshoot accepted by the user (decision).** 2.0 and 2.8 bar not run.
- Session log: not run (`COMx` placeholder, not the Due's port). Checks 1–10 open.
- K1 (skip keys): PASS. `s` in the purge and `s` in the sample both end their step with `SKIP`, and the next phases run.
- First full sample (`full_cycle 11 purge=y shield=n empty=n volume=1000`, slot 11): PASS, 1001 mL, flowmeter (uncalibrated), 128 s, pressure 0.56–0.59 bar. Not weighed. The emptying phases ended after about 1 s (F188), which led to the air hold change.

**Findings today:** F186 (outlet out of square, realigned by hand), F187 (encoder came loose; fixed), F188 (air hold 1 s; fixed to 20 s in code, not flashed). The fill loop does not print the pump power, so regulation to 3 bar is not visible in the log (open question, not blocking).

**Decisions today:** D33–D38 (see the table at the top). The offsets and the 20 s hold are decisions by the user. The overshoot acceptance (1.0 and 2.5 bar, 0.3–0.4 bar overshoot, cap 3.6 bar never reached) is the user's.

**Flash (user, 2026-10-23, after the 20 s change):** the user flashed again with the 20 s air hold. The latest build includes every change in this summary.

**Session log attempt (user, 2026-10-23):** [`tools/session_log.py`](../../tools/session_log.py) on COM6 did not work for the user. Cause not recorded; the user asked not to debug it now. Checks 1–10 are still open. Note the port for the next session.

**Not done (in order):**
1. (Done: flashed with the 20 s hold.)
2. K2 (new filter, `x` during the sample), K3 (`s` during the shield pump).
3. E1, E2 with the 20 s hold: does the line empty? (E1 checks the emptying, not just the timer.)
4. P1, S1 (weigh the shield bottle).
5. S1-B: `fill_test volume=1000 label=b1` on a new filter, weighed. Pass: 900–1100 g.
6. `calibrate_all` (answer no to the spool) and `full_cycle 11 volume=1000 purge=y shield=y empty=y`.
7. S1-D: `full_all count=3 volume=1000`.
8. Session log (fix the port) and checks 1–10, when convenient.
9. Commit and merge into `cowas-v2` only when the user asks.

**Warnings for the next session:** the encoder must stay tight (check with `encoder_repeat` after any mechanical work). The filter used in the first sample is the same slot 11 filter; use a new filter for S1-B. The pressure path leaves the O-ring LOCKED at the end (`pressure_ramp` locks it). The lock cycle prints a key prompt unless `auto` is given.

---

### Session 2026-10-23 (afternoon): stage 1 steps 1 and 2 on the bench

**Build:** the flashed build with the 20 s air hold (D38). Filter use: slot 11 K1 filter (already 1 L) for K2 only; slot 7 bare tube for K3, E1, E2. No new filter used.

**Results (user, bench, COM6 console):**

| Row | Setup | Result |
|---|---|---|
| Homing | `actuator_home` (rotor at slot 11) | PASS. `slot 11 aligned (error 0.08 deg)`, `done, O-ring UNLOCKED (known)`, `TEST\|actuator_home\|home\|PASS\|slot=11;err_deg=0.08;tol_deg=3.0` |
| K2 | Slot 11, `full_cycle 11 purge=y shield=n empty=n volume=1000`, `x` during sampling | **PASS.** `ABORT \| run aborted by x - pump stopped` about 2 s after the second `x` (see F190). `Pump P1 stopped`, valves closed, `ERROR \| step_sampling: stopped - pumps off, valves closed, O-ring LOCKED`. `oring_state`: LOCKED. Sample `end_reason=stop_key`, 3 mL. The TEST line reads `FAIL ... aborted=x`: that is the expected result of an abort, not a failed sample |
| K3 | Slot 7 bare tube, `full_cycle 7 purge=n shield=y empty=n volume=100`, `s` during the micro pump | **PASS.** `SKIP \| shield pump skipped by s` after 4495 ms of micro pump. Air push ran 4800 ms at 13 %. Sample 101 mL, `sample_end=volume`. TEST PASS |
| E1 | Slot 7 bare tube, `full_cycle 7 purge=y shield=n empty=y volume=100` | **PASS.** Purge 100 mL in 13955 ms (`end=cond`). Both emptying phases (through the purge, then through the filter) printed `End cond first met` at once, with the pressure at 0.01–0.13 bar. Each pump run stopped after about 7 pressure lines, about 20 s by count (the log has no emptying timestamp). `empty_step=removed`. TEST PASS |
| E2 | Same run as E1 | **PASS.** The emptying through the purge ended with `cond`, not `timeout`. Covered by E1 (decision in the plan) |

**Notes:**
- The emptying threshold (0.4 bar) is met from the first reading on the bare tube, so the air phase is ended by the 20 s hold, not by the pressure. E1 passes on its pass line (`cond`, not `timeout`). Check the emptying on a filter at S1-D; not blocking.
- The operator typed `slot12` before K2 instead of `slot11`. `full_cycle 11` moved to slot 11 anyway. No effect on the result.
- The log has no `Step empty: end=` line in the format of §2; the E1 result is read from `End cond first met`. Recorded as is.

**Findings:**
- **F189 (new, user):** a leak at the outlet of the manifold, at the 3D-printed thread (not at the O-ring seal). Not urgent, not for the prototype. Solve later (stage 2 outlet part; PTFE tape for now, low priority).
- **F190 (new):** a key pressed while the O-ring moves (the unlock/lock before the pump starts) has no effect. The first `x` in K2 was echoed and ignored; the second `x` stopped the sample. The abort still works once the pump loop runs. Fix in FW5 (key read during the lock moves). Not blocking.

**Steps still open:** P1 (step 3), S1 (shield grams, step 4), S1-B + S1-C (new filter, slot 11, step 5), S1-D (three new filters, step 6), session log checks 1–10 (not run, port not given).

---

### Session 2026-10-23 (late): slot offset, F191

**Symptom (user):** `slot7` now moves the rotor to the physical place of slot 10. The offset is the same for all slots (about 3 slots, 67.5°).

**Code checked (no change):** `build_slot_angle_table` ([`src/Core/manifold_geometry.cpp`](../../src/Core/manifold_geometry.cpp)) puts slot 7 at raw 0, the calibrated reference `purge_angle`, and every other slot at reference − n × 22.5°. `slotN` ([`src/Tests.cpp:290`](../../src/Tests.cpp)) calls `rotateMotor(N)`, which uses that table. The homing check and the `oring_state` nearest-slot report use the same encoder and table. The math is consistent. A uniform offset cannot come from the table.

**Cause (hypothesis, strong):** the encoder reads a different angle than the rotor for the same position. The encoder coupling or the manifold moved relative to the encoder shaft by about 3 slots. The software trusts the encoder, so every slot moves together. Homing still reports PASS, because it compares the encoder with the same table, and `encoder_repeat` checks repeatability only (F165). It cannot see a shift of the whole table.
- Same fault as F187: `cal` on slot 7 gave encoder 3926 on 2026-10-22 and 3145 on 2026-10-23, a difference of about 69°, or 3 slots. The shift came from the loose encoder.
- `purge_angle` has changed five times in two days (D33–D37). Each change was a re-measurement of an encoder reading, not a code change.
- Timing: `oring_state` after K2 read 186.50° (nearest slot 11, error 0.08°), so the encoder agreed with the table then. The shift came after that, during the K3/E1/E2 runs or between sessions.

**Consequence:** the runs after K2 may have been at the wrong physical port. Their logic results (abort, skip, emptying `cond`) still stand, but the port identity is not verified. P1 is on hold: `slot12` would have sent the seal test to the wrong port.

**Fix (procedure, needs the bench):**
1. Check the encoder coupling: tight, and the manifold does not turn on the encoder shaft.
2. `cal` unlocks the O-ring itself. Jog the rotor by hand, with the keys `cal` describes, to the physical slot 7 mark. Stop. Copy the printed `purge_angle` into [`include/Manifold.h`](../../include/Manifold.h).
3. Reflash (user flashes).
4. Check: `slot7` lands on the slot 7 mark, then `rotate_all dry order=fill auto` (15/15 at ≤ 1.00°), then `actuator_home` (aligned).

**Decision D39 (user, 2026-10-23):** `purge_angle` = **276.77** from `cal` on slot 7, encoder **3149.00** (276.77 = 3149 × 360 / 4096). Replaces 276.42 (D37). Changed in [`include/Manifold.h`](../../include/Manifold.h). Build (`pio run -e due`) and `pio test -e native` 163/163 pass. **Not flashed:** the user reflashes.

**Caveat (user):** the encoder seemed loose again when this was read. The new value is only as good as the coupling at the moment of the reading. Check it after the flash: `encoder_repeat 7 10 auto` (spread ≤ 0.50°), then `slot7` onto the physical mark.

**Future fix (not for the prototype, user):** the encoder coupling loosens. This is the third time in two days (F187, F191). It must be fixed mechanically (locking the coupling, not just tightening) before the pilot. Deferred, see stage 1 §8.1.

**User decision (2026-10-23, after D39):** `encoder_repeat` and `rotate_all dry order=fill auto` are **not** rerun. The user judges the encoder drift does not affect the calibration this morning, and the problem is solved for now. Recorded as the user's call, not as a test result. The only post-flash check kept: `slot7` on the physical mark by eye.

**Status:** F191 open. **D39 does not fix the symptom.** 276.77 differs from 276.42 by 0.35°, but the symptom is 3 slots (67.5°). After the reflash (user, 2026-10-23), `slot7` still lands on physical slot 10. A re-measurement at the same spot cannot change the table by 67.5°. The fault is in the reference, not in the drift. See the next section.

---

### Session 2026-10-23 (late, after the reflash): D39 did not fix it

**Observed (user):** after the reflash with `purge_angle` = 276.77, `slot7` still sends the rotor to the physical slot 10. The offset is the same for all slots.

**Analysis:**
- Table: slot 7 = raw 0 = `purge_angle`. Slot 10 = raw 3 = `purge_angle` − 67.5°. So commanding slot 7 stops the encoder at `purge_angle`, and the rotor is on physical slot 10. Therefore physical slot 10 reads 276.77° on the encoder now.
- If the physical slots are spaced as the table says (22.5° in the same direction), physical slot 7 is 3 slots further on, at 276.77 + 67.5 = **344.27°**. Candidate: `purge_angle` = **344.27** (not measured, to verify).
- The earlier cal of 2026-10-22 gave 345.06 (encoder 3926). That is 0.8° from 344.27. A hint that the early reading was on the right slot and the later readings (3134–3149) were on slot 10. Not proven.
- Two explanations, both to check by eye: (a) the rotor was on physical slot 10 when `cal` was run and was taken for slot 7; (b) the encoder shifted by 3 slots. (a) fits the 3-slot size better. Both need a physical check, not a re-cal.
- My earlier claim ("re-measure with `cal` and it is solved") was wrong. The re-measurement at the same spot could not move the table by 67.5°.

**Next (needs the user):** decide on 344.27 (to try, then reflash and eye-check `slot7`) or measure physically first. Not changed in code yet.

**Decision D40 (user, 2026-10-23):** the user confirms the rotor is aligned with the table but 3 slots off: `slot7` sends it 3 slots to the left, onto the physical slot 10. `purge_angle` = **344.27** (276.77 + 67.5), [`include/Manifold.h`](../../include/Manifold.h). This replaces 276.77 (D39). Build `pio run -e due` and `pio test -e native` 163/163 pass. **Not flashed:** the user reflashes. After the reflash: `actuator_home` (UNKNOWN at boot), then `slot7` with the eye check on the slot 7 mark.

**Note:** D39 was a mistake in my analysis (a 0.35° change against a 67.5° offset). The 2026-10-22 value 345.06 is within 0.8° of 344.27.

**Result after the reflash (user, 2026-10-23):** the 3-slot offset is solved. `actuator_home` reads `slot 10 aligned (error -0.09 deg)`, which matches the physical rotor on slot 10 under the new table. `slot7` now lands on slot 7 (user).

### P1, pressure behaviour (slot 12, used clogged filter): PASS

`full_cycle 12 purge=n shield=n empty=n volume=1000`. `TEST|full_cycle|12|PASS|...;sampled_ml=38;sample_end=clogged;rule=clogged`.
- Pressure never above 3.6 bar. `p_max` 3.528 bar, `p_mean` 3.009 bar. Pass.
- `pauses=68`, `paused_s=29`. Pauses between 3.35 and 3.41 bar, resume at 3.09–3.15 bar. Pump power fell to about 10–25 %.
- Flow: 24 mL in the first 60 s pumping window, 10 mL in the second, 4 mL in the third. Stopped by the clog rule at 38 mL after 213 s of pumping (284 s total). The flowmeter is uncalibrated and reads low at low flow (F150), so the mL counts are rough.
- The filter was a used one (slot 12), so the low flow is expected. Not a flow test.

**Stopping criteria, checked against the log:**
- Clog rule: fired as designed (4 mL in the window against 24 mL first, 17 %, under 30 %).
- Stall rule (under 3 mL per 60 s): not reached.
- Timeout (30 min): not reached.
- Caveat: the clog rule compares with the first window. A filter that is slow from the start is not flagged until its flow falls further. Not blocking.

**Finding F192 (new, user observation):** the O-ring unlocks while the line is still at about 2.9 bar. The log shows `Pump P1 stopped` at 2.87 bar, then `Valve V1 opened`, `O-ring unlocked`, and only then the purge emptying. The rotation starts before the pressure has dropped, so water sprays. Fix: wait for the pressure to fall (for example under 0.3 bar, with a timeout of about 20 s) before the unlock. Possibly an air push first. Not blocking for the prototype, but it affects the samples. Decide with the user.

**Fix F194 (2026-10-23, user request, built, not flashed):** the clog rule had no absolute floor, so a filter that was slow from the start (24, 10, 4 mL per minute) stopped only at 213 s. Added an absolute floor per 60 s pumping window: `FILL_CLOG_MIN_WINDOW_ML` = 50 mL (PLACEHOLDER, [`include/Settings.h`](../../include/Settings.h)). A window below it stops the run as `clogged` at once, from the first window. The relative rule stays. Code: `src/Core/fill_stop.{h,cpp}` (`FillWatch::slow_ml`), [`src/Hardware/Filter_watch.cpp`](../../src/Hardware/Filter_watch.cpp) (set in `begin` and `arm`). Native tests: `test_slow_first_window_stops_at_once`, `test_healthy_window_passes_the_floor` (165/165 pass). Build passes. Expected on the next run of this kind: `clogged` after 60 s with about 24 mL. Check on a new filter that the floor does not fire (a new filter should count well above 50 mL in its first minute).

**Fix F192 (2026-10-23, user: "fix it", built, not flashed):** [`src/Step_functions.cpp`](../../src/Step_functions.cpp) `step_sampling()`: after the pump stops and before the valves change and the O-ring unlocks, `wait_line_released()` waits (pump off, valves unchanged) until `read_p_in()` is under `PRESSURE_RELEASE_BAR` = 0.3 bar. Timeout `PRESSURE_RELEASE_TIMEOUT_MS` = 20 s (both PLACEHOLDER, [`include/Settings.h`](../../include/Settings.h)). On timeout or an invalid sensor read: `ERROR | step_sampling: line not released ... O-ring stays LOCKED`, then the usual stop (pumps off, valves closed). The O-ring does not unlock under pressure.
- **Not tested on the bench.** Open risk: the wait keeps the valves in sampling position, so the line vents only through the filter. On a clogged filter it may not reach 0.3 bar in 20 s and the run will stop on the timeout (safe, but the run ends). If so, the fix is a venting step (V1 open, pump off), which is not built, because the venting behaviour is not known.
- **Not changed:** the other unlock paths (`load_slot`, `fill_test` end, the `Tests_mech` tests). Production sampling only: `full_cycle`, `full_all`, operation sampling.
- Native tests 165/165. Build passes.

**Finding F193 (new):** the sterivex air phase stops on a wet filter. On this filter the pressure settled at 0.76 bar with the flow stuck at 38 mL. The stop threshold is 0.8 bar (`EMPTY_WATER_PRESSURE_STX_THRESHOLD`), so `cond` fired at once, and the 20 s hold ended the phase. The threshold cannot tell a wet filter from a dry one here. Note for S1-D: check the filter by weight after emptying.

**Status:** F191 open, D39 superseded by the 344.27 candidate (pending the user's OK).

---

### Session 2026-10-23 (late): flowmeter check with checkpoints, R1

**Correction:** the wet-filter weighing (old R1) and the shield grams (old R1) were not needed. Removed. The purpose is to check the flowmeter against weight.

**Plan (user):** pass water through one new filter until it clogs. Pause every 200 mL (50 mL later, when the flow slows), weigh the water collected so far, compare with the flowmeter at each pause. The flowmeter is accurate when fast and drifts when slow.

**Code (built, not flashed):** `fill_test` option `checkpoint=N` (mL, 0 = off, default). At every N mL the pump stops, the console prints `CHECKPOINT n at X mL`, the operator types the grams + ENTER (`x` ends the run), the line `CHECKPOINT|n=;flow_ml=;weight_g=;diff_ml=` is printed, and the pump restarts. The stop rules do not run during the wait; the stall window restarts after it. No grams within 10 min (`FILL_CHECKPOINT_WAIT_MS`, placeholder) ends the run as `checkpoint_timeout`.
- Build passes, native tests 165/165.
- **Open point:** the jug weighs the water that leaves the filter. The flowmeter sits upstream (to be confirmed). The difference therefore includes water held in the filter and the line. Read the first checkpoint for that offset.

**Roadmap:** R1 is now the flowmeter check (`slot11`, `fill_test volume=3000 checkpoint=200 label=cal1`). R2 is three in a row without weighing. The shield grams step is removed (F156 stays a stage 2 item).

**Change (2026-10-23, user): one run, not two.** `fill_test checkpoint=N` now uses N as the coarse step (200 mL) and switches to the fine step `FILL_CHECKPOINT_FINE_ML` (5 mL) when an interval's flow falls under `FILL_CHECKPOINT_SLOW_FRACTION` (70 %) of the first interval's flow. A `SLOW |` line marks the switch. Each `CHECKPOINT|` line now also prints `rate_mlmin` and `step_ml`.
- The stop rules are kept through the checkpoints: the stall and clog windows are no longer re-armed at each pause (`FilterWatch::resume_at`), so the wait is not counted as pumping time and the windows carry on. The pressure rule does not run during the wait. The timeout counts pumping time only (`t_waited`).
- The clog floor (F194, 50 mL per 60 s window) still applies, so a run ends at about 50 mL/min. The slowest checkpoints are therefore above that rate.
- Build passes, native tests 165/165. Not flashed, not tested on the bench.

### Incident 2026-10-23: checkpoint restart, cap trip on slot 11 (cal1)

**Run:** `fill_test volume=3000 checkpoint=200 label=cal1` on slot 11 (new filter). Flowmeter 200 mL, weight 198 g at checkpoint 1 (`diff_ml=2`, rate 141 mL/min). The first checkpoint worked.

**What went wrong:** the pump restarted after checkpoint 1 (`pump.stop()` then `pump.start_at(15)`). The sensor read −0.052 bar on the first pass after the restart. Then the pressure climbed from 3.38 bar (PAUSE line) to 3.64 bar, and the cap (3.6 bar) stopped the pump: `SAFETY | pressure exceeded cap (3.64 bar)`. The run ended `cap` at 203 mL, before checkpoint 2 (400 mL). The filter is not clogged (203 mL through it).

**What the cap did:** it stopped the pump at 3.64 bar, as designed. No damage seen (user to confirm).

**Cause: not confirmed.** Candidates, from the code:
- `Pump::set_power_live(0)` is the pause. After `stop()` and `start_at()` the pause did not stop the rise. The pause works in the normal run (39 pauses). What is different after a restart is not known.
- The sensor glitch at restart (−0.052 bar). Relay switching noise is a candidate, not verified.
- My checkpoint wait does not run the cap check while blocked. It is safe because the pump is stopped during a wait, but it is not proven.

**Decision:** no more runs on slot 11 until the restart is understood. Next: a reproduction test on the used clogged filter in slot 12 (no new filter), `fill_test volume=300 checkpoint=50`.

### Flowmeter check result (2026-10-23, R1, manual weighing, slot 11)

Four runs of `fill_test` on slot 11, the same filter, weighed by hand after each run (the checkpoint pause did not work on the bench; manual sends used instead).

| Run | End | Flowmeter (mL) | Weight (g) | Flowmeter / weight | Mean flow (flowmeter) |
|---|---|---|---|---|---|
| 1 | cap (3.64 bar, restart after a checkpoint) | 203 | 198 | +2 % | ~140 mL/min (fast) |
| 2 | clogged | 180 | 264 | −32 % | ~50 mL/min |
| 3 | stalled (60 s) | 1 | 24 | flowmeter misses the flow | ~24 mL/min (by weight) |
| 4 | stalled (60 s) | 1 | 18 | flowmeter misses the flow | ~18 mL/min (by weight) |
| Total | | 385 | 504 | | |

**Findings (corrected 2026-10-23, user):**
- **Flowmeter:** accurate at fast flow (run 1, about 140 mL/min: +2 %). At about 50 mL/min it reads 32 % low (run 2). Below about 25 mL/min it reads almost nothing (runs 3–4). So the flowmeter volume is recorded and used only while the flow is fast. Slow-flow volume comes from the weight. The threshold between 50 and 140 mL/min is not measured yet. This is F150, now measured.
- **Stop rules:** the stall rule (3 mL per 60 s) and the clog floor (F194) read the flowmeter. At slow flow the flowmeter sees almost nothing, so they can stop a run that is still passing water (run 3: `stalled` after 60 s, 24 g passed). The rules are right for a clogged filter. What is wrong is their reading of slow flow, not the rules.
- **Filter:** a new filter was put in slot 11 for this test (user). It passed about 500 g in total, then about 18 mL/min at 3 bar. A 1 L target is not reached on this filter at 3.15 bar.
- Run 1 gave the cap trip after the restart (incident above). Runs 2–4 did not restart a checkpoint. Checkpoints were sent by hand.

**Decided later (user):** the volume criterion is the weighed volume the filter gives, not 1 L. The last test is the 14-slot run (roadmap R1).

### P1 (pre-check, vent on a clogged filter): FAIL, cap trip after 2 s (2026-10-23)

`slot12`, `full_cycle 12 purge=n shield=n empty=n volume=300` (user). Pump started at once at 15 % on a dead-headed line (P 0.01 bar, flow 0). `PAUSE at 3.35 bar (1)`, then the pressure went on to **3.69 bar** and `SAFETY | max pressure (3.60 bar) exceeded - stopping pump`. `end=cap`, `p_max` 3.694 bar, 2 mL.

- The cap did its job. The pre-check did not test the vent: the run never reached the vent step.
- The rise was about 0.35 bar after the pause command. On the same filter in P1 (2026-10-23, earlier) the rise to 3.35 bar was similar, but the overshoot was small. The difference is the dead-headed line with no flow at the start.
- The earlier restart incident shows the same pattern: 3.38 → 3.64 bar after a restart at 15 %. Common factor: a start at 15 % on a line with no flow.
- Not decided: the cause (start rate, or the pause at 0 % not stopping the pump), and the fix (gentler start, or pause by relay). F192 is still untested.

### Fix F196 (2026-10-23, user: "implement the solution"): gentle start and relay pause in the operation pump

**Changes (built, not flashed; behaviour change for every operation run, listed as the rule requires):**
1. `CtrlPump::run` ([`src/Hardware/Pump.cpp`](../../src/Hardware/Pump.cpp)): the pump starts from 0 % (`start_at(0)`) and the PID output is capped by `core::pump_start_limit(elapsed, PUMP_START_RATE_PERCENT_PER_S)`, so the output rises at 5 %/s (0 to 15 % in 3 s). Before: 15 % at once.
2. Pause: the relay is cut (`pump_->stop()`), not only the output set to 0 %. Before: `set_power_live(0)`, relay on. Suspected cause of the overshoot after the pause (P1: 3.35 → 3.69 bar).
3. Resume: restart from 0 % with the same ramp (`start_at(0)`, `ramp_t0_`). Before: resume at the backed-off power.
4. `STX_SAMPLE_MILLILITERS` = 1000 (was 100). The operation volume, as decided (P2).
- New pure rule `src/Core/pump_start.{h,cpp}` with [`test/test_pump_start/`](../../test/test_pump_start) (3 tests). Native 168/168 pass. Build passes.
- `fill_test` (the test command) is **not** changed; it still uses `set_power_live(0)` for its pauses.

**Open risk:** the PID integral can wind up during the ramp (its output is capped, the integral is not). The pause resets it (`pid_->reset()`), so the effect is limited to the first pressure rise. Check in the P1 re-run.

**Next:** reflash, then P1 again (`slot12`, `full_cycle 12 purge=n shield=n empty=n volume=300`). Pass: no cap trip; the pressure stays under 3.6 bar; `Line released` (F192) within 20 s, or a clean stop with the O-ring LOCKED.

### P1 re-run after F196 (2026-10-23, user): partial pass, vent FAIL

`slot12`, `full_cycle 12 purge=n shield=n empty=n volume=300` on the used clogged filter.
- **F196 works:** no cap trip. `p_max` 3.379 bar. The pump starts from 0 % and rises. Every pause now drops the pressure (3.36 → 2.98 bar within seconds): the relay cut is effective. 18 pauses, 11 s paused.
- **Clog stop works:** `STOP | filter clogged`, 4 mL, 72 s of pumping (the 60 s window after the start).
- **F192 vent FAILS:** `ERROR | step_sampling: line not released below 0.3 bar in 20 s - O-ring stays LOCKED`. The run stopped cleanly with the O-ring locked, so the guard held. But the line does not vent through a clogged filter in 20 s, and the operation run would stop at every clogged filter. Blocks R1.
- Cause (likely): with V1 closed and Vman open, the pressure upstream of the filter can only leave through the clogged filter. A vent path is needed (V1 air inlet, pump off). Not tested: the valve physics are not confirmed.
- Next: measure the line pressure and test V1 by hand (pump off, O-ring locked, no filter change).


---

## Handover 2026-10-23: stage 1 state for the next chat

**Read first:** [roadmap stage 1](../roadmap/stage1-FINISHED-prototype.md) §0–§1, then [field/operator-tutorial.md](../field/operator-tutorial.md). Everything below is the record of this session.

### Where things stand
- **Flashed on the bench:** the build of early 2026-10-23 (air hold 20 s, D38; purge reference 276.42, D37; `actuator_home` at any angle, D36).
- **Built, not flashed (R0 flashes them):** D40 (`purge_angle` 344.27), F194 (clog floor), F192 (vent wait, parked), F196 (gentle start and relay pause, operation pump), `STX_SAMPLE_MILLILITERS` 1000, `fill_test checkpoint=` (not used).
- **Build:** `pio run -e due` passes. `pio test -e native` 168/168. `pytest tools/` 101 (run in a scratch venv: PlatformIO's Python has no pytest).
- **Next:** R0 (reflash, `actuator_home`), T1 (calibration, tutorial Part A), T2 (operator test, tutorial Part B: new filters in slots 1–2, empty slots 3–12, one clogged filter in slot 13, `full_cycle <slot> purge=y shield=y empty=y volume=1000`, one command per sample).

### Findings (this chat)
- **F186** outlet out of square with the manifold axis (realigned by hand, 2026-10-22).
- **F187** manifold encoder came loose (2026-10-23). The `cal` values 3926, 3134 and 3145 were read while it slipped. Re-tightened; a mechanical lock is needed before the pilot (open, F191).
- **F188** air hold of 1 s too short to empty the line; set to 20 s (D38). E1 and E2 pass with it.
- **F189** leak at the outlet of the manifold, at the 3D-printed thread (not the O-ring seal). Not urgent; PTFE tape; stage 2 outlet part.
- **F190** keys typed during the O-ring move (unlock/lock before the pump starts) are ignored. The abort still works once the pump runs. FW5.
- **F191** the 3-slot offset: `slot7` sent the rotor to physical slot 10. The table matched the encoder; the reference was 3 slots off. Fixed by D40 (`purge_angle` 344.27), checked by eye on 2026-10-23. The loose coupling (F187) is the likely cause, not a software fault. D39 (276.77) was a 0.35° change against a 67.5° offset and did not fix it.
- **F192** the O-ring unlocked while the line was still at about 2.9 bar: water sprayed (user, 2026-10-23). A fix was built: wait for the line to vent to 0.3 bar, 20 s timeout; on timeout the O-ring stays locked (`wait_line_released`, [`src/Step_functions.cpp`](../../src/Step_functions.cpp)). **Tested:** on a clogged filter the line does not vent in 20 s, so the run stops at `not released` (safe). **Parked (user):** the fix is a venting step (V1 air inlet, pump off), not built, valve behaviour not confirmed.
- **F193** the 0.8 bar sterivex emptying threshold reads 0.76 bar on a wet filter, so the air phase stops at once. The 20 s hold then ends the phase. Check by weight. Open.
- **F194** the clog rule had no absolute floor, so a filter slow from the start stopped only at 213 s. Added `FILL_CLOG_MIN_WINDOW_ML` = 50 mL per 60 s window (placeholder), `src/Core/fill_stop.{h,cpp}`. The floor reads the flowmeter.
- **F195** not used.
- **F196** the operation pump overshot the cap on a dead-headed line: 15 % at once took the pressure from 0 to 3.35 bar in about 3 s, and the pause (output 0 %, relay on) did not stop the rise (3.35 → 3.69 bar, cap 3.6 bar, SAFETY). Fix built: start from 0 % at 5 %/s (`core::pump_start_limit`, `src/Core/pump_start.{h,cpp}`); pauses cut the relay; restarts from 0 %. **Tested once on the bench:** no cap trip, max 3.379 bar, every pause drops the pressure, clog stop at 72 s. Changes every operation run.
- **Flowmeter (F6, F150), measured 2026-10-23 (manual weighing, one new filter, slot 11):**
  - Fast flow (about 140 mL/min): 203 mL against 198 g (+2 %).
  - About 50 mL/min: 180 mL against 264 g (−32 %).
  - About 24 and 18 mL/min: 1 mL against 24 g and 18 g (the flowmeter reads almost nothing).
  - Use the flowmeter volume only at fast flow; use weight below. The stop rules that read the flowmeter (stall, clog floor) can stop a run that is still passing water. Accepted for the field: a clogged filter should stop, and the weight records what passed.
- **Filter capacity:** a new filter in slot 11 passed about 500 g at 3.15 bar, then about 18 mL/min. A 1 L target is not reached on this filter at this pressure. Decision (user): the volume criterion is the weighed volume the filter gives, flagged with its stop reason.

### Decisions (this chat)
- **D33–D37:** purge reference changes (see the table at the top of this file). Superseded by D40.
- **D38:** air hold 20 s.
- **D39:** `purge_angle` 276.77. Superseded: did not fix the offset.
- **D40:** `purge_angle` 344.27 (276.77 + 67.5°). User confirmed by eye after the reflash.
- **User decisions:** skip `encoder_repeat` and `rotate_all` (2026-10-23; the morning calibration was accepted). Field volume 1000 mL, or less when the clogging criteria stop the run. Do not flash or commit without asking. Shield grams not measured in stage 1. `calibrate_all` not run in stage 1.
- **Checkpoints (`fill_test checkpoint=N`):** built, not used. The first checkpoint worked (200 mL on the flowmeter, 198 g). The restart after it went above the cap (incident below). Manual weighing was used instead.

### Incidents
- **Checkpoint restart, 2026-10-23 (cal1):** after checkpoint 1 the pump restarted at 15 %. The pressure rose from 3.38 to 3.64 bar and the cap stopped the run at 203 mL. The cap did its job. Same mechanism as F196, which addresses it. Not run again.
- **P1 first run, 2026-10-23:** `slot12`, `full_cycle 12 … volume=300`: the cap tripped after 2 s (3.69 bar). This led to F196.

### Operation and the metadata (checked this session)
- **Operation sequence:** `sample_process` (purge, sample, shield, unlock; START button and the Pi line). `full_cycle` and `full_all` run the same step functions. The operator command in the tutorial is `full_cycle <slot> purge=y shield=y empty=y volume=1000`. The START path is not tested.
- **Volume gap found:** `sample_process` takes its volume from `STX_SAMPLE_MILLILITERS`, which was 100 mL. Set to 1000 mL (decided). Any operation path must read this value.
- **Metadata:** [`src/Core/sample_record.cpp`](../../src/Core/sample_record.cpp) prints one JSON line per purge or sample pump run. The session tool ([`tools/session_log.py`](../../tools/session_log.py)) writes the `.log` line by line, each sample as its own JSON file as soon as the run ends, and `samples.csv`. Verified offline (`test_sample_record`; the tool's 101 tests). **Not run on the bench:** the first real run is T2.
- **Gaps:** filter ID, site, operator and weights are not in the machine's record (paper sheet in stage 1). `sample.id` is a counter from boot (`auto-N`).
- **Earlier session-log failure:** it was started with the placeholder `COMx`, not the Due's port (the Due was on COM6). Not a tool fault, as far as known.

### Code (comments added where changed)
- `src/Core/fill_stop.{h,cpp}`, [`src/Hardware/Filter_watch.cpp`](../../src/Hardware/Filter_watch.cpp), [`include/Filter_watch.h`](../../include/Filter_watch.h): F194 floor and `resume_at`.
- [`src/Step_functions.cpp`](../../src/Step_functions.cpp): `wait_line_released` (F192).
- [`src/Hardware/Pump.cpp`](../../src/Hardware/Pump.cpp), [`include/Pump.h`](../../include/Pump.h): F196 (start, pause, resume, ramp).
- `src/Core/pump_start.{h,cpp}`, [`test/test_pump_start/`](../../test/test_pump_start): the ramp rule, 3 tests.
- [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp): `fill_test checkpoint=` (not used; comments say so).
- [`include/Settings.h`](../../include/Settings.h): `FILL_CLOG_MIN_WINDOW_ML`, `PRESSURE_RELEASE_*`, `PUMP_START_RATE_PERCENT_PER_S`, `FILL_CHECKPOINT_*`, `STX_SAMPLE_MILLILITERS` = 1000; PLACEHOLDER markers where they are placeholders.
- [`include/Manifold.h`](../../include/Manifold.h): `purge_angle` 344.27 with its history.

### Open items for the next chat
1. R0: reflash, `actuator_home`, `slot7` by eye (repeat after the reflash: the 2026-10-23 check was on the build before F196 and the 1000 mL change).
2. T1: calibration (tutorial Part A).
3. T2: the operator test (tutorial Part B), then the log returned.
4. F192 vent step (parked, stage 2).
5. F193 check by weight in T2.
6. Decide the operator command before the lake: `full_cycle` (as in the tutorial) or a `sample` command around `sample_process`.
7. Commit and merge into `cowas-v2` only when asked.


### Session 2026-10-23 (final): audit, field metadata commands, operator tutorial

**Audit against the requests of this session (what is done, what is not):**
- Calibration procedure with skip notes: **done**, [`docs/field/operator-tutorial.md`](../field/operator-tutorial.md) Part A.
- Operator tutorial (calibration, then the operation run, metadata, what to send back): **done**. Corrected: the file names are `<date>_<tag>.log` and `<date>_<tag>_samples/` (the earlier text had a folder name that does not exist).
- Verify the metadata code: **done offline**. `test_sample_record` (native, golden line) and the session tool's tests (101 passed, run in a scratch venv: PlatformIO's Python has no pytest). **Not on the bench**: the first real run is the operator test.
- Revise the roadmap: **done** ([`docs/roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md), §0–§6).
- Comment the code: **done for every change of this session** (fill_stop, Filter_watch, Step_functions, Pump, pump_start, Tests_mech, Settings, Manifold.h, Step_functions.h). Not reviewed line by line for old code.
- Document the log with the relevant information: **done** (handover above, this entry).
- **Not done, or changed without agreement:**
  - **Two clogged filters** were asked for; the plan has one, in slot 13, last. Reason: with the vent parked, a clogged filter stops `full_all` at `not released`, so a second clogged slot after it is never reached. Open decision (roadmap §4).
  - **Flowmeter recording rule** (record the volume only at fast flow, above an x mL threshold): **not built**. Only a warning in the tutorial and the roadmap. Open decision.
  - **START button / Pi path** not tested. `run_sample` runs the steps of `full_cycle`, not `sample_process`.
  - **Operator command name:** `run_sample`, not `sample`, because a typed `sample…` line goes to the legacy Pi protocol after `stop` and would start `sample_process` at the next free slot.

**Built (not flashed):**
- `session` ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp)): asks site, date (YYYY-MM-DD), experiment name, operator (optional). Prints `META|session;…`. Kept in RAM; a reset needs it again.
- `run_sample <slot> [volume=mL]` ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp)): refuses without a session. Asks sample name (required), condition (required), time HH:MM (required), notes (optional). Prints `META|sample;…` (slot, name, condition, time, notes, site, date, experiment). Then runs `run_full_cycle` (purge, sample, shield, unlock; default volume `STX_SAMPLE_MILLILITERS` = 1000). The TEST line `run_sample` carries the same fields.
- The sample name becomes the **`id` of the sample record** ([`src/Step_functions.cpp`](../../src/Step_functions.cpp), `set_sample_record_name`), so the JSON file name carries it. Purges keep the `auto-N` id.
- Each question waits up to 10 min (`FIELD_ANSWER_WAIT_MS`, PLACEHOLDER); `x` cancels; a required answer is asked again while empty.

**Build:** `pio run -e due` passes. `pio test -e native` 168/168. The only warning ([`maintenance.cpp`](../../src/Hardware/maintenance.cpp), unused variable) is not from this session.

**Your change to `actuator_home`:** reviewed in the working tree ([`src/Hardware/Oring_lock.cpp`](../../src/Hardware/Oring_lock.cpp)): it runs at any angle, moves only the O-ring, the moves can be stopped by `x`, and it refuses nothing for alignment (WARN only). The tutorial's A1 now says so (run it at every start). Builds with the rest.

**Open items for the next chat:**
1. Reflash the current build (F196, D40, 1000 mL, F194, F192 parked, `session` / `run_sample`).
2. Decide: one or two clogged filters in T2.
3. Decide: build the flowmeter recording rule before T2, or after.
4. T1 (calibration, tutorial Part A), then T2 (tutorial Part B with `session` and `run_sample`), log returned.
5. Commit and merge into `cowas-v2` only when asked.


### Session 2026-10-23 (last): test layout, flowmeter confidence rule, START path

**Test layout (user):** 2 new filters in slots 7 and 8; 2 clogged filters in slots 9 and 10; all other slots empty tubing. One `run_sample` per slot, in the order 7, 8, 9, 10, then tubing runs. Each `run_sample` is one sample, so the first clogged stop (`not released`, F192) does not block the next slot. (`full_all` would stop at the first failure, which is why the operator test uses `run_sample`.)

**START path (user asked which start is right):** dead code. `SIC_control()` (the panel sample: right button, START) is commented out in `loop()` ([`src/main.cpp:222`](../../src/main.cpp)), so the panel does not sample. The Pi line `sample` (after `stop`) runs `sample_process` at the next free slot: not for the operator. Old code left in place; no deletion without OK. The operator uses `run_sample`.

**Flowmeter confidence rule (user: build it; log two values):** `src/Core/trusted_volume.{h,cpp}`. The volume is counted in 5 s windows (`FLOW_TRUST_WINDOW_MS`); a window counts with its whole volume when its mean flow is at least `FLOW_TRUST_MIN_ML_PER_MIN` = 100 mL/min (**PLACEHOLDER**, between the measured 50 mL/min (−32 %) and 140 mL/min (+2 %)). Two values are logged:
- `vol_trusted_ml` (sample record) and `trusted_ml` (TEST line): the confident volume.
- `vol_ml` (sample record) and `sampled_ml` (TEST line): the plain flowmeter total, not reliable at slow flow.
Applied in `CtrlPump::run` ([`src/Hardware/Pump.cpp`](../../src/Hardware/Pump.cpp), reset at the start, update each pass, close at the end: `CtrlPump::trusted_ml()`), in `fill_test` ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp)), and in the sample record (`src/Core/sample_record.{h,cpp}`, new field after `vol_ml`; golden line updated in [`test/test_sample_record/`](../../test/test_sample_record)). The session tool's samples.csv has a `vol_trusted_ml` column.

**Sample name as record id:** `run_sample`'s name is the `id` of the sample record (JSON file name), set through `set_sample_record_name()` ([`src/Step_functions.cpp`](../../src/Step_functions.cpp)). Purges keep `auto-N`.

**Tests:** `pio run -e due` passes. `pio test -e native` 172/172 (new: `test_trusted_volume`, 4 tests). Tool tests `pytest tools/` 101 (scratch venv).

**Not done / open:**
- Not flashed. Not run on the bench. The 100 mL/min threshold is a placeholder: set it after T2 from the tube runs (`trusted_ml` against the weight).
- The flowmeter update happens in the pump loop; a sample that is paused uses the windows as they fall (a pause in a window lowers its mean flow, so the confident part can be lower than a steady fast run).
- The clog rules still read the plain flowmeter total (F194 floor, stall rule): unchanged (accepted for the field, see the earlier entry).
- The operator test is T2 (`session`, then `run_sample 7`, `8`, `9`, `10`, then tubing). Log the `META` lines and the two volumes, return the folder.

**Review (2026-10-23, end of session):** stale section references in code comments (`§12`, which no longer exists in the roadmap) replaced in [`src/Core/sample_record.h`](../../src/Core/sample_record.h), [`src/Step_functions.cpp`](../../src/Step_functions.cpp), [`include/Settings.h`](../../include/Settings.h), [`tools/session_log.py`](../../tools/session_log.py). Build passes. No behaviour change.


### Session 2026-10-23 (clean-up): unused code archived, threshold set, roadmap cut to the operator test

**Unused code removed (archived, nothing deleted):** `docs/archive/code-2026-10-23/` with a README (what, why, evidence, how to restore) and copies of the changed files as they were before the removals (`before-removal/`).
- Whole files moved to `removed/`: `src/Misc/experiences.cpp`, `include/experiences.h` (no caller); `src/System/Sample.cpp`, `include/Sample.h` (nothing includes the header); `src/Connectivity/Serial_device.cpp`, `include/Serial_device.h` (no object, `extern serial` never defined or used).
- Inside files: `SIC_control()` (panel sampling; only a commented call in `loop()`), its prototype and its include; the `Serial_device` includes and `extern` lines in four files.
- **Checkpoint feature of `fill_test`** (user: remove): `read_checkpoint_grams()`, the `checkpoint=` argument and help text, the checkpoint state and block in the run loop, `t_waited`, `FilterWatch::resume_at()`, `FILL_CHECKPOINT_FINE_ML`, `FILL_CHECKPOINT_SLOW_FRACTION`, `FILL_CHECKPOINT_WAIT_MS`.
- Kept, checked and still used: `run_pump()`, `demo_sample_process()` (test menu `demo`), `step_dive()` (kept for the spool procedures; its only callers are a commented line now), `Test_openloop`, [`Settings.cpp`](../../src/System/Settings.cpp).
- Build `pio run -e due` passes. `pio test -e native` 172/172. No references left to the removed symbols (grep).
- **Caught and fixed during the removal:** the first check missed the `Serial_device` includes (a `head` cut the search). The build failed, the files were restored, and the removal was redone with a full search. Recorded so the next person checks with a full search.

**Flowmeter threshold set by me:** `FLOW_TRUST_MIN_ML_PER_MIN` = **110 mL/min** (was the placeholder 100). Reason: linear interpolation between the two measurements (−32 % at 50 mL/min, +2 % at 140 mL/min) gives −10 % at about 110 mL/min. Two points only; to check in T2 against the weight of the tube runs.

**Roadmap restructured:**
- [`docs/roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) now holds only the operator test: R0 (flash, `actuator_home`, `slot7`), T1 (calibration, tutorial §2), T2 (operator test, tutorial §3–4, the layout 7–8 new, 9–10 clogged, rest tubing, one `run_sample` each), R2 (commit, on request). Plus the values used in the test and the limits for it.
- `docs/roadmap/roadmap2.md` (new) holds the deferred work: A (before field use: vent step F192, stop rules at slow flow F150/F194, flowmeter calibration F6/F26, encoder lock F187/F191, relief valve and E-stop, seal at 3 bar F144, keys F190, recovery FW5/FW6), B (sample quality: shield F156/F159, record fields, wet-filter threshold F193, outlet leak F189, purge F30/F130), C (code clean-up), D (stage 2 and 3).
- [`CLAUDE.md`](../../CLAUDE.md), [`docs/README.md`](../README.md) and [`docs/software/code-guide.md`](../software/code-guide.md) updated to the new sections and the removals.

**Still open:**
- Flash the current build (R0), then T1 and T2.
- Check `trusted_ml` against the weight in T2 (threshold 110 mL/min).
- Decide on the remaining legacy code in roadmap 2, section C, with the helper.

**Roadmap change (2026-10-23, user):** `docs/roadmap/roadmap2.md` removed. Its items moved into [`docs/roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md): new §7b (owner per item) and a new work package HW9 (manifold encoder coupling). Stage 1 ([`stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md)) keeps only the operator test. Earlier entries that cite roadmap 2 refer to the file as it was that day.

**Code guide §9 (shield, 2026-10-23):** how `step_DNA_shield` works, how to calibrate the dose (`shield_calibrate`, LA3 Part A), how to check it on filters, how to prime. Found while reading: the production step still uses the 25 s shortcut, so the calibrated `DNA_SHIELD_FILL_MS` is not used yet (FW4 item 7); `DNA_shield_test()` has no callers (candidate for removal, not removed). No code change.

**Clean-up (2026-10-23, user):** `DNA_shield_test()` deleted (no callers; declaration removed). Build passes, native tests pass.

**Docs (2026-10-23, user):** the code guide rewritten ([`docs/software/code-guide.md`](../software/code-guide.md)): layout, operator path, main objects, the DNA shield, open points. Instructions removed; facts kept. The lab folder restructured ([`docs/lab/`](../lab)): [`README.md`](../../README.md) index; `lab-interface.md` (questions and decisions, IDs kept, the one-page summary moved to §1, the metadata table replaced by what is recorded today); `sop-filters.md` (the printed SOP, trimmed, with the console commands that exist; host commands `cowas …` removed); `la3-protocol.md` (shield protocol, trimmed; stale statements corrected: the pressure cap works, `EMPTY_STX_MIN_MS` does not exist, `full_cycle shield=n` exists). File names kept, so the references in code and tools still resolve. The earlier text of these files was not archived (the user does not use the archive); it is not in git yet either.

**Docs reorganisation (2026-10-23, user):**
- Renamed: `docs/lab/la3-protocol.md` → [`docs/lab/shield-protocol.md`](../lab/shield-protocol.md); `docs/lab/sop-filters.md` → [`docs/lab/filter-sop.md`](../lab/filter-sop.md). All references updated: code comments ([`Settings.h`](../../include/Settings.h), [`Tests.cpp`](../../src/Tests.cpp), [`shield_calibration.h`](../../src/Core/shield_calibration.h)), [`tools/analyze_la3.py`](../../tools/analyze_la3.py), [`tools/la3_thresholds.json`](../../tools/la3_thresholds.json), the stage 2 roadmap, the prompts, the code guide, the commands and protocol references.
- Lab meeting dropped (user). `docs/lab/lab-interface.md` removed from [`docs/lab/`](../lab); a copy is in `docs/archive/lab-2026-10-23/`. Its one-page summary and its data fields moved to [`docs/lab/README.md`](../lab/README.md). Its decisions are now in [`docs/lab/shield-protocol.md`](../lab/shield-protocol.md) §8, which lists each value with its current value and status. The `⟨LAB X⟩` markers were removed from the SOP. [`tools/la3_thresholds.json`](../../tools/la3_thresholds.json) stays `null` until the team decides.
- [`docs/README.md`](../README.md) rewritten as an introduction: what CoWaS is, a reading order, then the documents by topic. New root [`README.md`](../../README.md) (short, with the quick start).
- Stage 2 roadmap: LA1 is marked dropped; its lab-meeting references point to shield-protocol §8.
- Tests audited: 17 native test folders. All exercise code the firmware runs. One test function removed (archived in `docs/archive/tests-2026-10-23/test_pump_ramp_settings_window.cpp.txt`): it checked a constant against an old request, and tests no behaviour. Build passes, native tests 171/171, tool tests 101/101.
- Links: every relative link in [`README.md`](../../README.md), [`CLAUDE.md`](../../CLAUDE.md) and [`docs/`](..) (excluding the archive and LOG history) resolves.
- Not changed: historical LOG entries that name the old file names, and the archive. They describe the files as they were on their day.

**Audit of the requests of 2026-10-23 (user: "make sure everything is properly addressed"):**
- **Bug fixed: `abp_raw` was never dispatched.** The command was documented (help, the operator tutorial §2, the pressure messages), but `print_abp_raw()` had no caller. Added to the console ([`src/Tests.cpp`](../../src/Tests.cpp)). Found by checking each command of the tutorial against the dispatchers.
- **Shadowed branch removed:** `multi_sample` in [`Tests.cpp`](../../src/Tests.cpp) could never run (the stage 1 handler in [`Tests_mech.cpp`](../../src/Tests_mech.cpp) runs first). The branch and its function `test_multi_sample_loading()` are removed (archived). `multi_sample_full` is unchanged.
- **Dead code, second pass:** 13 functions with no caller removed (list in `docs/archive/code-2026-10-23/README.md` §6, text in `removed/second-pass/`). `fill_DNA_shield_tube()` removed (only a commented call). Kept, with a reason: `args_positional_count`, `slot_count`, `tube_volume_ml` (used by native tests; the last is the shield-protocol formula).
- **Stage 1 section references fixed:** the stage 2 roadmap cited `stage 1 §3` for values, now §2.
- **Commands reference** ([`docs/software/commands.md`](../software/commands.md)): the `multi_sample` row and the list of unreached functions updated.
- **Links and anchors:** every relative link and `#anchor` in [`README.md`](../../README.md), [`CLAUDE.md`](../../CLAUDE.md) and [`docs/`](..) (excluding the archive and LOG history) resolves.
- **Build** `pio run -e due` passes. **Native tests** 171/171. **Tool tests** 101/101.
- **Not verified on the bench:** the session and `run_sample` commands, the metadata files, the confident-volume rule (110 mL/min is a set value to check), the pump start and pause changes, and the vent wait (F192, expected to stop at `not released` on a clogged filter).

**Docs structure (2026-10-23, user):** folders now read README → roadmap → hardware → software → lab → field → archive → logs. Renamed: `docs/reference/` → [`docs/software/`](../software); `docs/LOG.md` → [`docs/logs/LOG.md`](LOG.md); `docs/test-logs/` → [`docs/logs/test-logs/`](test-logs). Paths updated in CLAUDE.md, the docs, the code comments, the host docstrings, and the session log tool's default folder ([`tools/session_log.py`](../../tools/session_log.py)). The links and anchors of [`README.md`](../../README.md), [`CLAUDE.md`](../../CLAUDE.md) and [`docs/`](..) resolve. Build, native tests (171) and tool tests (101) pass.

**Photos removed (user):** the photo steps of the filter SOP are gone. Stage 2 §7b has an optional item for photos, to do later if useful. [`docs/hardware/power.md`](../hardware/power.md) keeps one mention (a label photo for the audit), which is not a request for a file.

**Condensed (2026-10-23):** [`docs/lab/filter-sop.md`](../lab/filter-sop.md) (141 lines) and [`docs/lab/shield-protocol.md`](../lab/shield-protocol.md) (146 lines), from about 330 and 280; same steps and values, shorter wording, console commands that exist.

**Audit against the last commit (2026-10-23):** the functions missing from the last commit were checked one by one. All intended, except `test_2_remplissage_container_1m/40m`, removed by the range deletion of the second pass with no caller in the last commit; their text is archived. Functions removed before this chat (the A2 pressure code, `power_ramp`) are not part of this list.

**Not condensed in this pass:** [`docs/roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md) (556 lines), `docs/roadmap/stage2-prompts.md` (828), `docs/roadmap/stage3-prompts.md` (263), [`docs/software/protocol.md`](../software/protocol.md) (522, the host contract), [`docs/logs/LOG.md`](LOG.md) (1600, the history).

**Archive of removed code, restructured (2026-10-23, user: keep what may be useful, sort it):** `docs/archive/code-2026-10-23/` is now [`docs/archive/code/`](../archive/code), with an index (`code/README.md`) that says for each item where it came from, when and why it was removed, and whether it is reusable or superseded.
- `reusable/`: `DNA_shield_test`, `fill_DNA_shield_tube`, the checkpoint weighing (with its README), the spool procedures (`test_1`, `test_2`, `test_3`), `Sample` (sample bookkeeping), `Serial_device`, `experiences`, and the encoder, timer and clock utilities.
- `superseded/`: the A2 pressure sensor (`read_pressure2`, `calibrate_pressure2_zero`), the power-ramp controller (`power_ramp`, its test), the pressure-ramp soft limit (`ramp_soft_limit`, `test_soft_limit`, recovered from the last commit), the panel sampling (`SIC_control`), the old `multi_sample` test, and one constant-check test.
- `snapshots/2026-10-23-before-removal/`: five whole files as they were before the clean-up.
- Recovered from the last commit (`HEAD`), because they were not archived when removed: `DNA_shield_test`, `read_pressure2`, `calibrate_pressure2_zero`, the power-ramp files, `ramp_soft_limit`, `SIC_control`, the checkpoint loop block. Every function missing from the last commit is now in the archive, except the test functions in [`test/`](../../test), which still exist.
- [`docs/archive/README.md`](../archive/README.md) lists the archive; the old `tests-2026-10-23/` moved to `code/superseded/tests/`. Links inside the archived documents were repointed to the new layout.
- Build, native tests (171) and tool tests (101) pass.

**Session handover (2026-10-23):** `docs/logs/handover.md` is the entry point for the next session: the state, what is verified, what is built but not run on the bench, the risks, a short bench check, the open decisions, and where things are.

### Bench session 1 (2026-10-23, user): R0 checks, `session`, first `run_sample`

Source: a `pio device monitor` transcript on COM6 (not the session tool, see F200). Build: the one flashed for this session, which includes the purge reference 344.27 and the operation pump start.

**Results**

| Check (stage 1 §1, handover §5) | Result | Pass? |
|---|---|---|
| R0-2 `actuator_home` | slot 12 aligned, err −0.62°, state UNLOCKED | PASS |
| R0-3 `slot7` | Rotated to slot 7, O-ring locked. The mark was not checked by eye in the transcript | **Open:** the user checks the mark |
| Handover 5.3 `pressure` | 0.021 bar with the line open | PASS (about 0) |
| Handover 5.4 `abp_raw` | 20 frames, all `valid`, counts 1601–1607 (spread 6, about 0.002 bar), 23 °C | PASS (criterion ±20 counts) |
| Handover 5.5 `fill_test volume=300 label=smoke` | **Skipped by the user** (relevant only for clogging). Not run, no weight | Skipped |
| Handover 5.6 `session` | Site, date, experiment, operator asked. `META\|session;…` printed | PASS |
| Handover 5.6 `run_sample` | **FAIL**, see F197. The purge ran (100 mL, 16.6 s, max 0.60 bar, cap 3.6 bar, no SAFETY). Then the sample was refused (`rotateMotor: no slot -1`). Safe stop: pumps off, valves closed, O-ring LOCKED | FAIL |

**The purge in that run (for the record):** `vol_ml` 101, `vol_trusted_ml` 95, `p_mean_bar` 0.305, `p_max_bar` 0.600, `end_reason` `volume`, `purge_ml` 101 (target 100). The flowmeter at 0.1–0.6 bar read 12 → 95 mL in about 2 s. The flowmeter read 0 → 95 mL over the pressure rise from 0.01 to 0.59 bar. Placeholders on the record: `flow_uncal`, `ABP_FULL_SCALE_PSI`, `STX_MAX_PRESSURE`.

**Findings (new, open)**

- **F197 (firmware, open): `run_sample` without a slot runs with slot −1.** The user typed `run_sample` with no slot. `cmd_run_sample` ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp), `long slot = -1`) reads an absent slot as −1 and does not refuse it. The purge then ran (pumping, under the cap), and only the sample step refused. The operator's sample name, condition and time were asked, and the META line says `slot=-1`. Safe (the cap held and the rotation refused), but the run pumped water for nothing. **Proposed fix:** refuse before any question or pump if the slot is missing (`run_sample <slot>` is required). Behaviour change, not made yet (user decision, see the handover).
- **F198 (firmware, open): the TEST line of `run_sample` loses the metadata.** `run_full_cycle()` starts with `*kv = "slot=" …` ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp) near line 1354), which overwrites the `name=`, `condition=`, `time=`, `site=` and `experiment=` that `cmd_run_sample` put in `kv`. The TEST line in this run has none of them. They are still on the `META|sample;…` line. **Proposed fix:** append to `kv` instead of overwriting it.
- **F199 (firmware, open): the sample JSON carries only the name, and only for a sample.** The sample name becomes the record `id` (`set_sample_record_name`, [`src/Step_functions.cpp`](../../src/Step_functions.cpp)). The purge keeps `auto-N` by design, so this run's purge JSON is `"id":"auto-1"`. The JSON has no site, date, experiment, condition or time: those are not in the record fields. Check after F197 is fixed: is the sample JSON enough for the record, or are the fields needed too?
- **F200 (process, open): the metadata logging was not tested with the tool.** The transcript comes from `pio device monitor`. [`tools/session_log.py`](../../tools/session_log.py) was not used: `docs/logs/test-logs/2026-10-23_s1-1c.log` has no `META|` line and the `_samples` folder is empty. So the question "did the metadata logging work" is answered only from the console. The META lines were printed correctly. The `.log`, the CSV and the samples folder were not checked. **Next:** run the tool (`python tools/session_log.py --port COM6 --tag s1-1c`) for the operator test.

**Answers to the user's questions (2026-10-23)**
- *The `run_sample` mistake:* F197. The slot was not typed. The run was safe, but it pumped the purge first.
- *Did the metadata logging work?* Partly. The `session` and `run_sample` META lines are correct on the console. The TEST line lost the fields (F198). The JSON has the name only on a sample record (F199). The `.log` file was not written by this run (F200).

**Not done in this entry:** the code for F197 and F198 is unchanged. Both are behaviour changes and need the user's decision (rule 1 and the stage 1 "no new tuning" rule). The handover and the stage 1 status are updated to match.

### Session 2026-10-23 (after bench session 1): vent, threshold, safety check, inconsistencies

**Decisions (user, 2026-10-23)**
- **Vent step built (F192):** pump off, Vman closed, V1 open (air in), wait up to 20 s for under 0.3 bar, then the O-ring may unlock. Same valve state the emptying already uses before it unlocks. Not bench-tested: on the bench, watch that the line vents with V1 open (the sensor is on the pump side).
- **Confident-volume threshold = 50 mL/min** (`FLOW_TRUST_MIN_ML_PER_MIN`, was 110 by interpolation). Not checked against the tube-run weights (the user chose 50). **Note:** at about 50 mL/min the flowmeter read 32 % low (LOG, flowmeter check 2026-10-23), so the lower windows of this rule under-count.
- **F6 and F26 closed (user):** the flowmeter is calibrated (factor 77) and wired at 3.3 V. The 3.3 V wiring is the user's statement; the HW3 peak check was not done.
- **F193 dropped (user):** no weighing of wet filters.
- **F144 and the relief valve / E-stop accepted (user):** the software cap at 3.6 bar is the protection for now. Not raised again.
- **Shield calibration is for the helper (user):** `DNA_SHIELD_FILL_MS` is not wired into production. Production keeps the 25 s test value.
- **Framing:** the work is the user's alone; the helper helps later. The lab values are "to set with the helper" (shield-protocol §8).

**Changes in code**
- [`src/Step_functions.cpp`](../../src/Step_functions.cpp): `wait_line_released()` now sets the vent valves and stops the pump before it waits (F192).
- [`include/Settings.h`](../../include/Settings.h): `FLOW_TRUST_MIN_ML_PER_MIN` = 50.0. Comments of `FILL_STALL_MIN_ML` updated (flowmeter calibrated).
- [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp): comment on the flowmeter updated.
- **Safety fix (F201):** three loops that pumped without a cap check now check it on every pass: the panel Pump mode ([`src/main.cpp`](../../src/main.cpp)), `step_fill_container()` ([`src/Step_functions.cpp`](../../src/Step_functions.cpp)), and `calibrate_flow_sensor()` ([`src/Tests.cpp`](../../src/Tests.cpp)). Each stops the pump at the cap, as `enforce_pressure_safety()` does everywhere else. Same behaviour otherwise.

**Safety check (2026-10-23)**
| Invariant (CLAUDE.md rule 4) | Result |
|---|---|
| Pump never runs above the cap | Enforced in `start()`, `set_power()`, `set_power_live()` and the operation loops. Not polled before: the three paths fixed above (F201). **Open:** `purge_pipes_manifold` (Pi `purgePipesFunction`) and `calibrate_DNA_pump` (`cal_DNA`) (F202). Not used by the operator |
| Implausible P_in stops the pump | `enforce_pressure_safety()`: below −0.5 bar or the −99 fault value stops the pump. Checked |
| Sensorless test mode unreachable | `pump_open_loop_test()` refuses while `PRESSURE_SENSOR_AVAILABLE` is true. Checked |
| No rotation while locked or unknown | Every rotation goes through `rotateMotor()` or an `OringScope(UNLOCKED)` guard (refactored from `may_rotate()`; `OringScope` refuses UNKNOWN). Checked by reading the call sites |
| Pump start needs the O-ring locked | `Pump::start()` calls `oring_go_to(LOCKED)`, refuses if that fails. Checked |
| Every procedure has a timeout | Sample 30 min, fill 1800 s, emptying 60 s, vent 20 s, flow calibration 5 min, sensor waits. Interactive questions (`ask_ynx`, `wait_for_keypress`) wait for a person, `x` aborts; no timeout (human-paced, not changed) |
| STOP and abort accepted | `x` latches an abort checked at every step boundary (`aborted_stop`). Checked |
| Safe state after a stop | `stop_procedure()`: pumps off, valves closed, O-ring unchanged (locked after a sample). Checked |
| Settings with safety meaning | `STX_MAX_PRESSURE` 3.6 (user), pause 3.35, resume 3.15, `PRESSURE_RELEASE_*`. Matches the roadmap |
| Unexpected edits from the last session | `git diff` of the safety tokens compared with `HEAD`: no guard removed, only moved (`may_rotate` to `OringScope`). Checked |

**Known safety gaps still open:** F202 (above); F197 (`run_sample` with no slot pumps the purge before it refuses the sample; the cap holds); the `flow_uncal` flag is still printed on every record although the flowmeter is now calibrated (changing it changes the record format: user decision).

**Tests (2026-10-23)**
- `pio run -e due`: passes (after each change).
- `pio test -e native`: **171 test cases, all pass.** The "172" in the clean-up entry above was wrong; 171 is the count.
- [`tools/`](../../tools) (pytest): 101 pass. Run with the PlatformIO Python and pytest installed outside the repo.
- No new unit tests (the user asked for no more tests now). The vent and the cap polls are checked on the bench only.

**Inconsistencies fixed (2026-10-23)**
- Native test count: 171 everywhere (handover, stage 1, LOG).
- Archive path: [`docs/archive/code/`](../archive/code) in stage 2 §7b (the old `code-2026-10-23/` no longer exists).
- Flowmeter wording: "uncalibrated" removed from CLAUDE.md, stage 1, stage 2 §4 and §7b, the code guide and the code comments. The one left is the record flag `flow_uncal` (see above).
- "Team" framing removed from the lab folder, the handover, the root README and the host README (laptop wording instead).

**Open after this entry (stage 1):** `slot7` mark by eye; F197 and F198 fixes (waiting for the user's yes); `run_sample 7` rerun; vent test on a clogged slot in T2; metadata check with [`tools/session_log.py`](../../tools/session_log.py) (F200).

### Session 2026-10-23 (third): vent redesigned, record fields, flag removed, role wording removed

**Decisions (user, 2026-10-23)**
- **Vent (F192), redesigned:** the goal is to drop the pressure first. After the sample the pump is off and V1 and Vman are shut for 20 s, so the line drops by itself. Then the air vent V1 is open for 5 s. The sample is **never** stopped for the pressure. The O-ring unlock goes on whatever the readings. Only a faulty sensor read or an x / stop stops the step. **Risk (recorded):** the O-ring can unlock while the line is above 0.3 bar. Earlier the manifold sprayed water at about 2.9 bar (F192). Watch the `Vent:` line in T2.
- **F199 (fix):** the sample JSON carries `site`, `date` and `condition` (for the purge and the sample of a `run_sample`).
- **Flag `flow_uncal` removed** from the records. The flowmeter is calibrated (user).
- **F202:** left to the helper (fix or remove `purge_pipes_manifold` and `calibrate_DNA_pump`).
- **Role wording removed** from the roadmaps, the prompts, the code comments and the tool text. The history in this log is left as it was written.

**Changes in code**
- [`src/Step_functions.cpp`](../../src/Step_functions.cpp): `vent_line()` replaces `wait_line_released()`: pump off, V1 and Vman closed for `PRESSURE_DECAY_WAIT_MS` (20 s), V1 open for `PRESSURE_AIR_VENT_MS` (5 s), readings logged (`before`, `after the wait`, `after the air vent`, `released` or `NOT released`). The wait polls the console keys (`poll_run_key()`), so `x` or `stop` ends it. The call site no longer stops the sample for the pressure. A faulty sensor read stops it, and the O-ring stays locked.
- [`include/Settings.h`](../../include/Settings.h): `PRESSURE_RELEASE_TIMEOUT_MS` removed; `PRESSURE_DECAY_WAIT_MS` = 20000 and `PRESSURE_AIR_VENT_MS` = 5000 added (placeholders). `PRESSURE_RELEASE_BAR` stays, as the reading that the log reports as released.
- **F199:** [`src/Core/sample_record.h`](../../src/Core/sample_record.h) and `.cpp`: fields `site`, `date`, `condition` (NULL = not written). [`src/Step_functions.cpp`](../../src/Step_functions.cpp): `set_sample_record_meta()` (declared in [`include/Step_functions.h`](../../include/Step_functions.h)) sets them for a run, and `print_sample_record()` passes them on. [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp) (`cmd_run_sample`) sets them before the run and clears them after.
- **Flag removed:** `flow_uncal` is gone from `print_sample_record()`. The flags list now holds the two placeholders.

**Tests**
- [`test/test_sample_record/test_main.cpp`](../../test/test_sample_record/test_main.cpp): the golden line uses the new flag list; new test `test_record_writes_site_date_condition`. Native: **172 test cases, all pass** (171 before this test).
- [`tools/tests/test_session_log.py`](../../tools/tests/test_session_log.py): fixture flags updated. `pytest tools/`: 101 pass.
- `pio run -e due`: passes.
- No bench test yet of the vent or of the new fields.

**Inconsistencies fixed**
- The native count is 172 in the handover and the stage 1 docs (171 was the count before the new record test).
- "Team" wording removed from [`docs/roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md), `docs/roadmap/stage2-prompts.md`, [`include/Settings.h`](../../include/Settings.h) (comment), [`tools/analyze_la3.py`](../../tools/analyze_la3.py) (text), the lab folder and the root README. "Team" roles are now "Quentin, the helper".

**Docs updated for the vent and the record:** stage 1 §0, §2, §3, the T2 pass line; stage 2 §7b (vent row, F202 owner the helper); the code guide (vent row); the operator tutorial (`not released` row replaced by the `Vent: … NOT released` line); the lab README (end reasons); the session log doc (site, date, condition, flags).

**Open (for the user):**
- F197 (`run_sample` with no slot pumps the purge first) and F198 (the TEST line of `run_sample` drops the metadata). Both proposed, not made. What they are: F197 is a missing check on the slot argument; F198 is `run_full_cycle()` overwriting the fields passed in ([`src/Tests_mech.cpp`](../../src/Tests_mech.cpp), near line 1354).
- F202, owner the helper.
- The vent's unlock-at-pressure risk (above).

### Session 2026-10-23 (fourth): F197, F198, first `run_sample` on the old build, the session tool

**Bench run (user, `run_sample 7`, old build):** PASS. Purge 101 mL (trusted 95 mL, max 0.605 bar). Sample 1001 mL (trusted 992 mL, max 0.998 bar, end `volume`, 0 pauses). Shield 25 s (micro pump, timed) and 4.8 s air push at 13 %. Emptying: the water was pushed through the purge and the filter, and the sample ended LOCKED. The vent in this run was the old one (`Line released: -0.02 bar`), so the record flags and the vent are from the old build.

Two things in that run show the old build: the record still prints `flow_uncal`, and the vent line is the old text. The current build is not flashed.

**Decisions (user, 2026-10-23)**
- **F197 (fix):** `run_sample` with no slot takes the slot the manifold is on, read from the encoder (`manifold_nearest_slot()`). If the manifold is not on a sample slot (slot 0 or off the slot by more than the tolerance), it refuses before anything pumps.
- **F198 (fix):** the TEST line of `run_sample` keeps the name, condition, time, notes, site and experiment. `run_full_cycle()` appends the caller's fields instead of overwriting them.
- **F202 split:** `calibrate_DNA_pump` (`cal_DNA`) to the helper. `purge_pipes_manifold` (Pi `purgePipesFunction` only; no console command) is not used by the operator; proposed removal, pending Quentin's decision.
- **Later (after all stage 1 tests):** realign the manifold. It is misaligned again and leaks in the top slots (stage 1 §4).

**Changes in code (not flashed)**
- [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp): `cmd_run_sample` (F197: slot from the encoder when none is typed; F198: `kv` without the duplicate slot); `run_full_cycle` (F198: keeps the caller's fields); usage text shows `[slot]`.
- The build compiles: `pio run -e due` passes. Native tests 172 pass. Tool tests 101 pass.

**The metadata question (user: "was the metadata file created?").** No. The log you pasted came from `pio device monitor`, which only shows the lines. The files are written by [`tools/session_log.py`](../../tools/session_log.py), which opens COM6 itself. COM6 must be free, so close the monitor first. A dry run of the tool on the lines of this log (no serial port) wrote:
- the `.log` with both `META|` lines (`session` and `sample`), and every line of the run;
- the CSV row for the `TEST` line;
- one sample JSON per record (the purge and the sample), and `samples.csv`.

The dry run used the old build's lines, so the JSON still shows `flow_uncal` and has no site, date or condition. The current build adds those. Command to run, from the repository folder, with the monitor closed:

```
C:\Users\Quentin\.platformio\penv\Scripts\python.exe tools\session_log.py --port COM6 --tag s1-1c
```

pyserial is present in that Python (checked). The files go to `docs\logs\test-logs\`.

**Flag `flow_uncal` (earlier decision, removed in code):** still printed by the old build. Gone in the current code.

**Docs updated:** stage 1 (R0, T2, the limits, the later realignment note §4), stage 2 §7b (F202 split by owner), the handover (rewritten).

**Open:** flash the current build; run [`session_log.py`](../../tools/session_log.py) on COM6 for the operator test; check the sample JSON (site, date, condition, no `flow_uncal`); the vent on a clogged slot; `purge_pipes_manifold` removal (your decision); calibrate_DNA_pump (the helper); the manifold realignment after the stage 1 tests.

### Session 2026-10-23 (fifth): purgePipesFunction archived

**Decision (user):** `purgePipesFunction` goes to the archive.

**Changes:**
- Archived to [`docs/archive/code/reusable/manifold-flush/`](../archive/code/reusable/manifold-flush): `purge_pipes_manifold.cpp.txt` (the routine and its `run_pump()` helper, the only caller) and `pi-purgePipesFunction.py.txt` (the Pi button and function). Indexed in [`docs/archive/code/README.md`](../archive/code/README.md).
- Removed from [`src/Hardware/maintenance.cpp`](../../src/Hardware/maintenance.cpp) (routine, `run_pump`), [`include/maintenance.h`](../../include/maintenance.h) (prototype, `MAINT_FILL_TIME`, `MAINT_PURGE_TIME`), [`src/main.cpp`](../../src/main.cpp) (command branch and commented call), [`computer_interface/cowas_interface.py`](../../computer_interface/cowas_interface.py) (method, menu entry), [`computer_interface/cowas_loop.py`](../../computer_interface/cowas_loop.py) (switcher entry, function).
- Docs: [`commands.md`](../software/commands.md) (row removed, text fixed), [`code-guide.md`](../software/code-guide.md) (legacy list), [`Settings.h`](../../include/Settings.h) (comment), stage 2 §7b (row removed), handover.

**Build:** the link succeeded and the compiler reports no errors. `pio run -e due` then failed on the last step, writing `firmware.bin`: another process held the file (the open `pio device monitor` or the session tool). Close it and rebuild. Native tests: 172 pass.

**Still open:** `calibrate_DNA_pump` (`cal_DNA`) is the helper's (F202). The flash of the current build and the bench runs are in the handover.

### Session 2026-10-23 (sixth): first `run_sample` on the current build, verified; time, air drying, date check, tutorial

**Verification of the bench files (user, `run_sample 11`, current build)** — `docs/logs/test-logs/2026-10-23_s1-1c.{log,csv}` and `_samples/` checked line by line:
- All three files exist. The `.log` has the `META|` lines and the vent lines. The CSV has the TEST rows. `samples.csv` and two JSON files (purge `auto-1`, sample `cloged filter1`).
- The records carry `site`, `date`, `condition` (F199 works). `flow_uncal` is gone (works). The TEST line carries `name`, `condition`, `time`, `site`, `experiment` (F198 works).
- Vent: `1.68 bar` before, `0.35 bar` after the 20 s wait, `0.27 bar` after the 5 s air vent: `released`.
- Clogged sample: 17 pauses, stopped `clogged` at 4 mL, O-ring LOCKED, safe.

**Errors in the records (not corrected in the files):**
- **Date typed wrong:** `2026-1°-23` (a `°` for a `0`) in the `session` META line, in the purge and the sample JSON, and in `samples.csv`. The firmware accepted it. **Fixed in code:** `session` now asks again until the date is `YYYY-MM-DD` (digits and dashes).
- **Typos in the answers:** sample name `cloged filter1`, site `boisChamblard` (lower case; the first session had `BoisChamblard`). The tool cannot tell a typo from a name. The paper sheet is the only other record of these names, so check them there.
- The TEST line prints `trusted_ml= 0` with a space (the number is written with a leading space). Harmless, not changed.

**Changes (current build, to flash):**
- **Time question removed:** `run_sample` asks name, condition, notes only. The start and finish times come from the PC (below).
- **Sterivex air drying doubled:** `EMPTY_STX_AIR_HOLD_MS` = 40 s (was 20 s, the shared `EMPTY_AIR_HOLD_MS`). It is set for the sterivex emptying only; the purge emptying keeps 20 s. The sterivex phase may run `EMPTY_STX_EXTRA_MAX_MS` (20 s) longer than `EMPTY_MAX_TIME_MS`, so the longer hold can finish.
- **Date check** at `session` (see above).
- **Session tool ([`tools/session_log.py`](../../tools/session_log.py)):**
  - Start time = PC time of the `=== run_sample ===` line; finish time = PC time of the `TEST|run_sample|<slot>` line. Both are written into every record of the run (`run_start_pc`, `run_finish_pc`), in the JSON files and in `samples.csv`. The CSV has new columns `site`, `date`, `condition`, `run_start_pc`, `run_finish_pc`.
  - The default log folder is found from the tool's own location (the repository), so the tool works from any working folder. `--dir` still changes it.
  - Tests: 105 pass (4 new: run times into the records, no run times outside a run, the device lines stamp the run, the summary TEST line does not end the run).
- **Tutorial rewritten** ([`docs/field/operator-tutorial.md`](../field/operator-tutorial.md)): paths relative to the project folder; the command is `%USERPROFILE%\.platformio\penv\Scripts\python.exe tools\session_log.py ...`; the sample flow is `run_sample <slot>` + 3 answers, nothing else; `:quit` at the end of the day.

**Simple flow (answer to "I just send run_sample x and :quit at the end?"):** yes. Once per start: `actuator_home` (`state=UNLOCKED`) and `session`. Per sample: `run_sample <slot>` and the three answers. End of day: `:quit`. After any reset or new flash, `actuator_home` and `session` again.

**Builds and tests:** `pio run -e due` passes (current code). `pio test -e native`: 172 pass. `pytest tools`: 105 pass.

**Open:**
- The date in the 2026-10-23 records is wrong for the runs done before the fix (see above). Decide: keep the files as they are and note the date on the paper sheet, or re-run. Not edited.
- Flash the current build (the date check, the time removal, the air drying) — the `run_sample` of the last run was on the build with the time question.
- The bench check of the new air drying and the start/finish times (next `run_sample`).

### Session 2026-10-23 (seventh): handover removed, stage 1 status rewritten

- **`docs/logs/handover.md` removed** (user: no longer relevant). The entries above that name it are history and are left as written. [`docs/README.md`](../README.md) now points to the stage 1 roadmap and the log.
- **Stage 1 status ([`docs/roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) §0 and §1):** rewritten as **done** and **remaining**. Stage 1 is **not finished**: T2 (the operator test with the four filters and the tubing runs) has not started. The remaining items are: flash the latest changes, one bench check of them, the `slot7` mark by eye, the decision on the date of the 2026-10-23 records, T2, and returning the log. R2 (commit) only on request.
- **Bench state:** the vent and the records work on the bench (`run_sample 7` full sample, `run_sample 11` clog with the vent released). The latest changes (time question removed, 40 s sterivex air drying, date check, start and finish stamps, purge routine archived) are compiled and tested, not flashed.

### Session 2026-10-23 (eighth): stage 1 remaining list cut to T2

- **Done (user):** flash of the latest build, the bench check of the latest changes (`run_sample`, start and finish stamps), the `slot7` mark by eye.
- **Removed from stage 1 (user):** the decision on the date of the 2026-10-23 records. The records keep their date as written.
- **Moved to stage 2 §7b (user):** the `NOT released` vent case (the line stays above 0.3 bar after the wait and the air vent, and the O-ring unlocks anyway). Not seen on the bench yet. Deferred on purpose.
- **Stage 1 status:** not finished. The goal is T2 running end to end with the log returned. Remaining: T2 (slots 7, 8, 9, 10, then the tubing runs) and returning the log. The `trusted_ml` comparison with weighed tubing runs stays optional.

### Session 2026-10-23 (ninth): stage 1 roadmap made concise

- [`docs/roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md) rewritten shorter: status table, done list, T2 steps, values, known limits, later realignment. Same facts as before; the detailed evidence stays in this log.

### Session 2026-10-23 (tenth): manifold alignment added to stage 1

- Stage 1 now has step A, the manifold alignment, after T2 (user, earlier: after the tests). It is part of the stage 1 done condition. Steps: realign the frame and the manifold by hand; `cal` on slot 7 and paste the new `purge_angle`; check with `slot7` and one `run_sample 7`; log the value and encoder reading.
- Stage 1 `purge_angle` in the values table: 344.27 (D40), unchanged.

### Summary of the session 2026-10-23 (for the next session)

The day's changes, in one place. The detail and the evidence are in the entries above.

**Stage 1 state.** R0 and T1 done. Next: **A, the manifold alignment** (before T2: realign by hand, `cal` on slot 7, new `purge_angle` in [`Manifold.h`](../../include/Manifold.h), flash, check with `slot7` and one `run_sample 7`). Then **T2**, the operator test: `run_sample 7`, `8`, `9`, `10`, then the tubing runs. R2 (commit) only on request. There is no step B.

**Bench results (current build, flashed):** `actuator_home`, `pressure`, `abp_raw`, `session` PASS. `run_sample 7`: full sample, 1001 mL, trusted 992 mL. `run_sample 11`: clogged at 4 mL, vent released (1.68 → 0.27 bar). Log, CSV and sample files written and checked.

**Code changes (2026-10-23):**
- Vent (F192): pump off and valves shut 20 s, then air vent V1 open 5 s. Never stops a sample; only a faulty sensor read or an x / stop stops it.
- Flowmeter confidence threshold 50 mL/min (user). At about 50 mL/min the flowmeter read 32 % low (F150).
- Cap polled on every pass in the panel Pump mode, `step_fill_container()` and `calibrate_flow_sensor()` (F201).
- `run_sample`: no slot = the slot the manifold is on (F197); the TEST line keeps name, condition, site, experiment (F198).
- Sample records: site, date, condition (F199). `flow_uncal` removed.
- Sterivex air drying 40 s (purge keeps 20 s). Time question removed from `run_sample`.
- `session` checks the date is YYYY-MM-DD.
- `purgePipesFunction` / `purge_pipes_manifold` archived ([`docs/archive/code/reusable/manifold-flush/`](../archive/code/reusable/manifold-flush)), with its Pi button.

**Session tool ([`tools/session_log.py`](../../tools/session_log.py)):** start time (the `=== run_sample ===` line) and finish time (its TEST line) written into every record of the run, in JSON and `samples.csv`. Default log folder found from the repository, so it works from any folder. 105 tool tests pass.

**Tests and builds:** `pio run -e due` passes. `pio test -e native`: 172 pass. `pytest tools`: 105 pass.

**Docs changed:** stage 1 roadmap (short: status, done, steps, values, limits); stage 2 §7b (vent `NOT released` case deferred; F202 split: `cal_DNA` to the helper, purge removed); operator tutorial (rewritten, paths relative to the project folder, `run_sample <slot>` then three answers, `:quit` at the end); session tool guide; code guide; commands; CLAUDE.md (flowmeter, role wording); lab folder (no role framing, the helper sets the lab values); archive index; docs index. `docs/logs/handover.md` removed on the user's request.

**Decisions (user, 2026-10-23):** F6 and F26 closed (calibrated flowmeter, 3.3 V wiring, not measured). F193 dropped. F144 and the lack of relief valve and E-stop accepted, not to be raised again. The vent may unlock the O-ring at pressure. The record flag `flow_uncal` removed. The helper owns the DNA pump calibration and the shield calibration. Stage 1 is for the operator test; calibrations are not stage 1.

**Known risks still open:**
- The vent can unlock at pressure; the `NOT released` case is not seen on the bench (stage 2 §7b).
- At slow flow the flowmeter reads almost nothing (F150); the lower windows of `trusted_ml` under-count.
- `calibrate_DNA_pump` runs the pump with no cap poll in its loop (F202, the helper).
- The manifold is misaligned again and leaks in the top slots (step A).
- Typos are not caught in names and site; only the date is checked.

**Records to know about:** the 2026-10-23 files from the first run_sample sessions carry the date `2026-1°-23` (a typo at `session`, before the date check). They are kept as written; the paper sheet is the other record. Sample name `cloged filter1` and site `boisChamblard` are also as typed.

**Next session starts with:** step A (alignment), then T2. Flash after the `purge_angle` change. Log every bench value.

### Session 2026-10-23 (eleventh): the helper's tasks and a backup inlet in stage 1

- Stage 1 lists the helper's tasks (not blocking T2): the DNA shield volume calibration, the sterivex air drying time (`EMPTY_STX_AIR_HOLD_MS`), and the `cal_DNA` cap check (F202). Details in [`docs/for-felix.md`](../for-felix.md).
- Stage 1 lists a spare manifold inlet print. The print file is not in the repository, so its path must be added to stage 1 once found.

### Session 2026-10-23 (twelfth): documentation check before closing

- Links: all relative links in [`README.md`](../../README.md), [`CLAUDE.md`](../../CLAUDE.md) and [`docs/`](..) resolve, except one in [`host/README.md`](../../host/README.md) (pointed to [`docs/software/protocol.md`](../software/protocol.md) from the wrong folder). Fixed.
- Wording: no role words, "handover" or old session paths left outside the LOG history and the archive.
- Constants in [`Settings.h`](../../include/Settings.h) and [`Manifold.h`](../../include/Manifold.h) match the stage 1 values table (3.6 bar, 20 s and 5 s vent, 40 s sterivex air drying, 50 mL/min, `purge_angle` 344.27).
- [`docs/software/commands.md`](../software/commands.md): the two operator commands (`session`, `run_sample`) were missing from the reference. Added.
- Native tests 172 pass. Tool tests 105 pass. `pio run -e due` passes.
- Stage 1 roadmap and the the helper note checked against each other: the tasks match.

### Session 2026-10-23 (thirteenth): flowmeter is calibrated (user), wording fixed everywhere

- User: "the flowmeter IS calibrated" (factor 77). The earlier "uncalibrated" wording was still in the code and docs. Changed:
  - Console messages in [`src/Tests_mech.cpp`](../../src/Tests_mech.cpp) (`fill_test`, and the comparison line after a pump run): no longer say UNCALIBRATED. The comparison line now says to compare the water with the volume, and the flowmeter line points to `trusted_ml` for the fast-flow part (F150).
  - Comment in [`include/Test_openloop.h`](../../include/Test_openloop.h).
  - [`docs/roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md) (FW2 row), [`docs/software/commands.md`](../software/commands.md) (`fill_test` row), `docs/roadmap/stage2-prompts.md` (hardware line).
- Still true, and recorded: the flowmeter reads almost nothing at slow flow and about 32 % low at about 50 mL/min (F150). Calibration does not change that.
- Build passes. Native tests: 172 pass.

### Session 2026-10-23 (fourteenth to nineteenth): summary of the afternoon

Condensed on 2026-10-23 at the user's request. The findings and decisions are in the tables above (F187–F206, D35–D49). This entry lists what was done, in order.

**Manifold that does not turn (F203, open).** On 2026-10-23 the manifold sometimes did not turn. Closing other PC programs helped at times. Moving the cable connector made it turn. Two failed `slot11` attempts (19:40, 19:44) showed `rotateMotor ... timed out` followed by a Due reset. Likely a bad contact in the cable or connector. Not confirmed. Next time: note the console line, `oring_state` and the encoder reading.

**Prefield test (run 1, 19:56–20:16, 100 mL build, old spool answer).** Slot 11 (new filter): clogged at 382 mL sampled, 323 trusted. Slot 12 (clogged): 3 mL, vent not released (run went on). Slot 10 (empty tube): 1001 mL. Step times: 8 min 10 s, 5 min 32 s, 5 min 20 s. The user finished the test; no rerun.

**Changes (built; the user flashes):**
- Purge (D44): 500 mL spool off, 7 L spool on. The spool answer is asked at `session` (y/n). This replaced the factor-5 purge of D41 (150 mL / 12.85 L), which the user did not accept.
- Purge timeout (D46): 3 min spool off, 13 min spool on (estimate from slot 10: 1 L in 97 s, so 7 L ≈ 11 min).
- Clog floor (D43): 50 → 30 mL per 60 s window. Slot 11's 4th window was 36 mL.
- Step times (D48): purge, sample (with its emptying), shield and unlock timed; `STEP TIMES` line and `t_*_s` on the `TEST` line.
- Test folders (D45): `docs/logs/test-logs/<date>/<NN>_<tag>/` with `session.log`, `tests.csv`, `samples/`. Today's two runs moved into it.
- Rebuild (D49): `tools/session_log.py --rebuild <run folder>` rebuilds `samples/` from the transcript. If the samples are already there, it writes into the next numbered folder. A check at `:quit` writes `SAMPLES CHECK` if a sample file is missing.
- Clean-up (earlier): empty files and non-latest BoisChamblard logs removed; their facts are in this entry.
- Archive (D47): the flat runs of 2026-10-09 to 2026-10-19 are in [`docs/archive/test-logs-2026-10/`](../archive/test-logs-2026-10).

**Lost sample files (F206).** The six JSON files of the prefield run were missing from the samples folder, after the tool had closed. The tool does not delete files. Rebuilt from the transcript on 2026-10-23 (3 purges, 3 samples).

**Checks:** 172 native tests pass; `pio run -e due` passes; 109 tool tests pass.

**Open:** flash the build; bench check of the 13 min spool purge and of the `STEP TIMES` line; R2 commit when asked.

### Session 2026-10-23 (twentieth): documentation pass and file renames

**Why.** The user asked for the working documents to be short and clear for a person, and for every link to be right. The chat ended with a rename and a request to record the whole chat.

**Chat, in order (what was asked → what was done):**
1. **Stage 1 status and what is left** → stage 1 rewritten as status, "Left to do", and field limits. Done steps are in this log. Final list: the helper cleans the tubing; the helper's DNA shield dose; commit; field.
2. **Operator tutorial, session log start, print path** → print path removed. The start command uses `tools\session_log.cmd`. The tab-corrupted paths were fixed.
3. **F203 (manifold does not turn)** → logged in the findings table. Kept in stage 2 §7b and in the stage 1 limits. Not archived.
4. **Prefield test (slots 11, 12, 10)** → run 1 done (user). Results in this log, the 19:56 session.
5. **Purge and spool** → spool question at `session` (y/n). Purge 500 mL without the spool, 7 L with it (D44). Timeout 13 min with the spool (D46). The factor-5 rule (D41) was superseded.
6. **Clog floor** → 50 → 30 mL per 60 s window (D43).
7. **the helper: shield and air timing, tubing** → [`docs/for-felix.md`](../for-felix.md), written with names, not line numbers.
8. **Later decisions and items** → "forgot F203" added to stage 1. Step times for every step (D48). Rebuild numbers a new folder instead of refusing (D49). Older runs archived (D47). Test logs in one numbered folder per run (D45). Sample files rebuilt from the transcript (F206, closed).
9. **Concise documents** → test-logs README, operator tutorial, [`commands.md`](../software/commands.md) (165 → 108 lines), [`code-guide.md`](../software/code-guide.md), [`stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md), [`stage2-pilots.md`](../roadmap/stage2-pilots.md) (559 → ~390 lines), [`stage3-full-system.md`](../roadmap/stage3-full-system.md). The old detailed stage 2 text is in [`docs/archive/roadmap-stage2-details-2026-10-23.md`](../archive/roadmap-stage2-details-2026-10-23.md). The v1 step-ID map is in [`docs/archive/README.md`](../archive/README.md).
10. **Links** → every relative link in [`docs/`](..) and [`CLAUDE.md`](../../CLAUDE.md) checked: none broken. CLAUDE.md pointers to removed stage 1 sections fixed.
11. **Rename** → `stage1-prototype.md` is now [`docs/roadmap/stage1-FINISHED-prototype.md`](../roadmap/stage1-FINISHED-prototype.md). All references updated.

**Checks at the end:** 172 native tests pass, `pio run -e due` passes, 109 tool tests pass, no broken links.

**Still open (as of the end of this chat):**
1. The user flashes the build: purge 500 / 7 L, spool question, 13 min spool timeout, clog floor 30 mL, step times, numbered folders, rebuild and check.
2. Bench checks: the 13 min spool purge and the `STEP TIMES` line. Not run yet.
3. the helper: clean the tubing; DNA shield dose (`shield_calibrate`, then the production time).
4. F203 (manifold does not turn): next time, log the console line, `oring_state` and the encoder reading. Check the cable connector first.
5. Commit the stage 1 work (the user decides when).
6. Stage 2 week targets date from 2026-09-24: re-plan after the Pilot A debrief.
7. `stage2-prompts.md` (827 lines) and `stage3-prompts.md` (263 lines) are not condensed. Some prompts name files that do not exist: `docs/DEPLOY_PI.md`, `tools/labels.py`, `docs/PUMP_RESULTS.md`.
8. To confirm: the clog floor of 30 mL was chosen by the code (the user asked for "a bit more tolerant"); the 13 min spool timeout was confirmed by the user, and the 15 min estimate came from the assistant.

**Update (user, after the chat):** the week targets and calendar were dropped from stage 2 (milestones, step table, critical path and parts). The duration of the pilots (1–2 days, 1–2 weeks) stays. The open item "re-plan the week targets" is closed.

### Session 2026-10-23 (twenty-first): stage 2 and 3 prompts folded into the roadmaps

**Why.** The user does not copy the prewritten prompts, but the ideas in them are worth keeping. Stage 2 and stage 3 are now the only plans, written as short step cards.

**Done:**
- Backups of both prompt files: [`docs/archive/roadmap-stage2-prompts-2026-10-23.md`](../archive/roadmap-stage2-prompts-2026-10-23.md) and [`docs/archive/roadmap-stage3-prompts-2026-10-23.md`](../archive/roadmap-stage3-prompts-2026-10-23.md) (text unchanged, two links inside fixed to point to the roadmap).
- [`docs/roadmap/stage2-pilots.md`](../roadmap/stage2-pilots.md) (544 lines, was 392 + 828 of prompts): the same section numbers, so CLAUDE.md and the pinout file still point to the right places (§3, §4, §6, §7a, §7b, §11.2). Each step is a card: the goal, the work, and a "done when" with numbers. The good ideas are kept: the protocol's `RUN_CYCLE` and request-id dedupe, the test-output convention, the pure rules for rotation and cap, the bricks shared by tests and runs (FW4a), the supervisor and watchdog, the modes and recovery, the mission policies, the alert levels, the SOP and the shield and carry-over protocols, the VA1 campaign.
- [`docs/roadmap/stage3-full-system.md`](../roadmap/stage3-full-system.md) (155 lines): the P2 steps as short cards, with the lessons from stage 1 (MiniPID gains, the pump's low-power band, the 10 Hz trace), and the "prepared for, not built" table.
- The prompt files were removed from [`docs/roadmap/`](../roadmap). CLAUDE.md, [`docs/README.md`](../README.md) and the archive index point to the new cards and backups.
- Outdated values in the old prompts were not carried over: the 2.5 bar cap (the cap is 3.6 bar since 2026-10-19), and the 100 mL purge (now 500 mL / 7 L). The file names the prompts cited that do not exist (`docs/DEPLOY_PI.md`, `docs/DEAD_CODE_AUDIT.md`, `docs/DIAGNOSTICS.md`, `docs/BUGS_FOUND.md`, `docs/PUMP_RESULTS.md`, `tools/labels.py`) are not referenced any more.

**Checks:** link check passes (0 broken).

**Open:**
1. Stage 2 and stage 3 cards are a plan, not a schedule. Decide the order after stage 1's field use.
2. Open questions kept in the cards: the production run pause (FW4a), the lock deadlock after a reset (FW6, F71), the PRIME_SHIELD destination and duration (LA3).

**Review pass on the roadmaps (after session twenty-one):** stage 2 and stage 3 read end to end for structure and wording. Changes: the hardware checks were written twice (§7.4 and §9); §9 removed, and the sections renumbered (§9 future work, §10 old step IDs, §11 parts, numbers kept where CLAUDE.md and the pinout file cite them). VA1 tests numbered so "tests 1–8" is unambiguous. F5 status no longer contradicts FW2. The stop-rule and deadband sentences made exact. Stage 3's stage-1 leftovers are marked "deferred from stage 1". Link check: 0 broken.

---

### Session 2026-10-23 (log reorganisation): split, dates, links, names

**Scope.** No bench work and no firmware change in this session. Only the log files in `docs/logs/`, and the links that point to them, were changed.

**What was done**
- The log was split into three files: this chronological log (`LOG.md`), [findings.md](findings.md) and [decisions.md](decisions.md). The findings table and the decisions table moved out of this file. The bench-check rows of the old decisions table moved into the 2026-10-23 part of this file, as a proper table.
- Findings have a **Recorded** column: the date and a link to the first entry in this log that mentions the finding. Decisions have a date, and an **Entry** link to the entry where they are recorded. Dates are the entry dates, not always the day of discovery.
- Two decisions that were defined in the text but had no row, D39 and D40, now have rows in `decisions.md`.
- F189–F202 were defined only in the text. They now have rows in `findings.md`.
- File references in backticks are now links to the files where the file exists (163 links). Three links that were lost in an earlier condensing were restored.
- Names were replaced by roles ("the helper", "the user"). The word "team" was removed. The branch name in the timeline is written `<user>-sealingMotor-stepperMotor`.
- Checks: every identifier (F, D, PH, S1-), date and number from the earlier log is still present. Every link and anchor in the three files resolves.

**Changes to the earlier versions in this session (for the record)**
- An earlier condensed rewrite of the log, made in this session, was replaced by the restored version. It had dropped about 350 backticked references and three links. The condensed text is not kept in the repository.

**Open items**
1. About 74 references still point to files that were renamed or removed (for example `docs/LOG.md`, `docs/logs/handover.md`, `docs/roadmap/roadmap2.md`, `docs/roadmap/stage2-prompts.md`, `docs/test-logs/`). They are left as written, because they describe history. A note next to each would make them clearer.
2. `docs/for-felix.md` still has a person's name in its file name. Renaming it means updating links in about ten documents.
3. Names and "team" wording remain in other documents outside `docs/logs/` (lab, roadmaps, code guide, `docs/README.md`, a few code comments). They were not changed in this session.
4. `docs/logs/` is not yet in git. Commit only when asked.
5. The log is long (about 206 KB). Shortening is possible section by section, after review.

**Registers (2026-10-23, later):** [findings.md](findings.md) and [decisions.md](decisions.md) are now the authority for status and for decisions. The "open" list at the top of `findings.md` is generated from the table, not written by hand. Log entries record events and link to the register rows.

---

### Session 2026-10-23 (registers): status in the registers, generated open list

**Changes (docs only, no code or bench work):**
- [findings.md](findings.md) and [decisions.md](decisions.md) are the authority for status and decisions. Each has a note saying so. Log entries record events and link to the register rows.
- The "open" list at the top of `findings.md` is generated from the findings table, not written by hand. It lists the open findings with severity Blocking or High (15 of 53 open).
- The log header has a line saying the same.

**Checks:** all links in `LOG.md`, `findings.md` and `decisions.md` resolve.

**Open items:** unchanged from the entry above (stale references to renamed files, `docs/for-felix.md`, wording in other documents, the log is not yet in git).

---

### Session 2026-10-23 (stage1-field): branch and checks for the field-deployment push

**Why.** The S1-1 pressure work, the S1-1b pressure ramp (Part 2 reverted), the run-key, filter-watch, fill-stop, pump-start, sample-record and trusted-volume modules, and the docs reorganization had sat uncommitted on `s1-1-pressure-abp`. They are now on one reviewable branch, `stage1-field`.

**Done:**
- Committed in four commits, by area: docs reorganization (`5a5cce3`), host protocol and simulator (`6178611`), tools (`67d45bc`), firmware (`b8fdbd3`). Pushed to origin under the old name `CoWas-Stage1-Proto-FieldDeployment`.
- Local branch renamed to `stage1-field`. The remote rename (push under the new name, delete the old one) is not done yet.

**Checks (on `b8fdbd3`):** `pio run -e due` passes. `pio test -e native`: 172 of 172 pass. Host `pytest`: pass. `ruff check` and `ruff format --check`: pass. `pytest tools/`: 109 pass. The host venv was created in `host/.venv` (git-ignored).

**Open:**
1. Merge `shield_V2` into `stage1-field` before the pull request. The merge conflicts in 15 files (`include/Manifold.h`, `Pump.h`, `Settings.h`, `Tests.h`, `src/main.cpp`, `src/Tests.cpp`, `src/Step_functions.cpp`, and others). Which side wins in each file is a decision not yet taken.
2. Push the rename and delete the old remote branch name.
3. Open the pull request into `shield_V2`.
4. No bench run was done in this session. The hardware checklist for this step is not run.

---

### Session 2026-10-26: purge 3 L with the spool, and full power for the purge

**Why.** The spool-on purge was 7 L (D44). At about 10 mL/s that is about 11 min of purge, most of it dead volume. The user's view: 3 L is enough with the spool. Also, the purge should run at 100 % power (the one exception to the 60 % water limit, D21), while the 3 bar target and the cap stay. The user's note that the pressure never rises in the purge is not what the log says: see F207.

**Changes (built, not flashed; not committed):**
1. **Purge volume with the spool: 3 L** (`PURGE_SPOOL_ML` = 3000, `include/Settings.h`). Dead volume is 2.57 L (30 mL intake + 2.54 L spool). Without the spool it stays 500 mL. Decision D50. Replaces the 3 × dead volume rule for stage 1: stage 2 §3.7 rule 1 is updated to say so.
2. **Full-power exception for the purge** (D51). `PUMP_PURGE_MAX_POWER_PERCENT` = 100 (`Settings.h`). The limit moved into a pure function `core::pump_power_limit_percent()` (`src/Core/pump_power.{h,cpp}`): full power wins, then air, then water. `CtrlPump` got `set_full_power()` (`include/Pump.h`), used in the output clamp and the pause resume clamp (`src/Hardware/Pump.cpp`). `step_purge()` is the only caller that sets it. Pressure target (`OPERATION_TARGET_BAR` = 3.0) and cap (`STX_MAX_PRESSURE`) are unchanged.
3. **Spool timeout kept at 13 min** (D46). At 3 L it is only a safety; the volume stops the run. The comment now says so. Not changed; lower it if you want it shorter.
4. **Docs:** [stage2 §3.7 rule 1](../roadmap/stage2-pilots.md) and the §7b table row, [code-guide.md](../software/code-guide.md), [decisions.md](decisions.md) (D44 marked superseded in part; D50, D51), [findings.md](findings.md) (F207).

**Behavior changes (for the PR):**
- The spool-on purge is 3 L, not 7 L (purge time about 5 min at 60 %, less at 100 %).
- The purge runs at 100 % power. All other water runs stay at 60 %.
- The purge timeout is still 13 min with the spool (unchanged).

**Checks (this session):**
- `pio test -e native`: 176 of 176 pass (the new `test_pump_power` has 4 cases; `test_run_keys` now uses 3000).
- `pio run -e due`: SUCCESS. RAM 8.7 %, flash 30.9 %.
- Not run: host tests, the bench. Nothing was flashed.

**Not decided / open:**
1. **F207, the bench check (before the field):** run `full_cycle` or the purge with the spool on, with the pressure log (`VERBOSE_PURGE_PRESSURE`) and the manifold watched for leaks. Record `pressure_max` and the purge time. Pass if there is no leak at the seal and the cap is not reached. If the pressure rises a lot, reconsider the 100 %.
2. The 2.54 L spool volume is still geometry (F130). Weigh one spool-on purge to confirm the 3 L covers it.
3. Blank is not a purge: nothing was changed for it.
4. The branch: the work stays on `stage1-field` (the user's decision, 2026-10-26; not `cowas-v2`). Nothing has been committed; the user will commit.


---

### Session 2026-10-26 (decisions): summary

**Decided by the user:**
- Spool on: purge 3 L (D50). Without the spool: 500 mL, unchanged.
- Purge only: 100 % pump power, the one exception to the 60 % water limit (D51). The 3 bar target and the cap are unchanged.
- Blank is not a purge. Nothing changed for it.
- Work stays on `stage1-field`. Not `cowas-v2`. The user commits.

**Changed:** see the entry above. Files: `include/Settings.h`, `include/Pump.h`, `src/Hardware/Pump.cpp`, `src/Step_functions.cpp`, new `src/Core/pump_power.{h,cpp}`, new `test/test_pump_power/`, `test/test_run_keys/test_main.cpp`, `docs/roadmap/stage2-pilots.md` §3.7 and §7b, `docs/roadmap/stage1-FINISHED-prototype.md` (to-do item 4), `docs/software/code-guide.md`, `docs/logs/decisions.md`, `docs/logs/findings.md`.

**Checks:** `pio test -e native` 176/176; `pio run -e due` builds. Not run: host tests, bench, flash.

**Open:**
1. F207: bench check of the 100 % purge (pressure log, manifold watched) before the field.
2. Stage 1 to-do item 4: weigh one spool-on purge (F130).
3. The spool timeout stays 13 min (a safety only).
4. `findings.md`: F207 row sits above F206. Reorder when convenient.

---

### Session 2026-10-26 (later): sample loading order and availability

**Why.** The operator should not have to choose a sample number. `run_sample` picks the next loaded sample in the loading order, which keeps neighbouring samples apart (cross contamination).

**Decisions (user, 2026-10-26):**
- One loading order: `FILL_ORDER` (`include/Settings.h`) = 1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7. The tool reads it from there; nothing else copies it. Slot 7 is last.
- The state is kept by the PC tool per run folder, not on the Due. A `run_sample` without the tool still runs, with a warning, and its record is not saved.

**Changes:**
- `tools/sample_slots.py` (new): reads `FILL_ORDER`; `SlotBook` keeps each sample as available, used or unavailable in `slots.json`.
- `tools/session_log.py`: a new folder asks how many samples are loaded, then asks for confirmation that they are sorted on the device. `run_sample` (no slot) sends the next available sample; `run_sample <slot>` asks to erase a used or unloaded one. The `META|sample` line marks a sample used. The tool sends `log_on` at its start.
- `src/Tests_mech.cpp`: new `log_on` command (flag, reset at boot). `run_sample` prints a `WARNING` when no PC log is running. Help lines added.
- Docs: [session-log.md](../software/session-log.md) (order, numbering, tag, unavailable, erase, appended records), [commands.md](../software/commands.md) (`log_on`, `run_sample`), [stage3](../roadmap/stage3-full-system.md) P2-19 (sample records on the device: SD card or slave MCU).

**Behavior changes (for the PR):**
- Firmware: new `log_on`. `run_sample` prints a `WARNING` line before its questions when no PC log is running. The sample still runs.
- The tool: `run_sample` without a slot runs the next loaded sample in the loading order, not the slot the manifold is on.
- `FILL_ORDER` is unchanged.

**Checks:**
- `python -m pytest tools/`: 135 pass.
- `ruff check` and `ruff format --check` pass on the new files. `session_log.py` has one SIM115 (the `open()` in `Session`, from before this work) and was not ruff-formatted before.
- `pio run -e due`: SUCCESS, not flashed.
- A smoke run of `session_log.py` with a fake serial port (no hardware): the start questions, the next-sample pick, and the refusal of an unloaded slot behaved as specified.
- Not run: `pio test -e native` (no native code changed), the bench.

**Open:**
1. Bench check, once the user approves the flash: `run_sample` with and without the tool (the warning appears only without `log_on`), and a full sample with the record saved.
2. Anticlockwise direction: the user's statement. The code confirms only that the no-hole position is between slots 3 and 4 (`include/Manifold.h`).
3. The tool's state is not checked against the device, and a `run_sample` typed in `pio device monitor` skips the loading order.
4. A re-run of a slot is saved as a new record with the same slot number; the operator uses the last one.
5. Not committed. Work is on `stage1-field`; CLAUDE.md asks for a branch from `cowas-v2`.

---

### Session 2026-10-26 (timed sample list in the session tool)

**Why.** The night run needs samples at set times, without the operator typing each `run_sample` on time. The operator writes a list of samples and times; the tool runs it.

**Decisions (user, 2026-10-26):**
- A CSV file, one row per sample: `time` (HH:MM, PC clock), `name` (empty = the time), `condition`, optional `spool` (on/off) and `depth_m`.
- Before the run the tool shows the list and asks whether the samples fit the timeline. No automatic check (D52, F208).
- A sample due while the one before still runs waits until that one has finished.
- The real start and finish times are recorded.
- The spool depth is recorded only: the spool is disabled (`SPOOL_USE = false`).

**Decisions made in the code (to change if you want):**
- Rows stay in the order of the file. The first time is today. A time 6 hours or more before the row above is the next day (night run); a smaller step back is refused with its line number.
- An empty condition becomes `not given` (the device needs one).
- A run the device refuses releases its sample slot: the sample is still unused.
- A `run_sample` typed by hand is refused while samples are pending.
- While a run has no `TEST` line, the log says *still waiting* every 10 minutes.
- An error in the scheduler stops it with a message. The samples not run then need `run_sample` by hand.

**Changes:**
- `tools/sample_schedule.py` (new): reads the CSV, shows the list, runs it (`Schedule`), writes `schedule.csv`.
- `tools/session_log.py`: `--schedule FILE.csv`. Asks the fit question after the sample questions. Answers the device's name, condition and notes prompts. Refuses a hand-typed `run_sample` while samples are pending.
- `tools/tests/test_sample_schedule.py` (new): 49 tests.
- `tools/schedule_example.csv` (new).
- [session-log.md](../software/session-log.md): section "Scheduled samples".
- [stage 1](../roadmap/stage1-FINISHED-prototype.md): to-do item 6 and the quick field test ("Quick test: the timed sample list").
- [decisions.md](decisions.md) D52, [findings.md](findings.md) F208.

**Firmware:** no change. The tool relies on these device lines (`Tests_mech.cpp`): `ASK | sample name`, `ASK | condition`, `ASK | notes`; `REFUSED | run_sample`, `CANCELLED`, `Sample not started`; and `TEST|run_sample|<slot>|PASS|FAIL|...`. A run always ends with its `TEST` line (`cmd_run_sample` calls `test_result` after `run_full_cycle`). A `REFUSED | run_sample` line is printed only before the pump runs. If these texts change in the firmware, change the constants at the top of `tools/sample_schedule.py` too.

**Logic review (before the field test):** the first version had these holes. All fixed:
1. The list was sorted by clock time. A night run (23:30, 00:30) would be reordered, and 00:30 would count as already passed. Now: file order, next-day rule.
2. A run refused after its `META` line (O-ring state unknown) had marked its slot used. Now the slot is released.
3. A hand-typed `run_sample` during a schedule would collide with the scheduled one. Now refused.
4. A silent device blocked the schedule with no message. Now a 10-minute *still waiting* line.
5. An exception in the scheduler thread stopped it silently. Now reported.
6. `schedule.csv` gained a `scheduled` column (date and time), so the due time is in the file next to the real times.

**Tests:**
- `python -m pytest tools/`: 184 pass (49 for the schedule).
- End-to-end with a fake serial port (a scratch script, not in the repo): two samples, A at its time, B due during A. B waited for A's `TEST` line. The name, condition and notes reached the device. `schedule.csv` has the real times, both PASS. Exit code 0.
- Not run: the Due (no bench; not flashed). No firmware build (no firmware change).

**Open:**
1. The field test ([stage 1, quick test](../roadmap/stage1-FINISHED-prototype.md#quick-test-the-timed-sample-list-field)): not done yet. Do it before the night run.
2. F208: no check that the samples fit the timeline.
3. The schedule needs `session` on the Due before the first time. Without it the first run is refused and its slot released. Check the first row.
4. A `run_sample` typed in `pio device monitor` still skips the tool and the loading order (see the entry above).
5. Not committed. Work is on `stage1-field`; the user commits.

---

### Session 2026-10-26 (timed sample list, second pass)

**Why.** The user reviewed the timed list and asked for: dates in the file, a list that runs only when the operator starts it, a hand `run_sample` that asks first, a tutorial for Felix, and a logic check. This entry supersedes the 6-hour rule and the "fit your timeline" question of the entry above.

**Decisions (user, 2026-10-26):**
- The file has a `date` column: `2026-10-27`, `27/10/2026`, or `+N` (N days after the day the file is read). `+0` is today.
- **Nothing runs when the file is loaded.** `schedule start`, then `y`, starts it (D53).
- A `run_sample` typed by hand during the list asks to stop the list. After the hand run the tool says how to continue with `schedule start`.
- The field test puts the two samples 15 minutes apart (spool rule, the user's estimate; see F208).

**Decisions in the code (review them):**
- The file's dates are converted to full dates once, when it is read. The full list is saved in the run folder as `schedule.csv`, with the status, slot and real times of each sample. Restarting with the same tag goes on from that file.
- A saved list that has not started yet is replaced by an edited file. A list that has started is kept, and the message says so.
- A sample that was running when the tool closed is `interrupted` and is not run again.
- Rows are sorted by date and time. Equal times keep the file order.
- `;` or `,` separators, `,` or `.` as decimal point, and an Excel byte-order mark are all accepted.
- An empty condition becomes `not given` (the device needs one).
- A run that the device refuses after its name frees the sample slot.

**What the words mean (the user's questions):**
- `TEST|run_sample|<slot>|<PASS|FAIL|SKIP>|<k=v;...>`: the line the Due prints at the end of every `run_sample`. The result is in the middle. The list uses it to know the sample is finished (`summary` lines do not count).
- **The "watchdog":** if the Due goes quiet during a sample (no TEST line), the list would wait forever with no message. Now the log repeats "still waiting" every 10 minutes. It does not stop by itself.
- **The thread errors:** a bug in the list stops it with a message on screen and in the log. The samples not run yet stay saved, and `schedule start` continues them.

**Logic fixes (this pass):**
1. **Refused after the name:** the O-ring refusal comes after the name, so the slot was left used. Now `on_device_line` frees the slot when the device refuses after the name. This works for hand runs too.
2. **Stop race:** after `schedule stop`, a sample could still start: the waiting thread woke on the TEST line of the sample before, then sent the next one. The wait now checks the stop flag.
3. **Hand run during the list:** the question *stop the schedule?* is asked only while the list runs. A hand run is refused while a list sample is on the device.
4. **Midnight:** the old rule (6 hours) is gone. Each row has its date.
5. **Restart:** the list is loaded from the run folder, so a closed and reopened tool does not repeat or lose samples.

**Checked and corrected:** a run-start problem was first suspected (the tool's `=== run_sample ===` line). It is printed by `test_begin` (`src/Test_report.cpp`), after the name and before the pump run. No problem; the suspicion was wrong and no finding was kept.

**Changes:**
- `tools/sample_schedule.py`: rewritten. Dates, saved list, start and stop, thread, the refusal rule.
- `tools/session_log.py`: `schedule`, `schedule start`, `schedule stop`; the hand-run question; the refusal handling in `on_device_line`; `--schedule` reads the list (nothing auto-starts).
- `tools/schedule_example.csv`: new format.
- `tools/tests/test_sample_schedule.py`: rewritten, 64 tests. `tools/tests/test_sample_slots.py`: 3 new tests (refusal, no run start on a refusal before the name, a finished run).
- [operator-tutorial.md](../field/operator-tutorial.md): new section 4 (the CSV, starting, while it runs, restart).
- [session-log.md](../software/session-log.md): the schedule section rewritten for the developer.
- [stage 1](../roadmap/stage1-FINISHED-prototype.md): quick test (two samples 15 minutes apart, start, restart check, pass criteria).
- [decisions.md](decisions.md) D53, [findings.md](findings.md) F208 (kept, open).

**Checks:**
- `python -m pytest tools/`: 202 pass.
- End-to-end runs with a fake serial port (scratch scripts, not in the repo):
  - A: the list is shown, `schedule start` asks `n` (nothing starts), then `y`; two samples run in order; `schedule.csv` has the dates and real times.
  - B: a hand `run_sample` during the list asks to stop it; the hand run goes; the hint to `schedule start` appears.
  - C: the same tag reopened without `--schedule` loads the list: one sample PASS, one left.
- Not run on the bench. No firmware change. Not flashed.

**Open:**
1. The field test (stage 1, quick test): not done yet. Before the night run.
2. F208: no check that the samples fit the timeline. The 15-minute rule is the user's estimate, not a measured cycle time.
3. A `run_sample` typed in `pio device monitor` still skips the tool and the loading order.
4. Not committed. Work is on `stage1-field`; the user commits.

---

### Session 2026-10-26 (timed sample list, third pass)

**Why.** Review of the timed list (`--schedule`) before closing the session. Changes asked by the user: a generic file name, the `+N` base day, a simpler and better commented module, and tutorial text for Felix. This entry corrects the two earlier entries where they are now wrong.

**Corrections to the entries above:**
- `+N` counts from the day `schedule start` is first typed, not from the day the file is read (D54).
- The file name is free. `night.csv` in the tutorial was too specific; it is `schedule.csv` now.
- No "15 minutes between samples" rule is written anywhere. The 15 minutes in the stage 1 test are only the test's own spacing.
- The `=== run_sample ===` line is printed by the firmware (`test_begin`, `src/Test_report.cpp`). The F209 finding raised earlier was wrong and is not kept.

**Decisions (user, 2026-10-26):**
- Dates: `2026-10-27`, `27/10/2026`, or `+N`. `+0` is the day `schedule start` is first typed.
- The list runs only after `schedule start` and a `y`. The tool shows the dates before it asks.
- Stop, hand runs, closing the tool to work on the code, and restart: all work with the same tag, from the saved list.
- Rainbow CSV is only a colour aid. The repo does not use it; nothing needs it. The tutorial says so.

**Changes:**
- `tools/sample_schedule.py`: rewritten, shorter and simpler.
  - Dates stay as text (`+N` or a full date) until `schedule start`. `resolve()` turns them into full dates and sorts, in one place.
  - "Not run" status removed: a waiting sample stays `waiting` when the tool closes.
  - `preview()` shows the resolved dates without changing the list.
  - `start()` resolves, saves, then starts the thread.
- `tools/session_log.py`: `schedule_command` takes the day of the start. `--schedule` no longer takes a base day. Removed the duplicate launch message. Smaller refusal branch in `on_device_line`.
- `tools/tests/test_sample_schedule.py`: rewritten, 71 tests. `tools/tests/test_sample_slots.py`: no change in this pass.
- `tools/schedule_example.csv`: new column layout (`date` first).
- Docs: [operator-tutorial.md](../field/operator-tutorial.md) section 4 (generic file name, `+N` rule, no spacing rule, the optional CSV viewer); [session-log.md](../software/session-log.md) schedule section; [stage 1](../roadmap/stage1-FINISHED-prototype.md) quick test (`schedule.csv`, `+0` at start, restart check).
- [decisions.md](decisions.md) D54.

**Checks:**
- `python -m pytest tools/`: 209 pass.
- `ruff format` and `ruff check` on the new files: clean. One `SIM115` remains in `session_log.py` (the `open()` in `Session`), from before this work.
- End-to-end runs with a fake serial port (scratch, not in the repo):
  - A: the list shows; `schedule start` shows the `+N` dates and asks; the `+1` sample waits for tomorrow; the `+0` sample runs and gets PASS.
  - B: a hand `run_sample` during the list asks to stop it; the hand run goes; the hint to `schedule start` appears.
  - C: the same tag reopened without `--schedule` loads the saved list with one sample PASS and one waiting.
- Not run on the bench. No firmware change. Not flashed.

**Open:**
1. The field test (stage 1, quick test): not done yet. Before the night run.
2. F208: no check that the samples fit the timeline.
3. A sample that was running when the tool closed is not run again. The operator checks the device.
4. A `run_sample` typed in `pio device monitor` still skips the tool and the loading order.
5. Not committed. Work is on `stage1-field`; the user commits.

---

### Session 2026-10-26 (timed list, fourth pass: tests and open items)

**Why.** The user asked for only the useful tests, and for the open items to be parked in stage 2.

**Open items, moved to stage 2 [§7b](../roadmap/stage2-pilots.md#7b-carried-from-the-stage-1-operator-test):**
- F208: the timed list does not check that the samples fit the timeline (host, new step).
- A `run_sample` typed in `pio device monitor` skips the session tool: no record, no loading order, no timed list (FW7). A line in the stage 1 "Limits" also tells the operator to use the tool for every sample.
- The stage 1 field test (quick test) is still to be done before the night run. It stays in stage 1, not in stage 2.

**Tests:**
- `tools/`: 209 → 184 pass.
- Schedule tests (`tools/tests/test_sample_schedule.py`): 71 → 47. Removed: repeated date and time formats, one test per spool variant, variants of the plan-loading checks, a second clock test covered by the next-day test, and a stop test that repeated another. Each kept test checks one behaviour that matters (reading errors, saved list, start and resolve, device answers, end of a sample, waiting, stop, restart, hint, reminder, thread error).
- Slot tests (`tools/tests/test_sample_slots.py`): 29 → 28. Removed one trivial pass-through check. The rest cover the loading order and the run flow.
- Not touched: the earlier tests of `analyze_la3`, `analyze_la4`, `compare_tune`, `hw_test_log` and `session_log` (109 tests, from before this feature).
- `ruff check` and `ruff format --check` pass on the new files.

**Not run:** the bench, the flash. Not committed. Work is on `stage1-field`; the user commits.

---

### Session 2026-10-26: summary of the timed sample list (for closing the chat)

**Where it stands.** The session tool (`tools/session_log.py`) can run a list of samples at set times from a CSV file (`--schedule`). Nothing starts until the operator types `schedule start` and confirms. The list can be stopped, the operator can run samples by hand, and the tool can be closed and reopened with the same tag; the list resumes from `schedule.csv` in the run folder. Entries above give the detail: first pass (design), second pass (review and fixes), third pass (dates, names, cleanup), fourth pass (tests, open items).

**Decisions:** D52 (timed list), D53 (explicit start, saved list), D54 (`+N` counts from the first `schedule start`; no 15-minute rule written).

**Findings:** F208 (no fit check for the timeline, open), F210 (`run_sample` in `pio device monitor` bypasses the tool, open, fix FW7). Both are in stage 2 [§7b](../roadmap/stage2-pilots.md#7b-carried-from-the-stage-1-operator-test).

**Still to do:**
1. Stage 1 field test, the quick test in [stage 1](../roadmap/stage1-FINISHED-prototype.md#quick-test-the-timed-sample-list-field). Before the night run.
2. Bench: none needed for the tool. The firmware is unchanged.
3. Commit: the user, on `stage1-field`.

**Files of this work:** `tools/session_log.py`, `tools/sample_schedule.py`, `tools/sample_slots.py`, `tools/schedule_example.csv`, `tools/tests/test_sample_schedule.py`, `tools/tests/test_sample_slots.py`, `docs/field/operator-tutorial.md` (section 4), `docs/software/session-log.md`, `docs/roadmap/stage1-FINISHED-prototype.md`, `docs/roadmap/stage2-pilots.md` (§7b), `docs/logs/LOG.md`, `docs/logs/decisions.md`, `docs/logs/findings.md`.

**Tests:** `python -m pytest tools/`: 184 pass. Ruff clean on the new files.

---

### Session 2026-10-26 (stage 2 spec: sample duration from field logs)

**Why.** The timed list (`--schedule`) cannot yet tell whether a planned mission fits the time it takes. The user wants the sample time estimated from real field runs, and a warning when a plan does not fit. This is a spec only: nothing is implemented.

**What was specified** ([stage 2 §7.2, HO6](../roadmap/stage2-pilots.md), P1, after HO5):
- Input: the sample records and the step times of the session folders. The parameters that change the duration: spool or no spool, intake depth, target volume, pump power, filter state (clogged or stalled), shield time.
- Rule: read the last 10 session folders at each host start (all of them if there are fewer). Group the samples by parameter. A group is repeatable when its spread is within ±15 % of its median (placeholder). The estimate is the longest repeatable total plus 5 %. No repeatable group, no estimate.
- Use: warnings in the mission runner, never a block. The firmware does not read these timings.
- Trace: each estimate writes its inputs to the run folder.

**Open, for the user:** (a) "longest repeatable" as the maximum, or a percentile such as the 90th; (b) the repeatability numbers (3 samples, ±15 %) are placeholders until field data exists.

**Registers:** D55 (proposed). F208 now points to HO6.

**Not changed:** code, firmware, tests. Not committed; work is on `stage1-field`.

---

### Session 2026-10-26 (HO6 spec: verified and completed)

**Why.** Check the HO6 spec against the request before closing the chat.

**Decided by the user:** the value per key is the **90th percentile** of the repeatable group (not the maximum). The 3-sample and ±15 % numbers stay placeholders until field data.

**What the check found, and what changed in [HO6](../roadmap/stage2-pilots.md):**
- The spec had no stored table and no check of measured times against it. Added: `timings.json` in the host data folder (one file, history kept), checked and updated at each host start, with a report.
- "Longest" replaced by the 90th percentile (D55).
- Only runs that reached their full volume (`end_reason = volume`) are timed. Other runs are counted, not timed.
- Firmware version (`fw`) and calibration (`calib`) are keys: a change starts a new group.
- Only `purpose = sample` records are timed; each run is one timing, taken from its sample record.
- The total is `run_finish_pc − run_start_pc`. The step times `t_*_s` exist in the firmware source (added 2026-10-23), but no record on disk has them yet (checked 2026-10-26).

**Facts checked on disk:** `docs/logs/test-logs/2026-10-23/` has 8 records: 4 purge and 4 sample records. Of the 4 sample records, 1 is complete (`volume`) and 3 ended `clogged`. No key reaches 3 complete samples, so the estimator's first run on this data must give "no value". That is the first test of HO6.

**Registers:** D55 updated (90th percentile, stored table, placeholders). F208 points to HO6.

**Not changed:** code, firmware, tests. Not committed; work is on `stage1-field`.

---

### Session 2026-10-26 (HO6 recheck against the request)

**Gaps found and fixed in [HO6](../roadmap/stage2-pilots.md):**
- **Spool and depth are not in the sample record.** Spool is only on the TEST line; depth is not recorded at all. Added as prerequisites (owner: firmware and session tool).
- **Shield time and pump power were listed as keys.** Both are constant today, so they are not keys yet.
- **Keys were not defined as a string.** Defined: `spool; depth_m; target_ml; fw; calib`.
- **Clock:** the durations use the PC clock, so HO4 (trusted clock) is a dependency.
- **Structure for developers:** definitions, the `timings.json` schema, the run steps, a pure estimator with I/O apart, golden tests, and eight "done when" checks.

**Open, for the user:** whether the stored value follows the 10-session window (recommended) or only goes up. The request's "keeping the longest" can mean either. D55 and HO6 both list it.

**Not changed:** code, firmware, tests. Not committed; work is on `stage1-field`.

---

### Session 2026-10-26 (HO6: the stored value never decreases)

**Decided by the user:** the stored value per key never decreases: it is the largest candidate value seen. Outliers are excluded by the ±15 % repeatability rule. (Read as the second of the two options offered; the user's "yes except the outliers" was taken as this choice.)

**Changed in [HO6](../roadmap/stage2-pilots.md):** `value_s = max(value_s, window_s)`, where `window_s` is the candidate from the current window. Schema, start steps and done-when list updated (two new checks: never decreases, outliers excluded). D55 updated.

**Not changed:** code, firmware, tests. Not committed; work is on `stage1-field`.

---

### Session 2026-10-26: closing state (timed list and stage 2 specs)

**Done in code (`stage1-field`, not committed):**
- Timed sample list in the session tool: `--schedule FILE.csv`, shown at start, run only after `schedule start` and `y`. Stop, hand runs, close and reopen (same tag) all work. Dates `+N` count from the first start.
- Refused runs free their slot. Run times and tool tests: `python -m pytest tools/` 184 pass. Ruff clean on the new files.
- Firmware: unchanged. Not flashed. Not run on the bench.

**Done in docs:** operator tutorial section 4; session-log tutorial section; stage 1 quick test; stage 2 HO6 spec (sample timing from field records); decisions D52 to D55; findings F208 (now HO6) and F210 (FW7).

**Open:**
1. Stage 1 field test (quick test, two real samples). To do before the night run.
2. HO6 (P1, spec only): needs `spool` and `depth_m` in the sample record (firmware and session tool), HO3 and HO4 first. The 3-sample and ±15 % numbers stay placeholders until field data.
3. F210: a `run_sample` typed in `pio device monitor` bypasses the tool. Fix in FW7.
4. The 2026-10-23 records give no timing value yet (only one complete sample). The `t_*_s` step times are not on disk yet.
5. Commit: the user, on `stage1-field`.
### Session 2026-10-23 (stage1-field merged): PR #5, shield_V2 replaced

**What was done**
- Branch `stage1-field` pushed to origin (tip `1fdecff`). Old remote name `CoWas-Stage1-Proto-FieldDeployment` deleted.
- Old shield_V2 tip (`4503486`) kept at tag `archive/shield_V2-2026-10-23`.
- PR #5 (`stage1-field` into `shield_V2`) merged. `shield_V2` is now the shared base and has the same tree as `stage1-field`. Verified by git: no difference between the two trees.
- The shield_V2 merge had brought a second `test_all_components()` call and a second PID block into `setup()`, plus a duplicate declaration in `maintenance.h` and the removed `experiences.h`/`.cpp`. All were restored to the `stage1-field` version before the PR. No shield_V2 code is in the shared base other than through the tag.
- Firmware: `pio run -e due` passes on the final tree.

**Correction to the PR text.** PR #5 says "no bench run for this step". That was not checked against this log. Whether the bench was run for the run keys, fill stop, trusted volume, filter watch and pump start is still open.

**Open**
1. Hardware checklist for this step (CLAUDE.md rule 7: numeric pass criteria). Decide whether it is needed, after checking which of the new modules were bench-tested.
2. `ABP_FULL_SCALE_PSI = 60` is still a PLACEHOLDER (F126).
3. The `archive/shield_V2-2026-10-23` tag is the only record of the old shield_V2 code outside the branch history.

Co-Authored-By: Claude Haiku 5.5 <noreply@anthropic.com>

### Pre-field session (26-10-26)

1. modified the CW offset and the encoder manifold 0 beause it misaligned
2. Adapted DNA shield times. Reduced the filling time by 1 minute, because the sterivex was already filled.
3. The big pump duration was put to 5s instead of 6s (don't know if it includes the ramp). It pused some air inside during the venting. The pressure built up was already too much

**This might be adapted based on field observations, as the filter will behaved differently depending how it is clogged**

### Session 2026-10-26: ethanol manifold prime

**What changed**
- `purge_pipes_manifold()` now first pumps through `PURGE_SLOT` until the D13 flowmeter indicates 50 mL, with the existing 3-minute purge timeout. If the flow target is not reached, the pressure cap trips, the pump cannot start, the operator presses START, or a rotation fails, the routine stops without moving on to later slots.
- Added the `ethanol_purge` test-menu command and help/documentation entry so the routine is reachable.
- After priming, the routine meters 50 mL through each sample slot, then meters a final 50 mL through `PURGE_SLOT` to carry remaining ethanol through the purge outlet. Each metered run uses the existing 3-minute timeout and pressure cap.
- The previous trailing `step_purge(false)` was removed because it added a separate 500 mL/3 L pump operation after the slot flushes.
- Preserved the existing uncommitted `step_fill_container()` edit (kept commented out).

**Validation**
- `pio run -e due`: passes.
- `pio test -e native`: fails in existing tests outside this change: `test_golden_scale_angle_bit_exact` differs by one float bit, and `test_pressure_calibration` has a narrowing-conversion error for `INFINITY`.
- Hardware: not run; firmware not flashed.
- Before ethanol use: with no filters installed, verify at least 50 mL actual volume at the purge prime, each sample slot, and the final purge; verify timeout and pressure-cap stops prevent subsequent runs. The flowmeter is known to read low around 50 mL/min (F150), so each indicated target needs a gravimetric bench check.


### Session 2026-11-02: ethanol cleaning procedure (`ethanol_purge` rewritten)

**What changed (behavior change, user request)**
- `purge_pipes_manifold()` ([maintenance.cpp](../../src/Hardware/maintenance.cpp)) now cleans the tubing with 70 % ethanol, no filter installed: prime through the purge slot (`CLEAN_PRIME_SPOOL_ML` = 3 L with the spool on in `session`, else `CLEAN_SLOT_ML` = 0.5 L), then 0.5 L through each used slot in `FILL_ORDER`, then `CLEAN_AIR_SLOT_MS` = 30 s of air through each of those slots, then `CLEAN_AIR_FINAL_MS` = 1 min at the purge slot. All air goes in by V1 (V1 open, Vman closed, the `load_slot_air` setup), so the tubes stay in the ethanol and nothing is asked in between (user, same day: 1 L was 17 L of ethanol; no tube-lifting prompt). The intake line before Vman is therefore not emptied.
- Refuses at once if the O-ring lock state is UNKNOWN (run `actuator_home`). The shared pump PID is reset before every run.
- It uses `CtrlPumpFlow` (water) and `CtrlPumpNoWater` (air), so the pressure cap, the implausible-sensor stop, the timeouts and the `x` / `s` keys stay active. The old 50 mL open-loop routine (`ETHANOL_MANIFOLD_FLUSH_ML`) is replaced.
- **Safety exception:** the water runs use `set_full_power(true)` (100 %), the second use of the full-power exception after `step_purge`. A WARNING block is printed at every run and a y/n is required before anything moves. Comments in Pump.h, pump_power.h and Settings.h name both users.
- Unused slots: asked (y/n, then the numbers; commas, spaces or ranges), or given on the command line `ethanol_purge skip=3,7,12`. Pure parsing and list building in [clean_plan](../../src/Core/clean_plan.h), 10 native tests. The firmware has no file system, so a "file" option would have to be sent by the host as that command line.
- An air run that never reaches air level only warns (`timeout`, `skipped`); cap, sensor, abort, refusal or a failed rotation stops the procedure with pump off, valves closed, O-ring locked.

**Validation**
- `pio test -e native`: 196 passed (includes `test_clean_plan`).
- `pio run -e due`: passes (an earlier run failed in Tests_mech.cpp on the uncommitted spool-depth work, fixed meanwhile; not this change).
- Hardware: not run, not flashed. Review against the tested code: water runs = `step_purge` setup (`CtrlPumpFlow`, full power, V1 closed, Vman open, `goto_slot_locked`); air runs = `load_slot_air` setup (`CtrlPumpNoWater`, V1 open, Vman closed).

**Docs**
- [operator-tutorial.md](../field/operator-tutorial.md) §3b, run at the start of the day (§1 step 5, after `session`, before `fill_DNA`) and at the end (§3 step 1): the procedure, the full-power warning, the `skip=` option, what a WARNING means. [commands.md](../software/commands.md) row updated.

**Hardware checklist (before real ethanol)**
1. Dry run with water, no filters, outlets over a container: the prime and 0.5 L per slot reach 500 mL (weigh each); the flush at full power stays below `PRESSURE_CAP` (peak printed).
2. `ethanol_purge skip=3,7`: slots 3 and 7 are not visited; `skip=15` and `skip=0` are refused; `none` and a lone ENTER clean all 14.
3. Air: each slot ends `end=cond` after 30 s or more and the outlet tubing is dry. The final push at the purge slot goes in by V1: the intake line before Vman stays full (run the normal purge before sampling).
4. `x` during a flush and during an air push stops the pump and ends the procedure; the O-ring is LOCKED afterwards.
5. Check the flowmeter on ethanol (viscosity differs from water; F150).

### Session 2026-11-02: spool dive, sample, roll up, calibration

**Goal (user):** run samples at a depth from the schedule: unroll to the depth, purge, sample, shield, roll up with the tube emptying itself; go straight to the next depth if the next sample is waiting; samples at the same time one after the other in file order; a way to calibrate the unrolled length (2, 5, 10, 15 m only).

**Status:** built, `pio run -e due` and `pio test -e native` pass. **Not flashed, not run on hardware.** Bench checklist below.

**Decision:** D56. Findings: F211 (unmeasured spool facts), F212 (old bugs fixed), F213 (drain path unverified).

**Changes**
- `src/Core/spool_motion.{h,cpp}` (pure, native tests `test/test_spool_motion/`): depth to length, the move checks (arrived, wrong way, stalled, timeout), the approach speed, the calibration fit (drum diameter = old x tape/encoder, least squares, worst deviation).
- `Motor::drive / move_to_length / go_home` replace the unbounded loops: stops on stall (`SPOOL_STALL_MS` 4 s with no pulse), wrong way (5 cm), `SPOOL_MOVE_MAX_MS` (15 min), down end-stop, up switch, `x` or `stop`; counts the coast after the brake; slow at the ends. The position is `homed` or unknown (unknown after every boot: the first move rolls up to the top switch slowly). `Motor::start(int)` (the old `spool` test commands) goes through the same code.
- `Encoder`: `get_distance_f()`, `set_diameter()`. The 10 cm diameter moved to `SPOOL_DRUM_DIAMETER_CM`.
- `Step_functions.cpp`: `step_dive` (V1 open while the tube goes down), `step_rewind` (manifold to the purge slot, V1 and Vman open, roll up, both closed), `spool_active()` (= `SPOOL_USE` and the `session` answer), `spool_move_depth`, `spool_roll_up(drain)`. Without the spool answer `y` they do nothing, so spool-less days are unchanged.
- `run_sample [slot] [volume=] [depth=m]`: depth checked before anything is asked; dive before the purge (a failed dive pumps nothing); after the shield the device asks `spool: next sample depth` (ENTER or `x` = none), then keeps the tube out or rolls up; then the O-ring unlocks. TEST line gets `depth_m`, `spool_end`, `next_depth_m`, `t_dive_s`, `t_rewind_s`; `STEP TIMES` gets dive and rewind. A failed roll up = `FAIL`, `failed_at=rewind`.
- New commands: `spool_status`, `spool_height`, `spool_home [drain=]`, `spool_goto`, `spool_calibrate`.
- `tools/sample_schedule.py`: sends `depth=`, answers the next-depth question (next waiting spool sample due within 10 min, else nothing; nothing after `schedule stop`), depth limit 15 m. Same-time rows keep file order (stable sort, now tested).
- Settings: `SPOOL_USE = true`, `SPOOL_DRUM_DIAMETER_CM`, `SPOOL_MAX_LENGTH_CM = 4000` (40 m, raised from 1500 at the user's request; only 0 to 15 m is calibrated), `SPOOL_MOVE_MAX_MS`, `SPOOL_STALL_MS`, `SPOOL_WRONG_WAY_CM`, speeds, `SPOOL_CAL_MAX_DEV_PCT`. All PLACEHOLDER except the stall time. Runtime: `spool_height_cm`, `spool_diameter_cm`.
- Docs: operator tutorial (sections 1, 2, 4, 5), commands.md, session-log.md.

**Behavior changes**
1. `SPOOL_USE` true: the old `spool`, `40m`, `motor_spool`, panel and Pi spool commands are live again (bounded by the end switches; the encoder-based ones by 15 m).
2. `run_sample` takes `depth=` and asks one extra question when it is used.
3. `sample_process`, `abort_sample` and `demo` now really roll the tube up when the session says the spool is installed.

**Tests:** `pio run -e due` OK. `pio test -e native` 196 passed (10 new). `pytest tools/` 191 passed (7 new). `host/` untouched (its venv points at another checkout, not run).

**Minimum if there is no time:** (a) `spool_status`: switch `PRESSED` at the top; (b) `spool_goto length=1`: tape = 1.05 m ± 8 cm, else `spool_calibrate quick` and `y`; (c) `spool_home drain=n`. Why: the encoder maths multiplies the angle by 10 where a length needs a radius, so the constant acts as a 10 cm radius (20 cm drum), which is the full roll (user: drum 10 cm, tube 8 mm, 22 cm rolled, about 45 m of tube, 12 turns per layer). Layer model: reads about 5 % short at 1-2 m, 3 % at 5 m, 1 % at 10 m, 2 % long at 15 m. Assumes the encoder is on the drum shaft.

**Hardware checklist (before any water)**
1. `spool_status`: origin switch `PRESSED` with the intake at the top, `released` when moved down by hand; down end-stop `released`. If reversed: stop, F211.
2. `spool_home drain=n` from the top: returns at once. `spool_goto length=0.5`: the tube goes out about 50 cm, `PASS`, `length_cm` within 3 cm of the tape. A `wrong way` stop = encoder wiring (swap A and B).
3. Time a 2 m move. If it is longer than a few minutes, raise `SPOOL_MOVE_MAX_MS` and `SPOOL_HOME_SPEED`.
4. `spool_calibrate`: `PASS` (every stop within 3 % of the line). Put the diameter in Settings.h.
5. `x` during a move: stops within 1 s, `ERROR ... aborted`. Pull the encoder plug during a move: stops after 4 s with `stalled`.
6. Wet: `spool_goto 1`, then `spool_home` with a container under the purge outlet: the tube empties (F213).
7. `run_sample 7 depth=1` then a schedule of two spool samples 1 min apart: the second goes straight to its depth without a roll up (`spool_end=kept_out`).

### Session 2026-11-02: spool calibration (bench, dry)

**Data (user):** commanded 1 / 5 / 15 m, tape 0.97 / 4.77 / 14.72 m. The user does not need the drum diameter: the encoder measures a constant-radius wheel that turns with the linear travel, so only the scale matters.

**Fit check:** scale = Σ(tape x commanded) / Σ(commanded²) = 2456200 / 2510000 = 0.9786 (same formula as `core::spool_fit_scale`). Worst stop 2.6 % off (5 m). The drum value is 10.0 x 0.9786 = 9.786.

**Status:** the calibration was already done and `SPOOL_DRUM_DIAMETER_CM = 9.786` was already in `include/Settings.h` before this entry. No code change in this entry, so no behavior change and no new build or test run. The user reports the calibration was not off on the bench.

### Session 2026-11-02: schedule file, no spaces in names and conditions

**What changed (tool only, no firmware change)**
- [sample_schedule.py](../../tools/sample_schedule.py): a space in the `name` and `condition` columns becomes `_` when the row is read (`lake A` -> `lake_A`, `filter A` -> `filter_A`). The device never gets a space in these answers. Empty condition is now `not_given` (was `not given`).
- The names saved in the run folder (`schedule.csv`) already hold the `_` form.
- [schedule_example.csv](../../tools/schedule_example.csv): example rows use `lake_A` and `filter_A`.
- [operator-tutorial.md](../field/operator-tutorial.md) section 4: explains how to prepare the file (copy the example to the project folder, columns, PC clock, room between samples, what a bad row does) and the `_` rule. Section 2 keeps "no spaces" for the live `run_sample` answers, which the operator types by hand.

**Behavior change**
- Only the schedule tool. Schedule rows with spaces now run with `_`. A manual `run_sample` is not changed.

**Validation**
- `pytest tools/tests`: 191 passed. New test `test_spaces_in_a_name_and_a_condition_become_underscores`; the name test that expected `lake A` now expects `lake_A`; the default-condition test expects `not_given`.
- Hardware: not run (no schedule was run on the Due).

**Open**
- The spool is still disabled, so `spool` and `depth_m` in a schedule only reach the log (unchanged).

### Session 2026-11-02: ethanol_purge 0.1 L per slot

**User (2026-11-02):** `ethanol_purge` does not seem to flow anything (spool on, intake at 0.6 m). Asked for 0.1 L per slot instead of 0.5 L.

**Changes**
- `CLEAN_SLOT_ML` 500 -> 100 (`include/Settings.h`). Used for each slot flush, the ethanol total and the spool-off prime. The spool-on prime stays 3 L (`CLEAN_PRIME_SPOOL_ML`), not changed by the request.
- Comments, the help line, `maintenance.h` and the operator tutorial (section 3b) say 0.1 L.

**Full power (checked in code, not on the bench):** `ethanol_purge` calls `set_full_power(true)` (maintenance.cpp), `CtrlPump::run` limits the PID output with `pump_power_limit_percent(...)`, which returns `PUMP_PURGE_MAX_POWER_PERCENT` (100) when set, and `set_power_live` writes 100 % = 4095 on the 12-bit DAC. The start ramp (5 %/s) and the 3 %/update slew still apply. So the exception does reach the output. Why nothing flowed is not found in the code: the run has no stall watch (`filter_watch_` not set), so a dry pump only ends at the timeout (3 min per flush, 13 min for the spool prime). Unmeasured: priming a diaphragm pump at 0.6 m of suction lift with an empty spool line.

**Validation:** `pio run -e due` OK. `pio test -e native` 196 passed. Not flashed.

**Hardware checklist:** send the console lines `Ethanol prime: ... end=`, the `Pressure bar` and `Flow measured` lines printed during the prime, to see whether the pressure rises and whether the flowmeter moves.

### Session 2026-11-02: water fill of the spool tube before each sample

**User request:** with the spool unrolled a short distance (about 0.6 m), the pump cannot push water through the many turns of the empty tube. Fix: unroll to the session's deepest point with the air valve open, so the tube fills with lake water; prime the purge slot until the flowmeter shows flow; then roll up to each sample's depth while the pump runs. Samples at one time: deepest first, so the tube is filled once. The manifold's deployment valve is shut while the manifold turns, so the tube does not drop its water at the manifold.

**Changes**
- `session` (spool y): asks the session's deepest depth in m, ENTER = `SPOOL_SESSION_DEFAULT_MAX_M` (40 m). Warns above `SPOOL_CALIBRATED_MAX_M` (15 m). Stored in `spool_session_max_m`, printed in the META line as `spool_max_m=`.
- `run_sample ... depth=`: refused when the depth is deeper than the session's deepest point.
- `spool_fill_and_rise(depth_cm)` (Step_functions.cpp), called by `run_full_cycle` in place of `step_dive`:
  1. Dry tube (`spool_tube_wet` false): dive to the session's deepest point, V1 open.
     Wet tube: lower only if the sample is deeper than the tube is now.
  2. Prime: manifold to the purge slot, V1 shut, Vman open, pump (60 % water limit, PID 200/100/100, pressure cap) until the flowmeter counts `SPOOL_PRIME_FLOW_ML` (200 mL). Timeout `SPOOL_PRIME_MAX_MS` (5 min). No flow: stop, nothing more pumped.
  3. Rise to the sample depth while the pump runs: steps of `SPOOL_RISE_STEP_CM` (50 cm). Before each step: pressure read, implausible reading or above `STX_MAX_PRESSURE` stops it, PID output with the start ramp. After each step: at least `SPOOL_RISE_MIN_ML_PER_STEP` (5 mL) must have been counted, else the rise stops.
  4. Then the usual purge (3 L), sampling, shield. The next-depth question is unchanged.
- `spool_tube_wet`: set true only when the tube is kept out after a sample; cleared at the start of every fill and by every roll up. False after a boot.
- `goto_slot_locked()`: shuts Vman before the manifold turns (all callers; each caller opens Vman again where it pumps).
- Core: `src/Core/spool_fill.{h,cpp}`: the depth text parser (empty = default, comma or point), the rise goal, the flow check. Tests: `test/test_spool_fill/` (7 tests).
- Schedule tool (`tools/sample_schedule.py`): same-time samples run the deepest first (spool off last), then file order. Tests updated and 2 new.
- Settings: `SPOOL_SESSION_DEFAULT_MAX_M`, `SPOOL_SESSION_MIN_MAX_M`, `SPOOL_CALIBRATED_MAX_M`, `SPOOL_PRIME_FLOW_ML`, `SPOOL_PRIME_MAX_MS`, `SPOOL_RISE_STEP_CM`, `SPOOL_RISE_MIN_ML_PER_STEP` (the last five PLACEHOLDER).
- Docs: operator tutorial §2 and §5 (session answer, one sample, several samples), commands.md rows for `session` and `run_sample`.

**Behavior changes**
1. `session` with spool y asks one more question (the deepest depth).
2. Every sample with `depth=` fills the tube and rises while pumping before the purge (a short prime and the rise replace the plain dive).
3. `goto_slot_locked()` shuts the deployment valve during every slot change (also runs without the spool; harmless, the callers open it again).
4. The schedule tool orders same-time samples deepest first.

**Not verified (no hardware yet)**
- The pump keeps its power while the spool moves a 50 cm step; the pressure is only checked between steps. The move's own stall and time limits apply, but the cap is not checked during a move. The step length is the knob if this is a concern.
- The thresholds (200 mL prime, 5 mL per step, 50 cm steps) and the spool speed are guesses.
- The layer model is calibrated to 15 m only: at 40 m the real intake depth is not measured. The session answer is the guard against the ground.
- The schedule tool still accepts depths up to 40 m; the device refuses a depth deeper than the session's answer.
- Pumping 40 m of tube full of water at the purge: the pressure at the sensor during the prime is unmeasured (the cap stops the pump at 3.6 bar).

**Validation**
- `pio run -e due`: passes. `pio test -e native`: 203 passed (7 new). `pytest tools/`: 194 passed (2 new in this step).
- Hardware: not run, not flashed.

**Hardware checklist (numeric pass criteria)**
1. `session`, spool `y`: ENTER gives `spool_max_m=40.00`. `15` gives no warning. `16` is accepted with a warning. `41` and `abc` print REQUIRED and ask again.
2. `run_sample 7 depth=0.5` on a rolled-up tube: `Spool fill: dry tube, unrolling to the session depth 40.0 m`, `Spool prime: flow check PASS` with at least 200 mL counted, `Spool rise ended: 50 cm out`. Each rise step counts at least 5 mL. Pressure stays below 3.6 bar (peak printed).
3. Two samples at once, 15 m then 5 m: the second run prints no `dry tube` line and only rises (no dive).
4. A rise with no water (clamp the intake): the rise stops with `no flow in the last rise step`, the pump is off, valves closed, the tube is left out and nothing is sampled.
5. Slot change with the tube kept out: no water drops at the manifold outlet (0 mL in the container after the rotation).
6. `x` during a rise: the move stops within 1 s with `aborted with x`, pump off.

### Session 2026-11-02: corrections to the water fill (user review)

Replaces the design of the entry above where they differ.

**Changes**
- No pump during the roll up. After the flow check the pump is off and Vman is shut; the tube rolls up from the prime depth to the sample depth in one move (`spool_move_depth`), so the water stays in the tube. The 50 cm rise steps, the per-step pressure and flow checks, the 5 mL per step minimum and `spool_rise_while_pumping` are removed (`SPOOL_RISE_STEP_CM`, `SPOOL_RISE_MIN_ML_PER_STEP`, `core::spool_rise_next_goal_cm`, `core::spool_rise_flow_ok` and their tests).
- `SPOOL_PRIME_FLOW_ML` = 100 mL (was 200). The flow check ends with Vman shut.
- `session`: no 15 m warning (the calibration is trusted).
- `session` with spool `n`: no depth is asked or kept. The default is only read when the spool is on.
- `run_sample depth=` deeper than the session depth: warning, then `ASK | run_sample: solve it or proceed?`. `y` goes to that depth (a dry fill dives to the sample depth), anything else stops with nothing started. Was a refusal.
- `goto_slot_locked()` shuts Vman only when `spool_active()` (no change without the spool).
- Schedule tool: the tube stays full (`spool: next sample depth` answered) only for a sample with the same date and time as the one that just ended. A later sample rolls the tube up. Tests: the 10 min rule is replaced; 1 new test.
- `abort_sample()`: if the roll up fails with the spool on, the emptying (`step_empty`, which opens Vman) is not run, so the tube does not drain. Error printed, O-ring left as it is.

**Procedure audit: where Vman is open** (the deployment line, through which the tube drains by gravity when the pump is off)
- Open only while pumping through the tube: `step_purge` (purge), `step_sampling` (water phase), `load_slot_water`, `step_empty`, `spool_prime_flow`, `spool_roll_up(drain)`.
- Shut in: `load_slot_air` (air drying), the sampling's emptying and sterivex air drying (V1 open, Vman shut, from `vent_line` onward), `step_DNA_shield`, `spool_move_depth`, and every `stop_procedure`.
- Shut before every manifold turn with the spool on (`goto_slot_locked`).
- Checked and fine: the air drying of the filter never has Vman open. The one gap found was `abort_sample` (fixed above).

**Validation**
- `pio run -e due`: passes. `pio test -e native`: 199 passed (4 fewer: the rise tests removed). `pytest tools/`: 195 passed.
- Hardware: not run, not flashed.

**Hardware checklist (numeric)**
1. `session`, spool `y`, ENTER: `spool_max_m=40.00`. `session`, spool `n`: no depth question.
2. `run_sample 7 depth=0.5` on a rolled-up tube: `Spool prime: flow check PASS` with at least 100 mL counted, then `Spool fill: at ... cm out` with no pump between the prime and the end (check the pump's `Pressure bar` lines stop). Flow in the purge after the roll up: 0 mL counted, and the water stays in the tube (the next `run_sample` at the same time rolls up without a dive).
3. `run_sample 7 depth=50` with the session at 40 m: a warning and the `solve it or proceed` question. `n` starts nothing.
4. Two samples at the same time (15 m, 5 m) then one 10 minutes later: the first two share the fill; the third starts with `Spool fill: dry tube`.
5. A slot change with the tube out: no water in the outlet container (0 mL) after the manifold turns.
6. Air drying with the spool on: no water in the container at the manifold outlet (0 mL) during the drying (Vman shut).

### Session 2026-11-02: schedule depth check, values resolved, tutorial restructured

**User review:** (a) the schedule checks its spool depths against the session's depth at `schedule start`, and asks; (b) the placeholders are to be solved, not left open; (c) the tutorial is restructured (one flow, one sample or several at the same location, spool with the samples, end of the day last).

**Changes**
- **Schedule start (tools):** `depth_conflicts()` (`tools/sample_schedule.py`). `schedule start` prints `CONFLICT` with the waiting spool samples deeper than the session's `spool_max_m` (read from the device's `META|session` line, `Session.spool_max_m` in `tools/session_log.py`) and asks `y/n`. `y` starts the list and answers the device's `run_sample: solve it or proceed?` with `y` for each sample (`Schedule.accept_deeper`). No session spool depth (no session, or spool `n`) with waiting spool samples: `BLOCKED`, not started.
- **Same timepoint** (carried from the previous step, now in the docs): the tube is kept full only for a sample at the same date and time.
- **Placeholders resolved:**
  - `SPOOL_PRIME_FLOW_ML` = 100 mL. It is above the 30 mL intake path (`INTAKE_PATH_ML`) that the first water must clear. User: 100 mL is enough.
  - `SPOOL_PRIME_MAX_MS` = 4 min = 2 × (100 mL at `FLOW_TRUST_MIN_ML_PER_MIN`, 50 mL/min = 2 min). The timeout is now derived from the flow rule, not guessed.
  - Spool move time limit: the fixed 15 min (`SPOOL_MOVE_MAX_MS`) is replaced by `core::spool_move_limit_ms(distance, SPOOL_MIN_CM_PER_S, SPOOL_MOVE_MARGIN_MS)` = 60 s + distance / 2 cm/s. Tested in `test/test_spool_motion/`. A move slower than 2 cm/s ends as `timeout` (safe). The stall rule (4 s with no encoder pulse) and the wrong-way rule are unchanged and still stop a stuck spool at once.
  - Spool speed: the PWM settings (`SPOOL_SLOW_SPEED` 40 %, `SPOOL_HOME_SPEED` 30 %) are kept as design choices for a slow approach. The real cm/s is not known until a move has run on the bench, so every dive and rise now logs its speed (`Step dive ended: ... cm/s`). This is the one value that cannot be settled without the hardware: the first real move gives it.
- **Docs:** `operator-tutorial.md` restructured (temporary section removed; start of the day with the spool check; samples in 2a one sample / 2b schedule / 2c spool; troubleshooting; end of the day; ethanol cleaning as section 5 with the filters-out question; command list). `session-log.md`: the depth check and the same-timepoint rule. `commands.md`: the move time limit and speed logging.

**Behavior changes**
1. `schedule start` may ask one more question (the depth conflicts), and blocks spool samples when the session has no spool depth.
2. Spool moves end on a distance-based time limit (60 s + distance / 2 cm/s) instead of 15 min.
3. Every spool move prints its speed.

**Validation**
- `pio run -e due`: passes. `pio test -e native`: 200 passed (1 new: the move limit).
- `pytest tools/`: 199 passed (4 new: the conflict rules and the accepted-depth answer).
- Hardware: not run, not flashed.

**Hardware checklist (numeric)**
1. `session` spool `y`, 40 m. Schedule with one spool row at 15 m and one at 50 m: `schedule start` prints `CONFLICT` with the 50 m row only (the 15 m row is not listed). `n` starts nothing. `y` starts, and the device answers the 50 m sample's question with `y` without a stop.
2. `session` spool `n`, then `schedule start` with a spool row: `BLOCKED`, nothing starts.
3. A dive from a dry tube to 1 m: the `Step dive ended` line shows a speed in cm/s and is finite (no `inf`, no `0.00 cm/s` for a move that moved). Record the value: it is the first measured spool speed.
4. A move that stops on the timeout (`timeout`) is only possible below 2 cm/s: if the 1 m move ends on timeout, the spool is slower than the bound; set `SPOOL_MIN_CM_PER_S` lower and log it.
5. The prime at the purge slot: `Spool prime: flow check PASS` with at least 100 mL counted, in under 4 min.

### Session 2026-11-02: verification before the field (corrections)

Full review of the requests of this session against the code. Corrections made:

1. **Height bug (firmware):** the session depth plus `spool_height` can exceed the tube (40 m + 0.5 m = 4050 cm > `SPOOL_MAX_LENGTH_CM`). The fill would then fail on every sample after the move had started. `run_sample depth=` now refuses before anything moves when the deeper of session depth and `depth=` does not fit with the height (`REFUSED | run_sample: the deepest point of this session ...`).
2. **Accepted conflicts (tool):** the schedule's `y` at `CONFLICT` applied to every deep sample in the list. It now applies only to the rows listed in the conflict, matched by `sample_key()` (date, time, name, depth). A deeper row added or edited afterwards is left to the operator.
3. **Speed bound:** `SPOOL_MIN_CM_PER_S` lowered 2.0 -> 1.0 cm/s. With the true speed unknown, a bound that is too tight would fail every move with `timeout`. The stall rule (4 s with no encoder pulse) still stops a stuck spool; the time limit is the backstop (40 m = 67 min). Docs updated (tutorial, commands.md, session-log). The earlier LOG entries of this session keep their original wording.
4. **Stale text:** the tutorial's error row for `ERROR | spool rise` (that message no longer exists; the move error is `ERROR | step_dive: the spool did not get to the depth`); two roadmap/tool references to the tutorial's section 4 now point to section 2b.

**Checked and found correct**
- Session: ENTER = 40 m; range 0.5 to 40 m; a comma works; x cancels with nothing changed; no depth question when spool = n; the META line carries `spool_max_m` only when the spool is installed, and the tool reads it (tested with both cases).
- run_sample: depth without spool refused; depth deeper than the session: warning and `y/n`; anything but `y` cancels with nothing started.
- Fill: dry tube dives to the deeper of session depth and sample depth; wet tube is lowered only when the sample is deeper; the flow check runs with the deployment valve open and the pump; the valve is shut and the pump off before the roll up; the roll up moves no pump.
- Valves: the deployment valve is open only while water is pumped through the tube (purge, sampling water phase, flow check, drain roll up). Shut for air drying, sampling emptying, DNA shield, every spool move and every stop. `abort_sample` does not run the emptying after a failed roll up.
- Schedule: same timepoint only keeps the tube full; deepest first at the same time; `BLOCKED` without a session spool depth; conflicts asked once at start.
- Device answer timeout: a question the tool does not answer is cancelled by the device after its wait, which the tool reads as refused. The schedule does not hang.

**Known risks for the field (not fixed here, cannot be tested from the code)**
- **The fill dive keeps the deployment valve shut** (inherited from the earlier spool code, `step_dive`). Whether the tube fills in that state depends on the pipework, which the code does not show. If it does not fill, the flow check at the purge slot fails (`ERROR | spool fill: no flow`) and nothing more is pumped. A sample then ends as FAIL. If that happens on the first sample, the dive valve setting is the first thing to check.
- Spool speed is unmeasured. The first move logs its cm/s (`Step dive ended: ... cm/s`).
- The prime flow threshold (100 mL) and timeout (4 min) rest on the flowmeter's 50 mL/min confidence limit. Slower real flow fails the check safely.
- During the prime the pump works against the head of the tube at depth. The pressure cap (3.6 bar) stops the pump if the head is too high; that fails the sample safely.
- `host/` tests fail with `ModuleNotFoundError: cowas`: the host venv points at another checkout (see the 2026-11-02 log entry above). Not related to these changes.

**Validation (final)**
- `pio run -e due`: passes.
- `pio test -e native`: 200 passed.
- `pytest tools/`: 200 passed.

**Hardware checklist additions**
1. `session` spool y with the height set to 0.5 m and 40 m: the run refuses at once with the `more than the ... cm of tube` message, and no pump or spool move happens.
2. The first sample at depth: the dive line shows `cm/s`, the prime line shows `PASS`. If `FAIL`: stop, do not repeat the fill; check the deployment valve state of the fill dive.

### Session 2026-11-02: 40 m total tube, emptying at the end of each timepoint

**User request:** (1) the total tube (depth + spool height) never goes beyond 40 m, whatever the height; a session maximum of 40 m goes down to a total of 40 m; a schedule or a single sample that would pass 40 m total is warned before anything runs. (2) The main roll and unroll speed is not changed (neither faster nor slower). (3) Samples at the same timepoint are treated together: no emptying between them, deepest first, but the spool tubing IS emptied at the end of the group, before the next timepoint (the old `step_empty`, which was dropped on 2026-10-19 for the cycle).

**Changes (firmware)**
- `spool_reachable_cm()` (`src/Step_functions.cpp`): the deepest intake depth = 40 m minus the spool height (-1 when the height leaves no tube). `spool_fill_and_rise()` cuts the sample depth and the dive at it, and says so on the console (`Spool fill: depth ... cut to ...`). The dive is the deeper of the session depth and the sample depth, cut at the reachable depth.
- `run_sample depth=`: the depth is cut at the reachable depth with a warning, and the machine asks `run_sample: solve it or proceed?` (`y` = the cut depth, anything else = nothing started). The same question comes for a depth deeper than the session's deepest point. The former refusals (`depth + height ... more than the tube`) are removed. A height that leaves no tube is refused.
- `spool_status` prints `META|spool;height_cm=<cm>` (the PC needs the height for the 40 m check).
- Group end (`run_full_cycle`, `src/Tests_mech.cpp`): when the tube is rolled up (not kept out for the next sample at the same timepoint), `step_empty()` runs after the roll up. The TEST line gets `spool_emptied=y|NO`, `spool_empty_end=<end reason>` and `t_empty_s`. Only `cond` (air reached) counts as emptied. Otherwise the sample is `FAIL` with `failed_at=empty` and an ERROR line. `last_empty_end` is reset before the call, so a stale value cannot pass.
- Unchanged: the keep-out rule (kept only for the same timepoint, the next answer from the tool), the abort path (`abort_sample` already rolls up and empties).

**Changes (tool)**
- `total_overflows()` and `MAX_TUBE_M = 40` (`tools/sample_schedule.py`). `schedule start` reads the spool height from the device; without it, spool samples are `BLOCKED` (type `spool_status`). `CONFLICT` lists the session conflicts and the 40 m overflows (with the cut depth), one `y/n`. `y` answers the device's question for exactly the listed rows (`sample_key`, so a row added later is not answered).
- `session_log.py`: `Session.spool_height_cm` from `META|spool;height_cm=`.

**Speed (item 2):** unchanged. `git diff` against the baseline shows no change to `SPEED_UP`, `SPEED_DOWN` (100 %, the main roll), `SPOOL_SLOW_SPEED` (40 %, last 30 cm) or `SPOOL_HOME_SPEED` (30 %). The move time limit (`SPOOL_MIN_CM_PER_S`, 1 cm/s bound) only limits time; it does not change the speed.

**Behavior changes**
1. A depth with height that passes 40 m total is cut and asked, not refused.
2. The spool tubing is emptied at the end of each timepoint (after the roll up). Before this change it was not emptied at all in the cycle.
3. A failed emptying makes the sample FAIL (`failed_at=empty`).

**Not changed (decided)**
- A sample that fails before its roll up is left out (as before): the tube is not rolled up on a failure. Recover with `spool_home`, which empties too.

**Validation**
- `pio run -e due`: passes. `pio test -e native`: 200 passed.
- `pytest tools/`: 204 passed (4 new: overflow rules, the accepted-key limit, the device height and session lines).
- Hardware: not run, not flashed.

**Hardware checklist (numeric)**
1. `spool_height 50` (or any height > 0). `session` spool y, ENTER (40 m). `run_sample 7 depth=40`: warning `40.00 m + height 50 cm is 40.50 m of tube`, the question with `proceed to 39.50 m`. `n` starts nothing; `y` dives to 39.50 m (`Spool fill: depth 40.00 m cut to 39.50 m`).
2. `schedule start` with a 40 m row and height 50 cm: `CONFLICT`, the cut depth printed; `n` starts nothing.
3. Two samples at the same timepoint (15 m, then 5 m): the second line has no dive, no roll up and no emptying between them (`spool_emptied` is absent from the first TEST line). After the second: `spool_emptied=y`, `spool_empty_end=cond`.
4. A sample followed by a later timepoint: the tube rolls up, then the emptying runs; `spool_emptied=y` on the TEST line; the container at the purge outlet collects water until the air comes through.
5. A deliberately failed emptying (plug the air path): `ERROR | run_sample: the spool tubing was NOT emptied`, the sample `FAIL` with `failed_at=empty`.

### Session 2026-11-02: warnings first, operational order checked

**User request:** (1) the 40 m warning (and the depth warning) must come right at the start: at `schedule start` for a schedule, and at `run_sample` for a single sample. (2) The logs must show the real operational order, not the order in the file.

**Changes**
- `schedule start` (`tools/session_log.py`): the device-state checks and the `CONFLICT` list with its y/n now come first. The list of samples follows, then the start question. Before, the list was printed first.
- Single sample: `run_sample` checks the depth before the slot, the name or any other question, so the warning is already the first thing printed for a depth that needs an answer (unchanged, verified in the code).

**Operational order: checked, no change needed.** Simulation (`tools` scratch, fake device): rows in file order 1 m, 15 m, 5 m at 09:00 and one sample at 08:00. The device received `run_sample 1` (08:00), `2 depth=15`, `3 depth=5`, `4 depth=1`. `schedule.csv` lists the samples in the same order, with `started_pc` increasing. The run folder's `tests.csv` and `samples.csv` are written as each TEST line arrives, so they follow the same order. The sort used is: date, then time, then deepest first (spool off last). The sort is applied at `schedule start` and saved before any sample runs.

**Open point:** the request said "lowest depth to higher depth", but the rule agreed earlier is deepest first (the 15 m, 5 m, 1 m example). This step keeps deepest first. If shallowest first is wanted, the change is one sort key in `resolve()` (`tools/sample_schedule.py`).

**Tests**
- `tools/tests/test_sample_schedule.py`: a run with out-of-order rows sends and logs the samples in the operational order.
- `tools/tests/test_session_log.py`: the depth warning is printed before the schedule list, and a `n` starts nothing.
- `pytest tools/`: 206 passed. Firmware: no change in this step (`pio run -e due` and `pio test -e native` as in the previous entry).

**Hardware checklist**
1. `schedule start` with a row at 40 m and a 50 cm height: the first line after the command is `CONFLICT`, before `The schedule will be:`.
2. `run_sample 7 depth=40` with that height: the device's first question is `run_sample: solve it or proceed?`, before any slot or name question.
3. With rows listed 1 m, 15 m, 5 m at one time: `schedule.csv` and `tests.csv` show 15 m, 5 m, 1 m in that order.

### Session 2026-11-02: failed samples are emptied; audit of the whole session

**User decisions:** (1) the schedule runs deeper first, upper last (confirmed). (2) A failed sample must not leave water in the spool: whatever empties the spool, the next sample fills it again at the session's deepest point.

**Changes**
- `spool_roll_up_and_empty(t_rewind, t_empty, empty_failed)` (`src/Step_functions.cpp`): the one way to finish the spool tubing: roll up with V1 and Vman open, then `step_empty()`. Requires `last_empty_end == "cond"`. Used at the end of a timepoint, after a failed sample, and at the end of `sample_process`.
- Failed sample at depth (`run_full_cycle`, `src/Tests_mech.cpp`): rolled up and emptied. TEST line: `spool_end=rolled_up_after_failure`, `spool_emptied=y|NO`. The sample stays FAIL with its own `failed_at`. If the roll up itself fails, the log says `spool_end=FAILED` and the console asks for `spool_home`.
- After an `x`: the operator's stop still stops every pump. The roll up is a move (it drains the tube), so it still runs; `run_keys_reset()` clears the latch for it. The emptying pump is skipped: `spool_emptied=NO;spool_empty_end=aborted`, and `aborted=x` stays on the TEST line.
- `sample_process` (Pi `sampleFunction` and the console test menu): at its three error returns the spool is rolled up and emptied (`spool_cleanup_after_error`). On success the emptying replaces the old comment (with the spool only).
- Next sample after any emptying: the tube is dry (`spool_tube_wet` false), so it dives to `max(session depth, sample depth)` before its flow check. This was already the behaviour of `spool_fill_and_rise`; nothing new was needed there.
- `SPOOL_CAL_MAX_DEV_PCT` comment: no longer a placeholder; 6 % is the limit set on the 2026-11-02 bench data (worst stop 2.6 %).

**Docs:** tutorial sections 2c (step 6 covers failed samples), 3 (the move-failure row, the emptying row, the `x` row), 4 (end of day). `commands.md`: the `run_sample` row.

**Audit: each request of the session, and where it is implemented**

| Request | Status | Where / evidence |
|---|---|---|
| Ethanol cleaning 0.1 L per slot; filters-out question | done | `CLEAN_SLOT_ML`, `clean_ask_yes_no` in `maintenance.cpp` |
| Ethanol and purge at 100 % (exception) | done | `set_full_power`, `PUMP_PURGE_MAX_POWER_PERCENT` |
| Session: deepest depth, ENTER = 40 m, kept for the session | done | `ask_session_spool_max_depth`, `spool_session_max_m` |
| Spool off: no depth question, no depth kept | done | `cmd_session` (the question only when `spool=y`) |
| Fill: dry tube to session depth, flow check (100 mL) with the pump, then roll up with the pump off and Vman shut | done | `spool_fill_and_rise`, `spool_prime_flow`, `SPOOL_PRIME_FLOW_ML` |
| No roll up with the pump running; no 50 cm steps; no 5 mL check | done (removed) | `spool_rise_*` removed; grep finds none |
| Warning to the operator at the start of a schedule and of a single sample for a depth beyond the session or the 40 m total | done | `schedule_command` (CONFLICT first), `cmd_run_sample` (first check) |
| Total tube (depth + height) never beyond 40 m; cut and asked | done | `spool_reachable_cm`, `run_sample` cut, fill cut |
| Height reported to the PC for the check | done | `META|spool;height_cm=` (`spool_status`, `spool_height`) |
| Speed of the main move not changed | verified | git diff against baseline: no change to `SPEED_UP`, `SPEED_DOWN`, `SPOOL_SLOW_SPEED`, `SPOOL_HOME_SPEED` |
| Same timepoint: no emptying between samples; kept full | done | keep branch in `run_full_cycle`; the tool answers only for the same date and time |
| Deepest first at the same timepoint, upper last | done | `resolve()` sort key; test in `test_sample_schedule.py` |
| Emptying at the end of each timepoint (before the next one) | done | `spool_roll_up_and_empty` after the roll up |
| Failed sample emptied; next sample refills at session depth | done (this entry) | `run_full_cycle` failure branch; `sample_process` |
| Valve (Vman) shut while the manifold turns, only with the spool | done | `goto_slot_locked` |
| Valve open only while water is pumped through the tube | verified in code | list in the 2026-11-02 verification entry |
| Logs in real operational order | verified by simulation | test `test_the_schedule_runs_and_logs_in_the_operational_order` |
| Placeholders: prime volume and timeout, move limit, speed bound | done (derived, see the entries above) | `Settings.h` comments |
| Speed of the spool move: no measured value yet | open | every move logs `cm/s`; first bench move gives it |
| Tutorial: restructured, one flow, 2a one sample / 2b schedule / 2c spool; end of day at the end; temporary section removed | done | `docs/field/operator-tutorial.md` |
| Docs and LOG updated | done | this entry; the earlier entries of this session |

**Open risks (not changed here)**
- The fill dive keeps the deployment valve shut (inherited from the earlier spool code). Whether the tube fills in that state depends on the pipework, which the code does not show. If it does not fill, the prime's flow check fails and the sample is FAIL, after the failure cleanup. Check this first on the bench.
- The site height `HEIGHT_FROM_WATER` must be measured on site (`spool_height`). It is an operator measurement, not a constant.
- `PUMP_START_RATE_PERCENT_PER_S` is a pump placeholder (F196), not a spool value. Not changed here.
- `sample_process` failure returns now clean up, but its other steps are the legacy ones (no dive, no next-depth question). The field uses `run_sample` and the schedule.
- The host tests (`host/`) fail with `ModuleNotFoundError: cowas`: the host venv points at another checkout. Unrelated to this work.

**Validation (final)**
- `pio run -e due`: passes. `pio test -e native`: 200 passed.
- `pytest tools/`: 206 passed.
- Hardware: not run, not flashed.

**Hardware checklist additions**
1. A roll up after a forced failure (for example a stalled move at depth): the TEST line has `spool_end=rolled_up_after_failure` and `spool_emptied=y`. The next sample's first line is `Spool fill: dry tube, unrolling to ... (session depth ...)`.
2. `x` during a sample at depth: `spool_end=rolled_up_after_x;spool_emptied=NO` and `aborted=x`. No pump runs after the `x` (watch the pressure lines stop).

### Session 2026-11-02: rename planned (not done)

**User request:** note in the stage 2 roadmap (§7b) to rename `actuator_home` to `init`, and the valves to `valve_air` (today `valve_1`, the air inlet V1) and `valve_water` (today `valve_manifold`, the deployment line Vman). **Not to be changed now**: a rename could break the code.

**Done:** a row in [stage2-pilots.md §7b](../roadmap/stage2-pilots.md) with the scope (console commands, help, docs, tool messages and tests, C++ identifiers, Pi/host protocol if it names them) and the check to run (grep the whole repo, then the three test suites, behaviour unchanged). No code, test or doc command name was changed.

### Session 2026-11-03: purge_angle set by cal (user)

**User input:** `cal` on slot 7 with the O-ring locked gave encoder 536.00. The user asked for `purge_angle` = **47.11** (536 × 360 / 4096 = 47.109°).

**Change:** `purge_angle` = 47.11 in [`include/Manifold.h`](../../include/Manifold.h). The comment keeps the history. The value in the file before this change was 342.51, which is the one left by D40 (the D40 entry itself says 344.27; the file had 342.51 at this point, so the log and the file disagreed before this change).

**Validation:** `pio run -e due` passes. `pio test -e native`: 200 passed. The golden tests (`test/test_manifold_geometry`) pin the geometry code, not this value, so they still pass. `tools/golden/capture_manifold_golden.cpp` keeps its own copy of 342.07, which is only used when the goldens are regenerated on purpose.

**Not done:** not flashed (the user reflashes). Not committed. The fix needs a check on the bench: `actuator_home` should report a slot aligned, and `slot7` should land on the slot 7 mark by eye. If it does not, do not repeat `cal` before checking the encoder coupling (F187, F191).

### Session 2026-11-03: manifold overshoot offsets set by hand (user)

**User change (done by hand in the editor):** the overshoot offsets were corrected after `rotate_all dry` by the user.
- `angle_offset_pos` (clockwise, [`src/Hardware/Manifold.cpp`](../../src/Hardware/Manifold.cpp)): 5.6 → **3**.
- `angle_offset_neg` (counter-clockwise, [`include/Manifold.h`](../../include/Manifold.h)): 2.5 → **3.7**.

**This step:** only the comments were updated, to record the 2026-11-03 values and keep the earlier ones. No value was changed by Claude. `pio run -e due` passes. Not flashed.

**Check on the bench:** `rotate_all dry order=fill auto` (CW and CCW errors per slot) and `actuator_home` (aligned). Record the per-direction errors here. If one direction is still off, change only that offset.

### Session 2026-11-03: measuring the water surface (spool height)

**Question (user):** how to measure the height from the spool origin to the water surface by rolling the spool and reading the value.

**Answer (procedure, no code change):** the height is the tube length out (`spool_status`, `tube out = X cm out`) when the intake touches the water surface. Steps: origin switch `PRESSED`, unroll with `spool_goto length=<m>` in steps, read X at first contact, type `spool_height X`, then check with `spool_goto 0`. Added to [operator-tutorial.md](../field/operator-tutorial.md) section 1, step 7. The reading is about ±2 cm and `spool_height` is RAM only (repeat after a reset).

### Session 2026-11-03: field work at Lexplore (Lake Geneva)

Field notes are in the new [field log](field-lexplore-geneva-2026-11-03.md). Summary:
- **Plate orthogonality** off, redone on site. Frame change in V2 (stage 2 §7b).
- **Two manifold zeros, 3 slots apart.** The code does not zero the encoder (`setZeroSPI` is not called), so the cause is physical (encoder coupling). V2: home switch and mechanical lock.
- **Pump turns on flash or reset.** D42 (24 V enable) is not driven until `setup()`. Recommended: pull-down resistor on D42, and the first lines of `setup()` to drive D42 and the DAC to 0. Not changed here (safety, awaiting a go).
- **`spool_home`** (drain) did not move; `spool_home drain=n` solved it (user). The first failure's message was not recorded.
- **Offsets:** slot offsets moved with the plate; set by hand (see the offsets entry above).

No code change in this entry. Stage 2 §7b has the three open items.

**Spool home, cause (2026-11-03):** `spool_home` (drain=y) was refused with `O-ring lock state is UNKNOWN - run actuator_home`. Drain needs the lock state known (after a reset it is UNKNOWN). Fix: `actuator_home` first, or `spool_home drain=n`. Recorded in the [field log](field-lexplore-geneva-2026-11-03.md) §4.

**Field note for later (2026-11-03):** ask the sample notes at the end of the sampling, not at the start of `run_sample`. Recorded in the [field log](field-lexplore-geneva-2026-11-03.md) §5 as open item 6. Not done.

### Session 2026-11-03: used samples at start, forced slot column

**User request:** (1) at the start of the session, say which samples are already used, so the next run starts from the next available one. (2) A column in the schedule to force a slot for a row (instead of the automatic order).

**Changes (tools only, no firmware change)**
- **Launch question** (`tools/session_log.py`, `ask_used_before_start`): every start of the tool asks "Samples already used before this start?" (for example `1,2,3`, `1-3`; ENTER = none). Those are marked used in `slots.json`, and the next `run_sample` or schedule starts from the next available one. A bad answer is asked again. Parser: `parse_slot_list()` in `tools/sample_slots.py`.
- **Forced slot column** `force_slot` (`tools/sample_schedule.py`, input column after `depth_m`): empty = automatic; a sample number 1 to 14 = the row runs on that slot. A number outside 1 to 14 stops the file with the line number, as the other columns do. Old schedule files without the column still load.
- **Automatic order skips forced slots:** `next_available(exclude=...)`. The schedule reserves the forced slots of its waiting rows (`Schedule.reserved_slots()`), so an automatic row never takes them.
- **Forced rows run on their slot:** `Schedule.pick_slot(slot)`, wired in `session_log.py` to `route_run_sample`. The device receives `run_sample <slot>`.
- **At `schedule start`:** two rows forced to the same slot are `BLOCKED`. A forced slot that is used or not loaded asks `y/n`: `y` erases it and runs the row there, `n` starts nothing. This check comes before the depth warnings and the list.
- **Saved list:** the state file gets the `force_slot` column (after `depth_m`). The `slot` column still means the slot actually used.

**Open to confirm:** "used before this start" is read as *done, skip it*. If the operator also wants the samples that were used but not properly done to run again, that is a different answer (erase them at the start).

**Validation:** `pytest tools/`: 213 passed (7 new: the slot list parser, the excluded slots, the column read/check/save, the forced row, the launch question, the used-slot question at start, the duplicate block). Firmware: no change (not rebuilt).

**Docs:** tutorial section 2b (the column, the launch question, the start order). Hardware: nothing to test on the bench for this change.

### Session 2026-11-03: start question changed to "which samples are available"

**Correction:** the previous entry's question ("which samples are already used") did not replace the count question at a new folder ("How many samples are loaded?"). Both were asked. Fixed.

**Change (tools):** one question at every start: **"Which samples are AVAILABLE?"** Answer `all`, a list (`1-14`, `4-7,9,12-14`), or ENTER for no change. The listed samples become available (also used ones); available samples not listed become used; never-loaded samples stay unavailable. A new folder starts with all available, so the first answer decides. The count question and its order confirmation are removed (`ask_loaded`, and its two tests). The next sample follows the loading order (1, 2, 3, 14, 13, ..., 4, 5, 6, 7), so after `4-7,9,12-14` the next one is 14.

**Validation:** `pytest tools/`: 214 passed. Firmware: no change.

### Session 2026-11-03: spool position reset fixed (origin interrupts)

**Problem (field, DIEL run 04):** after the 20 m sample (tube out 2090 cm) the encoder read 0 with no move. The next sample (5 m) took the "lower" branch from 0 cm and unrolled about 5.9 m more (intake about 25.9 m, not 5 m). The 5 m sample was aborted with `x` during its purge, before any sampling.

**Cause:** `ISR_emergency_stop_up` / `_down` (`src/Hardware/Motor.cpp`) zeroed the encoder on a falling edge of the origin switch pin when `spool.direction == up` and the end-stop flag was clear. `direction` stays at the last move's value after the move stops, so an idle spool could be reset by a stray edge. The reset itself is not proven to come from a particular source (no move was logged in the window; the likely source is switching noise, not confirmed).

**Fix:** a flag `spool_moving` (volatile, `static`), set in `Motor::start()` for the spool, cleared in `Motor::stop()` and by a guard at the end of `Motor::drive()`. Both interrupts act only when `spool_moving` is true. Manual panel moves and the test commands go through `start()`/`stop()` too, so their end-stop protection is unchanged.

**Validation:** `pio run -e due` passes; `pio test -e native`: 200 passed. Not flashed. **Bench check after flashing:** with the spool idle (tube out, not at the top), tap the origin switch and read `spool_status`: the `tube out` value must not change to 0. Then a sample at depth with a following shallower sample: the second sample's `Spool fill: at ... cm out` must match its depth plus the height.

**Recovery of the run:** the 5 m sample (slot 11) was aborted before sampling and is not valid as a sample (wrong depth recorded as 5.00). The 1 m sample did not run. Slots 11 and 10 are free to use; slot 12 has the 20 m sample.

### Session 2026-11-03: end of chat, index and open items

This entry closes the working session of 2026-11-02 and 2026-11-03 (spool water fill, schedule, field work at Lexplore). Each topic has its own entry above. This index says where each one is, what is in the code, and what is still open.

**Where each topic is recorded**
- Spool water fill, the 40 m total, the group emptying, failed samples: [2026-11-02 water fill](#session-2026-11-02-water-fill-of-the-spool-tube-before-each-sample), [corrections](#session-2026-11-02-corrections-to-the-water-fill-user-review), [40 m total](#session-2026-11-02-40-m-total-tube-emptying-at-the-end-of-each-timepoint), [failed samples](#session-2026-11-02-failed-samples-are-emptied-audit-of-the-whole-session). Decisions D57 to D61.
- Schedule: depth warnings at start, the operational order, `force_slot`, the start question: [warnings first](#session-2026-11-02-warnings-first-operational-order-checked), [available samples](#session-2026-11-03-start-question-changed-to-which-samples-are-available), [forced slot](#session-2026-11-03-used-samples-at-start-forced-slot-column). Decision D64.
- Manifold: [purge_angle 47.11](#session-2026-11-03-purge_angle-set-by-cal-user), [offsets by hand](#session-2026-11-03-manifold-overshoot-offsets-set-by-hand-user). Decisions D62, D63.
- Spool height: [how to measure](#session-2026-11-03-measuring-the-water-surface-spool-height). Decision D68.
- Renames planned, not done: [2026-11-02 rename](#session-2026-11-02-rename-planned-not-done). Decision D67.
- Field work: [field log](field-lexplore-geneva-2026-11-03.md) (sections 1 to 9). Findings F214 to F223.
- The position reset of 2026-11-03 and its fix: [spool position fix](#session-2026-11-03-spool-position-reset-fixed-origin-interrupts). Decision D65.

**Code changed in this chat (files)**
- Firmware: `src/Step_functions.cpp` (fill, group emptying, failed-sample cleanup, `spool_roll_up_and_empty`), `src/Tests_mech.cpp` (run_sample depth rules, 40 m total, end-of-sample flow), `src/Hardware/Motor.cpp` (`spool_moving` guard, origin interrupts), `include/Settings.h` (spool fill, move limits, placeholders resolved), `include/Manifold.h` (`purge_angle` 47.11; offset comments), `src/Hardware/Manifold.cpp` (offset comment), `src/Core/spool_fill.{h,cpp}`, `src/Core/spool_motion.{h,cpp}` (move limit).
- Tools: `tools/sample_schedule.py` (deepest first, `force_slot`, total overflow, same-timepoint keep, `reserved_slots`), `tools/session_log.py` (start questions, conflict and forced-slot checks, session and height parsing), `tools/sample_slots.py` (`set_available`, `parse_slot_list`, `next_available(exclude)`).
- Tests: native 200 passed; tools 214 passed.

**Not flashed (the user flashes):** the spool origin-interrupt fix (F214). Whether the DIEL run's firmware had the `purge_angle` 47.11 and the offsets is not recorded (see the field log §8).

**Open at the end of the chat** (numbers in the field log, the findings and the decisions):
1. Spool origin-interrupt fix: flash, then the bench check (F214).
2. Fill dive keeps Vman shut: does the tube fill (F219).
3. Purge at 100 % and the spool prime at 60 %: print the applied power (F221).
4. Pump turns on flash or reset: pull-down on D42, and the first lines of `setup()` (F215).
5. Two manifold zeros, 3 slots apart, and the plate orthogonality: V2 frame and home switch (F216, F217).
6. Spool height in RAM only (F223). Sample notes at the end of sampling (F222).
7. Recovery of the aborted schedule: field log §9. The 5 m record of run 04 is invalid.

### Session 2026-11-03: handover and the field run's new bugs

**Requested:** keep track of the last questions and my fixes in the log; the next chat must be able to see what was done; the user is not convinced by some of it; document how to restart the schedule.

**Done:**
- HANDOVER.md (removed 2026-11-03; its content is in the [field log §12](field-lexplore-geneva-2026-11-03.md#12-restart-order-and-what-not-to-do-moved-here-from-the-handover-2026-11-03)) written: the physical state, the field run table, the status of each question, my mistakes, the open bugs, the restart order (steps A to G), and questions for the user.
- Findings F224 (move-to-origin time limit uses the encoder distance) and F225 (the schedule continues after an `x` or a failed roll-up) added as open, High.
- Field log §7 extended with the post-abort timeline (16:44 to 16:49). The 1 m row and the spool_home attempts are FAIL; the tube position is unknown.
- The main LOG and F214 wording: "fixed in code" means built only. The bench check is still to do.

**Not done:** no code change in this step. F224 and F225 are not fixed. F214 is built but not flashed.

**Correction to earlier entries:** the 2026-11-03 entries describe the spool-position fix (F214) as the cause. That is the likely cause, not a proven one. The field log says so in §7.

### Session 2026-11-02 and 2026-11-03: record of the chat (questions, answers, where documented)

Every question of the working chat, in order, with what was answered or changed and the entry that holds it. Headings are the entries above. "Answer only" means no code or doc change was asked.

| # | Question or request (user) | What was done | Where |
|---|---|---|---|
| 1 | Ethanol purge does not flow with the spool at 0.6 m; is the pump at 100 %?; 0.1 L per slot | Found the purge limit path (100 % is the ceiling); 0.1 L per slot set; prime at 60 % noted | `Session 2026-11-02: ethanol_purge 0.1 L per slot` |
| 2 | The pump does not push through the coiled empty tube. Fill the tube at the session depth first; ask the session depth (40 m default) | Fill, flow check and rise designed and built; corrections after review (no pump during the rise, 100 mL, same-timepoint rule) | `water fill of the spool tube before each sample`; `corrections to the water fill` |
| 3 | Verify the whole chat and triple-check the logic | Audit: height bug, speed, accepted conflicts; fixes and the known risks | `verification before the field (corrections)` |
| 4 | Restructure the tutorial; solve the placeholders; schedule concise | Tutorial restructured; prime and move-limit values derived | `schedule depth check, values resolved, tutorial restructured` |
| 5 | 40 m total tube; warning before the schedule; speed not to change; group emptying at the end of a timepoint | Total limit with cut and question; group emptying; speeds checked unchanged | `40 m total tube, emptying at the end of each timepoint` |
| 6 | Warnings first at launch; logs in the real run order | Warnings moved before the list; order tested | `warnings first, operational order checked` |
| 7 | Failed samples: empty the spool too; the next sample refills at the session depth | Cleanup after a failure; `sample_process` covered; audit table | `failed samples are emptied; audit of the whole session` |
| 8 | Note the renames for later (`actuator_home` to `init`, valves to `valve_air` and `valve_water`) | Note in stage 2 §7b only; nothing renamed | `rename planned (not done)` |
| 9 | Commit to git | Commit `005ec82` on `shield_V2` | git history (commit message of that commit) |
| 10 | How to use `session_log.cmd`; example for 3 Nov | Answer; the tutorial has the command | tutorial §1 step 2 |
| 11 | "Why did it not work?" COM6 access denied; `actuator_home` typed in PowerShell | Cause: two PlatformIO `device monitor` processes held COM6; the command went to PowerShell. Not logged before this entry | this table; tutorial troubleshooting table |
| 12 | The manifold was lost before (3 slots); what was done | Searched the log: F187 (loose encoder), F191 (3 slots), D35, D40 | `Session 2026-10-23 (late): slot offset, F191` (existing) |
| 13 | Plate orthogonality off; two zeros 3 slots apart; pump turns on flash or reset; slot offsets moved | Field log §1 to §3; findings F215 to F217; the zero cause is a hypothesis | `field work at Lexplore`; field log §1 to §3 |
| 14 | `spool_home` did not move | The refusal: O-ring state UNKNOWN; `drain=n` worked | `field work at Lexplore`; F218; field log §4 |
| 15 | `purge_angle`: encoder 536 gives 47.11 | Set in `Manifold.h`; built | `purge_angle set by cal (user)` |
| 16 | Offsets: clockwise and counter-clockwise are off; can I adjust them? | Answered (constants, reflash). The user then set 3 and 3.7 by hand; comments updated | `manifold overshoot offsets set by hand (user)` |
| 17 | How to measure the surface height by rolling the spool | Procedure in the tutorial §1 step 7 | `measuring the water surface (spool height)` |
| 18 | The command to set the water height | `spool_height <cm>`; RAM only; answered | tutorial §1 step 7; this table |
| 19 | Used samples at the start; a column to force the slot; then "which samples are available" | First attempt kept the count question (not what was asked); second attempt: one question with a list; `force_slot` column | `used samples at start, forced slot column`; `start question changed to "which samples are available"` |
| 20 | Where to put the schedule file; can it have any name | Answer: project folder, any `.csv` name, `--schedule <name>`; the first start copies it into the run folder | this table; tutorial §2b |
| 21 | 20 m then 5 m: where does the spool end? | The first answer ("no bug") was **wrong**. The log showed a position reset to 0; the 5 m run went about 5.9 m too deep | `spool position reset fixed (origin interrupts)`; F214; field log §6 and §7 |
| 22 | Fix the position loss now; how to finish the schedule | Interrupt guard built (not flashed). Recovery steps written (field log §9, handover §6) | `spool position reset fixed`; HANDOVER |
| 23 | Could `spool_home` run faster (a parameter)? | Answer: possible for an unknown position only; the homed roll-up is already at 100 %. Not implemented | this table |
| 24 | A test sample at 1 m without the DNA shield | Answer: `full_cycle ... shield=n` (tests). The user then chose `run_sample 7 depth=1` (with shield) as the normal path | this table |
| 25 | Is the pump at 100 % for the purge? | Answer: the ceiling is 100 %; the 3 bar controller limits it; the prime is at 60 %. Not printed. Not changed | F221 |
| 26 | Drag a schedule file? Order of commands? | Answer: pass `--schedule` at start, then `session`, then `schedule start` | this table; tutorial §2b |
| 27 | Field note: notes at the end of the sampling (later) | Deferred | field log §5; F222; D66 |
| 28 | Keep track of the last questions in the log; a new chat must see them; document the restart | Handover written; F224 and F225 found and recorded; this record | `HANDOVER.md`; `handover and the field run's new bugs` |
| 29 | The tube is at the top now | Recorded in the handover; the switch state still to confirm | the field log section 12 |

**Corrections to earlier entries in this record**
- Row 21: the first answer ("no bug") was wrong. The fix is in code, not flashed.
- Row 11: the COM6 failure was not in the log before this entry.
- Speeds: earlier entries said the speed was unmeasured. The field log now shows 31 to 37 cm/s at full speed (F220).

### Session 2026-11-03: origin-switch edge at idle (position 0 with no move), code fix

**Problem (field, DIEL run 04):** the spool position read 0 at 16:36:51 with the tube at 2090 cm out. No move was logged in that window and nobody pressed the switch.

**Cause (code):** the top-switch ISR (`ISR_emergency_stop_up`, `src/Hardware/Motor.cpp`) zeroed the encoder on any falling edge of D28 while `spool.direction == up` (left over from the last up move, the 4001 to 2090 cm rise). It did not check that a move was running, nor that the pin was still LOW. The only other encoder resets (`drive()` at lines 196 and 303) need a move, so they are excluded. The source of the edge itself is not known (electrical glitch on the line, or cable): nothing in the code drives D28 (checked: input only, no output on pin 28).

**Change (firmware, not flashed):**
- The ISR acts only while a move runs (`spool_moving`, from the previous fix). While idle it counts the edge and returns.
- During a move it also requires `digitalRead(BUTTON_SPOOL_UP) == LOW`, so a glitch that is already back HIGH does not zero the encoder.
- New counter: `spool_idle_edges_seen()` (`include/Motor.h`), printed by `spool_status` as "origin-switch edges ignored while idle since boot".
- The `SpoolMovingGuard` struct now sits above the `drive()` doc comment (it had split it).

**Tests:** `pio run -e due` passes; `pio test -e native`: 200 passed. Not flashed, no bench check yet.

**Bench check (after flash):** with the spool idle, `spool_status` must show the same tube length before and after a few taps of the switch. A count above 0 after taps is expected (the counter); a count that rises with nothing touched means the line glitches: check the cable and the switch wiring (D28), and add a pull-up or RC filter there.

**Not changed:** the 300 ms coast after a move: the flag is cleared by `stop()` before the coast, so a switch touch in that window is still ignored (a few cm error). Open, low.

### Session 2026-11-03: review of the spool position fixes (robustness)

**Review request:** check that the changes are robust and answer the request (after the 20 m sample, the next shallower sample goes straight to its depth; no lost position).

**Checked, no change:**
- Order: same-time rows run deepest first (`resolve()` in `tools/sample_schedule.py`). The recovery CSV of [field log §9](../../logs/field-lexplore-geneva-2026-11-03.md) loads in that order (20 m, 5 m on slot 11, 1 m on slot 10).
- Next depth: the 20 m row answers the device's next-depth question with 5.00 m (same timepoint). The fill then rises from the kept length to 590 cm (`spool_fill_and_rise`, wet tube, target shallower: no dive). The logic was right; the position was wrong.
- Button: a press during an up move is still seen (interrupt with LOW check, and the move loop's debounced `getState()` check). Cost: up to about 50 ms of travel after an interrupt miss.

**Added (firmware, not flashed):**
- **Kept-length check:** `spool_kept_length_cm` is stored when the tube is kept out (`Tests_mech.cpp`). `spool_fill_and_rise()` refuses with an ERROR, and pumps nothing, when the encoder reads more than `SPOOL_KEPT_MOVED_CM` (5 cm) away from it. A 2090 → 0 reset would now stop the next sample instead of diving to 25.9 m.
- **Coast gap closed:** `drive()` re-arms `spool_moving` after `stop()` so the 300 ms coast counts a switch touch. The guard clears the flag on return.

**Results:** `pio run -e due` passes; `pio test -e native`: 200 passed; `pytest tools/`: 214 passed. Not flashed, not bench-checked.

**Still open:** the cause of the falling edge on D28 (cable or switch wiring: bench check, `spool_status` idle-edge count); the recovery steps in field log §9 must be done after the flash.

### Session 2026-11-03: correction, the D28 cause is not proven

**Correction:** earlier entries and chat answers said the origin-switch interrupt on D28 zeroed the position in the field. That is the only code path that can zero the encoder while idle (checked: every `encoder.reset()`, every use of pin 28, every `start()`/`stop()` pair). It is **not proven**: no press, no edge and no move were logged, so what made D28 fall is unknown (cable, switch wiring or electrical noise). F214 now says so. D70 is proposed by Claude and is not yet confirmed by the user.

**Process mistakes in this session (recorded, not repeated):**
- My first search for the encoder resets skipped `src/Step_functions.cpp` (grep reported it as a binary file). Found later by searching as text.
- A first batch of edits, run through `python3`, did not apply (no Python on this name). The build reported at that point was on unchanged code. Re-run after the edits applied.
- I said "sure" about the switch cause before the evidence supported it.

**Every change made in this session (firmware not flashed; tests: `pio run -e due` passes, `pio test -e native` 200 passed, `pytest tools/` 214 passed):**
- `src/Hardware/Motor.cpp`: the origin-switch interrupt acts only while a move runs (`spool_moving`), only when the pin reads LOW, and counts idle edges. `drive()` re-arms the flag for the 300 ms coast. The `SpoolMovingGuard` doc comment is no longer split.
- `include/Motor.h`: `spool_idle_edges_seen()`.
- `src/Tests_mech.cpp`: `spool_status` prints the idle-edge count; a kept-out tube stores `spool_kept_length_cm`.
- `src/Step_functions.cpp`, `include/Step_functions.h`: `spool_fill_and_rise()` refuses, with nothing pumped, when a kept-out tube reads more than 5 cm away from where it was kept (F226).
- Findings: F214 reworded (the cause is not proven); F226, F227 added. Decisions: D69, D70 added.

**Field notes from the user (not implemented, see field log §11):** F228 (restart of an interrupted schedule) and F229 (erase prompt destroys the record).

### Session 2026-11-03: field day, Felix, DIEL and the night run

**Runs (detail and volumes: [field log §13](field-lexplore-geneva-2026-11-03.md#13-day-log-2026-11-03-felix-diel-and-the-night-run)):** Felix 3 samples and 1 blank PASS, 1 partial stopped by `x` (folder 03). DIEL: 20 m PASS (schedule, valid); 5 m and 1 m failed in the first attempts (`x` in the purge, the spool position reset, F214, F224, F225); 5 m redone by hand at 17:13 (the sample is valid, 83 mL clogged; the run is FAIL at the roll-up, a timeout after a reading of 0); 1 m by hand at 17:37 (PASS, clogged at 515 mL). Night plan: `tools/DIEL-schedule02.csv`, 12 samples, not started yet.

**Changes made (not flashed unless stated):**
- `src/Hardware/Motor.cpp`, `include/Settings.h`: every move to the origin switch has a fixed limit, `SPOOL_HOME_MAX_MS` = 20 min (F224, D72), kept for stage 1; to revise (stage 2 §7b). **Needs a flash.**
- `tools/sample_schedule.py`: the schedule stops on `spool_end=FAILED` and on any refused sample (F225, D71). Tool only: restart the tool.
- `tools/tests/test_sample_schedule.py`: two tests for the stop rules.
- Findings: F214 reopened (17:18 to 17:34, reading 0 with no move); F224 and F225 status; F230 (USB drop 13:00); F231 closed (slot 1 was test only). Decisions: D71, D72, D73 (night run accepted by the user, with the risks listed).
- Field log: §13 (day log, runs, night plan, open items); §9 marked superseded; §12 step B updated.

**Tests:** `pio run -e due` builds; `pio test -e native` 200 pass; `pytest tools/` 216 pass.

**Hardware checks not done:** the 20 min limit on the board; `spool_status` idle-edge count; the reading at rest.

**Open:** F214 (what zeroes the reading while idle), F225 (`x` does not stop the list), F230 (USB), F224 (20 min kept for stage 1, to revise in stage 2 §7b), F215 (pump on reset: the hardware pull-down on D42 is not confirmed), the build on the board (not recorded; purge_angle 47.11 and the offsets in field log §8 to confirm).

### Session 2026-11-03: schedule stops on x and restarts at a row

**Why (user, F225):** after an `x` the list ran on to the next sample. The operator needs to stop, solve the problem, and restart at the sample that failed.

**Changes (tool only, no flash needed; restart the tool):**
- `tools/sample_schedule.py`: an `x` (`aborted=x` on the TEST line) stops the list, like a failed roll-up (`spool_end=FAILED`) and a refusal. No question is asked: the list stops and says why. New status `skipped`. `restart_rows(samples, row)` builds the list for a restart (copies): rows before the row that waited become `skipped`; the row and every later row that did not pass wait again; PASS rows stay. `Schedule.start` and `preview` take that list.
- `tools/session_log.py`: `schedule` numbers the rows. `schedule start <row>` shows the list it will run, then asks `y/n` as before. A bad row number is refused.
- Tests: `pytest tools/` 220 pass (new: x stops the list, restart rules, a re-run row is cleared, `schedule start <row>` changes nothing until `y`).
- Tutorial §2b: the list stops by itself after `x`, a failed roll-up or a refusal; the restart steps; the command table.
- Findings F225 fixed in code (tool). Decision D74.

**Not changed:** the firmware. The 20 min origin limit (F224, D72) still needs a flash, as before.

**Known limits:** a re-run takes the next available filter (a failed filter stays used). The restart does not check the tube: the operator checks `spool_status` first (tutorial §2b). `ruff check` reports one old warning in `session_log.py` (SIM115, line 215, not in this change).

**Addendum (same session): filters and the schedule file.**
- **Filter release (D75):** a run that stops before the filter is loaded (`failed_at=dive` or `failed_at=purge`) frees its slot again (`SlotBook.release` in `tools/sample_slots.py`, called from `on_device_line` in `tools/session_log.py`). The restart can use that filter. A run stopped from `sampling` on keeps it used. Test: `test_a_run_stopped_before_its_filter_was_loaded_frees_the_slot`.
- **Schedule file:** unchanged. The input CSV is only read at the first start; the list of the run (with the status of each row) is saved in the run folder's `schedule.csv`. Older state files load as before; the new status `skipped` only appears after `schedule start <row>`.
- `pytest tools/`: 221 pass.
- Tutorial §2b updated (filters, the schedule file).
- Flash: the firmware change of this session is still only F224 (`SPOOL_HOME_MAX_MS`, 20 min, kept for stage 1). Flashing resets the Due: the spool position is then unknown until `spool_home`, and `spool_height` must be typed again.

### Session 2026-11-03: purge_angle second shift (340.75)

**What happened (user):** the manifold was 3 slots clockwise, the other way from the morning shift. The user redid `cal` on slot 7 with the O-ring locked: encoder 3877.00, `purge_angle` 340.75.

**Check:** 3877 × 360 / 4096 = 340.75°. From the morning value 47.11° (encoder 536) the step is 340.75 − 47.11 = −66.4°, about 3 slots (67.5°) within 1.1°. The value is the user's `cal` result, taken as it is.

**Change:** `purge_angle` = 340.75 in [`include/Manifold.h`](../../include/Manifold.h). The comment keeps the history (47.11, 342.51 and the earlier values). The offsets are unchanged (`angle_offset_pos` = 3, `angle_offset_neg` = 3.7), as the user asked.

**Tests:** `pio run -e due` builds; `pio test -e native` 200 pass. The manifold goldens pin their own reference (342.07 in `tools/golden/`), so they do not depend on this value.

**Not checked:** the slots by eye (`slot7`, then one `run_sample` on a known slot) after the flash; `encoder_repeat`. The cause (encoder slip or rotor shift, F216) is still open. Two shifts in one day point to the coupling; the mechanical check is in the field log §2.

**Flash:** needed for this value, as for F224. Flashing resets the Due: after it, `actuator_home`, `spool_status`, and `spool_height` again.

**Logs:** [decisions D76](decisions.md), [F216](findings.md), [field log §8](field-lexplore-geneva-2026-11-03.md#8-settings-changed-by-hand-today).

### Session 2026-11-03: manifold shifts tracked, early fix in the roadmap

**Issue (user):** the manifold shifted 3 slots twice on 2026-11-03 (morning, `purge_angle` 47.11; later, 3 slots clockwise, `purge_angle` 340.75; D76). Offsets unchanged.

**Logs updated:**
- [field log §14](field-lexplore-geneva-2026-11-03.md#14-manifold-shifts-issue-log-open-early-fix-in-the-roadmap): the issue, the `purge_angle` history table from 2026-10-12 to today, what is known and not known, and what to log at each shift. The §8 settings table shows both 2026-11-03 values.
- [F216](findings.md): the second shift and the early fix.
- [D77](decisions.md): the coupling is an early fix.

**Roadmap:**
- [stage 2 HW9](../roadmap/stage2-pilots.md): marked **EARLY FIX, before the next field run**, no dependency on HW6. §7.4 gives the fix steps. §7b row points to it.
- [stage 1 "Left to do" 7](../roadmap/stage1-FINISHED-prototype.md): the same item for the stage 1 list.

**Not done:** no hardware change; the coupling lock is a mechanical job for the user. The cause is not known yet (encoder slip or rotor shift).

### Session 2026-11-03: false origin press, fail-safe, recovery

**Trigger (field, 21:22):** the 1 m sample's rise hit the origin switch before the goal (`origin switch reached before the goal`). The spool was then down, not up. The user believes the up button was pressed by accident (not by the team). The program must not lose the zero on a press it cannot trust.

**Cause (code):** the origin interrupt `ISR_emergency_stop_up` braked the motor and zeroed the encoder on any raw edge during a move, with no debounce and no check of the length. The button itself is debounced (50 ms), but the interrupt was not. F232.

**Changes (firmware, built, not flashed):**
- `src/Hardware/Motor.cpp`, `ISR_emergency_stop_up`: does nothing during a move. Idle edges are still counted (`spool_status`).
- `Motor::drive`: the move loop takes a press only when the button reads held (debounced) and `core::spool_origin_press_real(homing, length, SPOOL_ORIGIN_TOL_CM)`: within 30 cm of the zero, or homing from an unknown position. Otherwise: a WARNING with the length, and the move goes on (the stall rule still stops a stuck spool).
- A stall with the switch held, going up, is the origin: the drum is at the top, so the zero is reset there (position lost).
- A move to the origin from an unknown position has a 30 min limit (`SPOOL_HOME_UNKNOWN_MAX_MS`, D79). From a known position it stays 20 min (`SPOOL_HOME_MAX_MS`).
- The start check: a switch held while the encoder says the tube is out now says "release the switch, or check the wiring".
- `spool_height` is not touched by any press (the height is the typed value; the zero is the encoder).
- `src/Core/spool_motion.h/.cpp`: `spool_origin_press_real`. `test/test_spool_motion/`: a test for it.

**Tests:** `pio run -e due` builds; `pio test -e native` 201 pass. `pytest tools/` not rerun (no tool change).

**Not done:** bench check on the spool (idle tap, a press during a move). Residual risk: a false press during a homing move (unknown position) is still taken as the zero.

**Recovery steps:** [field log §15](field-lexplore-geneva-2026-11-03.md#15-false-origin-press-the-spool-state-and-the-steps-to-continue). Decision D80: the failed sample is retried from scratch.

**Review of the false-press change (same session, before the flash):**
- Fixed: the start check of a move (switch held, tube reads out) took the switch as the zero up to 90 cm (3 × the slow zone). It now uses the same 30 cm rule as a press during a move. Without this, a stuck switch could set a zero 60–90 cm wrong.
- Checked, no change: the down end-stop interrupt is not attached (`main.cpp`, commented out), so only the debounced loop acts on it. The spool motor has no "MANIFOLD" ID, so the move guard applies to it.
- `pio run -e due` passes; `pio test -e native` 201 pass.

### Session 2026-11-03: retry before the filter, and homing confirmation

**Retry (tool, `tools/sample_schedule.py`, D81):** a FAIL with `failed_at=dive` or `purge` (the filter was not loaded), the spool not left out (`spool_end` not `FAILED`) and no `x`, sends the same row again, on the next free filter (the released one first). Up to `RETRY_MAX` = 2 tries; then the list stops. The run loop is now index-based so a row can repeat (`_run`); `_end` decides. Tests run the real loop: a retry twice then stop; an `x` never retried; a spool left out never retried; a failure after the sampling goes on as before (`pytest tools/`: 225 pass).

**Late rows:** a row whose time has passed runs as soon as the device is idle (`_wait_for_time`). `schedule start 3` therefore runs the 1 m row (planned 20:24) at once, then waits for the 23:50 group, then 04:00 and 07:00. Only row 3 is late.

**Homing confirmation (firmware, `src/Hardware/Motor.cpp`, D82):** after a press is taken (move loop or a stall with the switch held), the switch must still read held after the spool has stopped. If released, the move fails with "press not confirmed", and the zero is not changed. Built; `pio run -e due` passes, `pio test -e native` 201 pass. Not flashed.

**Not changed (user's decisions):** the pump on reset (F215: no change, hardware pull-down not done); height check, stall time, and stop-on-any-FAIL were not implemented.

### Session 2026-11-03: retry on the same filter, 300 ms hold

**Retry (tool, D81 amended):** a failed try before the filter was loaded is repeated on the **same** filter (the slot of the failed try, kept in `Schedule.retry_slot`), not on the next free one. Two tries at most. After that the row stays FAIL, its filter stays free, and **the list goes on** with the next row at its own time. The `x` and "spool left out" stops are unchanged. Tests run the real loop (`tools/tests/test_sample_schedule.py`, "a failure before the filter"): same filter three times, then the next row; `pytest tools/`: 226 pass.

**Hold (firmware, D83, not flashed):** with the top switch held, the spool stops when it has counted no pulse for `SPOOL_HOLD_STOP_MS` (300 ms), so the push against the top lasts at most about 0.35 s. The earlier 4 s stall branch for a held switch is removed. A press within 30 cm of the zero is still taken at once, and the zero still needs the switch to be held after the stop (D82). `pio run -e due` builds; `pio test -e native` 201 pass.

**Still open:** a bench check of the press, the hold and the homing (F232); the firmware is not flashed.

### Session 2026-11-03: review of the night run logic

**Checked (code re-read, not bench-tested):**
- Scheduler: the run loop repeats a row only on a retry; the next row starts only when the row is finished; late rows run at once; `schedule start <row>` marks earlier waiting rows as skipped; the retry uses the same filter (`retry_slot`), is limited to 2, and never follows an `x` or a spool left out. After the tries are used up the row stays FAIL and the list goes on. `retry_slot` is cleared when a row ends.
- Filter release (D75) happens on the TEST line before the scheduler sees it (reader order in `session_log.py`), so the retry finds its filter free.
- Firmware move: a press is taken only within 30 cm of the zero, or while homing, or when the switch is held and the drum has counted no pulse for 300 ms. The zero is set only if the switch is still held after the stop. The start check uses the same 30 cm rule. The interrupt does nothing during a move.
- Retries happen on dive failures (stall, timeout, wrong way, origin before the goal, prime with no flow, a kept-out mismatch). The spool is rolled up first (`rolled_up_after_failure`), so a retry dives from the origin.

**Corrections found in this review:**
- `failed_at=purge` only happens with an `x` (the purge returns false only on abort), so the retry on the purge is in practice an `x` case, which is excluded. Harmless, but not what the decision text suggested.
- `RETRY_MAX` comment and the stale retry entries are fixed.

**Open risk (F233):** a purge that stops on the pressure cap goes on to the sampling. Not changed: it needs a decision and a flash.

**Tests:** `pytest tools/` 226 pass; `ruff check` on the scheduler passes. `pio run -e due` and `pio test -e native` (201) passed on the last firmware change.

### Session 2026-11-03: change register (all changes of the day, board state)

**Board state.** Flashed by the user after the 340.75 `purge_angle` change (the flash came after the 20 min origin limit): the origin interrupt fix (F214), the kept-out length check (F226), the coast fix (F227), `SPOOL_HOME_MAX_MS` = 20 min (F224), `purge_angle` 340.75 (D76). **Not on the board:** everything in the table marked "not flashed". A reset or flash resets the spool position and the height (`spool_height` is RAM only).

| # | Area | Change | Files | Board | Decision / finding | Detail |
|---|---|---|---|---|---|---|
| 1 | Spool origin interrupt | Acts only during a move; idle edges counted (`spool_status`) | `src/Hardware/Motor.cpp`, `include/Motor.h`, `src/Tests_mech.cpp` | flashed | D65, D70, F214 | [spool position fix](#session-2026-11-03-spool-position-reset-fixed-origin-interrupts) |
| 2 | Kept-out tube | A fill refuses if the encoder moved more than 5 cm from where the tube was kept | `src/Step_functions.cpp`, `src/Tests_mech.cpp` | flashed | D69, F226 | [review](#session-2026-11-03-review-of-the-spool-position-fixes-robustness) |
| 3 | Coast | The switch is re-armed during the 300 ms coast | `src/Hardware/Motor.cpp` | flashed | F227 | same |
| 4 | Origin move limit | Fixed 20 min for a move to the origin from a known position (`SPOOL_HOME_MAX_MS`) | `include/Settings.h`, `src/Hardware/Motor.cpp` | flashed | D72, F224 | [false press entry](#session-2026-11-03-false-origin-press-fail-safe-recovery) |
| 5 | Manifold | `purge_angle` = 340.75 (cal on slot 7, encoder 3877) | `include/Manifold.h` | flashed | D76, F216 | [manifold issue](field-lexplore-geneva-2026-11-03.md#14-manifold-shifts-issue-log-open-early-fix-in-the-roadmap) |
| 6 | Origin switch, false press | The move loop accepts a press only within 30 cm of the zero, or when homing; elsewhere a WARNING and the move goes on. A switch held with no pulse for 300 ms is the origin. The interrupt does nothing during a move | `src/Hardware/Motor.cpp`, `src/Core/spool_motion.h/.cpp`, `include/Settings.h` (`SPOOL_ORIGIN_TOL_CM`, `SPOOL_HOLD_STOP_MS`) | **not flashed** | D78, D83, F232 | [false press](#session-2026-11-03-false-origin-press-fail-safe-recovery), [hold](#session-2026-11-03-retry-on-the-same-filter-300-ms-hold) |
| 7 | Origin, start check | A move that starts with the switch held uses the same 30 cm rule | `src/Hardware/Motor.cpp` | **not flashed** | F232 | [review](#session-2026-11-03-review-of-the-night-run-logic) |
| 8 | Origin, confirmation | **Removed** (D82): it made the first `spool_home` fail in the field (22:10). The press is taken at once again | `src/Hardware/Motor.cpp` (the code is gone) | removed | D82 | [confirmation removed](#session-2026-11-03-confirmation-removed-d82) |
| 9 | Homing from an unknown position | 30 min limit (`SPOOL_HOME_UNKNOWN_MAX_MS`); 20 min stays for a known position | `include/Settings.h`, `src/Hardware/Motor.cpp` | **not flashed** | D79 | [false press](#session-2026-11-03-false-origin-press-fail-safe-recovery) |
| 10 | Tests (firmware) | `spool_origin_press_real` (`test_origin_press_only_where_the_origin_can_be`) | `test/test_spool_motion/test_main.cpp` | — | — | — |
| 11 | Scheduler: stops | Stop on `spool_end=FAILED`, on any refusal, on any `x`; the message says how to restart | `tools/sample_schedule.py` `_end` | tool (restart the tool) | D71, D74, F225 | [x and restart](#session-2026-11-03-schedule-stops-on-x-and-restarts-at-a-row) |
| 12 | Scheduler: restart at a row | `schedule start <row>`; rows are numbered; `restart_rows` skips earlier waiting rows and keeps PASS rows; `skipped` status | `tools/sample_schedule.py`, `tools/session_log.py` | tool | D74 | same |
| 13 | Filter release | A run stopped before the filter was loaded (`failed_at=dive` or `purge`) frees its slot (`SlotBook.release`) | `tools/sample_slots.py`, `tools/session_log.py` | tool | D75 | [addendum](#session-2026-11-03-schedule-stops-on-x-and-restarts-at-a-row) |
| 14 | Retry | A FAIL before the filter was loaded is repeated on the **same** filter, twice; then the row stays FAIL and the list goes on with the next row at its time | `tools/sample_schedule.py` (`_run`, `_end`, `_retry_wanted`, `retry_slot`, `RETRY_MAX`) | tool | D81 (amended) | [retry on the same filter](#session-2026-11-03-retry-on-the-same-filter-300-ms-hold) |
| 15 | Late rows | Rows whose time has passed run at once (existing behaviour, checked) | — | tool | — | [review](#session-2026-11-03-review-of-the-night-run-logic) |
| 16 | Tutorial | §2b (stops, restart, filters, retry), §3 (warnings, lost position, spool_home), command table | `docs/field/operator-tutorial.md` | docs | — | — |
| 17 | Roadmap | HW9 early fix (stage 2 §7.4, stage 1 "Left to do" 7); §7b rows for 20 min / 30 min limits and the two zeros | `docs/roadmap/stage2-pilots.md`, `docs/roadmap/stage1-FINISHED-prototype.md` | docs | D77 | [manifold issue](field-lexplore-geneva-2026-11-03.md#14-manifold-shifts-issue-log-open-early-fix-in-the-roadmap) |

**Not changed on purpose (user decisions):** the pump on reset (F215: the 24 V relay can close before `setup()`; needs a hardware pull-down and a test), a height check after a reset, stop-on-any-FAIL, the stall time, the down end-stop interrupt (not attached, `main.cpp`).

**Open:** F233 (a purge stopped by the pressure cap goes on to the sampling; proposed fix needs a flash), F232 bench check (press, hold, homing), F214 bench check (idle tap, idle-edge count), F230 (USB drop), the 20 min and 30 min limits to revise (stage 2 §7b).

**Tests at the end of the day:** `pio run -e due` builds; `pio test -e native` 201 pass; `pytest tools/` 226 pass; `ruff check` on the scheduler passes.

**Superseded entries:** the "stall with the switch held" wording in [the false press entry](#session-2026-11-03-false-origin-press-fail-safe-recovery) (replaced by the 300 ms rule, D83) and the 4 s stall rule in F232 and D78 (corrected in place).

### Session 2026-11-03: confirmation removed (D82)

**Field result (22:10):** the first `spool_home` after the reflash failed with `origin switch press not confirmed`. The drum had rolled up from an unknown position, the switch was pressed during the roll, and the switch was not held once the spool had stopped. The zero was therefore never set (correct for the rule), and the homing failed. The schedule started row 3, the dive failed the same way, and the list stopped on `spool_end=FAILED`. Nothing was pumped.

**Change (user decision, 2026-11-03):** the confirmation after the stop is removed from `src/Hardware/Motor.cpp` (`drive()`). The press is taken at once again, as before, with the other rules kept:
- the origin press counts only within 30 cm of the zero, or when homing (D78, F232);
- a switch held with no encoder pulse for 300 ms is the top (D83, `SPOOL_HOLD_STOP_MS`);
- the start check uses the 30 cm rule;
- the interrupt does nothing during a move, and idle presses are counted only;
- the 30 min limit for a homing move from an unknown position (D79) and the 20 min limit from a known one (F224) are kept.

**Note for the bench check (F232):** the 300 ms hold is now the only check against a false zero at the top. The field result suggests the switch is not held once the drum stops, so the bench check must show where the switch sits at rest.

**Checks:** `pio run -e due` builds; `pio test -e native` 201 pass. Not flashed: the firmware needs a flash for this change.

**Documents updated:** D82 (removed), F232 (confirmation removed), the change register row 8, the tutorial's warning rows, the roadmap §7b row, and the field log §16 addendum.

### Session 2026-11-03: record of the chat (second half), handover

Continues [the record of 2026-11-02 and 2026-11-03](#session-2026-11-02-and-2026-11-03-record-of-the-chat-questions-answers-where-documented). The chat ended here so the user can flash and continue in a new chat. Everything below is in the logs; this entry only lists it.

**Questions and results, in order**
1. **Schedule file and loaded times:** the tool loaded `tools/DIEL-schedule02.csv` at 20:01:43 (first group 20:10). It was edited to 20:24 later. Answer: `schedule start` never re-reads the CSV; only a tool restart on a not-started list does. (Field log §16.)
2. **20 m failure at 20:10:** the prime counted 4 mL in about 2 min. The operator's `x` at 20:13:50 stopped it. The retry at 20:24 primed 101 mL and the sample passed (1001 mL, 835 trusted). (Field log §16.)
3. **Filter rule:** a run stopped before the filter is loaded frees its slot (D75). Restart at a row keeps PASS rows and skips earlier waiting rows (D74). The schedule file is not changed by a restart.
4. **`x` during a scheduled sample:** the list stops, with no question (D74, F225). It was built as a stop, not a question, because an unattended run cannot answer one.
5. **Manifold shifts:** 3 slots, twice on 2026-11-03. `purge_angle` 47.11, then 340.75 (D76). Shift history and the early fix HW9 are in the field log §14 and roadmap (D77).
6. **Origin switch and the 21:22 failure:** a false press during a rise reset the zero. The interrupt braked and zeroed on any raw edge (F232). Fixes built (D78, D79, D83), and the start check uses the same 30 cm rule.
7. **Retry:** a failure before the filter is loaded is retried on the same filter, twice. Then the row stays FAIL and the list goes on with the next row at its time (D81, amended). Retries cover dive failures; a purge fails only on `x` (corrected in the review entry).
8. **Hold:** a switch held with no pulse for 300 ms is the top (D83). The 4 s stall rule for a held switch is removed.
9. **Confirmation after the stop (D82):** built, then removed after the 22:10 field failure. The first `spool_home` after the reflash failed because the switch was not held once the drum stopped. The check is gone; the other changes are kept.
10. **Pump on reset (F215):** explained. No change made; the hardware pull-down needs a test first.
11. **Answers not implemented (user decisions):** a height check after a reset, stop-on-any-FAIL, the stall time, and the purge pressure rule (F233, open, needs a decision and a flash).
12. **Steps after the flash:** field log §15 (and §16 addendum for 22:10). `slot7` was confirmed working after the flash.

**Board state at handover.** Flashed before the night run: the origin interrupt fix (F214), the kept-out check (F226), the coast fix (F227), the 20 min origin limit (F224), and `purge_angle` 340.75 (D76). Being flashed now, per the user: the 30 min homing limit, the 30 cm and 300 ms rules, and the removal of the confirmation (D78, D79, D83, F232). The scheduler tool changes (retry, restart, stops, filter release) load only in a restarted tool. The full register is in the change register entry above.

**Open items for the next chat**
- Bench check after the flash: where the top switch sits at rest (`spool_status`), the idle-edge count, the 300 ms hold, and the 30 min homing time (F232, F214).
- F233: a purge stopped by the pressure cap goes on to the sampling. Decision needed; proposed fix needs `purge_end=` on the TEST line and a flash.
- Night run: rows 1 and 2 PASS, row 3 (1 m, slot 3) free after the 22:10 failure, rows 4–12 waiting (23:50, 04:00, 07:00). The spool position is unknown after the reflash.
- F230 (USB drop at 13:00), F215 (pump on reset, hardware), HW9 (manifold coupling, early fix).

**Start of the next chat:** read this entry, the change register above it, and field log §15–§16. Then continue from the spool steps in §15, then `schedule start 3`.

### Session 2026-11-04: night review, pressure sensor and manifold encoder

**What happened (details in [field log §17](field-lexplore-geneva-2026-11-03.md#17-night-run-2026-11-04-pressure-sensor-and-the-manifold-encoder)):**
- Pressure sensor: fell from about 3 bar to 0.04 bar at 00:58:55 while the pump ran. The software did not stop: the dead-sensor rule only catches a negative reading (it did at 04:34:02, -0.51 bar). The sensor is out, with water spilled (F234).
- Manifold encoder: SPI checksum errors at 04:17 and 04:34. The manifold could not rotate. The 04:00 group failed (F235, same class as F203 and HW9).
- The list stopped at 04:34 (`spool_end=FAILED`). Nothing is pumping: the O-ring is locked, the pumps are off, and the valves are closed.

**Change (firmware, not flashed):** `encoder_read [n]` (`src/Tests_mech.cpp`, `cmd_encoder_read`): n raw reads of the manifold encoder, one frame each, OK or BAD (checksum), with a summary. Read only, never moves the manifold. Registered in the known commands and the help. The tutorial command table has the row (D84). `pio run -e due` builds; `pio test -e native` 201 pass.

**Open:** F234 (the flat-pressure rule, decision needed), F235 (cable and connector check), the pressure sensor (reinstall and check, F126), the samples after 00:58:55 (no valid pressure record), slots 10 and 11 (not sampled, marked used), the 5 m tube (out, may hold water).

### Session 2026-11-04: night review, diagnostics, handover

**Full chain, samples and state:** [field log §18](field-lexplore-geneva-2026-11-03.md#18-start-here-the-night-of-2026-11-0304-the-fault-chain-and-the-state-handover). Read it first.

**Summary:**
- 00:58:55: the pressure sensor reads 0.04 bar and stays there while the pump runs. The software only catches a negative reading, which happened at 04:34 (F234).
- 04:17 and 04:34: bad encoder checksums, so the manifold could not rotate, and the 04:00 group failed. The list stopped at 04:34 (F235).
- Diagnostics (05:39 on): the sensor alone gives `00 00 00` (MISO stuck low). The encoder alone is steady (162.25°). The sensor breaks the shared SPI bus (F36).
- Also observed: the panel LEDs go off when the sensor is unplugged (F237); the console froze and needed a reset (F238).
- Reset planned: cut the 24 V pump supply first (D86).

**Changes in this session:**
- Findings: F234 and F235 updated (cause confirmed), F236 open, F237 and F238 new.
- Decisions: D85 (no pump sample without a verified sensor) and D86 (cut 24 V before a reset), both proposals to be confirmed.
- Field log §18 (the handover) and the field log §17 addendum.
- Firmware: `encoder_read` built earlier (not flashed; the board does not have it yet).

**State at handover:** the list is STOPPED. Rows 1–6 PASS, rows 7–8 FAIL (no sample taken; slots 11 and 10 are marked used but unsampled), row 9 (04:00 1 m) and rows 10–12 (07:00) waiting. **A `schedule start` runs the waiting rows at once (their times have passed).** The spool is out and its zero is not trusted. The encoder is unreliable with the sensor connected.

**Addendum (2026-11-04, sensor debug):** with the sensor unplugged the encoder is stable (162.25°) and `slot7` works. The sensor alone gives `00 00 00`. The sensor's datasheet is not in the repo, so whether a short destroyed it is unknown. Debug plan (no meter first, then a borrowed meter, then replacement): field log §18.7. No hardware change in this step. Open: F234, F235 (the cause is the sensor side of the shared bus).

**Addendum (2026-11-04, after the reset):** with the encoder connected, `abp_raw` reads valid, steady frames (1634–1643 counts, about 0 bar, 5 °C). The sensor's temperature is also reported per frame. `slot0` works. Partial recovery only: the fault came and went before, so repeated `oring_state` runs, a wiggle test and a manometer check are required (field log §18.8, F234, F235, D85).

### Session 2026-11-04: fix of the pressure sensor fault (field)

**Result (user, field):** the pressure sensor was dried and refitted in the tubing after the reset. `abp_raw` reads valid and steady, the manifold encoder reads correctly with the sensor connected, `slot0` works. The sensor reports temperature in each frame.

**Cause (best explanation):** water on the sensor pins shorted the shared SPI bus, which corrupted the encoder frames (F235, F36). Permanent damage is not excluded.

**Logs changed:** field log §18.8 and §18.9 (the fix and the restart plan); F234 and F235 (fixed in the field, to be confirmed); D85 stays until the manometer check. The handover memory note was removed at the user's request (its index line too); the handover is field log §18.

**Restart:** `schedule start 7`, not `6`, after the spool steps and the slot decision (field log §18.9). Rows 1–6 keep their status.

**Still open:** the manometer check (F126), the repeated encoder reads and the cable wiggle test, the LEDs (F237), the console freeze (F238), F236 (the double read in `oring_state`), F233 (purge at the pressure cap), F215 (pump on reset).

**Addendum (2026-11-04, user checks):** `oring_state` valid with the sensor refitted; cable wiggle test passed; LEDs on with the sensor connected (the LEDs with the sensor unplugged were not re-tested, so F237 stays open). No manometer check: the user reports the calibration did not change (D85 amended by the user). Slots 10 and 11 are to be freed at the next tool start (`4-11` at the available question). Restart: `schedule start 7` (field log §18.10). Rows 7–9 then the 07:00 group run one after another; a late 07:00 row runs at once.

### Session 2026-11-04: changing the times of the remaining rows (procedure and known limit)

**Decision (user):** the 07:00 group is to run at 08:00. The tool has no way to change a row's time in a saved list.

**Used:** the user's own file (the one I created, `tools/<the user's own file>`, was removed at the user's request; the user writes the file), with the six rows still to run (04:00 group, 08:00 group, day +1). Restart the tool with `--schedule` on that file after renaming the folder's `schedule.csv` to `schedule_before_restart.csv`. Plain `schedule start` runs the waiting rows. The original 12-row file must not be used with `--schedule`: the passed rows would run again. Field log §18.11 has the steps.

**Known limit (F239, open, host):** a saved list's times and depths cannot be changed in place. Future fix: `schedule time <row> <HH:MM>` / depth for waiting rows only, and a `schedule reload`.

**Files:** the user's own file (the one I created, `tools/<the user's own file>`, was removed at the user's request; the user writes the file) (new, parses: 6 rows). The 05 folder's `schedule.csv` is to be renamed by the operator, not deleted.

**Addendum (2026-11-04):** the new schedule file has six rows (04:00 and 08:00 groups). At the available-samples question answer `4-5,8-11`. Renaming the saved `schedule.csv` (not deleting) works: the code never reads a file with another name. This procedure is a stopgap, not a fix: a time change belongs in the tool (F239). Not built now: it needs design and tests, and it could break tonight's run.

**Addendum (2026-11-04):** the file `tools/DIEL-schedule03-remaining.csv` that was created for this procedure is removed at the user's request. The user writes the new file (the 04:00 and 08:00 groups only, six rows) and gives its path with `--schedule`.

**Addendum (2026-11-04): dates in a schedule file.** `+N` counts from the day `schedule start` is typed (`_dt.date.today()` in `session_log.py`), so it moves across midnight. Use full dates (`2026-11-04`) for rows after midnight. The tool accepts ISO dates.

### Session 2026-11-04: run 01 started, the open spool question, notes for later

**Run:** the new run folder `2026-11-04/01_DIEL-session2-2026-11-03`, schedule file `tools\DIEL-schedule03.csv` (user's). Start at 06:27–06:30 (details: field log §18.12). `actuator_home` PASS (error 2.55°). Row 1 (04:00 20 m, slot 11) started at 06:30:24; the prime passed with 0.52 bar (sensor works); the purge started 06:34.

**Open, urgent (F241):** the first dive started from "0 cm out" with no `spool_home` after the reset, although the firmware starts with `homed = false`. The physical position is not verified. Check the tube and `spool_status` now.

**Check:** slot 6 is listed as available for the 08:00 1 m row; confirm a filter is loaded there.

**Notes for later:** F239 (schedule changes inside the tool) and F240 (the tube pulled sideways under a current: the depth is inaccurate, and friction near the up-button guide). Both in field log §19 and the roadmap.

**Status 2026-11-04, 08:02 (run 01):** 04:00 group PASS (20 m slot 11 at 06:55, 5 m slot 10 at 07:21, 1 m slot 9 at 07:42). 08:00 20 m (slot 8) running (purge at 08:02). 08:00 5 m and 1 m waiting (slots 5 and 6). **New evidence for F241:** at 07:41 the 1 m rise logged `origin switch held at 188 cm out` (ignored, the move went on to 190 cm). A top press at 188 cm means the encoder and the tube disagree by about 1.9 m. Check the tube and `spool_status` before the 5 m and 1 m rows run.

**Addendum (2026-11-04): leak at the DNA shield inlet.** A small leak at the one-way valve and T connector, from a tube bent too sharply; tightened on site (Sirflex, user). The shield step is time-based, so a leak can reduce the shield dose for samples run while it was open (for example 064935387, 04:00 20 m). Time of the leak unknown. Field log §18.13; F242.

**Status 2026-11-04, 08:53 (run 01):** 08:00 20 m (slot 8) PASS; 08:00 5 m (slot 5) PASS at 08:51. 08:00 1 m (slot 6): first try FAIL at 08:52:06 (`failed_at=dive`): the rise from 590 cm to 190 cm hit the top switch at 587 cm. The switch press was ignored at first, then the drum stopped against the top and the switch was taken as the zero (`re-homed`). Retry 1 of 2 on slot 6 is now diving to 39.10 m. **Evidence for F241 again:** the encoder said 590 cm while the tube was physically at the top. The zero was reset at the top, so the retry's dive counts from the real top. Check the tube and `spool_status` after the sample.

**Final status 2026-11-04, 09:54 (run 01): schedule finished.** All six rows PASS: 04:00 group (20 m slot 11, 5 m slot 10, 1 m slot 9) and 08:00 group (20 m slot 8, 5 m slot 5, 1 m slot 6, PASS at 09:14 after its retry). Tool message: "every sample has run". **Pattern to check:** the top switch was reported pressed at 188 cm out during the 1 m rises at 07:41 and at 09:13 (and at 587 cm during the failed first try at 08:52). A press at about 188 cm while the drum still turns suggests contact near the up-button guide (F240) or a position error (F241). Check the tube and `spool_status` before the next spool run.

**End of day 2026-11-04:** steps and the record to fill in are in field log §18.15: check `schedule` (six PASS), spool by eye and `spool_status`, cut the 24 V supply, ethanol cleaning, `:quit`, copy the run folder. The depth of three samples is in doubt (F241, §18.14).

### Session 2026-11-04: depth-doubt homing (D87) and the 10:26 finding

**Finding:** the last roll-up (09:13) did reach the top switch. At 10:26 a manual `spool_home` found the tube about 10 m out while the encoder read 0 (switch at −998 cm). No logged command between 09:14 and 10:25; panel LEFT/RIGHT spool buttons are unlogged. The encoder did not count that move (F241, field log §18.16).

**Change (firmware, built, not flashed):**
- `include/Motor.h`: `depth_doubt` flag and `depth_in_doubt()`.
- `src/Hardware/Motor.cpp` (`drive`): a far top-switch press in a move sets the flag; a clean homing move to the switch clears it; a start with the switch held where the tube can be at the origin clears it.
- `src/Step_functions.cpp` (`spool_fill_and_rise`): with the flag set and a dry tube, the spool homes to the origin before the sample. A wet tube stops with an ERROR and nothing pumped.
- Build: `pio run -e due` passes; native tests 201 pass. Not bench-tested.

**Decision D87.** Findings F241 updated. Field log §18.16.

**Open:** who or what moved the tube between 09:14 and 10:25; the encoder against a physical mark; the flash.

### Session 2026-11-04: end of this chat, record and handover

**Chat record (order of the questions):** the schedule file (dates as full dates; rename-and-restart as a stopgap, F239); the run 01 start, its six PASS rows, and the schedule finish at 09:14; the top-switch presses (07:41, 08:52, 09:13) that disagreed with the encoder by 140–590 cm (F241, §18.14); the depth-doubt homing rule (D87, built, not flashed); the 10:26 manual `spool_home` that found the tube about 10 m out with the encoder at 0 (no logged command; the panel buttons are unlogged, F243; nobody moved it, user); the DNA shield inlet leak (F242, judged no issue); the request to home the spool between every sample (D88, next chat); the withdrawn Monday plan (D89).

**Logged this chat:** field log §18.12–18.17 and §19; LOG entries (run 01 status, the depth-doubt change, the end-of-chat record); decisions D85 (amended), D87, D88, D89; findings F233–F243 as updated; roadmap §7b (F240, D88, F243). Tutorial unchanged.

**Handover:** field log §18.17 has the state and the next chat's order. The prompt for the next chat is in the chat itself.

### Session 2026-11-04: archive of the Monday keep-out, per-sample priming, move log

**User request (2026-11-04):** the Monday plan (samples at one timepoint run deepest first, the tube is kept out between them, no new dive or prime) is withdrawn (D89). Archive only the edits that made that plan work, keep the rest. Put back: before each sample the tube dives to the session's deepest point, primes, then rises to the sample depth. Log each spool move's time and speed, so that the depth can be checked afterwards.

**Archived (not deleted):** `docs/archive/code/superseded/spool-keep-out-removed-2026-11-04.patch`, the diff from the Monday code to the code now in the tree. Undo: `git apply -R` on that patch from the repo root (checked with `git apply --check -R`: it applies cleanly). The index is `docs/archive/code/README.md`.

**Changes (firmware):**
- `src/Tests_mech.cpp` `run_full_cycle`: after the shield the tube is always rolled up and emptied (`spool_roll_up_and_empty`). The next-depth question (`ask_next_spool_depth`) and the keep-out branch are removed.
- `src/Step_functions.cpp` `spool_fill_and_rise`: the dive is always from a dry tube, to `dive_cm` (the deeper of the session depth and the sample depth, cut at the 40 m total). The wet-tube branch, the lowering move and the kept-length check are removed. `spool_roll_up_and_empty` no longer clears `spool_tube_wet`. Removed: `spool_tube_wet`, `spool_kept_length_cm`, `SPOOL_KEPT_MOVED_CM` (`include/Step_functions.h` externs too).
- `include/Settings.h`: `SPOOL_NEXT_ANSWER_WAIT_MS` removed (only the removed question used it).
- Kept (not Monday keep-out): the 40 m total, the spool height rule, the water fill and prime, the emptying of failed samples, the depth-doubt homing (D87), the encoder read (`encoder_read`), the origin-switch rules.
- **New:** `src/Hardware/Motor.cpp` `SpoolMoveReport`: one line per move, printed after the motor stops, e.g. `Spool move: down to 3900 cm | time 152.3 s | encoder 0 -> 3899 cm (3899 cm passed) | average 25.6 cm/s | start 60 % | ok`. A move refused before it starts is not logged (its reason is printed by the caller).

**Changes (tools):**
- `tools/sample_schedule.py`: the sort is date, time, then file order (the pre-Monday key). `_depth_or_zero`, `_next_depth`, `NEXT_GRACE_MIN` and the `spool: next sample depth` answer are removed. The module docstring is rewritten.
- Tests: the keep-out tests and the deepest-first test are removed; the operational-order test now expects file order at 09:00; a test that the time order holds whatever the depth is kept. `tools/tests/test_session_log.py`: the `META|spool;kept_out` line is replaced by a neutral line.
- Docs updated: `docs/field/operator-tutorial.md` (steps 5 and 6, the rows order, the position check now reads `Spool move:` lines), `docs/software/commands.md` (`run_sample`), `docs/software/session-log.md` (the schedule rules).

**Behavior changes (for the PR list):** (1) no next-depth question on the console any more; (2) every spool sample dives to the session depth and primes before the purge; (3) the tube is emptied after every sample, not only at the end of a timepoint; (4) same-time rows run in file order, not deepest first; (5) each spool move prints one `Spool move:` line.

**Tests:**
- `host/.venv/Scripts/python.exe -m pytest tools/`: 220 passed.
- `pio run -e due`: SUCCESS (build only, not flashed).
- `pio test -e native`: 201 of 201 succeeded.
- `ruff check` on the three edited Python files: 5 RUF059 and 1 I001 warnings, all present before this change (checked on the snapshots). Not fixed here.

**Not done:** not flashed (the tree also holds the unflashed D87, D83, D79 and `encoder_read` builds from 2026-11-04; the next flash will carry all of them: ask which build is on the board). Not committed. The field log (section 18 and 19) is not changed: its "build not flashed" list still applies.

**Hardware checklist (numeric pass criteria, to run on the bench before the next field spool run):**
1. `run_sample <slot> depth=<m>` on a dry tube: the console shows `Spool fill: dry tube, unrolling to <session depth> m`, then `Spool prime: flow check PASS` with at least 100 mL, then `Spool fill: at <cm> cm out, sample depth <m> m`, where `<cm>` is the sample depth plus the height within 0.5 m.
2. Two samples at the same time (the second shallower): the second also starts with a dive to the session depth (`dry tube, unrolling to`), not with a rise from the first depth.
3. Every move prints a `Spool move:` line. Pass: the time is positive, the encoder passed equals the length of the move within 2 %, and the average speed is between 20 and 40 cm/s (the Motor speed table; a first reading, to be measured).
4. After every sample `spool_emptied=y` on the TEST line.
5. Check a move with a stopped spool (stall): the `Spool move:` line still prints, with the reason in its last field.

**Review (same day, after the user's note "you changed too many things").** The requested change is one rule: every sample first goes down to the session's deepest point and primes, then rises to its depth. Everything below is either that rule or a consequence of removing the keep-out:
- Needed for the rule: the keep-out branch and its question (`run_full_cycle`), the wet-tube branch, the lowering move and the kept-length check (`spool_fill_and_rise`), `spool_tube_wet` and `spool_kept_length_cm`, the `spool_tube_wet` clear in `spool_roll_up_and_empty`, the matching `_next_depth` answer and `NEXT_GRACE_MIN` in the schedule tool, and the tests and docs that describe the keep-out.
- Consequences, kept: `SPOOL_NEXT_ANSWER_WAIT_MS` (only the removed question used it); the removed-function header comment.
- **Judgment call (flagged for the user):** the deepest-first sort of same-time rows. It was part of the withdrawn Monday plan (D89) and has no use once every sample dives. It is reverted to date, time, file order. To keep it, change one key in `resolve()` and restore the test `test_samples_at_the_same_time_run_the_deepest_first_then_the_file_order` (in the archive patch).
- Added as asked in the first message: `SpoolMoveReport` in `src/Hardware/Motor.cpp`. It prints after `stop()` and the coast loop, so it runs outside the motion loop. It is not printed for a move refused before it starts.
- Kept unchanged: the 40 m total, the height rule, the fill and prime, the failed-sample emptying, the depth-doubt homing (D87), `encoder_read`, and the origin-switch rules.

**Robustness checked by reading:** the dive always starts from the origin, because the previous sample ends with `spool_roll_up_and_empty` (which drains to the origin). A failed dive, prime or purge goes through the same roll up and empty. A prime with no flow fails the sample and stops the pump (`stop_procedure`). The `dive_len` check still refuses a dive outside the tube. A depth-in-doubt tube goes home before the dive.

**Cost to know:** each sample now includes a full dive, a prime (at most `SPOOL_PRIME_MAX_MS`, 4 min), and an emptying. The schedule's timing estimates (D55) and the field's per-sample duration should be measured again on the first run.

**Verified after the review:** `pio run -e due` SUCCESS; `pio test -e native` 201 of 201; `pytest tools/` 220 passed; `git apply --check -R` on the archive patch still applies. Not flashed; not committed.

### Session 2026-11-04: pressure sensor to be glued to the tubing (note only)

**User request:** the pressure sensor should be fixed better to the tube so it does not jump out under pressure again (F234). The user proposes gluing it.

**Recorded (documentation only, no code change, nothing flashed):**
- `docs/roadmap/stage2-pilots.md` §7b: a hardware row for the mount (glue, strain relief on the cable, then `abp_raw` and the manometer check).
- `docs/logs/field-lexplore-geneva-2026-11-03.md` §21: the problem, the proposal, the checks before and after the glue cures, and the interim rule.

**Open:** the glue has not been done. Check the sensor and the manometer after it is done, and record the results here.

**Decision (user, 2026-11-04): spool origin homing keeps its 20 min limit (`SPOOL_HOME_MAX_MS`), no retry.** A move to the origin that does not reach the switch in time fails and is reported. Retrying until the switch is pressed was not built: an unbounded loop breaks the timeout rule (ROADMAP §6 rule 4). The unknown-position limit (30 min, D79) is unchanged. Nothing changed in the code. The stage 1 roadmap is closed, so the sensor-mount note was taken out of it and kept in field log §21 and stage 2 §7b.

### Session 2026-11-04: end of this chat (keep-out archive, per-sample priming, move log, sensor mount, homing limit)

**Asked, in order:**
1. Archive the Monday edits that let samples at one timepoint run in a row without a dive to the session depth (D89 withdrawn). Keep the rest. Put back: every sample dives to the session's deepest point, primes, then rises to its depth.
2. Log the time and speed of every spool move, so the depth can be checked after a fault.
3. After "you changed too many things": review the change; check the logic and robustness; document the decisions and why.
4. Note in the roadmap and field log that the pressure sensor must be glued to the tube (user's proposal).
5. Confirm the 20 min homing limit; no retry. Then, because the stage 1 roadmap is closed, take the sensor note out of it.
6. Log this chat.

**Done (code):**
- Keep-out removed (`src/Tests_mech.cpp` `run_full_cycle`, `src/Step_functions.cpp` `spool_fill_and_rise`, `spool_roll_up_and_empty`, `include/Step_functions.h`, `include/Settings.h` `SPOOL_NEXT_ANSWER_WAIT_MS`). Every sample dives from a dry tube to the deeper of the session depth and the sample depth, primes, rises, then rolls up and empties.
- Schedule tool (`tools/sample_schedule.py`): the deepest-first sort and the next-depth answer are removed. Same-time rows run in file order (a judgment call, flagged).
- Move log: `SpoolMoveReport` in `src/Hardware/Motor.cpp`, one `Spool move:` line per move, printed after the motor stops.
- Tests and docs updated: `tools/tests/test_sample_schedule.py`, `tools/tests/test_session_log.py`, `docs/field/operator-tutorial.md`, `docs/software/commands.md`, `docs/software/session-log.md`.

**Archive:** `docs/archive/code/superseded/spool-keep-out-removed-2026-11-04.patch` (reversible with `git apply -R`; checked). Index: `docs/archive/code/README.md`.

**Checks run:** `pio run -e due` SUCCESS; `pio test -e native` 201 of 201; `pytest tools/` 220 passed. Ruff warnings in the Python tests were there before this change.

**Decisions:** D90 (per-sample dive and prime; why in the decisions table). The 20 min homing limit stays, no retry (LOG entry on this date).

**Documentation only, not built:** the pressure sensor is to be glued to the tube (F234). Recorded in field log §21 and stage 2 §7b. The stage 1 roadmap is not changed.

**Open, to check on the bench:**
- Not flashed, not committed. The tree also holds the unflashed D87 homing, the 300 ms hold, the 30 min unknown-position limit (D79), and `encoder_read`. Ask which build is on the board before the next flash.
- Bench checklist (LOG, archive entry, items 1 to 5): dive and prime sequence, same-time rows, `Spool move:` lines, `spool_emptied=y`, stall case.
- The 30 min unknown-position homing has not been timed at the 30 % homing speed.
- Each sample now adds a full dive, prime and emptying. Re-measure the timing estimates (D55).
- Glue and strain relief on the pressure sensor are not done. Check the sensor and the manometer afterwards and log the results.
- F241 (spool position) and F240 (tube pulled sideways) are still open; the move log is the first tool for them.

### Session 2026-11-04: wrong start answer (slots 4-5 typed as 5-6), the 1 m sample lost (F244)

**What happened.** At the start of field run 01 (2026-11-04, 06:28:12) the operator answered the availability question `5-6,8-11`. The filters in use were slots 4-5 and 8-11. Slot 4 was then marked used and slot 6 available, so the automatic order went 8, 5, 6 and never used slot 4.

**Samples as run** (`docs/logs/test-logs/2026-11-04/01_DIEL-session2-2026-11-03/`: `schedule.csv`, `samples/`):

| Row | Slot | Result | Filter |
|---|---|---|---|
| E1_T5_20M_R1 (08:00) | 8 | PASS, 675 mL sampled, 604 trusted | correct |
| E1_T5_05M_R1 (08:00) | 5 | PASS, 793 mL sampled, 675 trusted | correct (slot 5 is in the intended set) |
| E1_T5_01M_R1 (08:00), try 1 | 6 | FAIL at the dive (08:52:45), retried on slot 6 | **no filter** |
| E1_T5_01M_R1 (08:00), try 2 | 6 | PASS at 09:14:23, 1001 mL, `sample_end=volume` | **no filter: water passed through the manifold to the drain** |

**Intended.** The 1 m sample was meant for slot 4 (the answer should have been `4-5,8-11`). Slot 4's filter is still unused. The 5 m sample would also have gone to slot 4 next, by loading order, if 4 had been available. It took slot 5 only because of the wrong answer; the filter that holds its water is slot 5, so its record is correct.

**Consequences.**
- The 1 m sample of 2026-11-04 is lost. Its record `samples/090733257_E1_T5_01M_R1.json` (`slot: 6`) must not be used as data. The record is not flagged in the file; flag it when the data are analysed (rule 5). The failed try at 08:52 has no sample record.
- `slots.json` still marks slots 4 and 6 as used. Before any further run in this session, the book must be corrected: slot 4 available, slot 6 unused.
- Nothing was re-run and no file was changed in the session folder.

**Why nothing caught it.** `set_available()` in `tools/sample_slots.py` applies the typed set as is, with no read-back. The start screen showed every loaded filter as "(used)" or free, but the answer was not compared with the filters physically loaded.

**Finding:** F244 (`docs/logs/findings.md`), host, status open. Proposed fix (not built): read back the parsed list ("slots 4, 5, 8, 9, 10, 11 available: confirm") before it is applied; flag a sample run on a slot whose filter was not confirmed loaded.

**Open (user):** confirm the slot 4 filter is the one still unused and correct slots.json before the next run (the tool can erase slot 4 from the book, or the user can set the list again).

**Fix built (user agreed, 2026-11-04), in the working tree, not committed, not flashed (host tool only):**
- `tools/session_log.py` `ask_available_at_start`: the parsed list is read back ("available ...; becomes used ...") and applied only on `y`; `n` asks the list again; ENTER keeps the state as it was.
- `tools/tests/test_session_log.py`: the start-question tests now answer the read-back; new tests for the read-back text, for `n` changing nothing, and for `all` with a yes.
- `pytest tools/`: 222 passed. `ruff check` on the two files: the 2 warnings (SIM115, I001) and the format note were there before this change, on lines not touched.

**Not built:** the flag on a sample whose filter was not confirmed loaded. It would need a host-side flag on the sample record, which is a design choice; say if you want it.

**Not done (needs you):** slots.json is still wrong (slots 4 and 6 marked used). Changing it is a data edit, so it was not changed.

### Session 2026-11-04: ethanol purge questions and the locked refusal

**Asked (user, during the ethanol purge):**
1. The question "Were some of the slots 1-14 not used, so they do not need cleaning?" is confusing. Ask instead whether all slots need cleaning; on no, ask which ones do not.
2. `ethanol_purge` refused with `O-ring lock state is LOCKED - run actuator_home first`. The machine knows the lock is LOCKED, so it could unlock and go on.

**Cause of 2:** the gate in `purge_pipes_manifold` (`src/Hardware/maintenance.cpp`) used `core::may_rotate()`, which accepts only UNLOCKED. The production path does not need that: `rotateMotor()` already unlocks through `OringScope(UNLOCKED)` and locks again on return, and `goto_slot_locked()` ends with the lock. So a LOCKED state needed no refusal; only UNKNOWN (a blind unlock move, see the `Oring_lock.cpp` header) does.

**Changes (not flashed, not committed):**
- `src/Hardware/maintenance.cpp` `ask_unused_slots`: asks **Do all slots need cleaning?** (y = all slots, no more questions). On n it asks **Type the slot numbers that do NOT need cleaning**. `skip=` on the command line is unchanged.
- `src/Hardware/maintenance.cpp` `purge_pipes_manifold`: the gate is now `core::may_unlock()`, which refuses UNKNOWN only. The refusal text for UNKNOWN is unchanged.
- `docs/field/operator-tutorial.md` (steps 2 and 3) and `docs/software/commands.md` (`ethanol_purge` row): the new question, and LOCKED is no longer a reason for `actuator_home`.
- Decision D91 in `docs/logs/decisions.md`.

**Checks:** `pio run -e due` SUCCESS (`maintenance.cpp.o` rebuilt); `pio test -e native` 201 of 201. The change is not exercised by a native test (the purge needs the hardware); a bench check follows.

**Bench check (to do, with the operator):**
- Lock state LOCKED (after a sample): `ethanol_purge`, answer `y` to all, the first line shows the purge slot, no REFUSED line, the O-ring is unlocked for the first slot and locked at each slot.
- Answer `n`, then `3,7`: the list printed is FILL_ORDER without 3 and 7 (the purge slot is not in it), and slots 3 and 7 are not visited.
- Lock state UNKNOWN (after a reset): still REFUSED with `run actuator_home first`.
- A LOCKED O-ring at the end of the run: locked (rest state), as before.

**Note:** the new question uses the same y/n and `x = cancel` format as the other questions of the procedure, so the prompt reads `ASK | Do all slots need cleaning? (y/n, x = cancel)`.

**Review before flashing (2026-11-04, same day):**
- `src/Hardware/maintenance.cpp` `ask_unused_slots` restructured: the yes/no question is asked once; an invalid list is refused and asked again as a list (before, it went back to "Do all slots need cleaning?"). With a bad `skip=` argument the list is asked directly, not the yes/no question.
- `tools/session_log.py` `ask_available_at_start`: the read-back confirmation also accepts `yes`.
- Checks: `pio run -e due` SUCCESS; `pio test -e native` 201 of 201; `pytest tools/` 222 passed; `host` pytest 364 passed. `ruff check tools/session_log.py`: one SIM115 at line 215, which is not in the lines changed (it was there before).
- Not verified: the ethanol purge on the bench (needs the hardware).

### Session 2026-11-04: end-of-day review of the last two days (folders 2026-11-03 and 2026-11-04)

**Asked:** check all session logs of the last two days and document what is relevant in the docs and the logs.

**Checked:** the 10 run folders (`docs/logs/test-logs/2026-11-03/01` to `05`, `2026-11-04/01` and `02`): every `tests.csv` run and sample record, the operator lines (`>`, `!`), and the device warnings, errors, refusals and aborts.

**Already documented, not changed:** 2026-11-03 folders `02` to `05` (field log §1 to §16), 2026-11-04 folder `01` (field log §17 to §20, LOG 2026-11-04 entries).

**Not documented before, now added:**
- Folder `02_DIEL4-session4-2026-11-04` (13:01 to 19:03): field log **§22** (set-up, the schedule table of 9 rows, all PASS, the blank, the spool test, the ethanol purge, the F241 count, open items).
- Folder `01_field-leman-2026-11-03`: its only event (an `actuator_home` FAIL at 11:07, slot 14, error −4.75°) is listed in field log §22.7. No samples.

**Changed:**
- `docs/logs/field-lexplore-geneva-2026-11-03.md` §22 (new).
- `docs/logs/findings.md`: **F245** (new, open): `spool_goto length=0.2` moved 0 cm for 3.8 s and was ended by `x` (17:50). **F241** update row (16 origin-switch warnings over the two runs).
- `docs/logs/decisions.md`: **D91** status: built and flashed, used on 2026-11-04 18:38 (14 slots, 0 warnings). **D92** (new): the blank is run with `run_sample <slot>` and no `depth=`, the spool answered `y`, the 3 L purge, no spool movement (verified in `run_full_cycle`).
- `docs/field/operator-tutorial.md`: the blank procedure (section 2d).

**Verified in the logs:**
- The schedule (`DIEL-schedule04.csv`) ran all 9 rows, all PASS, with the read-back start answer `1-3,9-14`.
- The D91 build is on the board: the ethanol purge asked `Do all slots need cleaning?` and ran with the O-ring LOCKED and no refusal. Still not checked on the board: the `n` branch and the UNKNOWN refusal.
- Operator input: a pasted line `célofanes ASIN B00PQPUARCspool_home` reached the console at 17:46:35 and was sent in front of `spool_home`. Worth a check before pasting.
- Typos in the console: `actutor_home`, `actuator_homle`, `ethnol_purge`, `thanol_purge`. None changed a result, but the order matters after an UNKNOWN state.

**Open:** F245 (the 0 cm `spool_goto`), F241 (origin warnings), F240 (tube sideways), the D91 bench checks, and the blank's tube position at 16.4 cm.

**Not changed:** `docs/roadmap/stage1-FINISHED-prototype.md`. Its status line is from 2026-10-23 and its "Left to do" list has no item for these runs. Say if you want a status update there.

### Session 2026-11-05: up-switch board swelled and jammed the switch (F246)

**What happened (user, 2026-11-05, during the E2 field run):** the wooden board that holds the spool's up switch had taken up the lake water, swelled, and got stuck. This caused a problem with the switch (the user's description; the exact effect on the moves is not in the log).

**Fix (on site, user):** the part was taken apart, the extra wood was cut away, and it was put back together. Nothing else was changed in the code or the set-up.

**Why it matters:** wood in this place takes up water again, so the same jam can come back. The fix is a repair, not a material change.

**Recorded:**
- **F246** (`docs/logs/findings.md`), mechanical, status fixed on site, the board is still wood.
- **Stage 2 §7b** (`docs/roadmap/stage2-pilots.md`): replace the board with a material that does not take up water in the V2 spool, and check the switch travel after each reassembly.

**Not checked:** the switch travel after the repair, and the E2 run's spool moves on that day. Check the switch with `spool_status` and an idle tube (tap the switch, the position must not change) before the next spool run.

### Session 2026-11-05: spool prime retried at the same depth (F247)

**What happened (2026-11-05, E2 run, the first sample of the schedule, 20 m row):** the 40 m prime (`spool fill`, the flow check at the purge slot) found no flow twice: try 1 at 16:25 and try 2 at 16:44, both timed out at 240 s with 0 mL. The pump started at power 0 as on every prime. After the operator restarted the schedule at 16:47, try 3 at 16:49 counted 101 mL in about 2 min (peak 0.11 bar), with the same valves (V1 closed, V_manifold open, O-ring locked) and the same depth (4001 cm). The sample then ran.

**Checked before changing the code:**
- The air valve is not the cause: V1 is closed for the prime, as on the good primes (2026-11-04 13:08 onward, 101 mL, peak 0.36–0.53 bar).
- The firmware is not the cause: no source file changed after the last build (2026-11-04 12:54). Yesterday's primes ran on that build.
- Suspects, not decided: the spool (the homing before the sample stalled at 16:26, "encoder not counting", after the up-switch repair of F246), a kink or catch in the tube at the top, an air leak at the top connection, air trapped at 40 m, or a partial block at the intake. Logged as F247.

**Change (not flashed):**
- `src/Core/spool_fill.h`, `src/Core/spool_fill.cpp`: `spool_prime_may_retry(end_reason, attempt, max)`: true only for `timeout` and while attempts are left. Native tests in `test/test_spool_fill/test_main.cpp` (2 new tests).
- `include/Settings.h`: `SPOOL_PRIME_TRIES = 3` and `SPOOL_PRIME_RETRY_PAUSE_MS = 10 s` (PLACEHOLDER, not measured).
- `src/Step_functions.cpp` `spool_prime_flow`: the loop. Each try is one flow check at the purge slot (the first try does the slot move; the retries do not move the manifold). A pause between tries with the pump and the deployment valve off. Each try prints `Spool prime (try n of 3): flow check PASS/FAIL, end=...`. An `x` between tries stops at once.
- Not changed: the roll up, the depth, the dive. A retry does not roll the tube up; the good prime at 16:49 was a plain re-prime at 40 m.
- Documentation: operator tutorial section 2c (what the machine does on no flow), decisions D93, findings F247.

**Checks:** `pio run -e due` SUCCESS; `pio test -e native` 203 of 203 (the 2 new tests pass).

**Bench check (to do):**
- With the tube at 40 m, a try that gets no flow is followed by a second try, with the log line `try 2 of 3`, and the total time for three timeouts is about 12.7 min (3 × 240 s + 2 × 10 s), which stays inside the step's timeouts.
- A pump cap or an `x` during a try is not retried.
- Record how many tries the next 40 m primes need (F247 gives the first data: 3 tries on 2026-11-05).

**Not done:** the cause (F247). Check the tube by hand at the top, the top connection, and the intake before the next 40 m sample. The `SPOOL_PRIME_RETRY_PAUSE_MS` value is not measured.

### Session 2026-11-05: prime at 100 % (and the DAC check)

**Asked (user):** the prime may go to 100 %; implement the exception. Make the relevant changes. Then: where is the DAC?

**Why:** the prime ran at the 60 % water limit (F221), while the purge runs at 100 %. Today's slow flow (about 100 mL/min against about 460 mL/min yesterday, see the previous entry) is not explained by the 60 % cap, because yesterday's primes ran at 60 % and passed. The user decided the prime can use the full power anyway, so that the prime and the purge run at the same limit.

**Change (not flashed):**
- `src/Step_functions.cpp` `spool_prime_flow`: `ctrl.set_full_power(true)` on each try. Target `OPERATION_TARGET_BAR` and the cap are unchanged. The PID output is limited only by the full-power limit (100 %), as in the purge.
- `include/Settings.h`: the comment on `PUMP_PURGE_MAX_POWER_PERCENT` now lists the spool prime as an exception.
- Decision D94; F221 updated (the prime is decided; the applied-power print is still open); stage 2 §7b row updated.

**Checks:** `pio run -e due` SUCCESS; `pio test -e native` 203 of 203.

**Bench check (to do):**
- A 40 m prime at 100 %: the flow check passes in fewer tries than 16:25 and 16:44 today, and the pressure stays below the cap (3.6 bar, STX_MAX_PRESSURE).
- The applied power is still not printed. Print it (F221) before judging the flow. Today's purge shows the flow is low even at the 100 % limit, so the pump side is the first suspect (F247).

**DAC:** the pump speed is driven by DAC1 (`PUMP_PIN`, Due pin DAC1, listed as D67 in `docs/hardware/pinout.md` line 94). The output is about 0.55–2.75 V, not 0–3.3 V (F25): the code writes 0–4095 (12-bit), so 100 % is about 3.3 V in the code, above the range measured earlier. The relay on D42 is the real stop. To check on the bench: measure DAC1 with a multimeter (0 % / 60 % / 100 % command, pump off and on).

### Session 2026-11-05: human error, plastic instead of the 20 um prefilter (F247 closed)

**What happened (user, 2026-11-05):** the 20 um prefilter was not in the line. A plastic piece was installed between the two filters instead, and that clogged the prefilter. This explains the slow flow and the failed primes of the day.

**Effect on today's runs:**
- The 40 m primes at 16:25 and 16:44 found no flow (0 mL and 14 mL), and the retry at 16:49 passed (see the entries above).
- The first purge (16:52 onward) ran at about 100 mL/min, against about 460 mL/min the day before, with the pressure near 0.02 bar.
- E2_T1_20M_R1 failed five times, and no filter was used in any try: 16:06 (dive, ended by the operator with `x`), 16:26 and 16:27 (the 40 m prime timed out, then the homing stalled), 16:47 (the prime did not run, the rewind did not finish), and 16:59 (purge, ended by the operator with `x` because the flow was too slow).
- The purge record `auto-1` at 17:20 reached 3001 mL at volume end, which fits the prefilter being in place again by then. The fix time is not in the log: the user to confirm.

**What this changes in the earlier entries:**
- F247 (the prime with no flow) is **closed**: the cause was the prefilter. The retry (D93) and the 100 % prime (D94) were made while the cause was unknown. Both stay: the retry is harmless when a prime really is blocked, and the full-power prime matches the purge. Neither was needed for this fault.
- The homing stall of 16:26 ("encoder not counting", F246 area) is **not** explained by the prefilter. It is still open; check the spool by hand before the next spool run.
- F221 (print the applied power) stays open: the power is still not logged.

**Recorded:**
- **F248** (`docs/logs/findings.md`): the human error, and the gap (nothing in the set-up asks for the prefilter).
- **Tutorial** (`docs/field/operator-tutorial.md`, section 1): a warning box on the prefilter, with the check of the flow at the first purge (about 450 mL/min).

**Open:** add the prefilter check to the paper sheet and to the start-of-day routine; decide whether the software can ask for it at `session`.

### Session 2026-11-05: pause messages (F249)

**Asked (user):** "why does it print this line? it sounds like there was an issue. improve it". The line in question was the cap-margin pause in `E2_T1_20M_R2` (the `PAUSE at 3.37 bar (113)` and `RESUME at 1.70 bar` lines).

**What the lines meant, and what was wrong with them:**
- `PAUSE at 3.37 bar (113)`: the normal cap-margin pause (`FILL_PAUSE_BAR` 3.35 bar, the pump off until `FILL_RESUME_BAR` 3.15 bar). The count of 113 is for the whole sample. The line did not say that it is normal.
- `RESUME at 1.70 bar`: the number is the reading at the restart, not the level that ends the pause. The pause had lasted 0.2 s, because the reading was already below 3.15 bar. That looked like a fault, and it is the real problem (below).

**The real problem:** after the pump stopped at 3.37 bar, the reading fell to 1.70 bar in 0.2 s, then swung between 0.67 and 1.88 bar, while the flow stayed at about 995 mL/min. A drop of 1.7 bar in 0.2 s is unlikely from the line alone, so the reading is suspect (sensor or wiring), or the line is noisy. The control logic is not changed: this is a diagnosis item (F249).

**Change (log only, built, not flashed):** `src/Hardware/Pump.cpp`:
- `PAUSE n in this run (cap margin, normal): pressure X bar, pump off until Y bar`.
- `RESUME after T ms: reading Z bar is at or below Y bar, pump restarts from 0 % (ramp 5.0 %/s)`.
- `WARNING | pause of only T ms: the pressure was already Z bar when the pump restarted. Check the pressure reading (F249)` for a pause under 1 s.

**Checks:** `pio run -e due` SUCCESS. No native test covers this (it is the pump driver).

**Still to do:**
- Check the pressure reading with the pump stopped: `abp_raw` steady at rest and the manometer (F126, F234).
- Consider changing `Pump started at once with power 0` (it sounds like a problem and is the normal start from 0 %).
- The control behaviour (pause at 3.35 bar on every surge) is unchanged. It is safe, but it cuts the flow each time the pressure reaches the margin.

### Session 2026-11-05: pause check, spool or not (F249)

**Asked (user):** is the pressure drop after a pump stop a consistent effect of the spool module (a water column pulling back when the purge stops), so that the pauses are too long only with the spool? Verify.

**Method:** every `PAUSE` and `RESUME` line in the 10 run folders (`docs/logs/test-logs/*/*/session.log`), with the time between the two lines, the pressure at the pause, and the reading at the resume. Sessions split by the `META|session ... spool=` answer; the 2026-10-23 sessions have no spool answer and are treated as "no spool". The sample depth was taken from the last `META|sample` line before each pause.

**Results:**
- **Pause length:** 0.2 s in every case, with or without the spool (1525 spool pauses, 65 without). The pauses are not longer with the spool. The pause is one control step: the pump is stopped and restarted before the pressure falls to the resume level.
- **Drop after the stop:** median about 2.3 bar in every spool session (1525 pauses), and 0.8 bar in the 2026-10-23 sessions (65 pauses). The 2026-10-23 sessions do not match each other (one median 0.75 bar, the other 1.5 bar, p90 2.2 bar), so the no-spool comparison is not clean.
- **Depth:** the drop does not depend on the depth: 0.3 m: 1.75 bar, 1 m: 2.33, 5 m: 2.27, 20 m: 2.25 bar (medians). A water column draining back would give about 2 bar at 20 m and about 0.1 bar at 1 m. It does not.

**Conclusion:** the water-column explanation is **not supported**. The spool is not shown to change the drop either: the 2026-10-23 sessions differ in too many ways to tell. What the data do show is that the pause never waits: the reading is already below 3.15 bar at the first check after the stop, so every pause is 0.2 s.

**Next check (bench, pump off, line shut):** read the pressure every 100 ms for 10 s after the pump stops, with the tube empty and then full (spool), and with `abp_raw` at the same time. A real line drop would decay smoothly; a sensor or wiring fault would jump.

**No code change.** The control logic is unchanged; the messages from the earlier entry stay.

**Follow-up (2026-11-05, user: "if there are improvements to do you can"):**
- `src/Hardware/Pump.cpp`: `Pump P1 started at once with power 0` now reads `Pump P1 started, output 0 % (the controller ramps it from here)`. The old text sounded like a fault; the output is the start level, and the controller ramps it up (log only, built, not flashed).
- **Not changed:** the pause logic. It already holds the pump off until the reading is at or below the resume level; it resumes at once only because the reading is already below that level after the stop. Changing that without the bench check above would change the control of every purge and sample.

### Session 2026-11-05: improvements from the field log (without touching the Lexplore run)

**Asked (user):** look at the field work log; improve what can be improved without affecting the current Lexplore experiment; prepare the rest for later.

**Done (no effect on the running experiment):**
- `docs/field/operator-tutorial.md`: section 3 explains the normal messages (cap-margin pauses, the 10-minute reminder, depth in doubt, prime tries, the stalled spool move, the blank's `NOT moved`, `Sample N erased`); section 2b says that a `[FAIL]` row needs `schedule start <row>`; a warning against pasting into the console; section 4 gives the run report.
- `tools/run_report.py` (new, read-only, stdlib): summarises a run folder from its `session.log` (pauses with their length and drop, purge flow at each minute, prime tries, spool warnings, errors, x aborts, refusals). It does not import or touch the session tool. Tests: `tools/tests/test_run_report.py`. `pytest tools/`: 228 passed. Run on today's folder: purge flow 49 to 501 mL in 5 min, which matches the hand check.
- `docs/logs/field-lexplore-geneva-2026-11-03.md` §23: the review, the list of what was done, and what is prepared (with the reason it was not done now).

**Prepared, not done:** the end-of-sampling notes question (F222), the schedule time command (F239), the applied-power print (F221), the flash of the built firmware (prime at 100 %, pause messages, pump start wording; the prime retry is already on the board), the pressure-reading check with the pump off (F249), the prefilter check at `session` (F248), the sensor mount (§21), the spool friction (F240), the up-switch board (F246), the 16:26 homing stall, and the D91 bench checks. All are in field log §23.2.

**Not changed:** the board (the message changes are not on it yet; today's log shows the prime retry is), the pause control logic, the running session tool.

### Session 2026-11-05: documentation brought up to date

**Asked (user):** make sure all documentation is up to date.

**Changed:**
- `CLAUDE.md`: the spool is no longer "never run" (in field use since 2026-11-03, open items listed); the spool table row and the spool hazard bullet updated (the unmeasured items are the drum diameter and the drain path only).
- `docs/roadmap/stage1-FINISHED-prototype.md`: status line updated to 2026-11-05 (field use continues).
- `docs/logs/field-lexplore-geneva-2026-11-03.md`: the two "spool prime at 60 %" open-item rows marked as superseded (D94).
- `docs/software/session-log.md` §9: the run report tool.
- `docs/field/operator-tutorial.md`: the report command line checked (it reads `tools\run_report.py`).

**Checked, not changed:** `docs/README.md` (the index is still right), the historical LOG entries that say "60 %" for the prime (they describe the day they were written).

**Open:** the firmware on the board versus the working tree. The prime retry is on the board; the 100 % prime and the pause and pump-start message changes are not confirmed (see the field log §23.2).

### Session 2026-11-05: spool mechanical failures (F250)

**What happened:** after the stall at 19:53 (E2_T1_01M_R2), three mechanical faults were found on the spool module, in this order: the wooden frame screw holding the level-wind guide loosened (probably moisture in the wood); the drive belt lost its tension and could not turn the drum; the 3D-printed level-wind guide cracked, so it no longer turned on its shaft.

**Fixed on site:** screw re-tightened, belt secured with zip ties, guide glued to its shaft and drive wheel with super glue (trial, not verified under load).

**Recorded:** F250 (`docs/logs/findings.md`), field log §24, and the field report (issue 4, and next steps).

**Open:** check the move log against these faults; replace the wooden parts and the printed guide in the next version; check the glue under load. Keep the spool out of service until the guide turns freely over the full length.

**Roadmap (2026-11-05, user):** the spool parts are a short-term change, not a long-term fix. Added **HW10** to `docs/roadmap/stage2-pilots.md` §7.4, before the next spool run: replace the wooden parts with water-resistant material, print the level-wind guide in a tougher material with metal fixings, check the belt tension, and test 5 roll-ups and 5 dives over 40 m with no stall. Until HW10 is done, the spool runs only with a hand check before each sample.

### Session 2026-11-03/04: ethanol purge after the Tuesday night restart not in the log (doubt recorded)

The team is almost sure that the ethanol cleaning was done after the restart on Tuesday night (3 November 19:50), but no session log shows it. The first cleaning in the logs is 4 November 18:38. The samples taken in between, with the restarts at 4 November 06:27 and 13:01, are listed in field log §25, with the samples before and after that could be affected. If the bioinformatics shows cross-contamination in the listed samples and not in the later ones, the purge was probably not done.

**Update (user, 2026-11-05):** all other ethanol purges were done, including those before the 4 November restarts (06:27 and 13:01). They are confirmed by the team, not by the logs. The only doubt is the Tuesday-night purge (3 November, after the restart at 19:50). Field log §25 is updated.

### Session 2026-11-05: end-of-series handover

**Status at 23:10:** E2 schedule complete (nine rows PASS; row 6 restarted at 20:02 after the spool fault and passed). Ethanol cleaning at the end of the day: the first attempt was stopped by the operator with `x` after 9 s; the second, from 22:17, completed at 22:41 (14 slots, 0 warnings). Session closed.

**Records:** 19 sample files, 18 rows in the table. The extra file `165901327_auto-1.json` is the 16:59 purge stopped by `x`; kept as a record, not a sample.

**Firmware:** the prime retry is on the board. The 100 % prime and the pause and pump-start messages are not yet confirmed on the board; flash after the next run.

**Before the next run:** spool checked by hand and `spool_home` only when safe (HW10 not done); prefilter in place; ethanol cleaning before a new filter set; session answers and `schedule` checked; planned times and filter IDs on the paper sheet.

**Repository:** changes since `591c0ea` are not committed (listed in field log §26). New: `tools/run_report.py` (with tests) and the field report draft.

**Handover details:** field log §26.

### Session 2026-11-06: two-pass roll up, false zero (F251), roadmap notes

**Asked (user):** always home twice, so that an already-up spool takes no time and a spool that was not up is finished; add the cartridge idea to roadmap 2 and the reservoir idea to roadmap 3; document everything.

**Changed (built, not flashed):**
- `src/Hardware/Motor.cpp` `go_home`: two passes of the roll up to the origin (`SPOOL_HOME_PASSES` = 2 in `include/Settings.h`). Decision D95. Native tests 203 of 203; `pio run -e due` SUCCESS.
- `docs/software/commands.md`: `spool_home` notes the two passes.
- `docs/roadmap/stage2-pilots.md` §7b: the three-cartridge system (one hollow-tube cartridge for the cleaning, two for the filters).
- `docs/roadmap/stage3-full-system.md` notes: the pluggable reservoirs of bleach, ethanol and waste.

**Found (F251, open):** on 6 November the drum stopped at the top-switch holder at about 188 cm and 590 cm out; the hold rule set the encoder to 0 while the tube was still out. The two-pass roll up does not fix this, because the switch is still held when the second pass starts. Samples from 08:00 onwards may have been deeper than recorded (field log §27). Open decision: a hold stop far from the zero is not an origin.

**Not done:** the false-zero rule (needs the decision above), the mechanical fixes (HW10, F246, F250), the flash.

**Details:** field log §27.

### Session 2026-11-06: blank procedure in the operator tutorial

**Asked (user):** update the tutorial on how to run a blank sample.

**Changed:** `docs/field/operator-tutorial.md` §2d rewritten. Key points: never `depth=0` (it still dives, primes and rises to 0); no `depth=` at all for a blank; set `spool_height` after a reset; `session` with spool `y` for the 3 L purge; start the schedule only after the blank, since it takes the next free slot when each row starts; the first scheduled sample moves the tube.

**Checked in the code:** `run_sample` with `depth=0` enters the spool path (`run_full_cycle` with `use_depth` true), so it moves the spool. A run with no `depth=` does not.

**Follow-up (user):** the blank can run during a schedule, but the schedule must be stopped while it runs (the tool asks). Tutorial §2d updated: before the schedule, or stop, blank, restart; the slot rule (the schedule takes the next free slot; a blank before the schedule takes slot 1 for the list 1-3,14, leaving 2, 3, 14 for the rows); give the slot explicitly if the blank runs later; the tip is placed in milliQ by hand and the spool does not move.

**Follow-up (user):** the blank needs at least 4 L of milliQ, preferably 5 L (purge 3 L plus the prime). The blank primes more slowly than a sample. Both are now in tutorial §2d.

### Session 2026-11-06: milliQ blank keeps reaching the cap margin (F252)

**Observed (user):** the milliQ blank kept stopping at the cap margin while filling the filter, which is surprising as milliQ is pure.

**Record so far:** E2-Blank (folder `01_E2-DIEL3-session6-2026-11-06`), started 14:27, still running at 14:42: 57 pauses in 15 min, pressure 3.36 to 3.40 bar at each stop, about 440 mL passed (about 30 mL/min). E1-B1 (4 November) had 29 pauses. Each stop drops the reading by about 2.5 bar in 0.2 s (F249).

**Recorded:** F252 (`findings.md`), field log §28, stage 2 §7b (roadmap 2). The final numbers go in when the blank ends.

**Not changed:** the code and the tutorial. A blank is not a clean control until the cause is found.

**Follow-up (user, 2026-11-06):** the same pauses were seen on the last blank. Pump and PID tuning is a future item in stage 2 §7b: fewer pauses, a steadier flow reading, and a faster fill. First clue: the pauses start at 3.35 bar against a 3.0 bar target, so the pressure overshoots by about 0.35 bar before the margin stops the pump.

**Lead (code, 2026-11-06):** `CtrlPump::run` pauses and resumes on the raw pressure reading, while `Settings.h` says the resume uses the filtered reading. A single low raw reading restarts the pump at once (0.2 s pauses). Recorded in F252 and field log §28; not changed.

**Correction 2 (user, 2026-11-06):** E2-Blank is valid: about 800 mL of milliQ passed. The tip left the water only when the flow stopped changing and the pressure was low, which is the end of the run, so the `clogged` end reason is the end of the water, not a filter clog. F252 is closed as a clog; the pressure pauses stay under F249 and the tuning item. The tutorial §2d now asks to check the tip stays in the milliQ and to note any time it leaves the water. The record's notes field holds `slot1` (the operator's note for the slot, entered at the start of the run). The record itself cannot be edited, so the correction is kept here and in field log §28.

### Session 2026-11-06: end of chat, summary

The chat covered the E2 schedules of 5–6 November, the spool false zero (F251, two-pass roll up, D95), the spool mechanics (F250, HW10), the blank procedure (tutorial §2d: no `depth=`, milliQ 4 to 5 L, the slower prime, stop and restart of the schedule, slot order), the milliQ blank read first as a clog and then corrected (F252; the intake was out of the water), the pump and PID tuning and the raw versus filtered pressure lead, the sample notes question (end of sampling, but not for a schedule), the three-cartridge and reservoir ideas, and the ethanol doubt for the Tuesday night (§25).

Where each item is written up, and the open items: field log §29.

**Update (user, 2026-11-06):** E2-Blank is valid (about 800 mL of milliQ passed; the tip left the water only when the flow stopped changing, at the end). The false zero is mitigated by the two-pass roll up (accepted); the drum stopping at the holder and the stricter origin rule go to roadmap 2. The spool repair is temporary and goes to roadmap 2. The depths after 08:00 are not treated as uncertain: the zero seemed to work and the later dive and rise distances were normal. The firmware flash waits for the end of the current schedule; the user will commit later.

### Session 2026-11-06: remaining work at the end of the chat

The full list of what remains is in field log §30, by owner: firmware (flash at the end of the current schedule, then check the new messages and the two passes), spool (hand check before each spool run, the -186 cm reading, F245), roadmap 2 (HW10 permanent version, F251 residual, F246, pump and PID tuning, the raw versus filtered pressure, F249 bench check, F234 sensor glue, F216 frame, F248 prefilter check, F222 notes question, F239 schedule times, cartridges), and records (the field report after the 24 h repeat, nothing committed yet). HW9 is not recorded as done in the logs; check it.

### Session 2026-11-06: expired filter in the 16:00 sample (F253)

E2_T7_20M_R1 (the first row of schedule 3, 16:00) uses the last filter available. It is from a different batch and expired in March 2025; it is the only expired filter in the campaign. There was no choice. Recorded as F253 and field log §31: the filter ID, batch, expiry and the reason on the paper sheet; compare its pressure and flow with the other 20 m samples; flag the sample for the analysis.

### Session 2026-11-06: operator tutorial rewritten, Bois Chamblard log, report update

**Asked (user):** shorten and restructure the operator tutorial (operation first, appendices for problems), use the field experience, add a field log for the Bois Chamblard runs (Felix, 26–29 October, no spool) and update the field report with both campaigns.

**Changed (documents only):**
- `docs/field/operator-tutorial.md` rewritten: sections 0 Before you leave, 1 Start, 2 Samples (by hand, schedule, spool, blank), 3 End of the day, 4 Ethanol cleaning, command table; Appendix A problems, B spool height, C restarting a schedule. Corrected: the time per sample without the spool is about 20 min (was "5–8 min"; Felix's runs took 18–22 min), about 30 min with it; the run report command path (`tools\run_report.py`, was mistyped); the AVAILABLE question moved to the start of the day. New: `actuator_home` before `schedule start`, `s` versus `x`, full dates in schedule files past midnight.
- New `docs/logs/field-bois-chamblard-2026-10.md` (samples, events, lessons, open points).
- `docs/field/field-report-draft-2026-11.md` now covers both campaigns.

**Not changed:** code, firmware, tests. Nothing committed.

### Session 2026-11-06: protocols added as context, ethanol doubt closed

**Asked (user):** use the Bois Chamblard and Lexplore protocols and the project description only as context (the logs are the record), leave the air sampling out, and document what is relevant. The team's notes prove that the ethanol purge after the Tuesday-night restart was done.

**Changed (documents only):**
- Field log Lexplore §25: the Tuesday-night doubt is resolved (done, proven by the team's notes); §26 to §30 pointers updated. New §32: plan versus site (expeditions, depths, time points, time per sample, IDs, blanks).
- `docs/logs/field-bois-chamblard-2026-10.md`: air work marked out of scope; new §5 plan versus site (cadence, 500 mL purge instead of 2 to 3 L, blank, lake sequence, manual cleaning steps).
- Field report: the ethanol item reads as closed; new "Plan versus site" paragraph.
- Operator tutorial §3: check the Sterivex are nearly full of DNA Shield (about 2.2 mL) when the filters come out.

**Not changed:** code, firmware, tests. Manual (Niskin) sampling is left for the user's later input.

### Session 2026-11-06: prefilter at each ethanol purge, plans archived

**Asked (user):** on Lexplore the 20 µm prefilter was used for the bacterial work and changed at every ethanol purge; keep the original pasted documents in the archive, organised.

**Changed (documents only):**
- Operator tutorial: section 0 says the Lexplore prefilter is changed at every ethanol cleaning; section 4 step 1 takes it out and replaces it after the cleaning.
- Lexplore field log §32 and the field report: the prefilter row and item 5 carry the same rule. Bois Chamblard log §5: the protocol's 100 µm prefilter is noted as not recorded, with the Lexplore 20 µm fact.
- Archive: new `docs/archive/field-plans/` with the three original documents, verbatim with a one-line header each: `bois-chamblard-field-protocol-2026-10.md`, `lexplore-first-step-project-description.md`, `lexplore-field-protocol-2026-11.md`. Listed in `docs/archive/README.md` and linked from the field logs.

**Not changed:** code, firmware, tests. Findings F248 unchanged (the prefilter rule is procedure, recorded in the tutorial).

### Session 2026-11-06: every timed step in the sample file (D96, built, not flashed)

**Asked (user):** the sample JSON does not hold the start, end and duration of every step. Change the code so it does, there is no cost to an extensive metadata file.

**Changed (firmware, built, not flashed):**
- `src/Core/step_log.{h,cpp}` (new, pure): `core::StepLog` (48 steps, fixed size, counts the dropped ones) and `format_steps_record`, one JSON line `{"t":"steps","id","slot","steps":[{name,start_ms,end_ms,dur_ms}...],"steps_dropped"}`. Integers only.
- `include/Run_steps.h` (new) and `src/Step_functions.cpp`: the run's log, `run_steps_begin()`, `StepScope` (closes the step on every return path), `last_sample_record_id()`. Steps: `spool_fill`, `spool_prime` (each try), `purge`, `sample`, `sample_pump`, `sample_empty_purge`, `sample_empty_stx`, `shield`, `shield_micro`, `shield_air`, `rewind`, `empty`, `unlock`.
- `src/Hardware/Motor.cpp`: every spool move (`spool_up`, `spool_down`) is a step, from its existing scope object.
- `src/Tests_mech.cpp` `run_full_cycle`: resets the log at the start, scopes the run steps, prints the steps line after `STEP TIMES`.
- `tools/session_log.py`: parses `{"t":"steps"` lines, adds `steps` and `steps_dropped` to the sample file of the same id (live and in `rebuild_samples`). A steps line with no record is reported in the log, not dropped. Found and fixed on the way: the live lock is not re-entrant, so the message is written outside it.

**Behavior change:** none on the machine. One more console line per run, and the sample JSON has two new keys. The sample record line itself is unchanged.

**Not covered yet:** the O-ring lock and unlock inside the steps (they are in the console log), the vents, and the pump runs of the purge-only and calibration paths (no steps line, because they do not go through `run_full_cycle`). Old sample files have no steps. Their `STEP TIMES` is in their `session.log`.

**Tests:** `pio test -e native` 213 passed (10 new in `test/test_step_log/`). `pytest tools/` 235 passed (7 new in `tools/tests/test_session_log.py`). `pio run -e due` SUCCESS: RAM 13.8 %, flash 36.9 %.

**Not done:** flash (the next run after the schedule), commit. After the flash: check one `{"t":"steps"` line in the log and the `steps` in the new sample file.

### Session 2026-11-06: field report rewritten around validation and failure causes

**Asked (user):** the schedule row and the "32 of 47" count were unclear; separate faults of the autonomy software from faults of broken parts; "main finding" is not a finding; drop the plan-versus-site and steps sections; describe the two field works shortly, with the depths.

**Checked against the run folders** (`tests.csv`, `session.log`): LéXPLORE 3–6 Nov has 68 run_sample attempts: 58 started by the schedule (43 PASS, 15 FAIL), 10 by hand (8 PASS, 2 FAIL). 45 different automatic samples (E1 24, E2 21), 44 ended PASS. All 51 PASS runs completed the shield step. The 32 of 47 in the earlier draft was not reproduced from the logs; it is replaced. Failed runs by cause: autonomy and procedure 8, set-up 4, broken parts 4, test stopped by hand 1.

**Changed (documents only):** `docs/field/field-report-draft-2026-11.md` rewritten: summary, the two field works, results, validation criteria, failures by source, next steps. The Bois Chamblard log keeps its CoWaS-only content.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-06: air/water runs moved, manual sampling compared with the autosampler

**Asked (user):** the air/water runs at LéXPLORE belong to the Bois Chamblard project, not to the DIEL study; revise the counts and the report. Manual sampling timings for E1 and E2 supplied: compare with the autosampler.

**Changed (documents only):**
- The air/water runs of 3 November (folder `03`: 3 samples, 1 blank, 1 stopped by `x`) are in the Bois Chamblard log, new section 6. The LéXPLORE log §13 points there.
- `docs/logs/manual-sampling-lexplore-2026-11.csv`: the manual sheet, as supplied.
- LéXPLORE log §33: pairing of the 21 manual samples with the automatic filtration windows (offsets in minutes). From E1 T6 on, all 18 pairs are within 15 min (mean absolute 4.9 min); the three E1 T5 pairs are 43 to 75 min later.
- The field report: counts revised (LéXPLORE: 62 CoWaS runs, 46 PASS, 16 FAIL; 45 different automatic samples, 44 PASS; excluding the air/water runs and the 3 Nov test); "46 of 46" PASS runs with the shield; a new section on the manual comparison.

**Checked:** the user's "one automatic sample lost to a loading error" is most likely `E2_T1_20M_R1`: five failed runs from the prefilter error, but the sample passed on its sixth run (1001 mL), paired with the manual sample at 17:21. The only automatic name with no PASS is `E1_T1_05M_R1`. Both are written in the report and LOG §33.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-06: field report counted per sample, split at the spool fixes

**Asked (user):** count samples, not runs; debugging runs before the spool fixes count as one fail per sample; say that the spool can lose its position and is re-zeroed when in doubt; note the retry and filter reuse; DNA Shield varies with clogging and is not measured; the human-error sample.

**Checked in the logs:** the cut is 3 Nov 22:49 (first sample whose first run came after the spool-zero reflash). Before it: 6 samples, 6 failed runs, 4 needed more than one run, 2 finished by hand (E1_T1_05M, E1_T1_01M). After it: 39 samples, all through the schedule; 6 needed more than one run; 10 failed runs, all before the filter was loaded (filter reused). Two spool-zero events after the fix were retried by the schedule within 2 to 5 min (E1_T5_01M, E2_T1_01M_R1). The sample of the prefilter error (E2_T1_20M_R1) passed on its 6th run. The DNA Shield micro pump runs 190 s in 50 of 51 runs (one stopped early); no shield volume is recorded.

**Changed (documents only):** `docs/field/field-report-draft-2026-11.md` rewritten: per-sample and per-run tables, before and after the spool fixes, validation wording ("Almost", "Criterion"), causes after the fixes with their recovery, a DNA Shield section (dose measured and adapted to clogging, roadmap 2 or 3).

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-06: manual sampling protocol, sheet and volume note

**Asked (user):** record the manual sampling information (times, labels, the protocol), and report that E1_T1_20M_R1 passed about 1250 mL instead of 1000 mL.

**Changed (documents only):**
- `docs/field/manual-sampling-protocol.md` (new): the 11 steps of the manual protocol as supplied, the labels (E1/E2, T1 to T7, depths, R1/R2), and the differences from the CoWaS (2 L against 1 L through the filter; 1 mL shield by hand against the fixed-time pump; timing).
- `docs/logs/manual-sampling-lexplore-2026-11.csv`: already filed; the labels and the time points are now written down in the protocol and in LéXPLORE log §33 (the time points: 4 pm, 8 pm, midnight, 4 am, 8 am, noon, 4 pm, as the user listed them).
- LéXPLORE log §34 (new): E1_T1_20M_R1, record 1001 mL (944 trusted) against about 1250 mL measured by the user.
- `docs/logs/findings.md`: F254 (flowmeter gap on E1_T1_20M_R1, to check against a measured volume).
- Field report: the manual protocol and the volume difference (2 L against 1 L) in the manual section; the volume gap in the validation table.

**Not changed:** code, firmware, tests. The operator tutorial is not changed (the manual protocol is a separate document). Not committed.

### Session 2026-11-06: manual sheet corrected, manual protocol corrected, Lexplore protocol rewritten

**Asked (user):** discard E1-T5-20M-R1, E1-T5-05M-R1 and E1-T5-01M-R1 as manual samples (never taken); 12 is noon, and there was no manual 08:00; a manual filter gets 1 L, not 2 (the first litre is a purge with no filter); E1_T1_20M_R1's 1250 mL is an automatic sample, measured with a bucket to check the flowmeter, which under-reads; rewrite the Lexplore protocols from what was done, with materials and methods.

**Changed (documents):**
- `docs/logs/manual-sampling-lexplore-2026-11.csv`: the three E1-T5 rows removed.
- LéXPLORE log §33: the three E1-T5 pairs removed; 18 manual pairs; time-point labels (T3 midnight, T6 noon); manual samples at E1 T6 and T7, E2 T1 and T2 only.
- LéXPLORE log §34 and F254: the bucket check is the real volume (1250 mL), the flowmeter counted 1001 mL: it underestimates at this flow.
- `docs/field/manual-sampling-protocol.md`: 1 L through the filter after a 1 L purge with no filter; the labels; the time points; the materials; what differs from the CoWaS.
- `docs/field/lexplore-field-protocol.md` (new): rewrites the planned protocol in `archive/field-plans/` as it was run: aim and design, materials to prepare before the trip, before-run checks, automatic and manual sampling, blanks, filters and labels, end of day, known limits. The archived plan is kept.
- Field report: E1 T5 removed from the manual table; the manual volume (1 L); the bucket check in the validation table; the protocol linked.

**Not changed:** code, firmware, tests. The operator tutorial is unchanged. Not committed.

### Session 2026-11-06: manual blank and pump, fails counted as problems

**Asked (user):** the manual blank is kept with the manual samples (18 manual samples plus 1 manual blank); the manual pump is a Vampire Sampler (Bürkle, Huber lab); the Summary wording for "after the spool fixes"; recount the fails so that a run of failures on the same problem, until it is resolved, counts as one fail.

**Recount (DIEL study, 3–6 Nov, CoWaS, excluding the air/water runs and the test):** 16 failed runs, grouped into 8 problems, each resolved. Before the spool fixes (first run to 3 Nov 22:49): 3 problems (spool position 16:45–17:35 with 3 runs, prime no flow 20:15, spool position 21:22–22:10 with 2 runs). After: 5 problems (sensor 04:19–04:34, 2 runs; spool zero 08:52, 1 run, retried automatically; prefilter 16:06–16:59, 5 runs; spool zero 19:20, 1 run, retried automatically; spool stall 19:53, 1 run). The earlier counts (per run, per sample) are replaced.

**Changed (documents only):**
- Field report: results by problem; the "What fails" table by problem; the Summary wording for the spool fixes and the retry, as supplied by the user; the manual pump and the manual blank.
- Manual protocol: the manual pump, and the 18 + 1 blank.
- LéXPLORE protocol: the manual pump, the blank, the manual sample count.
- LéXPLORE log §33: the manual blank kept with the manual samples.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-06: runs named by project, not by person

**Asked (user):** no need to explain the excluded runs in the report; name the runs by project (air/water at Bois Chamblard, DIEL study on LéXPLORE), never by person.

**Changed (documents only):** the report title and sections use "air/water at Bois Chamblard" and "DIEL study on LéXPLORE"; the sentence listing the excluded runs is removed. The Bois Chamblard log section 6 is "Air/water runs at LéXPLORE (3 November)". The LéXPLORE log §13 heading and its move note use the project names; F231 uses "air/water run". Folder paths keep their real names.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-06: documentation pass (end of the chat)

**Asked (user):** document everything relevant from the chat in the logs at the right places; the expired filter is E1_T7_20M_R1, not E2_T7_20M_R1.

**Corrected:** F253 and field log §31 name E1_T7_20M_R1 (4 Nov, slot 11, PASS, 1001 mL). E2_T7_20M_R1 (6 Nov, slot 2) is not recorded as expired; its filter to be checked on the paper sheet.

**Status found while checking (16:47, 6 Nov):** schedule 3 is in progress. E2_T7_20M_R1 PASS (16:00 to 16:30, 806 mL, `clogged`, slot 2), E2_T7_05M_R1 running (slot 3), E2_T7_01M_R1 waiting (slot 14). The two slots were predicted before the start and were as predicted. Recorded in field log §35; not counted in the report until it ends.

**Where the chat's content is recorded:**
- Schedule and slot questions, and the tutorial: operator tutorial (filters in date; §0 and §4 already had the prefilter rule); LéXPLORE protocol §3.
- Tutorial rewrite, archive, Bois Chamblard log: LOG 2026-11-06 entries; `docs/logs/field-bois-chamblard-2026-10.md`; `docs/archive/field-plans/`.
- Step timings (code): LOG D96 entry; `docs/software/session-log.md` §5, `docs/software/code-guide.md`; decisions D96.
- Field report: `docs/field/field-report-draft-2026-11.md` (results by problem, manual comparison, DNA Shield, validation).
- Manual sampling: `docs/field/manual-sampling-protocol.md` (1 L through the filter after a 1 L purge; pump Vampire Sampler, Bürkle, Huber lab; labels; time points); `docs/logs/manual-sampling-lexplore-2026-11.csv` (E1-T5 removed; E2 manual blank kept as a blank).
- Manual pairs and the timing table: field log §33 (18 manual samples, one manual blank).
- Flowmeter and 1250 mL: field log §34; F254; roadmap stage 2 §7b (bucket checks at several flows).
- DNA Shield dose: F255; roadmap stage 2 §7b (measure the volume, dose from the filter state; most relevant follow-on); the field report's DNA section.
- Fail counting: decisions D97; field log §36 (8 problems, 16 failed runs).
- Protocol for the DIEL study: `docs/field/lexplore-field-protocol.md` (replaces the plan in the archive; materials, methods, known limits); decisions D98.
- Naming by project (air/water, DIEL study): LOG 2026-11-06 entry "runs named by project"; no person's name for those runs in the documentation.

**Not changed:** code, firmware, tests. The schedule on the board is not touched. Not committed.

### Session 2026-11-06: second flowmeter bucket check

**Asked (user):** sample E2-T7-05M-R1: flowmeter 549 mL, real volume 800 mL; update the logs.

**Recorded:** field log §37; F254 (second data point, about 31 % low, error grows at lower flow); the field report volume line; roadmap stage 2 §7b flowmeter row. The end reason, trusted volume and flow of the run are not entered (not given); read them from the record.

**Not changed:** code, firmware, tests. Not committed.


### Session 2026-11-06: manifold over-travel and lost link at LéXPLORE (hypotheses only)

**Asked (user):** record what was observed after E2_T7_05M_R1, and all hypotheses, because the cause is not known; add more hypotheses if there are any.

**Recorded:** F257 (Blocking, open) and [field log §38](field-lexplore-geneva-2026-11-03.md#38-manifold-over-travel-and-lost-link-after-e2_t7_05m_r1-2026-11-06).
- The manifold kept turning until it tangled in the tubing and cables. The computer lost the CoWaS (COM6 not detected). A burnt smell, not sure. The platform generator started at the same time.
- Samples: E2_T7_20M_R1 and E2_T7_05M_R1 collected (shield of the 5 m added by hand); **E2_T7_01M_R1 never taken**; the ethanol purge of the three used slots is not done.
- Hypotheses H1 to H10 (rain added by the user; power dip, frozen Due with the driver on, reset and repeated command, stalled motor, surge or short on the USB bridge, encoder noise, laptop side, software state, O-ring left unlocked), the evidence to collect, and a bench plan.
- Main point: the 15 s timeout and the guards run in the Due and cannot stop a frozen or dead Due. A stop independent of the Due is proposed for the bench work.

**Found while checking:** the motion guard (`manifold_guard`, `MANIFOLD_STALL_MS` and others, referring to F256) is in the working tree and not committed. F256 is not in the register yet and no log entry says the guard was flashed. Not changed here. F257 was used for this event so that F256 stays free for the guard.

**Not changed:** code, firmware, tests. Not committed.

**Update (user, end of the day):** rain added as H10 (some drops fell; the CoWaS is somewhat resistant to rain). The debugging is left for another time.

### Session 2026-11-09: power-dropout robustness proposed (PW1), driver question, report

**Asked (user):** the CoWaS should be robust to a power dropout at any time, with everything checked at restart; suggest what to implement and document it in roadmap 2 (early) and in the logs. The platform power shutdown is the most likely source (so not a CoWaS fault, but the CoWaS must still cope). Could a part of the manifold driver overheat or burn so that COM6 stops working? Document the observations in the field logs and the report, without a firm conclusion in the report.

**Recorded**
- Stage 2 §7.0 (new, early): PW1, power-dropout robustness. Hardware: safe pin defaults (pull-downs and the A4988 pull-up), a stop that works with a frozen Due (heartbeat-fed relay on the 24 V actuator rail through an external watchdog), logic and motors on separate rails with hold-up, an optional DC-UPS or battery, fuses and an isolated USB link, brown-out reset. Firmware: safe levels in the first lines of `setup()`, reset cause logged, a motion journal in non-volatile memory, a boot checklist that checks everything is off and flags the interrupted slot `SUSPECT`, an absolute turn limit on the manifold, a supply log, no automatic resume of a half-done sample. Bench test of dropouts in every phase. Also a row in the §7 table and a bullet in §3.2. **Proposed only; awaiting the user's go.**
- F258 (High, open): no defined behaviour for a power dropout. F257 updated with the user's assessment.
- Field log §38, update 2026-11-09: the user's assessment, the fact kept apart from the hypotheses, and the analysis of the driver and COM6 question. Short answer: heat alone is unlikely (thermal shutdown, and the driver is not on the USB path); a short or a burnt connector putting motor voltage on the Due's pins, or a supply collapse from a stalled motor, are possible. On a standard Due the 16U2 bridge shows a COM port even when the SAM3X is dead, so "no COM6 at all" points more to the USB path, the 5 V at the bridge or the laptop. Six cheap tests are listed.
- Field report draft: summary bullet, "Incident at the end of the campaign (6 Nov, under investigation)" in section 4 (observations, hypotheses, next steps, no conclusion), and remaining task 9. The counts of the report are not updated for schedule 3.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-10: incident reviewed against the final log, field report rewritten, sample list

**Asked (user):** weigh the hypotheses (platform power shutdown, no stop order after power returned, driver overpower or burn, and the effect on COM6); keep only the main hypothesis in the report; check the logs of the final samples; rewrite the report to show that the CoWaS worked and to give field feedback by cause; list the automatic samples (blanks first) and the manual samples in an Excel file.

**Recorded**
- [Field log §39](field-lexplore-geneva-2026-11-03.md#39-the-incident-reviewed-against-the-final-log-and-the-hypotheses-weighed-2026-11-10): the final log lines (`O-ring unlocked` 16:54:41, no `O-ring locked`, no error line, USB lost 16:55:45), the flash attempt of 10 Nov (COM6 is seen; `No device found`: the USB bridge answers, the SAM3X does not), a table of nine hypotheses with weights, the answers to the user's questions, and the tests that separate them. **Correction:** §38 and the first draft of the report said "no COM6"; that was true in the field, but on 10 Nov the port is seen.
- F257 updated.
- Field report rewritten (`docs/field/field-report-draft-2026-11.md`): 68 of 70 samples for the whole experiment (49 CoWaS filters = 47 samples + 2 blanks, plus 19 manual = 18 + 1 blank); Felix's 19 runs; what the field work fixed in the spool; feedback grouped by cause (prototype hardware, spool software, set-up and handling, measurement limits, design limits); the linear actuator holding the seal; the incident with the main hypothesis only, marked in progress.
- `docs/field/cowas-lexplore-sample-list-2026-11.xlsx`: sheet "Automatic (CoWaS)" (49 rows, the two blanks first) built from the sample JSON files; sheet "Manual" (19 rows: the blank first, then 18 samples with the paired CoWaS sample and the offset).

**To confirm with the user**
- The figure "68 of 70": the 68 is verified from the records (49 + 19). Counting the plan gives 69 (48 automatic + 2 CoWaS blanks + 18 manual + 1 manual blank), so the second missing sample is not identified in the logs.
- Which sample was lost to the operator entry error (the user's memory: slots 5-6 given instead of 4-5). No record found.
- The sheet label `E2_T3_01M_R1` was recorded as `E1_T3_01M_R1` in the E2 folder; the sheet uses the corrected name and says so.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-10 (second part): report revised

**Asked (user):** say "autonomous sampler", not CoWaS; "manual sampling" with Niskin bottles and the portable peristaltic pump (Vampire Sampler), not "hand sampling"; the project is Bois Chamblard, and its samples come first, next to LéXPLORE, to show the sampler working on two sites; the Bois Chamblard river work is not a 24 h series; keep the linear actuator (seal) and the manifold incident apart; the missing sample is lost "to an incident involving a power surge on the LéXPLORE platform"; short, bullet points, clear.

**Changed (documents only):** the report was rewritten on these lines (two-site summary, 14 Bois Chamblard samples with 13 completed, the 6 scheduled river samples from 27 Oct 20:45 to 28 Oct 16:10 that ran unattended overnight, 68 of 70 at LéXPLORE). The Excel sheets use "autonomous sampler".

**Label of the E2 T3 1 m sample:** `E1_T3_01M_R1` was typed in the E2 schedule file (`tools/DIEL-E2-schedule2.csv`) and so in the record. It is the E2 sample: the run is in the E2 session (experiment DIEL6, date 2026-11-05), slot 3, 01:04 to 01:36 on 6 Nov, between E2_T3_05M_R1 and E2_T4_20M_R1, and the real E1_T3_01M_R1 already exists (4 Nov, slot 12). To confirm: the label written on the filter tube.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-11: report states the purpose

**Asked (user):** say in the report that the field work tested the robustness of the revised code, an extensive upgrade; focus on the robustness, sampling and leak-sealing improvements.

**Changed (documents only):** the field report has a "Purpose of the field work" line under the summary intro and a short "revised firmware under test" list (robustness, sampling, leak sealed) at the end of section 2. Not committed.

### Session 2026-11-11 (second part): size of the revision, with numbers

**Asked (user):** make it obvious in the report, with numbers, that the code revision is massive.

**Measured (`git diff --numstat baseline-v0 HEAD`, committed work only; baseline-v0 is 2026-09-24, 70 commits since):** `src` and `include`: 85 files, +9,385 / −1,582 lines (50 new files, 29 modified, 6 deleted); the firmware went from about 8,284 lines in 58 files to about 16,300 in 102 (working tree, uncommitted work included). `src/Core` (pure logic): 42 files, +3,325. `test/`: 20 files, +2,888, none removed (the baseline had one test, the manifold geometry). `host/`: 35 files, +6,732. About 60 % of the original firmware files were modified or removed, and about 80 % of the original lines are unchanged: the drivers were kept.

**Changed (documents only):** the report's section 2 has a "Size of the revision" list, and the purpose line says "major revision". Not committed.

### Session 2026-11-11 (third part): size of the revision, corrected baseline

**Asked (user):** the project started on 4 August, so everything since is the user's revision; the numbers are higher.

**Baseline corrected:** the git history before the user's first commit (2026-09-23, parent `c75c79b`, merge of 2024-07-09, work by earlier authors) is the code the project started from. The user's August to September work is inside the first commit, so `baseline-v0` (2026-09-24) was too late as a baseline. Measured against `c75c79b`, working tree: `src` and `include` 48 files, about 5,600 lines at the start, about 16,600 now (107 files); about +13,200 / −2,200 lines; 58 new files, 26 modified, 4 deleted of the original 48; `src/Core` +3,484 lines; `test/` about 4,900 lines (the start had none: one README only); `host/` 35 files, +6,732. Committed work alone: +12,691 / −2,183, 72 commits.

**Changed (documents only):** the "Size of the revision" list in the field report. Not committed.

### Session 2026-11-11 (fourth part): what changed, not lines of code

**Asked (user):** lines of code are not a good measure; explain what was actually changed, and keep only a few impressive numbers.

**Changed (documents only):** the field report's section 2 now lists what changed (seal, pressure, safety, sampling, spool, operation, verification), with the numbers kept as secondary: 221 firmware unit tests in 22 suites (`RUN_TEST` count), 316 further tests for the host and the tools (111 + 205 test functions), 51 documented console commands, 187 findings and 109 decisions logged; the firmware about 5,600 to 16,600 lines. Checked against the 2024 start: it had an ABP driver and a `stop_pressure` option but no linear actuator or O-ring lock, so the report does not say that nothing stopped the pump before. Not committed.

### Session 2026-11-11 (fifth part): full extent of the work in the report

**Asked (user):** the report must show the full extent of the work: calibration functions, test loops and functions, run functions, scheduling, logging, the computer interface, everything around the pressure sensor, the spool robustness, the many robustness improvements, the seal, the new manufactured (3D-printed) parts.

**Changed (documents only):** the "What changed" list in section 2 of the field report is now organised by area (seal and parts, pressure sensor, calibration functions, test functions, run functions, scheduling, logging, computer interface, firmware robustness, spool, verification). Parts named from the LOG (O-ring seal for the PTFE, PLA inserts, concentric double O-ring, sensor adapter, reprinted outlet). Not committed.

### Session 2026-11-11 (sixth part): the original pressure sensor handling was not usable

**Asked (user):** the original pressure-sensor driver and `stop_pressure` option existed but were flawed and not functional in practice; say so.

**Checked in the 2024 code (`c75c79b`):** the sensor reading was used in `step_purge` and `step_sampling` only to detect an empty line (low-pressure threshold, `stop_pressure`); the SPI clock was 800 kHz (F124: garbage frames with a valid status, fixed 2026-10-09 with 100 kHz); the conversion was a fixed formula with no calibration (`BAR_FACTORa/b`; F126, the scale still disagrees with the manometer); no pump code limited the pressure. **Changed (documents only):** the "Pressure sensor" bullet in the field report's section 2. Not committed.

### Session 2026-11-11 (seventh part): review of the main log, work missing from the report, counts corrected

**Asked (user):** go through the main log and see which relevant improvements the report forgot.

**Added to the report (from the log):** the per-sample spool routine (fill with lake water, prime, rise, roll up and empty also after a failed sample, 40 m total, move log); the purge volume (3 L with the spool), the vent and the confident-volume rule; the ethanol cleaning procedure; the sample book and the run report tool; calibration of the manifold reference; the operator tutorial, the field and manual protocols, the lab protocols, the pin map, the command list, and the findings and decisions registers.

**Found in the log, and a correction:** the entry "wrong start answer (slots 4-5 typed as 5-6), the 1 m sample lost (F244)" identifies the operator-entry error: **E1_T5_01M_R1** (4 Nov). Its run went to slot 6, which holds no filter, so it is not a valid sample, though its record says PASS 1001 mL. So the auto sampler has **46 valid samples and 2 blanks** at LéXPLORE (48 filters), not 47 and 49, and with the 19 manual ones **67** samples are collected, out of 69 planned by the records (48 + 2 + 18 + 1). The user's figure was 68 of 70; the 70 and the 68 do not follow from the records, so the report now uses 67 of 69. The sheet lost the E1_T5_01M_R1 row (48 rows). The field log §33 had guessed the wrong sample for the human error; a correction is added there. The field log §36 problem 5 (E1_T5_01M, retried, PASS 09:14) still says PASS: the run passed but had no filter.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-11 (eighth part): the report split in two

**Asked (user):** split the report in two independent reports: a field report (successes and feedback, short bullets) and a report on what was done since August and why (structured; the field validation; what was revised, such as the spool diving to the deepest point at every sample; the strategies to shorten a sample). Check against the log, the decisions and the findings. Use "auto sampler" sparingly.

**Written (documents only):**
- `docs/field/field-report-draft-2026-11.md`, rewritten: summary, the two field works, results, what worked, field feedback (spool, hardware, set-up, power incident, measurement limits, design limits), next steps.
- `docs/revision-report-2026-11.md` (new): summary, starting point (August 2026), how the code was rebuilt and why (testable logic, one place per safety rule, logged changes), changes by area with the reason for each, sampling time, what the field work validated and revised, next. Linked from `docs/README.md`.

**Facts checked for the reports:** highest recorded pressure 3.44 bar in 155 records (cap 3.6 bar); vents: about 240 `released`, one `NOT released` (3 Nov 17:33); DNA Shield micro pump ran 190 s in every field run (250 s in the 2024 code); field step medians at LéXPLORE: purge 432 s, dive 263 s, filtration 813 s, shield 238 s, total 1724 s; Bois Chamblard total 1208 s; purge 7 L → 3 L (D50), full-power purge (D51), deployment-line emptying removed (D23), per-sample dive (D90, the group plan D60 withdrawn by D89); the full-power prime (D94), two-pass roll-up (D95) and step log (D96) built, not confirmed on the board. The 2024 code had an SD text log, disabled; no Python interface in that code.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-11 (ninth part): appendices

**Asked (user):** put further details in appendices.

**Added (documents only):** field report appendices A (samples planned and collected per series), B (the 8 problems at LéXPLORE), C (median time per step, from the `t_*_s` fields of the passed runs), D (manual and machine timing at LéXPLORE). Revision report appendices A (timeline), B (main settings before and now, with the reason: from decisions D19, D21, D32, D38, D43, D50, D51 and findings F124, F161), C (unit test suites and counts, 221 in all; `manifold_guard` built after the field, not flashed), D (spool changes made in the field, with their decision numbers). Not committed.

### Session 2026-11-11 (tenth part): the sampling logic and the pump history in the revision report

**Asked (user):** detail the logic (clogging criteria, end criteria of a sample, the pump logic) and what was tried and failed on the pump (PID values), the tries in an appendix.

**Added (documents only):** revision report section 4 "The sampling logic": pump control (PID to 3.0 bar, 10–60 % with water, 100 % for air and open lines, start from 0 % at 5 %/s, pause at 3.35 bar, resume under 3.15 bar, cap 3.6 bar, implausible-sensor stop); the end rules of a sample with their criteria and record names (volume 1000 mL; clogged: < 30 mL per 60 s pumping window, or the mean of the last two windows < 30 % of the first from the third window; stalled: < 3 mL per window; pressure held 20 s above 3.15 bar while paused; 30 min timeout; cap, sensor; `x`, `s`); the confident volume; the other steps (purge, spool prime, vent, DNA Shield, emptying, rotation). Appendix E "Pump control: what was tried", from F132, F133, F135, F137–F142, F145–F146, F149, F161, F196, D16–D19, D26, D27. Values checked in `include/Settings.h`, `src/Core/fill_stop`, `src/Hardware/Filter_watch.cpp` and `src/Step_functions.cpp`. Sections renumbered (sampling time 5, field 6, next 7). Not committed.

### Session 2026-11-11 (eleventh part): the field report's problem appendix completed

**Asked (user):** appendix B did not list all the problems.

**Cause:** it listed only the 8 problems that failed a run (field log §36, D97). **Changed (documents only):** appendix B of the field report is now B1 (the 8 failed-try problems), B2 (other problems at LéXPLORE, from F215–F258: manifold shifts and plate not square, USB drop, pump relay at reset, schedule after a failed roll-up, spool tube pulled by current, dead sensor unnoticed for about 3.5 h, console freeze, DNA Shield tube leak, wrong slot list, expired filter, spool move that did not start, swollen wood and mechanical failures, false zeros of 6 Nov, MilliQ blank at the pressure margin, the power incident, pump pauses, flowmeter and DNA Shield dose) and B3 (Bois Chamblard events, from its log §3). Not committed.

### Session 2026-11-11 (twelfth part): clean revision of the revision report

**Asked (user):** the cap was raised step by step "because we could": the Sterivex takes at least 3 bar and lower pressure gives no flow and clogs the filter too fast. Make the amount of work obvious without insisting; no criticism of the previous version; factual and professional; how it works now compared with before, and why; a clean revision.

**Changed (documents only):** `docs/revision-report-2026-11.md` rewritten: a neutral summary and a "scope at a glance" table (original against revised); each area in section 3 as Before / Now / Why; the pressure-limit rationale as given by the user (also in appendix B and E); neutral wording throughout ("original", "approaches tested", no judgments).

**Checked in the 2024 code (`c75c79b`):** the sample was collected in a container for a fixed time (`FILL_CONTAINER_TIME`) and then pushed through the filter until a low-pressure threshold or a time limit (`EMPTY_WATER_PRESSURE_STX_THRESHOLD`, `EMPTY_CONTAINER_TIME_FILTER`); the purge was three container purges (`PURGE_NUMBER`); pump powers 90 % (water) and 80 % (flush); `step_dive` and `step_rewind` existed in the sample sequence; DNA Shield 250 s. The 100 mL sample volume dates from the revision, not from the original. Not committed.

### Session 2026-11-11 (thirteenth part): reports folder, wording

**Asked (user):** say "before August 2026", not "2024 version"; the pressure sensor was calibrated against the manometer and the user trusts the calibration; make a report folder with both reports and the tutorial.

**Changed (documents only):**
- `docs/reports/` created: `field-report-2026-11.md`, `revision-report-2026-11.md`, `lexplore-sample-list-2026-11.xlsx` (moved from `docs/field/` and `docs/`), and a `README.md` index that links the operator tutorial. The links in the reports, `docs/README.md`, the LéXPLORE log and `findings.md` (F255) were updated. Earlier LOG entries still name the old paths.
- The operator tutorial stays in `docs/field/operator-tutorial.md`: ten files link to it (including `tools/sample_schedule.py`, which prints its path), so moving it would break them. The folder index links to it. A move with the links fixed can be done on request.
- Revision report: "before August 2026" replaces "2024" in the text and the table headings; the remark on the sensor scale (factor 2.4) is removed, the sensor is described as calibrated against the manometer.

**Not changed:** code, firmware, tests. Not committed.

### Session 2026-11-11 (fourteenth part): reports made more concise

**Asked (user):** make the reports more concise without losing important information; details can go to appendices.

**Changed (documents only):** `docs/reports/field-report-2026-11.md`: the "What worked" section merged into "Results", the design limits into the hardware feedback, the summary and next steps shortened (main text about 75 lines; appendices A to D unchanged). `docs/reports/revision-report-2026-11.md`: summary cut to three bullets, code organisation to four bullets, calibration, tests, cleaning and handover merged into one block, the field section condensed (main text about 120 lines including the logic tables; appendices A to E unchanged). Nothing removed that is not repeated elsewhere in the same report or in an appendix. Not committed.

### Session 2026-11-11 (fifteenth part): checks before the commit

**Run before committing:** `pio run -e due` SUCCESS; `pio test -e native` 221 passed (one failure first: `test_a_stop_after_turning_stalls` in `test/test_manifold_guard`, whose timing assumed the last 3-degree progress step at exactly 1000 ms; it is at about 938 ms, so the stall comes 62 ms earlier. The guard is right; the test now checks OK at 1000 ms + stall time − 100 ms and STALLED at +1 ms); host pytest 364 passed (needs `PYTHONPATH=src` in this venv: the editable install of `cowas` is not active); `pytest tools` 235 passed. **Correction:** the revision report gave "316 more tests" (a count of test functions); the pytest counts are 364 and 235.
