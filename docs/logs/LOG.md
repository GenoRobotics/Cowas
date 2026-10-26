# CoWaS project log

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
