# Field log: Lexplore platform, Lake Geneva (2026-11-03)

Specific log for the field work on the Lexplore platform. The run folders of the day are in `docs/logs/test-logs/2026-11-03/`. Findings are added to [findings.md](findings.md) when they need a fix.

## 1. Mechanical alignment (major, frame)

- **The plate's orthogonality was off.** It was redone on site.
- This is not fixable in the software. It needs the frame to be changed in the **V2 redesign** (roadmap [stage 2](../roadmap/stage2-pilots.md) and [stage 3](../roadmap/stage3-full-system.md)). Recorded in stage 2 §7b.
- Consequence: the manifold slot offsets moved when the plate was redone. The offsets were set again by hand (see §2 and LOG 2026-11-03, offsets). The same offsets may move again after any frame work. Another reason for the new manifold structure.

## 2. Manifold: two zeros, three slots apart

**Observed (user):** the manifold sometimes switches from one zero to another, exactly 3 slots apart (67.5°).

**Code check (2026-11-03):**
- The encoder is absolute (AMT22). `setZeroSPI()` (`src/Hardware/Manifold.cpp`) is not called anywhere, so the software does not zero the encoder at boot or at reset. The software cannot create a second zero.
- So the second zero is physical: the rotor and the encoder shaft have moved relative to each other by a multiple of one slot (3 slots this time). The most likely cause is the loose encoder coupling (F187, F191).
- The code has no independent reference (no home switch on the manifold), so `actuator_home` cannot tell the two zeros apart. It only checks that the encoder agrees with the table.

**Action:**
- Now: check the encoder coupling is tight and the rotor does not turn on the shaft. Then `actuator_home` and `slot7` by eye, as in LOG 2026-11-03.
- V2: add a physical home switch on the manifold (independent of the encoder), and lock the coupling mechanically. Recorded in stage 2 §7b.

## 3. The pump turns on flash or reset

**Observed (user):** flashing the code or pressing reset makes the pump turn.

**Code check:**
- The pump's 24 V enable is pin D42 (`PUMP_ENABLE`). It is only configured in `setup()` (`Pump::begin`, then `stop()`).
- Until `setup()` reaches that point (after reset, or while flashing), D42 is not driven by the program. The pin floats or follows its default, so the 24 V relay can close.
- The DAC pin (`DAC1`, the speed) is also only set in `setup()`.
- `Pump::stop()` keeps a 500 ms delay before the enable goes LOW; the comment in the code says that removing it lets the pump run on reset.

**Recommended fix (not made yet, awaiting your go):**
1. Hardware: a pull-down resistor (about 10 kΩ) on D42 to GND, so the relay stays open while the pin is not driven. Check with a multimeter at reset.
2. Firmware: as the first statements of `setup()`, set D42 and the DAC output to 0 (`pinMode`, `digitalWrite`). This shortens the window but cannot remove the reset itself, so the hardware pull-down is the real fix.

**Status:** open. Do not flash or reset with the pump connected to 24 V until this is checked.

## 4. Spool

- **`spool_home` did not move.** Cause found (user's console, 2026-11-03): `REFUSED | spool_home (or spool_home drain=n): O-ring lock state is UNKNOWN - run actuator_home`. After a reset the O-ring state is UNKNOWN, and the default `spool_home` (drain=y) needs it known, because it first moves the manifold to the purge slot. The refusal printed nothing moving, so it looked like no action. **Fix:** `actuator_home` first (needs the manifold near a slot), then `spool_home`; or `spool_home drain=n` at once (no manifold move, no drain).
- The spool height is measured with the method in the tutorial (section 1, step 7). Its value is RAM only: repeat after any reset.

## Open items (summary, superseded by the end-of-day table below)

| # | Item | Owner | Where |
|---|---|---|---|
| 1 | Plate orthogonality off (redone on site) | V2 frame | stage 2 §7b |
| 2 | Two manifold zeros, 3 slots apart; no independent home | V2 structure | stage 2 §7b |
| 3 | Pump turns on flash or reset (D42 floats until setup) | HW + FW | §3 above |
| 4 | Slot offsets moved with the plate; set by hand | field | LOG 2026-11-03 |
| 5 | `spool_home` refused after reset (O-ring state UNKNOWN); `actuator_home` or `drain=n` | field, recorded | §4 above |
| 6 | Field note for later (not done): ask the sample's notes at the **end** of its sampling, not at the start of `run_sample` | firmware + tool, later | §5 below |
| 7 | Spool position reset (interrupt), fixed in code, to flash, bench check pending | firmware | §6 |
| 8 | Fill dive keeps Vman shut: does the tube fill? (pipework) | hardware | LOG 2026-11-02 |
| 9 | Purge at 100 % not confirmed; spool prime at 60 % | firmware | LOG 2026-11-03 index |
| 10 | Pump turns on flash or reset (see item 3, not yet fixed) | HW + FW | §3 |
| 11 | Schedule recovery: the 5 m sample of run 04 is invalid; the 1 m sample was not run (§9) | field | §9 |

## 5. Field note for later (not done)

- Ask the sample notes (the `notes` question) at the **end** of the sampling, not at the beginning of `run_sample`. The operator then knows what happened during the sample (clogging, leak, odd readings) before writing the note.
- Decided 2026-11-03: keep for later, no change made now. Scope when done: the device's question order in `run_sample` and the schedule tool's answer order (`tools/sample_schedule.py`).

## 6. Spool position reset during a multi-depth run (fixed in code, to flash)

- **What happened:** after the 20 m sample the spool position read 0 with no move. The next sample went about 5.9 m deeper (intake about 25.9 m instead of 5 m). The 5 m sample was aborted during its purge (no sample taken). The 5 m record (`depth_m=5.00`) is **not valid**.
- **Cause and fix:** see LOG 2026-11-03 "spool position reset fixed". The origin-switch interrupts now act only while a move runs.
- **Before running the spool again:** flash the fix, then check the idle position as described in the LOG.

## 7. Day log, 2026-11-03 (DIEL run)

- **Site and session:** Lexplore platform, Lake Geneva. Session meta: site `lexplore`, experiment `DIEL`, operator `quentin`, spool installed, deepest point 40 m. Spool height 90 cm (`spool_height 90`).
- **Run folder:** `docs/logs/test-logs/2026-11-03/04_field-DIEL-2026-11-03/` (`session.log`, `tests.csv`, `samples/`, `schedule.csv`, `slots.json`).
- **Schedule:** one timepoint, 16:10, with 20 m, 5 m and 1 m rows (slots 12, 11, 10 by the automatic order).
- **Timeline (log times):**
  - 16:10:00 — 20 m sample starts on slot 12. Dry tube dives to 39.10 m, purge primed (flow check PASS, 101 mL).
  - 16:13:27 — the spool is at 2090 cm out (20 m depth plus 90 cm). Purge starts (3000 mL target).
  - 16:36:51 — the 20 m sample ends PASS. The next row (5 m) is kept out, so the device prints `META|spool;kept_out=y;length_cm= 0;next_depth_m=5.00`. **The length reads 0. The spool was at 2090 cm.** Nothing moved in between.
  - 16:36:58 — the 5 m sample starts on slot 11. The code takes the "lower" branch and dives from 0 to 590 cm out: the tube really goes from 2090 to about 2680 cm out, so the intake sits at about 25.9 m depth.
  - 16:44:24 — the operator types `x` during the 5 m purge. The purge is aborted before any sampling, so slot 11 has no sample. The rewind rolls the tube up from the wrong reading. The 1 m row has not run.
- **After the `x` (the tool did not stop, see F225):**
  - 16:44:26 — rewind starts from the reset reading (0 cm) with a 60 s limit (F224). The tube was really about 2680 cm out.
  - 16:45:28 — rewind **timeout** after 62 s (the 60 s margin). The 5 m row is written FAIL. The tool starts the 1 m row at once (16:45:29).
  - 16:45:31 — the 1 m row dives from 0 to 4000 cm out (39.10 m) with the reading wrong; `x` at 16:45:50 stops it after 19 s.
  - 16:46:27 — the rewind from 750 cm out is stopped by `x`. The 1 m row is FAIL. Nothing was sampled on slot 10.
  - 16:47:36 — `spool_home drain=n`: timeout after 60 s, encoder reads −426.1 cm (the switch was never seen).
  - 16:48:53 — `spool_home` (drain): timeout after 60 s again; the last reading is `length_cm=0.0`.
- **Update (user):** the tube is at the top. The encoder reading is still unreliable (last value 0.0). Confirm the origin switch with `spool_status`; `spool_home drain=n` then resets the reading (field log §12, step D).
- **Status at the end of the day (from the log):** the tube position was **unknown** in the firmware. The last encoder reading is 0.0 after a timeout. Check the origin switch and the tube by eye before any spool command.
- **Diagnosis:** F214 (interrupt reset) is the likely cause of the first wrong position. It is not proven (no edge was logged). The later timeouts come from F224. The schedule not stopping is F225.
- **Data status:** the 20 m sample (slot 12) is valid. The 5 m record (slot 11, `depth_m=5.00`) is **not valid**: its purge ran at about 25.9 m and the sample was not taken. Slot 11's filter is clean. Slot 10's filter is clean.

## 8. Settings changed by hand today

| Setting | Before | Now | Set by | Where |
|---|---|---|---|---|
| `purge_angle` | 342.51 | **47.11** (`cal` on slot 7, encoder 536.00, morning), then **340.75** (encoder 3877.00, second shift the same day) | user | `include/Manifold.h` |
| `angle_offset_pos` (CW) | 5.6 | **3** | user | `src/Hardware/Manifold.cpp` |
| `angle_offset_neg` (CCW) | 2.5 | **3.7** | user | `include/Manifold.h` |
| Spool height | 0 (RAM) | **90 cm** (typed after the reset) | user | `spool_height 90` |

Whether the firmware that ran the DIEL run contains the `purge_angle` 47.11 and the offset values is not recorded. Check the flashed version before the next run.

## 9. Recovery plan for the aborted schedule (superseded: the 5 m and 1 m were run by hand, see §13)

1. **Stop the old list:** `schedule stop` in the tool. The old list still has the 1 m row waiting, and it must not start on the old firmware.
2. **Before flashing:** cut the 24 V pump supply (F215). Then flash the fix: `pio run -e due -t upload`. The reset clears the spool height and the manifold position.
3. **After the boot:** `actuator_home`, then `spool_status`. If the origin switch is not `PRESSED`, run `spool_home drain=n`. Then `spool_height 90`.
4. **Bench check of the fix:** with the spool idle and the tube out, tap the origin switch, then read `spool_status`. The tube length must not drop to 0.
5. **New run folder:** the old folder's schedule has started, so the file is ignored there. Make a new CSV with the 5 m row on slot 11 and the 1 m row on slot 10, at the same time:

   ```
   date,time,name,condition,spool,depth_m,force_slot
   2026-11-03,17:30,E1_T1_05M_R2,sample,on,5,11
   2026-11-03,17:30,E1_T1_01M_R2,sample,on,1,10
   ```

   Start the tool with a new tag and the new file: `tools\session_log.cmd --port COM6 --tag field-DIEL-2026-11-03-b --schedule redo.csv`.
6. **Start questions:** "Which samples are AVAILABLE?" → `10,11`. Then `session` (spool `y`, ENTER for 40 m), then `schedule start`, then `y`.
7. **Check the first run:** the 5 m sample must log `Spool fill: at 590 cm out, sample depth 5.00 m`. The 1 m sample must rise from 590 to 190 cm out. If the 5 m reading is anything else, `schedule stop` and call the engineer.

Manual alternative: `run_sample 11 depth=5`; at its "next sample depth" question type `1`; then `run_sample 10 depth=1`; at its question press ENTER.

## Open items (end of 2026-11-03)

| # | Item | Owner | Where |
|---|---|---|---|
| 1 | Plate orthogonality off (redone on site): frame change in V2 | V2 frame | §1, F217 |
| 2 | Two manifold zeros, 3 slots apart; no independent home | V2 structure | §2, F216 |
| 3 | Pump turns on flash or reset (D42 not driven until setup) | HW + FW | §3, F215 |
| 4 | Slot offsets moved with the plate; set by hand | field | §8, D62, D63 |
| 5 | `spool_home` refused after reset (O-ring UNKNOWN): `actuator_home` or `drain=n` | field | §4, F218 |
| 6 | Notes question at the end of the sampling (deferred) | later | §5, F222 |
| 7 | Spool position reset: fix in code, to flash; bench check | firmware | §6, F214, D65 |
| 8 | Fill dive keeps Vman shut: does the tube fill? | hardware | F219 |
| 9 | Purge at 100 %; spool prime at 60 %; print the applied power | firmware | F221 |
| 10 | Recovery of the aborted schedule (§9) | field | §9 |

## 10. Review and correction (2026-11-03, after the field run)

- The cause of the first wrong position is **not proven**. The code path is the origin-switch interrupt (F214), but no press, edge or move was logged, so the trigger is unknown. §6 and §7 say "likely"; read them as "possible".
- Fixes built, not flashed: the interrupt acts only during a move, only when the pin reads LOW, and idle edges are counted (`spool_status`). A kept-out tube is refused by the next fill if the encoder reads more than 5 cm away (F226). See the LOG review entry.
- Bench check before the next spool run: idle tube, tap the switch, read the idle-edge count and `tube out`.

## 11. Field notes from the user (to implement later, not done)

1. **Restart after an interrupted schedule (F228).** If the schedule is stopped (or the tool is closed), the operator must be able to get back in. Today a started list is ignored in its run folder (§9 step 5), so a new folder is made by hand. Wanted: on the next start, the tool shows what was done and what is left (from the state file: each row's status and slot), then asks where to restart, including an earlier timepoint even when it is now after that timepoint's time.
2. **The erase prompt must not hide the earlier sample (F229).** When a row's forced slot is used (or the operator types `run_sample <slot>` for a used slot), the tool asks to erase it, and the slot becomes available again. `erase` does not delete the saved records, but the slot then looks empty and nothing links the old sample to it. Wanted: the earlier sample is archived under its slot (for example in an `archive/` folder of the run, with its slot and time in the name) and the slot gets a new sample, so the records stay linked.

## 12. Restart order and what not to do (moved here from the handover, 2026-11-03)

The handover file was removed on 2026-11-03 (the user's request). Its restart steps are kept here. Each step needs the operator's go.

- **Step A, physical check:** note the tube position and the origin switch state (`spool_status`).
- **Step B, code (2026-11-03 update):** F224 fixed in code (fixed 20 min limit for origin moves, not flashed). F225 partly fixed (tool: stops on a failed roll-up and on a refusal; `x` still open). F214 and F226 to F227 are built, not flashed. Checks: `pio run -e due`, `pio test -e native`, `pytest tools/`.
- **Step C, flash:** with the pump's 24 V supply cut first (F215).
- **Step D, position after the reset:** (1) `actuator_home`. (2) `spool_status`; if the origin switch does not read PRESSED, run `spool_home drain=n` and watch the tube, and do not use `drain=y` until the manifold state is known. (3) If the switch reads PRESSED and the tube is at the top, `spool_home drain=n` moves nothing and resets the reading to 0 (the pressed-switch branch of `Motor::drive`, accepted when the reading is within 90 cm of 0). `spool_status` must then show `tube out` 0. (4) `spool_height` with the measured value (RAM only).
- **Step E, bench check of the interrupt fix:** idle tube at the top, tap the origin switch, `spool_status`: `tube out` stays 0, and the idle-edge count is shown.
- **Step F, one manual sample first:** `run_sample 11 depth=5`; the log must show `Spool fill: at 590 cm out, sample depth 5.00 m, tube full`. Only then continue.
- **Step G, the redo schedule:** section 9 above, with a new tag and a new CSV.

**Do not:**
- run `schedule start` in the old folder `04_field-DIEL-2026-11-03` (its list has started; nothing is waiting, but the state is misleading);
- run `spool_home drain=y` before F224 is fixed;
- run any schedule before Steps A to F are done.

**Open questions for the operator (asked 2026-11-03, not yet answered in the logs):** is the origin switch pressed now (`spool_status`)? Was the DIEL run firmware flashed with `purge_angle` 47.11 and the new offsets (not recorded, see section 8)? Should F225 be fixed before any spool command?

## 13. Day log, 2026-11-03 (Felix, DIEL and the night run)

Run folders (`docs/logs/test-logs/2026-11-03/`): `01` leman (no samples or purges recorded), `02` lexplore test, `03` felix, `04` DIEL. The night run has no folder yet (see the plan below).

### Runs and results (from the sample records)

| Folder | Slot | Sample | Volume (trusted, mL) | End | Result | Note |
|---|---|---|---|---|---|---|
| 02 | 1 | test, 12:49 | 1001 (1001) | volume | PASS | see F231 |
| 03 Felix | 2 | LEX-W1 | 152 (86) | clogged | PASS | |
| 03 Felix | 1 | LEX-W2 (test) | 128 (124) | stop key | FAIL | test, stopped by `x` at 13:52 (F231) |
| 03 Felix | 3 | LEX-W2-02 | 82 (27) | clogged | PASS | |
| 03 Felix | 14 | LEX-W3 | 882 (784) | clogged | PASS | |
| 03 Felix | 13 | LEX-BLANK-3 (blank) | 113 (90) | clogged | PASS | |
| 04 DIEL | 12 | E1_T1_20M_R1 (20 m) | 1001 (944) | volume | PASS | valid; the schedule, 16:10 |
| 04 DIEL | 11 | E1_T1_05M_R1, 1st try | none | aborted | FAIL | `x` in the purge at 16:44; no sample; the 5 m record of 16:37 was **not valid** (dive to 25.9 m, F214) |
| 04 DIEL | 10 | E1_T1_01M_R1, 1st try | none | aborted | FAIL | the schedule went on to it after the failed 5 m; `x` at 16:46; no sample |
| 04 DIEL | 11 | E1_T1_05M_R1, by hand 17:13 | 83 (39) | clogged | FAIL | valid sample (fill at 590 cm confirmed). The run is FAIL only because the roll-up timed out afterwards (see below) |
| 04 DIEL | 10 | E1_T1_01M_R1, by hand 17:37 | 515 (454) | clogged | PASS | |

Felix ran 3 samples and 1 blank to PASS. Slot 1 was a test (stopped by the operator). The LOG shows experiment `aireau`, operator `felix`; the project name in the notes is Bois Chamblard.

### Events

- **Spool height:** 90 cm typed at 15:58 (RAM only). The DIEL schedule could not start until the height was known (the tool blocks spool rows without it).
- **Spool rewinds (folder 04):** `spool_home` timed out four times (16:47, 16:48, 16:50, 16:51, each 60 s), then PASS at 16:52 (59.0 s, close to the old limit). The 17:34 rewind of the 5 m sample also timed out at 62 s (F224, the old 60 s limit).
- **Position reading (F214, reopened):** 20 m sample ended at 2090 cm out, the reading showed 0 at 16:36. The 5 m sample at 17:18 filled correctly at 590 cm, and the reading showed 0 again at 17:34 (during purge and shield, no move logged).
- **Operator `x`:** at 17:34 the "next sample depth" question got `x` (= none, roll up). The roll up then timed out (reading 0). This 5 m run was by hand (17:13), not the schedule. The schedule's own failure (continuing after the 16:45 roll up, F225) happened at 16:45–16:46.
- **USB:** the lexplore test run lost the port at 13:00 (`PermissionError`, F230). Reopened at 13:02.
- **Filters:** all clogged runs ended with low trusted volume (27 to 784 mL). The 5 m filter clogged at 83 mL (39 trusted).
- **Slot 1:** test only, both runs (12:49 and 13:52). No field sample used it (F231, closed).

### Code state at the end of the day

- Firmware: "code has been flashed" (user). **Which build is not recorded** (the purge_angle and offset values in §8 are not confirmed either). The 20 min origin limit (`SPOOL_HOME_MAX_MS`, F224, D72) was added after the flash: **it needs a flash**. Without it, a rewind on a wrong reading uses the old 60 s limit.
- Tool (no flash): the list stops on `spool_end=FAILED` and on any refusal (D71). Restart the tool before the night run.
- Tests: `pio run -e due` builds; `pio test -e native` 200 pass; `pytest tools/` 216 pass.

### Night run (plan, not started)

- File: `tools/DIEL-schedule02.csv`, 12 rows: 20:00 (20/5/1 m), 23:50 (20/5/1 m), 04:00 (20/5/1 m), 08:00 (20/5/1 m). Checked: the dates are `+0`/`+1`, all depths are inside the 40 m total, and each group takes about 85 min.
- Slots: the first 12 in the loading order, `1,2,3,14,13,12,11,10,9,8,4,5`, if these are the 12 reloaded filters (**check the physical slots**).
- Before 20:00 (checklist):
  1. Flash the 20 min origin limit, or accept the old limit. Then `actuator_home` if the O-ring is UNKNOWN, `spool_status` (height 90 cm, length 0, tube at the top, idle-edge count), `spool_height 90` again after any reset.
  2. New run folder with a new tag (not `04`), `session` (spool y, 40 m), answer "available" with the 12 slots, then `schedule start` with `DIEL-schedule02.csv`.
  3. Restart the Python tool first, so the new stop rules are loaded.
  4. Laptop on AC, cable locked, Windows updates paused, no typing on the console.
- Accepted risks (D73): the pressure cap is the only pressure protection; a reset turns the pump on (F215); an `x` does not stop the list (F225); a USB drop ends the schedule (F230).

### Open items (end of the day)

| # | Item | Where |
|---|---|---|
| 1 | Position reading reopened (F214): check `spool_status` before any spool run | §6, F214 |
| 2 | 20 min origin limit: flash it (F224). Kept for stage 1; revise in stage 2 §7b | §12 step C, F224 |
| 3 | `x` does not stop the schedule (F225): closed in code, tool restart needed | F225 |
| 4 | USB drop during the test run (F230): check cable and port | F230 |
| 5 | Slot 1 used twice (F231): closed, both tests | F231 |
| 6 | Pump turns on at reset (F215): no fix yet | §3 |
| 7 | Build on the board not recorded; purge_angle / offsets (§8) | §8 |

§9 (the recovery CSV) was not used: the 5 m and 1 m were run by hand (§13). §12 step B is partly done (see F224, F225).

**Update (after the night plan, 2026-11-03):** the tool now stops the list on any `x`, on a failed roll-up and on a refusal, and `schedule start <row>` restarts at a row (LOG 2026-11-03 "schedule stops on x and restarts at a row"; D74). F225 is closed in code. The 20 min origin limit (F224) is kept for stage 1 and needs a flash; revise it in stage 2 §7b. A run stopped before its filter was loaded (dive or purge) frees its slot (D75). The schedule file is not changed by a restart.

## 14. Manifold shifts: issue log (open, early fix in the roadmap)

**Issue:** the manifold reads as shifted by about 3 slots (67.5°) after a while, in either direction. It happened twice on 2026-11-03: in the morning (`purge_angle` to 47.11) and again later that day, 3 slots clockwise (`purge_angle` to 340.75). Each time the user redid `cal` on slot 7 with the O-ring locked. Nothing in the software zeroes the encoder (F216, §2), so the shift is physical: the rotor and the encoder shaft move relative to each other.

**`purge_angle` history (from [`Manifold.h`](../../include/Manifold.h) and [decisions](decisions.md)):**

| Date | Encoder (cal, slot 7) | `purge_angle` | Note |
|---|---|---|---|
| 2026-10-12 | 3884 | 341.37 | |
| 2026-10-20 | 3077 | 270.44 | realignment (F165) |
| 2026-10-22 | 3926 | 345.06 | encoder loose (F187) |
| 2026-10-23 | 3134, 3145, 3181 | 275.45, 276.42, 279.58 | encoder re-tightened; D40: 344.27 (3 slots) |
| 2026-11-03 morning | 536 | 47.11 | before: 342.51 |
| 2026-11-03 later | 3877 | 340.75 | 3 slots CW from 47.11 (−66.4°) |

**What is known:**
- The shifts are about 3 slots each time (67.5° = 768 encoder counts at 4096 per turn), and the encoder reads a different value at the same slot. Either the rotor slipped on the shaft or the shaft slipped in the encoder. Both are mechanical.
- The offsets (`angle_offset_pos` 3, `angle_offset_neg` 3.7) were not changed in any of the shifts.
- Not known: the time between the two shifts; whether a shift happens during a rotation, or after a long run of rotations; whether the O-ring lock or the seal drags the rotor.

**Early fix (roadmap, stage 2 HW9, before the next field run):** lock the coupling mechanically (clamp or set screw, with a witness mark), then check it with `encoder_repeat` and one `cal`. Decision D77.

**Each time, log:** the time of the shift, the `cal` encoder value, the number of rotations since the last `cal`, and whether the manifold was locked. Keep the history table up to date.

## 15. False origin press: the spool state and the steps to continue

**What happened (21:22):** the 1 m sample (slot 3) dove to 40 m, primed, and rose towards 1 m. The origin switch was held during the rise (the user believes the up button was pressed by accident). The encoder was reset there, so the rise stopped and the zero was wrong. Nothing was pumped. The spool is now **down**, not up, and its zero is not trusted. The 20 m (slot 1, retry at 20:24) and the 5 m (slot 2) are valid. Slot 3 is unused (failed before the filter was loaded): it is free for the 1 m.

**Fixed in code (not flashed):** F232, D78, D79. The press is discarded unless the tube can be at the origin (±30 cm) or the spool is homing. Details in [LOG 2026-11-03 false origin press](LOG.md#session-2026-11-03-false-origin-press-fail-safe-recovery).

**Steps to continue (after the reflash):**

*Before the flash*
1. `schedule stop` in the tool.
2. Look at the tube by hand: where it is, and whether it holds water. Note it.
3. Cut the 24 V pump supply (F215) before the flash.

*After the flash*
4. Restart the tool with the same tag, folder 05, and no `--schedule`. At "which samples are AVAILABLE": ENTER (slots 1 and 2 used, slot 3 free).
5. `session`: spool `y`, ENTER for 40 m.
6. `spool_height 90`. The height is RAM only: type it after the reset.
7. `actuator_home`. The O-ring state is UNKNOWN after the reset, and `run_sample` and `spool_home` drain need it known.
8. `spool_home` (drain, the default). The spool rolls up slowly to the origin switch, up to 30 min. Watch it. It empties the tube through the purge slot.
9. `spool_status`: origin switch `PRESSED`, tube out `0`. If it does not get there, stop and call the engineer.
10. `schedule`: the rows are numbered. Row **3** is the 1 m (slot 3).
11. `schedule start 3`, then `y`. Row 3 runs at once (its time has passed): a dry tube dives to 39.10 m, primes 100 mL, rises to 1 m, samples, rolls up. Then the 23:50, 04:00 and 07:00 groups run at their times.
12. Check the first lines of row 3: `Spool fill: dry tube, unrolling to 39.10 m`, then `Spool prime: flow check PASS`, then `Spool fill: at 190 cm out, sample depth 1.00 m, tube full` (1 m plus the 90 cm height). If the rise stops early, `schedule stop` and call the engineer.

**Watch for:** `WARNING | spool: origin switch held at ... cm out ... ignored`. One is normal. Repeats mean the switch wiring or the button needs a check. Tell the engineer.

**Open:** a bench check of the fix (F232). A false press during a homing move is still taken as the zero (residual). Pump on reset (F215). The manifold shifted again (§14).

**Addendum (before the flash):** a failure before the filter was loaded is tried again on the same row, twice, then the list stops (D81). A press is taken as the zero only if it is still held after the spool stops (D82). `schedule start 3` runs row 3 at once (its 20:24 time has passed), then the 23:50, 04:00 and 07:00 groups at their times.

## 16. Night run so far (folder `05_DIEL-session2-2026-11-03`, to 21:45)

| Time | Row (slot) | What happened | Result |
|---|---|---|---|
| 20:10 | 20 m (slot 1), first try | Spool dive to 39.10 m, then the water prime. Water came slowly (3 mL/min from 20:12). At 20:13:50 `x` was typed in the tool; 4 mL counted of 100. Prime failed before the filter was loaded | FAIL, filter 1 released |
| 20:13:50 | — | The `x` stopped the list (new rule). Nothing else ran | STOPPED |
| 20:23 | list | The tool re-read `DIEL-schedule02.csv` (20:23:01; the list had not started), then `schedule start` at 20:23:40. The 20 m row was run again, planned at 20:24 | — |
| 20:24 | 20 m (slot 1), second try (operator's `schedule start`, not the automatic retry, which was not yet built) | Prime 101 mL in 1.5 min. Sample ended by volume: 1001 mL, 835 mL trusted | **PASS, valid** |
| 20:53 | 5 m (slot 2) | Kept out from 20 m to 5 m, same timepoint | **PASS, valid** |
| 21:22 | 1 m (slot 3) | Rise from 590 cm towards 190 cm. The origin switch was held during the rise (the user believes the up button was pressed by accident). The encoder was reset there; the run failed before sampling (`origin switch reached before the goal`). The spool is now **down** | FAIL, filter 3 free |
| 21:42 | — | The user typed `schedule stop` | list stopped |

**State at 21:45:** the spool is down and its zero is not trusted. The tube is not at the origin. Slot 3 is free. Rows 4–12 (23:50, 04:00, 07:00) are still waiting in the run file.

**Board state:** the firmware on the board has the 20 min origin limit and the `purge_angle` 340.75. It does **not** have the false-press fix (F232), the homing confirmation (D82), the 30 min homing limit (D79) or the 300 ms hold (D83). The tool with the retry and the restart is only loaded in a restarted tool.

**Steps:** see §15, which still applies. The restart of row 3 (`schedule start 3`) runs the 1 m row at once, then the 23:50, 04:00 and 07:00 groups at their times. Full register of the changes: [LOG 2026-11-03 change register](LOG.md#session-2026-11-03-change-register-all-changes-of-the-day-board-state).

**Addendum (22:10, after the reflash):** the first `spool_home` from the unknown position failed with `origin switch press not confirmed`: the switch was not held once the drum stopped, so the zero was not set. `schedule start 3` then ran row 3, the dive failed the same way, and the list stopped on `spool_end=FAILED`. Nothing was pumped. The confirmation after the stop was removed (D82, LOG "confirmation removed"). The bench check must show where the switch sits at rest, since the 300 ms hold is now the only check at the top.

## 17. Night run 2026-11-04: pressure sensor and manifold encoder

**Pressure sensor (F234).** The last reading above 0.5 bar was at 00:58:52 (1 m sample, 23:50 group, slot 12). At 00:58:55 it read 0.04 bar, and it read 0.03–0.04 bar from then on. The pump kept running, and the software did not stop it: the only dead-sensor rule is for a negative reading (the first one came at 04:34:02, `implausible pressure reading (-0.51 bar)`, about 3.5 h after the drop). The 1 m sample went on to `filter stalled` at 01:00 (317 mL), then emptying. Every later run had a flat pressure: the prime at 04:02 counted 101 mL with 0.04 bar, and the 04:04 and 04:25 purges ran the same way. The sensor was found popped out, with water spilled (user).

**Manifold encoder (F235).** Bad SPI checksums at 04:17:25 (rotation to slot 11 refused: `could not get a starting position`) and at 04:34:11 (slot 10, and the rewind to the purge slot). Both runs failed; the encoder looked dry.

**What the logs say about the samples**

| Row | Sample | Pressure | Result | Note |
|---|---|---|---|---|
| 1–6 (20:24 and 23:50 groups) | 20 m, 5 m, 1 m | valid up to 00:58:52 | PASS | the 1 m (row 6, slot 12) was running at the drop; its last ~2 min have no valid pressure |
| 7 | 20 m, 04:00 (slot 11) | flat from 01:00 | FAIL, failed at sampling | purge ran with no pressure reading; rotation failed |
| 8 | 5 m, 04:00 (slot 10) | flat | FAIL, spool left out | rotation failed; the rewind could not run; the tube is out |
| 9–12 | 1 m (04:00), 07:00 group | — | waiting | the list is stopped |

**State now (2026-11-04, about 05:10).** The list is stopped (`spool_end=FAILED`). The pressure sensor is out and water is spilled. The manifold encoder gives bad checksums. The 5 m tube is out and may hold water. Slots 10 and 11 were not sampled; the tool marks them used.

**Before anything runs again**
1. Reinstall the pressure sensor and check it against the manometer (F126). A reading that does not move with the pump is a fault.
2. Check the manifold encoder cable and connector. Flash the new `encoder_read` command first; then `encoder_read 50` shows BAD frames if the link is still faulty. `oring_state` gives one reading without a flash.
3. Empty the spilled water, and check the tube and the purge outlet.
4. After 1–3: `actuator_home`, `spool_home` (drain), `spool_status`, then decide the rows with the operator.

**Software gap (for a decision):** a pressure reading that stays flat at about 0.03 bar while the pump runs at power is not a fault in the code. The fix would be a rule for a flat reading during pumping (F234); needs a decision and a flash.

**Addendum (2026-11-04, after drying the pressure-sensor pins):** `oring_state` is still intermittent: 162.95°, 354.20°, no valid reading, 162.95°. The 191° jump with no movement means the link or the encoder is still unreliable. The slot line gives slot 0 with 2.20° error for both angles, which does not fit. Do not rely on the slot line. No spool or manifold rotation until the encoder reads steadily. Checks: wiggle the encoder cable and connector while repeating `oring_state` (a change with the cable means the cable or connector); check the encoder shaft coupling for slip (HW9, F216); after the flash, `encoder_read 50` must show zero BAD frames.

## 18. START HERE: the night of 2026-11-03/04, the fault chain and the state (handover)

**Read this first in any new chat.** Everything below is also in LOG 2026-11-04 (night review, diagnostics, handover) and in F234–F238.

### 18.1 Timeline (log times, 2026-11-03 to 2026-11-04)

| Time | Event | Result |
|---|---|---|
| 20:10 | 20 m (slot 1), first try: the prime counted 4 mL in 2 min; `x` typed at 20:13:50 | FAIL before the filter, list stopped by the `x` rule |
| 20:24 | 20 m (slot 1), second try: prime 101 mL | PASS, 1001 mL (835 trusted) |
| 20:53 | 5 m (slot 2) | PASS |
| 21:22 | 1 m (slot 3): the origin switch was held during the rise (false press); the zero was reset | FAIL (F232) |
| 22:10 | first `spool_home` after the flash: the switch was not held after the stop | FAIL, list stopped; confirmation removed (D82) |
| 22:19–22:49 | 1 m (slot 3) from `schedule start 3` | PASS, 519 mL (335 trusted), clogged |
| 23:50–01:06 | 20 m (slot 14) PASS 1001 mL; 5 m (slot 13) PASS 1001 mL; 1 m (slot 12) stalled at 317 mL (293 trusted) | PASS |
| **00:58:52** | last pressure reading above 0.5 bar (3.07 bar) | normal |
| **00:58:55** | pressure reads 0.04 bar and stays at 0.03–0.04 bar; the pump keeps running. Nothing stops it | **pressure sensor fault starts (F234)** |
| 01:00:06 | 1 m sample ends `filter stalled` (2 mL/min) | the row is PASS in the tool; its last ~2 min have no pressure |
| 04:00 | 04:00 group starts | prime 04:02 counts 101 mL with a flat 0.04 bar |
| 04:04–04:17 | 20 m purge (slot 11 filter): ended `timeout` at 2353 mL (no pressure data) | |
| 04:17:25 | **first bad encoder checksum** (`bad checksum after 3 attempts`); the rotation to slot 11 is refused; sampling stops | 20 m FAIL, no sample taken (F235) |
| 04:19:57 | 20 m row FAIL; the spool rolls up (`origin switch held at -354 cm`, ignored, then home) | the list goes on |
| 04:23–04:34 | 5 m (slot 10): prime 101 mL; purge 04:25 with a flat pressure | |
| 04:34:02 | the first negative reading: `SAFETY implausible pressure reading (-0.51 bar)`; the purge ends `sensor` at 1547 mL | the only dead-sensor catch, about 3.5 h late |
| 04:34:11–04:34:34 | bad checksums again; rotation to slot 10 refused; rotation to slot 0 (purge) refused, so the rewind cannot run | 5 m FAIL, `spool_end=FAILED`, the list **STOPPED** at 04:34:36 |
| 05:39 | `abp_raw` with the pressure sensor connected and the encoder unplugged: `00 00 00` on every frame (MISO stuck low) | the sensor side is shorting the shared bus |
| after 05:40 | `oring_state` tests (times not logged): encoder unplugged: 162.25° four times, no bad frames. Sensor connected: 162.95°, 354.20°, no valid reading | the encoder is fine alone; the sensor breaks the bus |
| evening | the user dried the sensor pins; LEDs on the display go off when the sensor is unplugged and come back when plugged (F237); the monitor froze (F238) | a reset is planned |

### 18.2 Samples (from `samples/samples.csv`, folder 05)

- **Valid, through the night:** 20 m (slot 1, 20:24), 5 m (slot 2), 20 m (slot 14), 5 m (slot 13).
- **Pressure lost during the sample:** 1 m (slot 12, 23:50 group). The record is `stalled`, 317 mL (293 trusted). Its purge ended at 00:55 (before the drop). The sampling ran from about 00:58: treat the pressure record for the last ~2 min as invalid.
- **1 m (slot 3, 22:19):** `clogged`, 519 mL (335 trusted), PASS. Before the pressure fault.
- **Failed, no sample taken:** the 20 m (slot 11, 04:00) and the 5 m (slot 10, 04:00). Both purges ran without a pressure reading (the 5 m purge was counted at 1547 mL, and water was spilled). Slots 11 and 10 were **not sampled**, but the tool marks them used.
- **Not run:** the 1 m of the 04:00 group (row 9), and the 07:00 group (rows 10–12).

Total: 6 PASS, 2 FAIL, 4 waiting. The 04:00 1 m row's time has passed, so **a `schedule start` would run it at once.**

### 18.3 Hardware state

- **Pressure sensor:** shorts MISO to ground, or is damaged (`abp_raw` `00 00 00` with the sensor alone). Water spilled from its connector. Not usable.
- **Manifold encoder:** good when unplugged from the sensor (162.25°, stable). Intermittent when the sensor is connected.
- **O-ring:** LOCKED (as last read). After the reset it is UNKNOWN; `actuator_home` needs a valid encoder reading, so it cannot run until the sensor is out of the bus.
- **Spool:** out, not rolled up, zero not trusted; the tube may hold water. The purge slot and the manifold position are unverified (the encoder is unreliable).
- **Pump:** off (`end=sensor`). The 24 V supply status is not recorded.
- **Panel LEDs** (D22/D23): off when the pressure sensor is unplugged, on when it's plugged (F237).
- **Console:** frozen after the last command; a reset is planned (F238).

### 18.4 Before the reset (safety)

1. **Cut the 24 V pump supply** (F215): after a reset the pump relay state is unknown.
2. Keep the pressure sensor unplugged until it is replaced or checked.
3. Do not type anything into the console during the reset.

### 18.5 After the reset: steps for the next chat (in order)

1. Restart the tool on folder 05 (same tag, no `--schedule`), answer ENTER to "available".
2. `oring_state` (expect UNKNOWN after the reset). Encoder: must be a valid reading, steady over several reads, with the pressure sensor **unplugged**. If not: the encoder cable or connector (F235), no further steps.
3. Only after step 2 is steady: `actuator_home`; this needs the manifold within ±3° of a slot.
4. Replace the pressure sensor (or its cable). Then `abp_raw` must show a steady reading at rest (about 20 lines within about ±20 counts; about 1638 counts at 0 bar on this part). Check against the manometer (F126).
5. Only after 4: `spool_height 90`, `session`, `spool_home` (drain), `spool_status`.
6. The rows: do **not** run `schedule start` until the operator decides the 20 m and 5 m rows (slots 10 and 11, not sampled) and the 04:00 1 m row (row 9). Then `schedule start 9` (or the chosen row).

### 18.6 Open items (all linked)

F234 (pressure sensor fault and the missing flat-reading rule), F235 (encoder checksums; cause confirmed as the shared bus with the sensor), F236 (oring_state reads twice; a 2-bit checksum), F237 (panel LEDs off when the sensor is unplugged), F238 (console frozen), F215 (pump on reset), F233 (purge at the pressure cap goes on), HW9 (the manifold coupling, early fix), F230 (USB drop at 13:00), F232 (bench check of the origin press and the 300 ms hold).

### 18.7 Pressure sensor: can it be permanently damaged, and how to debug without a meter (2026-11-04)

**Observed:** without the pressure sensor, `oring_state` is stable (162.25°, slot 0, no bad frames). `slot7` works. With the sensor connected alone, `abp_raw` reads `00 00 00` on every frame (MISO stuck low).

**Cannot be decided from the repo:** the sensor's datasheet (ABPDANV150PGSA3) is not in the repo, so its pin map and its short-circuit rating are unknown. A short on a data line may or may not destroy this part. Water corrosion on the pins or inside the connector is the more likely cause.

**Debug plan, no meter needed:**
1. Look at the sensor pins and the connector with a magnifier: residue (green or white), burn marks, bent pins. Clean with 99 % isopropyl alcohol.
2. Dry the sensor and connector for at least 24 h in a warm, dry place with desiccant. No direct heat.
3. Plug in the sensor alone, run `abp_raw`. Steady counts (about 1638 at 0 bar on this part) mean moisture; `00 00 00` means go on.
4. If a spare sensor or cable exists: swap one at a time.
5. Borrow a multimeter (any electronics shop or lab). Power off. Resistance from the sensor's MISO pin to GND: low means a short in the sensor or its cable; high means the short is on the Due side, and that must be checked.
6. If it still fails: replace the sensor (same part number, same pinout).

Do not probe the pins with the Due powered. The Due's SPI pins are 3.3 V only.

**Decision (D85 stays):** no pump-based sample until the sensor passes `abp_raw` and matches the manometer.

### 18.8 Partial recovery after the reset (2026-11-04)

After the reset, with the encoder still connected, `abp_raw` reads valid frames: counts 1634–1643 (spread 6–7, about 0 bar), temperature 5 °C. `slot0` works. The earlier `00 00 00` is gone. The sensor also reports its own temperature in each frame (the tool prints it).

**Not yet proven:** the fault came and went before, so one good run is not enough. Still to do before any pump run: `oring_state` ten times with both connected (all valid, steady within about 0.1°); a wiggle test of the sensor cable while running `abp_raw`; a manometer check (F126, D85). Then the handover steps in §18.5.

### 18.9 Fix (2026-11-04): the pressure sensor is back, the encoder reads

**Result (user):** after the reset, the pressure sensor was dried and put back in the tubing. `abp_raw` gives valid, steady frames (see 18.8). With the sensor connected, the manifold encoder reads correctly, and `slot0` works. The manifold is good.

**Cause (best explanation, not proven):** water on the sensor's pins shorted the shared SPI bus (MISO) to ground. The encoder frames were corrupted whenever the sensor was connected (F235, F36). Drying the pins and refitting the sensor cleared the short. Whether the sensor is permanently damaged is not known.

**Still to confirm before any pump run (D85):**
- The sensor matches the manometer (F126). Not reported yet.
- `oring_state` ten times with both connected: all valid, steady. Not reported yet.
- The wiggle test on the sensor cable. Not reported yet.
- The panel LEDs: they went off when the sensor was unplugged (F237). Check they stay on.

**Restart plan (not yet done):** the saved schedule (folder 05) has rows 1–6 PASS, rows 7 (04:00 20 m, slot 11) and 8 (04:00 5 m, slot 10) FAIL with no sample taken, and rows 9 (04:00 1 m), 10–12 (07:00) waiting.
- Do not use `schedule start 6`. Row 6 is the 23:50 1 m, which already passed; it would run again.
- Use **`schedule start 7`**. It re-runs the failed 04:00 rows 7 and 8, then row 9 and the 07:00 group. Rows 1–6 are not touched.
- Slots 10 and 11 are marked used in the tool, although no sample was taken. Decide first whether to erase them (`run_sample 11` asks to erase). Otherwise the restart takes the next free filter in the loading order (slot 9).
- Before the restart: the spool is still out. Run the steps in 18.5 (`session`, `spool_height 90`, `actuator_home`, `spool_home` drain, `spool_status`).
- The 04:00 rows' times have passed, so `schedule start 7` runs them at once.

### 18.10 Checks passed and the restart (2026-11-04, user)

**Passed:** `oring_state` valid with the pressure sensor refitted; the wiggle test of the sensor cable passed; the panel LEDs are on with the sensor connected. Not re-tested: the LEDs when the sensor is unplugged (F237 stays open until that's checked). No manometer check: the user reports the calibration did not change, so the sensor's calibration is trusted (D85 amended).

**Slots:** the slots file (folder 05) shows slots 10 and 11 as used, although no sample was taken; slots 4–9 are available. At the next tool start, answer the available-samples question with **`4-11`**. Slots 1–3 and 12–14 stay used.

**Restart order:**
1. Restart the tool on folder 05 (no `--schedule`), answer `4-11` to the available question.
2. `session` (spool `y`, ENTER for 40 m), `spool_height 90`, `actuator_home`, `spool_home` (drain), `spool_status`.
3. `schedule`, then `schedule start 7`, then `y`.

**What `schedule start 7` does:** rows 7 (04:00 20 m), 8 (04:00 5 m) and 9 (04:00 1 m) run at once, one after another; each starts only when the one before has ended (about 28–30 min each, so about 1.5 h). The filters are taken in loading order: slot 11 for row 7, slot 10 for row 8, slot 9 for row 9. Then the 07:00 group (rows 10–12) runs. Each of those waits for 07:00 if the clock is still earlier. If the clock is already past 07:00 when the group is reached, it runs at once, one after another, as "late" rows. Rows 1–6 are not touched.

### 18.11 Changing the time of the remaining rows (2026-11-04)

**Why:** the tool cannot change the time of a row in a running list. The 07:00 group is to run at 08:00.

**Procedure used:** the user's own file (the one I created, `tools/<the user's own file>`, was removed at the user's request; the user writes the file) holds only the six rows still to run (04:00 20 m, 5 m, 1 m, then 08:00 20 m, 5 m, 1 m, day +1). Rows 1–6 are not in it, so they cannot run again.

1. Close the tool. Rename, do not delete, the folder's `schedule.csv` (it holds the statuses of all rows): `schedule_before_restart.csv`.
2. Restart the tool on folder 05 with the new file: `tools\session_log.cmd --port COM6 --tag <the same tag> --schedule tools\<the user's own file>`. The tool reads it because the folder has no saved list.
3. Answer the available-samples question `4-11`. Then `session`, `spool_height 90`, `actuator_home`, `spool_home` (drain), `spool_status`.
4. `schedule` (check the six rows and their times), then `schedule start` (no row number: the new file has only waiting rows), then `y`.

**Traps:**
- Do not use the original `DIEL-schedule02.csv` with `--schedule`: all 12 rows would run again.
- Do not delete `schedule.csv` without renaming it: the statuses of rows 1–8 would be lost.
- `schedule start 7` belongs to the old saved list. In the new file, row numbers start again at 1.

**Known limit, to fix later (F239). The rename-and-restart procedure above is a stopgap, not a fix: it is error-prone, and a time change should be done inside the tool.** a running or saved list cannot have a row's time or depth changed. Any change needs a new file, a new run folder or a rename, and a restart. This is error-prone in the field.

**Correction and confirmation (2026-11-04):** the new file has six rows: the 04:00 group (20 m, 5 m, 1 m) and the 08:00 group (20 m, 5 m, 1 m). The available-samples answer is the slot numbers, `4-5,8-11` (slots 4, 5, 8, 9, 10, 11). Do not use `4-11`: it would also free slots 6 and 7, which are not loaded.

**Renaming:** the saved `schedule.csv` is renamed (for example `schedule_previous_2026-11-04.csv`). The code only reads a file named `schedule.csv`, so the renamed file is never read again. A rerun with `--schedule` reads the new file and creates a new `schedule.csv`.

**This approach is not good.** It works for one night, but it is error-prone: a wrong file, a missing rename, or a file with passed rows would run samples again or lose the statuses. Changing a time should be done inside the tool (a `schedule time` command for waiting rows, saved to the list). That is not implemented; it needs design and tests, so it is recorded in F239 and stage 2 §7b, not built now.

**Dates in the schedule file (2026-11-04):** use the full date, not `0` or `1`. `+N` counts from the day `schedule start` is typed (the tool uses today's date at that moment), so after midnight `0` means 2026-11-04, and the 04:00 rows would be wrong. Write `2026-11-04` in the date column for the 04:00 and 08:00 rows. The format is `YYYY-MM-DD` (ISO, accepted by the tool). The file has the columns `date,time,name,condition,spool,depth_m`.

### 18.12 Run 2026-11-04 (folder `01`): start, and the open spool question

**Start (log times):**
- 06:27–06:28: the tool started with `tools\DIEL-schedule03.csv` (the user's file). The available-samples answer was `5, 6, 8, 9, 10, 11`.
- 06:28:58: `session` (lexplore, 2026-11-04, DIEL3, quentin, spool y, 40 m).
- 06:29:40: `actuator_home`: slot 0 aligned, error 2.55° (within 3°), O-ring UNLOCKED. PASS.
- 06:30:02: `spool_height 90`.
- 06:30:18: `schedule start`, then `y`. The six rows: 04:00 20 m / 5 m / 1 m (passed, so they run at once), then 08:00 20 m / 5 m / 1 m.
- 06:30:24: row 1 (04:00 20 m) started on slot 11.
- 06:32–06:34: dive to 4000 cm, prime PASS (101 mL, peak 0.52 bar: the pressure sensor works), rise to 2090 cm (20 m). 06:34:13: purge started (3000 mL).

**Open, to check now (F241):** no `spool_home` was sent after the reset. The dive log says "spool from 0 cm out", which means the firmware believed the spool was at its origin. After a boot it should say "unknown position" and roll up first. Nothing in the log explains why. The physical length of the tube is not verified. **Check the tube at the drum by eye, and `spool_status`, before the sampling finishes.** If the tube looks longer than about 20 m, or the origin switch is not pressed, stop the list (`schedule stop`) and use `x` for the purge if needed.

**Check slot 6:** the 08:00 1 m row will use slot 6 (the loading order gives 11, 10, 9, 8, 5, 6 to the six rows). Slot 6 was in the available list, so confirm a filter is physically loaded there. Slots 4, 5, 8, 9, 10 and 11 are the ones you loaded.

**Status at 06:36:** row 1 running (purge), rows 2–6 waiting. Samples are recorded as usual; the folder is `2026-11-04/01_DIEL-session2-2026-11-03`.

## 19. Notes for later (future improvements)

- **Schedule changes inside the tool (F239):** a row's time or depth cannot be changed in a saved list. Today's workaround (rename the saved list, restart with a new file of the remaining rows) is a stopgap. Future: `schedule time <row> <HH:MM>` (and depth) for waiting rows only, saved to the list; `schedule reload` for an unstarted list.
- **Spool tube pulled sideways (F240):** when there is a current, the tube is pulled sideways. First, the depth is inaccurate: the tube is at an angle, not straight, so the intake depth is wrong. Second, the spool module has friction close to the up button, where the tubing passes through a small guide piece. Future: measure the depth error at a known current; check and reduce the friction at the up-button guide. Until then, the depth of a spool sample is an estimate.

### 18.13 Leak at the DNA shield inlet (2026-11-04, user)

**What:** a small leak at the connection between the one-way valve and the T connector of the DNA shield inlet. The tube was starting to come off the T connector. The cause is a tube bent too sharply at that joint, the same problem seen before. Fixed on site by tightening the connection with Sirflex tubing (user's words).

**When:** the time it was noticed and first started is not recorded. Check it against the schedule: the 04:00 group ran 06:30–07:42, and the 08:00 20 m was running at 08:02.

**Effect to check:** the DNA shield step is time-based (the micro pump runs for a fixed time, not a measured volume). A leak on the shield inlet can mean less shield reached the filter than planned. Samples run while the leak was open may have a reduced shield dose. The file `064935387_E1_T4_20M_R1.json` (04:00 20 m) is one of them if the leak was already there at 06:49. Do not treat the shield as complete for those samples until the time of the leak is known.

**Action:** check the other tubing joints at the DNA shield and the one-way valve for the same bend; route the tube so it is not bent sharply at the T connector.

**Update (user):** the leak was very small and was seen around the time of the 08:02 status check. The user judged it no issue, so no sample is flagged for shield dose. Only the joint was tightened; the other shield joints are still to be checked for the same bend.

### 18.14 Top-switch presses on 2026-11-04: did the motor turn? (run 01)

| Time | Move | Switch pressed | Motor time before the press (approx.) | Result | Reading at the press |
|---|---|---|---|---|---|
| 07:41:24 | Rewind from 190 cm (04:00 1 m) | 0.8 s into the move | about 1.6 s, then home | the rewind ended home | 188 cm |
| 08:52:05 | Rise 590 → 190 cm (first try, 08:00 1 m) | 0.8 s into the move | about 1 s, then re-homed | `re-homed`, the sample retried | 587 cm |
| 09:13:38 | Rewind from 190 cm (08:00 1 m, retry) | 0.8 s into the move | about 1.6 s, then home | the rewind ended home | 188 cm |

**Reading:** at about 35 cm/s, one second of motion is about 35 cm. So in each case the tube was physically within about 50 cm of the top when the switch was reached, while the encoder said 188–590 cm. The encoder (or the drum position) was wrong by 140–590 cm at those times. The motor did turn, so it was not already at the top, but it was close.

**Affected depths (possibly wrong):** the 04:00 1 m (dive to 190 cm at 07:22), the 08:00 5 m (dive to 590 cm at 08:26, the 08:52 event shows the tube was at the top while the encoder read 590 cm), and the 08:00 1 m (dive to 190 cm at 08:57). The 08:00 20 m and the 04:00 20 m dives reached their encoder values without a switch event; their depths are not confirmed either.

**Not known:** whether the encoder drifted, the drum slipped on the shaft, or the switch sits lower than the top of travel. F240 (friction near the up button) is a candidate for the switch events; F241 is the position error.

**Check now:** at the drum, measure the tube's length from the switch to a mark, and compare it with `spool_status`. Keep spool samples off until this is answered.

### 18.15 End of day, 2026-11-04 (run 01): steps and record

**Steps (in order):**
1. `schedule`: all six rows must show PASS (they did at 09:14). Nothing waiting.
2. Spool: look at the tube and the drum by eye, then `spool_status`. Record the origin switch state and the tube length below. If the tube is out, or the switch is not PRESSED, run `spool_home` (drain) and check again. Do not leave the tube out overnight. Note any water in the tube.
3. Pump: cut the 24 V pump supply.
4. Ethanol cleaning of the tubing, with every filter out (tutorial section 5). Check the one-way valve and T connector joints (F242).
5. Type `:quit` + ENTER in the tool. The log closes.
6. Copy the run folder `docs\logs\test-logs\2026-11-04\01_DIEL-session2-2026-11-03\` (session.log, tests.csv, samples, schedule.csv, slots.json) and the paper sheet to the engineer.

**Record (to fill in):**
- Time of `:quit`: ________
- `spool_status` after step 2: origin switch ________; tube out ________ cm; water in the tube: yes / no
- Drum and tube by eye (distance from the switch to the tube mark): ________
- Filters used today: slots 11, 10, 9, 8, 5, 6 (PASS). Slots 1, 2, 3, 12, 13, 14 used earlier. Unsampled and free: none recorded.
- Samples with a position in doubt (F241): 04:00 1 m (slot 9), 08:00 5 m (slot 5), 08:00 1 m (slot 6). Their depth is not confirmed.
- Ethanol cleaning done: yes / no, time ________

### 18.16 Did the spool home after the last sample? And the depth-doubt rule (2026-11-04)

**After the last sample (08:00 1 m, retry):** the roll-up did run and reach the top switch: `Step rewind ... spool rolling up from 190 cm out` at 09:13:38, `home, 1.7 s`. The switch was reached 1.6 s into the move, so the tube was already near the top, not at 190 cm (see 18.14).

**Later, 10:26 (your `spool_home`):** the spool started at "0 cm out" and turned 139.8 s. The switch was reached at **−998 cm** on the encoder. So the tube was physically about 10 m out while the encoder read 0. The log has no commands or device lines between 09:14 and 10:25. The front panel's LEFT and RIGHT buttons move the spool (`main.cpp`, `ctrl_motor_spool`) and are not logged. The encoder did not count a ~10 m move. That is a second fault on top of F241: either the tube came out without the drum turning, or the encoder did not count.

**To find out:** did anyone use the panel spool buttons, or pull the tube, between 09:14 and 10:25? Record the answer here.

**Rule implemented (user request, not flashed):** a top-switch press that disagrees with the encoder sets a depth-doubt flag (`Motor::depth_doubt`). A homing move that reaches the switch without such a press clears it. Before the next sample, while the filter is still unused, a dry tube rolls up to the origin first (`spool_fill_and_rise`), so the depth of that sample is known. A wet (kept-out) tube is not rolled up there; it stops, and the failure path empties it. Built: `pio run -e due` passes; native tests 201 pass. Not bench-tested. Needs a flash.

### 18.17 End of this chat (2026-11-04): state, decisions and what the next chat does

**State at the end of this chat:**
- Run 01 (`2026-11-04/01_DIEL-session2-2026-11-03`): the schedule is complete. Six rows PASS (04:00: 20 m slot 11, 5 m slot 10, 1 m slot 9; 08:00: 20 m slot 8, 5 m slot 5, 1 m slot 6). Nothing is waiting.
- The spool was homed to the top at 10:28 (`spool_home`, 139.8 s, switch at −998 cm on the encoder before the homing). The tube should be at the top now: check with `spool_status`.
- The pressure sensor is refitted and reads (§18.8–18.10). The encoder reads with the sensor connected (§18.9). Manometer check waived by the user (D85 amended).
- Firmware on the board: the build flashed after the 340.75 change. Not on the board: the depth-doubt homing (D87), the 300 ms hold build, the 30 min homing limit, the `encoder_read` command (D84). Confirm which build is flashed before the next run.

**Decisions from this chat:**
- D87 (built, not flashed): homing before a sample when the depth is in doubt.
- **D88 (requested, not implemented):** home the spool between every sample, so the depth of each sample is known. Reason: if the spool loses its height or its position, homing between samples brings it back. The next chat implements this after the user confirms the design (see the prompt).
- **D89 (withdrawn by the user):** the Monday plan to prime at 40 m once, then run all samples from the deepest to the surface without a new prime. The user will remove it. Do not implement it.

**Findings from this chat:**
- F241 (open, High): depth position. Three top-switch presses disagreed with the encoder by 140–590 cm (07:41, 08:52, 09:13). The 10:26 homing found the tube about 10 m out with the encoder at 0, and nobody moved it through a logged command. Nobody used the panel buttons (user, 2026-11-04). The cause is not known. The next chat should bench-check the encoder against a physical mark.
- F243 (new, Medium): the panel LEFT and RIGHT spool buttons move the spool without a log line (`main.cpp`, `ctrl_motor_spool`). Any manual move is invisible in the record. Fix: log every panel move. Not implemented.
- F240 (open, Medium): sideways pull and friction near the up button (§19). Not measured.
- F234 and F235 (fixed in the field, user): the sensor is dried and refitted, and the encoder reads with it connected. Confirm at the next run.
- F236 (open): `oring_state` reads the encoder twice; a 2-bit checksum can pass a wrong frame.
- F237 (open): panel LEDs with the sensor unplugged: not re-tested.
- F238 (open): the console froze after the faults. Not explained.
- F239 (open, stopgap): a schedule's times cannot be changed inside the tool. Rename-and-restart is a stopgap, and it is error-prone.
- F242 (fixed on site, user judged no issue): a small leak at the DNA shield inlet, from a tube bent too sharply.

**The next chat (prompt in the chat; also here):**
1. Read §18 (START HERE), §18.12–18.17 and §19 of the field log, and the last LOG entries.
2. Do not flash, commit or delete files without asking. Do not implement D88 until the user confirms the design.
3. Ask the user the open questions: which build is on the board; whether the encoder is checked against a mark; the depth-doubt flash.
4. The planned work: (a) home the spool between samples (D88); (b) log panel spool moves (F243); (c) bench checks for F241 and F240.

## 20. Change of the spool rule (2026-11-04, user): every sample primes from the session's depth

- **Rule now:** before each spool sample the tube dives to the session's deepest point, primes at the purge slot (flow check, 100 mL), then rises to the sample depth. After the sample the tube is rolled up and emptied. No tube is kept out between samples, and no next-depth question is asked.
- **Same-time rows** run in file order (the Monday deepest-first order is withdrawn).
- **New log line:** every spool move prints `Spool move: ... | time <s> | encoder <a> -> <b> cm (<n> cm passed) | average <v> cm/s | start <p> % | ok|reason`. Use it to check the depth after a fault (F241).
- **Not yet flashed.** Build and tests pass (LOG 2026-11-04 archive entry). Archive: `docs/archive/code/superseded/spool-keep-out-removed-2026-11-04.patch`. Decision D90 (why: LOG and decisions.md).
- **Bench checks before the next spool run:** LOG 2026-11-04 archive entry, checklist items 1 to 5.

## 21. Pressure sensor mount (2026-11-04, user's proposal)

- **Problem:** the pressure sensor jumped out of its mount under pressure during the night run. The loss went unnoticed for about 3.5 h (F234). It was found popped out, with water spilled.
- **Proposal (user):** glue the sensor to the tubing so it cannot move under pressure. Add a strain relief on its cable.
- **Before the glue dries:** check the sensor is not under tension from its cable, and that no water reaches the sensor's connector.
- **After the glue has cured:** run `abp_raw` (steady at rest, within about ±20 counts), then the manometer check (F126). Log both in LOG.
- **Until it is glued:** check the mount at each session start and after each sample. A reading that falls to about 0 bar while the pump runs means stop and call the engineer.
- Roadmap: stage 2 §7b hardware row (F234). Stage 1 is closed, so it is not changed.

## 22. Afternoon run 2026-11-04 (folder `02_DIEL4-session4-2026-11-04`): schedule, blank, ethanol

Written at the end of the day from the session log and the sample records (user's request, 2026-11-04). Times are local (+01:00).

### 22.1 Set-up (13:01 to 13:05)

- Start answer `1-3,9-14` (read back, then `y`). Slots 4 to 8 were marked used (the answer had left them out). The read-back check, added after F244, worked in use.
- `actuator_home` was typed with a typo (`actutor_home`) and then ended by `x` at 13:02:51 during the overdrive lock. The lock state went UNKNOWN; a second `actuator_home` at 13:03 passed (slot 0, error 2.90°).
- `spool_height 90` was typed at 13:03:47 (RAM only, as usual after a reset).
- `session` at 13:04: site `lexplore`, date 2026-11-04, experiment `DIEL4`, operator `quentin`, spool `y`, deepest point 40 m.
- `spool_home` at 13:04:53, then `schedule start` at 13:05:53 with `DIEL-schedule04.csv` (9 rows, all `spool on`).

### 22.2 The schedule: all 9 rows ran, all PASS

| Row | Name | Slot | Depth (m) | Result | Sampled / trusted (mL) | End | Finished |
|---|---|---|---|---|---|---|---|
| 1 | E1_T6_20M_R1 | 1 | 20 | PASS | 1001 / 863 | volume | 13:35:59 |
| 2 | E1_T6_20M_R2 | 2 | 20 | PASS | 1001 / 875 | volume | 14:06:32 |
| 3 | E1_T6_05M_R1 | 3 | 5 | PASS | 635 / 541 | clogged | 14:34:11 |
| 4 | E1_T6_05M_R2 | 14 | 5 | PASS | 880 / 739 | clogged | 15:05:27 |
| 5 | E1_T6_01M_R1 | 13 | 1 | PASS | 1001 / 876 | volume | 15:36:43 |
| 6 | E1_T6_01M_R2 | 12 | 1 | PASS | 1001 / 938 | volume | 16:04:49 |
| 7 | E1_T7_20M_R1 | 11 | 20 | PASS | 1001 / 846 | volume | 16:35:50 |
| 8 | E1_T7_05M_R1 | 10 | 5 | PASS | 809 / 634 | clogged | 17:06:38 |
| 9 | E1_T7_01M_R1 | 9 | 1 | PASS | 1001 / 890 | volume | 17:37:17 |

- Every row ended with `spool_end=rolled_up`, emptied, and the schedule closed with "every sample has run, been refused or been skipped" at 17:37:17.
- While each sample ran, the tool printed `still waiting ... no TEST line yet` about every 10 minutes. That is the normal wait, not a fault.
- Warnings in the device log: `spool: origin switch held at ... not at the origin: ignored, the move goes on` appeared 12 times this afternoon (at 588, −598, 188, −199, −201 and −198 cm out, during moves). The morning folder `01` has 4 more. This is the F241 pattern (section 22.6).

### 22.3 The blank (negative control), 17:56 to 18:20

- After the spool test in 22.4, the blank was run as `run_sample 4`, with no `depth=`. The tool asked to erase slot 4 (`Sample 4 erased: available again`), answered `y`.
- The device printed `WARNING | run_sample: the spool is installed but no depth= was given: it is NOT moved, the intake stays where it is`. The spool did not move (confirmed in the code: a run without `depth=` skips the dive, the prime and the roll-up).
- Purge `3000 mL, spool installed` (the 3 L purge, as wanted). Name `E1-B1-00M-R1` (the name uses `-`, not `_`), condition `blank`. Result PASS, 1001 mL sampled, 943 mL trusted, end `volume`, slot 4.
- The tube was left at 16.4 cm out, not rolled up. The operator's description: the tip of the spool was in the milliQ blank water (not the lake). The spool height (90 cm) refers to the lake, so it does not describe this blank.

### 22.4 Spool test before the blank (17:46 to 17:50)

- At 17:46:35 the console received `célofanes ASIN B00PQPUARCspool_home`: a pasted line was sent in front of `spool_home`. The device ran `spool_home` (PASS, 29 s to the origin switch). Check the line before pressing ENTER when pasting.
- `spool_goto length=0.1` (12 cm, PASS), `length=0.15` (16.4 cm, PASS), `length=0.2` (target 20 cm): **FAIL, aborted with `x` at 17:50:51**. The move had passed 0 cm in 3.8 s (encoder 16 → 16 cm). Not explained: the operator pressed `x` 3.3 s after the start, so the move may not have started, or may have been held. Open question for the next bench check.

### 22.5 Ethanol purge, 18:37 to 19:03 (the D91 change, in use)

- Several lines were mistyped before the command (`ethnol_purge`, `thanol_purge`, `z`, `eth n`). `ethanol_purge` then started with `WARNING | NO FILTER may be installed` and asked **`Do all slots need cleaning? (y/n, x = cancel)`**. The operator answered `y`.
- `Slots to clean (14), in this order: 1 2 3 14 13 12 11 10 9 8 4 5 6 7`. The O-ring was LOCKED at the start, and no REFUSED line appeared (the D91 change works on the board).
- Result: `Ethanol cleaning complete: 14 slots cleaned and dried, 0 warning(s)`, O-ring LOCKED at the end, 19:03:15.
- This confirms that the board has the D91 build (the question text is only in that build). The branch `n` (ask which slots do not need cleaning) was not used here.

### 22.6 Spool origin warnings (F241 update)

- The 16 origin-switch warnings in folders `01` and `02` on 2026-11-04 happened at positions from −598 to +588 cm during moves. They were ignored by the rule of F232 (not at the origin, so the move went on). No move was stopped by them in this run.
- Whether the switch is wrongly pressed, or the encoder is wrong, is still open (F241). The move log (`Spool move:`) is the tool to check.

### 22.7 Earlier folder (2026-11-03 `01_field-leman-2026-11-03`)

- No sample was run. The only event is `actuator_home` FAIL at 11:07 (slot 14, error −4.75°, tolerance 3.0°), recorded here so the folder is not unexplained. No action taken.

### 22.8 Open items from this afternoon

- The `spool_goto length=0.2` move that passed 0 cm (22.4).
- The F241 origin warnings (22.6).
- The `n` branch of the ethanol question and the UNKNOWN refusal of `ethanol_purge` (the D91 bench checks, LOG 2026-11-04) are not yet tested on the board.
- The blank tube at 16.4 cm: check where the intake is before the next spool sample.
