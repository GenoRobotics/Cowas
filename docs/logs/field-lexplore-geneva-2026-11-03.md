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
| 9 | Purge at 100 % not confirmed; spool prime at 60 % (superseded 2026-11-05: the prime may run at 100 %, D94) | firmware | LOG 2026-11-03 index |
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
| 9 | Purge at 100 %; spool prime at 60 % (superseded 2026-11-05: 100 %, D94); print the applied power | firmware | F221 |
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

## 13. Day log, 2026-11-03 (air/water runs, DIEL and the night run)

**Moved (user, 2026-11-06):** the air/water runs of this day (folder `03`) are in the Bois Chamblard log, section 6 of [the Bois Chamblard log](field-bois-chamblard-2026-10.md). They are not counted in the LéXPLORE E1/E2 results. The text below is kept as first written.

Run folders (`docs/logs/test-logs/2026-11-03/`): `01` leman (no samples or purges recorded), `02` lexplore test, `03` air/water, `04` DIEL. The night run has no folder yet (see the plan below).

### Runs and results (from the sample records)

| Folder | Slot | Sample | Volume (trusted, mL) | End | Result | Note |
|---|---|---|---|---|---|---|
| 02 | 1 | test, 12:49 | 1001 (1001) | volume | PASS | see F231 |
| 03 air/water | 2 | LEX-W1 | 152 (86) | clogged | PASS | |
| 03 air/water | 1 | LEX-W2 (test) | 128 (124) | stop key | FAIL | test, stopped by `x` at 13:52 (F231) |
| 03 air/water | 3 | LEX-W2-02 | 82 (27) | clogged | PASS | |
| 03 air/water | 14 | LEX-W3 | 882 (784) | clogged | PASS | |
| 03 air/water | 13 | LEX-BLANK-3 (blank) | 113 (90) | clogged | PASS | |
| 04 DIEL | 12 | E1_T1_20M_R1 (20 m) | 1001 (944) | volume | PASS | valid; the schedule, 16:10 |
| 04 DIEL | 11 | E1_T1_05M_R1, 1st try | none | aborted | FAIL | `x` in the purge at 16:44; no sample; the 5 m record of 16:37 was **not valid** (dive to 25.9 m, F214) |
| 04 DIEL | 10 | E1_T1_01M_R1, 1st try | none | aborted | FAIL | the schedule went on to it after the failed 5 m; `x` at 16:46; no sample |
| 04 DIEL | 11 | E1_T1_05M_R1, by hand 17:13 | 83 (39) | clogged | FAIL | valid sample (fill at 590 cm confirmed). The run is FAIL only because the roll-up timed out afterwards (see below) |
| 04 DIEL | 10 | E1_T1_01M_R1, by hand 17:37 | 515 (454) | clogged | PASS | |

The air/water project ran 3 samples and 1 blank to PASS (counted in that project). Slot 1 was a test (stopped by the operator). The LOG shows experiment `aireau`, operator `felix`; the project name in the notes is Bois Chamblard.

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

## 23. Improvements from the field work (2026-11-05), done and prepared

Reviewed the field log (§§ 1 to 22) for items that can be improved without changing the running Lexplore experiment. Rule used: **done now** only if it touches documents, a new offline tool, or nothing the running machine or session tool uses. **Prepared** if it needs a firmware flash, a change to the session tool that is running, or a hardware change. Nothing on the board or in the running tool was changed.

### 23.1 Done now (no effect on the running experiment)

| Item | Where | Source |
|---|---|---|
| Normal-looking messages explained (cap-margin pauses, the reminder every 10 min, depth in doubt, prime tries, the stalled spool move, the blank's `NOT moved`, `Sample N erased`) | operator tutorial, section 3 table | §§ 18, 22; today's log |
| A `[FAIL]` row is not waiting: a plain `schedule start` skips it; use `schedule start <row>` | operator tutorial, section 2b | 2026-11-05 (E2 row 1) |
| Do not paste text into the console; read the line before ENTER | operator tutorial, section 3 | 2026-11-04 (pasted line before `spool_home`); 2026-11-05 (typo `shcedule_start`) |
| Prefilter warning (the 20 um prefilter goes first, nothing between it and the filters) | operator tutorial, section 0 | 2026-11-05 (F248) |
| Blank procedure, no spool movement | operator tutorial, section 2d | §22.3, D92 |
| Read-only run report: pauses, purge flow at each minute, prime tries, spool warnings, error and abort counts | `tools/run_report.py`, tests in `tools/tests/test_run_report.py` (pytest tools/: 228 passed) | today's manual checks (purge flow 49 to 501 mL in 5 min on 2026-11-05) |
| Run report added to the end-of-day steps | operator tutorial, section 4 | this list |

### 23.2 Prepared, not done (needs a flash or a change to the running session tool)

| Item | What is needed | When | Ref |
|---|---|---|---|
| Ask the sample notes at the **end** of the sampling | device question order in `run_sample` and the schedule tool's answer order | after the campaign | §5, F222 |
| Change a schedule row's time or depth inside the tool | `schedule time <row> <HH:MM>` and `schedule reload` (tool) | after the campaign | F239 |
| Print the applied pump power on the purge and prime lines | firmware: `Pump.cpp` (the commented print) and the purge and prime lines | flash after the campaign | F221 |
| Prime at 100 % and the retry (3 tries at the same depth) | the retry is on the board (`Spool prime (try n of 3)` in the 17:10 and 17:42 log lines). The 100 % prime is not confirmed in the log: check the build before the next prime | bench check after the campaign | D93, D94, F247 |
| Pause and resume messages (cap margin, short pause warning), and the pump start wording | built, not on the board yet (today's log still shows `started at once` and no `RESUME after`) | flash with the next firmware change | F249 |
| Pressure reading after a pump stop (100 ms trace, pump off and line shut) | bench test, no code change until it is read | between runs, pump off | F249, F126 |
| Prefilter check at `session` (a question, or a tool warning) | session tool change | after the campaign | F248 |
| Pressure sensor glued to the tube, strain relief on the cable | hardware, between runs, with the machine off | between runs | §21, F234 |
| Spool friction at the up-button guide, depth error at a current | mechanical | after the campaign | F240 |
| Up-switch holder: replace the wood with a material that does not take up water | mechanical (V2) | V2 | F246 |
| Spool homing stall of 16:26 on 2026-11-05 (encoder not counting) | check the tube and the top by hand; not explained by the prefilter | before the next spool run | F246, F247 |
| `ethanol_purge` branch `n` and the UNKNOWN refusal | bench check of D91 | next ethanol cleaning | LOG 2026-11-04 |

### 23.3 Not changed on purpose

- The pause logic. The pump restarts after 0.2 s because the reading is already below the resume level at the stop. Changing it would change every purge and sample on the Lexplore run (see F249).
- The running `session_log` tool. The board has the prime retry (the `try n of 3` lines are in today's log). The 100 % prime and the message changes (pause, pump start) are not confirmed on the board.

## 24. Spool mechanical failures (2026-11-05, evening)

Found after the stall at 19:53 (the 1 m sample E2_T1_01M_R2, F250). Three parts, in this order. The exact timing of the fixes is not in the log.

1. **Level-wind guide, not the belt.** The guide is the part that moves left and right in front of the drum while the spool rolls back, so that the line lays evenly on the drum (a *level-wind* or traverse guide). It is 3D printed.
2. **Wooden frame screw loosened.** The guide's mounting screw is in the wooden structure of the spool. The front screw had come loose, so the guide moved. Probably the wood got wet again (a different wood from the top-switch holder of §22, which also swelled). Wood should not be used in a water sampler. Fixed by re-tightening the screw.
3. **Drive belt slack.** With the mounting loose, the belt that drives the drum was not under tension and could not turn it. Fixed by re-tightening and securing the belt with zip ties.
4. **Level-wind guide cracked.** The 3D-printed guide cracked at the same time or soon after, so it no longer turned on its shaft. Trial fix: super glue between the guide, its shaft and the drive wheel. **Not verified under load.**

**Effect on the record:** the stalls of 4 and 5 November (F245, F247 area) and the 19:53 failure are consistent with these faults, but the move log has not yet been checked against them.

**Next:** check the move log for stalls against these events; replace the wooden parts of the spool with water-resistant material, and print the guide in a tougher material with metal fixings; check the glue under load before relying on it. Keep the spool out of service until the guide turns freely by hand over the full length.

## 25. Ethanol purge after the Tuesday night restart: not in the log, confirmed done (resolved 2026-11-06)

**Resolved (user, 2026-11-06):** the team's own notes prove that the ethanol purge was done after the Tuesday-night restart (3 November). The doubt below is closed: no sample is affected, and no sample needs a contamination flag for it. The purge is still absent from the session logs, so the proof is the team's notes, not the repository: keep them with the run records. The text below is kept as first written.


**The doubt, stated plainly.** The team is almost sure that the ethanol cleaning was done after the restart on Tuesday night (3 November) and before the Wednesday samples were taken. **No ethanol purge appears in any session log between the restart at 19:50 on 3 November and the cleaning at 18:38 on 4 November** (the only one in the logs; §22.5). The record is therefore incomplete, and this note is here so that it is not lost. If the purge was done and not logged, the doubt is harmless; the table below says which samples would be affected if it was not.

**Why it matters:** the tubing carries water from one sample to the next. A missed cleaning between two filter sets could carry DNA from one sample into the next. In the bioinformatics, cross-contamination in the samples listed below would point to a missed cleaning.

**Samples taken with no logged cleaning before them** (restart 3 November 19:50 to the cleaning of 4 November 18:38). Slot numbers are the manifold slots, depths in metres, `PASS` means a filter was used:

| Time | Slot | Sample | Depth | Result |
|---|---|---|---|---|
| 3 Nov 20:53 | 1 | E1_T2_20M_R1 | 20 | PASS |
| 3 Nov 21:21 | 2 | E1_T2_05M_R1 | 5 | PASS |
| 3 Nov 22:49 | 3 | E1_T2_01M_R1 | 1 | PASS |
| 4 Nov 00:20 | 14 | E1_T3_20M_R1 | 20 | PASS |
| 4 Nov 00:47 | 13 | E1_T3_05M_R1 | 5 | PASS |
| 4 Nov 01:06 | 12 | E1_T3_01M_R1 | 1 | PASS |
| 4 Nov 06:55 | 11 | E1_T4_20M_R1 | 20 | PASS |
| 4 Nov 07:21 | 10 | E1_T4_05M_R1 | 5 | PASS |
| 4 Nov 07:42 | 9 | E1_T4_01M_R1 | 1 | PASS |
| 4 Nov 08:25 | 8 | E1_T5_20M_R1 | 20 | PASS |
| 4 Nov 08:51 | 5 | E1_T5_05M_R1 | 5 | PASS |
| 4 Nov 09:14 | 6 | E1_T5_01M_R1 | 1 | PASS |
| 4 Nov 13:35 | 1 | E1_T6_20M_R1 | 20 | PASS |
| 4 Nov 14:06 | 2 | E1_T6_20M_R2 | 20 | PASS |
| 4 Nov 14:34 | 3 | E1_T6_05M_R1 | 5 | PASS |
| 4 Nov 15:05 | 14 | E1_T6_05M_R2 | 5 | PASS |
| 4 Nov 15:36 | 13 | E1_T6_01M_R1 | 1 | PASS |
| 4 Nov 16:04 | 12 | E1_T6_01M_R2 | 1 | PASS |
| 4 Nov 16:35 | 11 | E1_T7_20M_R1 | 20 | PASS |
| 4 Nov 17:06 | 10 | E1_T7_05M_R1 | 5 | PASS |
| 4 Nov 17:37 | 9 | E1_T7_01M_R1 | 1 | PASS |
| 4 Nov 17:57 | 4 | E1-B1-00M-R1 (blank, no spool movement) | - | PASS |

Also in this window, with no filter used: the failed tries of 3 November 20:15, 21:22, 22:10 (slots 1 and 3) and of 4 November 04:19 and 04:34 (slots 11 and 10), and the failed try of 4 November 08:52 (slot 6). A failed try before the filter is loaded does not use a filter, but water may have passed through the tubing.

**Samples before the restart** (no restart in between): the 3 November samples from 12:55 to 19:50 in folders `02` to `05` of 3 November. Nothing in the logs cleans the tubing before the restart either; the last cleaning before the field period (2 November) is not in the logs at hand.

**Samples after the cleaning** (cleaning on 4 November, 18:38 to 19:03; the only one in the logs): from 5 November 16:06 onwards (the E2 series, folder `01_E2-DIEL1-session5-2026-11-05`). These are not affected by the gap above.

**Action:** keep this table with the sample records. If cross-contamination shows up in the bioinformatics for samples in the first table and not for those after the cleaning, the Tuesday-night purge was probably not done. Note also that a restart on 4 November at 06:27 and at 13:01 had no logged cleaning either; the same check applies to them.

**Update (user, 2026-11-05):** the team is sure that every other ethanol purge was done, including the cleanings for the restarts of 4 November (06:27 and 13:01), and that the only one in doubt is the Tuesday-night purge (3 November, after the restart at 19:50). None of these is in the session logs; they are confirmed by the team only. The table above therefore applies to the Tuesday-night doubt alone: if the purge was done, no sample is affected.

## 26. Handover: end of the E2 series (2026-11-05, 23:10)

**Status**
- E2 schedule (`01_E2-DIEL1-session5-2026-11-05`): all nine rows PASS. Row 6 (E2_T1_01M_R2) was restarted with `schedule start 6` at 20:02 after the spool fault (§24) and passed at 20:32. The three E2_T2 rows then ran from 20:32 to 22:12.
- Session closed at 23:10 (`:quit`).
- Ethanol cleaning of the tubing at the end of the day: first attempt at 22:15 stopped by the operator with `x` after 9 s (the prime of the purge slot). Second attempt from 22:17 completed at 22:41: 14 slots cleaned and dried, 0 warnings, O-ring locked.
- Spool: the mechanical repairs of §24 are done on site and not verified under load. HW10 (roadmap §7.4) is not done.

**Records to reconcile**
- The samples folder holds 19 records, the sample table 18. The extra one is `165901327_auto-1.json`: the purge of 16:59 on 5 November, stopped with `x` by the operator. The table does not add aborted purges. Keep the record; it is not a sample.

**Firmware on the board**
- On the board: the prime retry (`Spool prime (try n of 3)` in the log).
- Not yet confirmed on the board: the prime at 100 % (D94) and the pause and pump-start message changes (F249). Flash after the next run, then check them in the log.

**Before the next run (checklist)**
1. Spool: check the guide, the belt and the top switch by hand; `spool_status`; `spool_home` only when safe. Confirm with the team whether the spool runs before HW10.
2. Prefilter in place, first in the line (section 1 of the operator guide); check the flow at the first purge.
3. Ethanol cleaning before the new filter set, and at the end of the day (as done on 5 November).
4. Session answers (site, date, experiment, spool, deepest depth), then `schedule` to check the rows.
5. Write the planned times and filter IDs on the paper sheet.

**Still open (details in the logs)**
- Manifold alignment: the main limitation, needs the frame redesign (F216, F217).
- Pressure sensor: glue to the tubing (F234).
- Spool mechanics: HW10 (F250, F246).
- Spool top-switch readings and the stalls: check the move log against the mechanical faults (F241, F245).
- Applied pump power not logged (F221).
- Tuesday-night ethanol purge: not in the log, proven done by the team's notes (§25, resolved 2026-11-06).

**Repository (not committed, for the user to decide)**
- Branch `shield_V2`, last commit `591c0ea`.
- Changed since: `src/Hardware/Pump.cpp`, `src/Step_functions.cpp`, `src/Core/spool_fill.*`, `include/Settings.h`, the tests, the operator guide, the session-log guide, the roadmaps, the CLAUDE.md status lines, the logs.
- New: `tools/run_report.py` with its tests, `docs/reports/field-report-2026-11.md`.
- Firmware built, tests passing: `pio run -e due`, `pio test -e native` (203), `pytest tools/` (228).

**Next run:** scheduled by the user. The time, the rows and the depths are in the user's schedule file, not yet in the logs; add them when the run starts.

## 27. The false zero and the two-pass roll up (2026-11-06)

**What was seen.** At the end of the 08:00 group the 1 m sample (E2_T5_01M_R1) finished, but the spool stayed about 1 m out (the operator's observation). The log shows why: the roll up reported "home" after 0.7 s, with the encoder going from 190 to 0 cm (266 cm/s; a roll up runs at about 40 cm/s). The drum had stopped at the top-switch holder at about 188 cm. The switch stayed pressed with no pulses for 300 ms, and the hold rule (D83) accepted that as the origin, so the encoder was set to 0 with the tube still out (F251).

The same happened at 12:58 (590 cm out, 0.7 s). At 12:00 the next sample's roll up found the switch held at −186 cm: the encoder and the switch no longer agreed about where the zero is.

**Effect on the samples (user, 2026-11-06).** The concern was that the intake could have been deeper than recorded after a false zero. The user checked and the zero seemed to work: the later dives and rises measured normal distances at normal speed (dive about 99 s at 40 cm/s, rises consistent with the encoder). So the depths after 08:00 are not treated as uncertain. The one reading still unexplained is the -186 cm at the 12:00 roll up, which the zero check by hand would settle.

**What was changed (built, not flashed).** Every roll up to the origin now runs two passes (`SPOOL_HOME_PASSES` = 2; D95, `src/Hardware/Motor.cpp` `go_home`). When the spool is already at the switch, the second pass is short (about 0.3 s, the hold time). When the first pass ended short, the second one finishes it, and each pass keeps its own stall, direction and time limits.

**What this does not fix.** A false zero is still accepted: when the switch is held, the second pass takes it at once. The real fix needs a decision (open, F251): a hold stop more than 30 cm from the zero is not an origin, so the roll up fails and the schedule stops. The mechanical cause comes first (the drum stops at the holder: the belt tension and the level-wind guide, F250; the up-switch holder, F246; HW10 in stage 2 §7.4).

**Before the next spool run:**
1. Check the tube and the drum by hand: the tube length from the top switch to a mark, and the level-wind guide running freely.
2. Compare with `spool_status`. If the encoder reads 0 while the tube is out, do not trust the zero.
3. Note the samples from 08:00 onwards with the depth in doubt on the sheet.

**Ethanol cleaning and cartridges (for the roadmaps).** The ethanol purge needs a full change of tubing, which costs a session. Proposed in stage 2 §7b: three cartridges, one with hollow tubes always for the cleaning and two with filters, so that the next cartridge can be cleaned while the first one runs. Reservoirs of bleach, ethanol and waste that plug on and off are in the stage 3 notes.

## 28. The milliQ blank keeps reaching the cap margin (2026-11-06)

**Correction 2 (user, 2026-11-06).** E2-Blank is **valid**: about 800 mL of milliQ passed. The tip left the water only when the flow stopped changing and the pressure was low, which is the end of the run. So the `clogged` end reason reflects the end of the water, not a filter clog, and the first correction ("not valid") is withdrawn. The sections below are kept as first recorded.

**What was seen.** The blank run E2-Blank (folder `01_E2-DIEL3-session6-2026-11-06`) started at 14:27 with the tip of the spool in milliQ, no spool movement, and a 3 L purge. While filling the filter, the pressure repeatedly reached the cap margin (3.36 to 3.40 bar) and the pump stopped: 57 pauses in the first 15 minutes, one every 6 seconds. About 440 mL had passed by 14:42, which is about 30 mL/min, slower than a lake sample. This run is still going; the final numbers go in the log when it ends.

**Why it is surprising.** MilliQ is pure. It should not clog a filter, so the resistance is probably not in the water or the filter itself. The earlier blank E1-B1 (4 November, 17:57) also had repeated pauses: 29 in about 23 minutes, and it still reached 1001 mL. (Whether that blank was also milliQ is not written in the log; the operator's note says so.)

**Effect.** Every pause drops the reading by about 2.5 bar in 0.2 s, the same pattern as F249. The pauses cut the flow each time, so the blank is slower, and the pressure record is harder to read. The flow is also low, so the blank is not a clean control until the cause is found.

**Not yet known.** Still to check, now that the intake is the first suspect: when the tip left the water, and whether the pauses started then. The pressure reading after a pump stop (F249) is still a separate question (bench check with the pump off).

**Next:** finish the record and note the final pauses and volume; check the filter, the tube and the tip by hand before the next blank; compare the pauses of the next blank with a lake sample.

**Roadmap:** stage 2 §7b (the open item), F252.

**Later (user, 2026-11-06):** the same pauses were seen on the last blank too. Tuning the pump and the PID could cut the pauses, give a steadier flow reading for the flowmeter, and fill the filter faster. This is a future item (stage 2 §7b). A first clue from the logs: the pauses start at 3.35 bar, while the operation target is 3.0 bar, so the pressure overshoots its target by about 0.35 bar before the margin stops the pump. Each stop then drops the pressure by about 2.5 bar (F249).

**Lead for the tuning (code, 2026-11-06):** the Settings comment says the pump resumes when the *filtered* pressure is 0.2 bar below the pause level. The code compares the *raw* reading for both the pause and the resume. So one low raw reading restarts the pump at once, which is what the 0.2 s pauses show. Code and comment disagree; this is to settle in the tuning work (F252). Not changed.

## 29. Summary of the chat, 2026-11-06 (for the next chat)

Where each item of the day is written up. Read these sections, not the chat.

| Item | Where |
|---|---|
| E2 schedule 2 (DIEL2, samples 2026-11-05 23:59 to 2026-11-06 12:00) and its runs | this log, §26 (handover) and the session folder `02_E2-DIEL2-session6-2026-11-05` |
| The false zero at 08:00 and 12:58: the drum stopped at the top-switch holder, and the hold rule set the encoder to 0 with the tube out | §27, F251 |
| Two-pass roll up to the origin (D95) | §27, decisions D95 |
| The spool mechanics: level-wind guide, frame screw, belt, the cracked printed guide | §24, F250, roadmap stage 2 §7.4 HW10 |
| Blank procedure: no `depth=`, the milliQ volume (4 to 5 L), the slower prime, schedule stop and restart, the slot order | operator tutorial §2d |
| The milliQ blank with pauses: first read as a clog, then corrected (the intake was out of the water) | §28, F252 |
| Pump and PID tuning (future), the raw versus filtered pressure lead | stage 2 §7b; §28; F252 |
| Sample notes question: at the end, but not in a schedule | stage 2 §7b |
| The three cartridges (one hollow for the cleaning, two for filters) | stage 2 §7b |
| Reservoirs of bleach, ethanol and waste | stage 3, notes for the next machine version |
| Ethanol cleaning doubt for the Tuesday night (resolved: done, proven by the team's notes) | §25 |
| Field report draft | `docs/reports/field-report-2026-11.md` |

**Open at the end of the chat:**
- The E2-Blank is valid (about 800 mL of milliQ passed; the tip left the water at the end of the run, §28).
- The spool false zero (F251): mitigated by the two-pass roll up (user). The stricter rule and the drum stopping at the holder are roadmap 2 questions.
- The spool mechanics (HW10) are not done. Check the tube by hand before each spool run.
- The depths after 08:00 on 6 November: the zero seemed to work (user); no change to the records. The -186 cm reading at 12:00 is still unexplained (§27).
- The firmware on the board: the prime retry is on it; the 100 % prime, the pause and pump-start messages, and the two-pass roll up are not. The flash waits for the end of the current schedule (user).
- The pump and PID tuning, and the raw versus filtered pressure lead (F252).
- The schedule 3 (`tools/DIEL-E2-schedule3.csv`): three samples at 16:00 (E2_T7 20 m, 5 m, 1 m). The blank goes first, in slot 1, with the tip in the milliQ and no spool movement (tutorial §2d).
- Nothing committed. Changes since `591c0ea` are listed in §26.

## 30. Remaining work (end of the chat, 2026-11-06)

Everything still open, by owner. Each line points to the place where it is written up. "Roadmap 2" is `docs/roadmap/stage2-pilots.md`; "stage 3" is `docs/roadmap/stage3-full-system.md`.

**Firmware (flash at the end of the current schedule, user)**
- Flash the current build: the 100 % prime (D94), the two-pass roll up (D95), the pause and resume messages and the pump start wording (F249). The prime retry (D93) is already on the board.
- After the flash, check in the log: `Spool prime (try n of 3)` and the prime at 100 %; `PAUSE n in this run` and `RESUME after T ms`; the two passes in `spool_home` (duration of the second pass, about 0.3 s when already at the switch).
- Print the applied pump power on the purge and prime lines (F221). Not built.

**Spool (before each spool run, until the permanent version)**
- Check the tube and the drum by hand, the level-wind guide running freely, and `spool_status`. The temporary repair works for now (user).
- Check the -186 cm reading seen at 12:00 on 6 November against the tube by hand (§27).
- F245 (a `spool_goto` that moved 0 cm on 4 November): repeat from a known position with the move log. Still open.

**Spool, roadmap 2**
- HW10 permanent version: water-resistant mounting, drive belt tension, level-wind guide in a tougher material with metal fixings, test over 40 m (stage 2 §7.4, §7b).
- The drum stopping at the top-switch holder (F251), and the stricter origin rule (a hold stop far from the zero is not an origin). Not built.
- The up-switch holder: no wood in the water path (F246).

**Pressure and pumping, roadmap 2**
- Pump and PID tuning: the overshoot from the 3.0 bar target to the 3.35 bar margin, the pauses, a steadier flow reading (F150), a faster fill (stage 2 §7b).
- The raw versus filtered pressure discrepancy: `Settings.h` says the resume uses the filtered reading, the code uses the raw reading (F252).
- Bench check of the pressure reading after a pump stop, with the pump off (F249). Before any pump-based sample is judged on its pauses.
- Pressure sensor: glue it to the tubing (F234), then check against the manometer. The fix is simple, and it is the most critical item for an unattended sampler.

**Manifold**
- Alignment (F216, F217): the frame must hold the alignment; this is the main limitation and needs the frame redesign.
- HW9, the manifold encoder coupling lock (early fix before the next field run): the log does not record it as done. Check and record.

**Procedure and tools**
- Prefilter: put the check in the start-of-day routine and on the paper sheet; decide whether the software asks at `session` (F248).
- Ethanol cleaning: before each new filter set, and at the end of the day. The Tuesday-night purge (3 November) is not in the session log but is proven done by the team's notes (§25).
- Blank: the tutorial §2d is up to date (no `depth=`, 4 to 5 L of milliQ, slower prime, stop and restart of a schedule, the slot order, the tip in the water until the end). Keep the time the tip leaves the water on the paper sheet.
- Sample notes question: ask at the end of the sampling; for a schedule the design is open (roadmap 2, F222).
- Schedule times and depths cannot be changed in the tool (F239). Future command (roadmap 2).
- `tools/DIEL-E2-schedule2.csv` row 3 is named `E1_T3_01M_R1` while the others are `E2_*`. It was run under that name; check the name when the record is filed.
- Schedule 3 (`tools/DIEL-E2-schedule3.csv`): three samples at 16:00. The blank goes first in slot 1, then 2, 3 and 14.

**Cartridges and reservoirs (roadmap 2 and stage 3)**
- Three cartridges: one with hollow tubes always for the ethanol cleaning, two for the filters, so that the next cartridge is cleaned while one runs (stage 2 §7b).
- Pluggable bleach, ethanol and waste reservoirs (stage 3, notes for the next machine version).

**Records and documents**
- The field report draft (`docs/reports/field-report-2026-11.md`) is to be updated after the 24 h repeat.
- Nothing is committed. The changes since `591c0ea` are listed in §26; the user will commit later.
- The ethanol cleaning at the end of 5 November (22:17 to 22:41, 14 slots) is in §26.

## 31. An expired filter in the E1_T7_20M_R1 sample (2026-11-06)

- **Sample:** E1_T7_20M_R1 (DIEL study, E1, 4 Nov, slot 11, 16:15 to 16:28 filtration window, PASS, 1001 mL, 846 mL trusted, end `volume`).
- **Filter:** the last filter available. It is from a different batch from the other filters of the campaign, and it expired in March 2025. It is the only expired filter used in the campaign.
- **Correction (user, 2026-11-06):** an earlier note of this section named E2_T7_20M_R1. That was wrong. E2_T7_20M_R1 (6 Nov, slot 2) is not recorded as expired; its filter has to be checked on the paper sheet.
- **Why:** there was no other filter. The choice was made by the team on the day.
- **Why it matters:** if the sample behaves differently (the flow, the pressure, the end reason, the volume), or shows a different result in the bioinformatics, the expiry and the batch are the first things to check. The sample itself ran normally.
- **To record on the paper sheet:** the filter ID and batch, the expiry date, and the reason it was used.
- **For the analysis:** flag E1_T7_20M_R1 as using an expired, different-batch filter (F253).
- **For the next runs:** check the expiry date of every filter when it is loaded (added to the LéXPLORE protocol).

## 32. Plan versus what happened (2026-11-06)

The field protocol and the project description were written before the field work and are context only (kept in [archive/field-plans/](../archive/field-plans/): [protocol](../archive/field-plans/lexplore-field-protocol-2026-11.md), [project description](../archive/field-plans/lexplore-first-step-project-description.md)). Several things changed on site; the logs are the record. (Manual Niskin sampling is documented separately.)

| Item | Plan | On site |
|---|---|---|
| Expeditions | 2–3 Nov and 5–6 Nov, 10:00 to 10:00 | E1 from 3 Nov 16:10 (access and set-up first) to 4 Nov evening; E2 from 5 Nov 16:00 to 6 Nov 16:00 |
| Depths | 2, 20, 30 m, three intakes | One intake on the spool: 20, 5 and 1 m, tube total limited to 40 m |
| Time points | 7 per 24 h, every 4 h | E2: T1 to T7 every 4 h, three samples per time point (about 85 min per group). E1: irregular (T1 16:10, T2 20:00, T3 23:50, T4 04:00, T5 08:00, T6 13:05 with R1 and R2, T7 16:35) because of the faults of §6, §15–§17 |
| Time per sample | about 20 min | about 28 to 30 min (dive, prime, 3 L purge, sample, shield, roll up, emptying) |
| Manifold | one change after about 14 samples | 14 slots; filters reloaded between series, used-slot lists given at each tool start |
| Sample IDs | `E1_20261102_1000_02M_AUTO_R1` | `E<expedition>_T<time point>_<depth>M_R<replicate>`, for example `E2_T7_20M_R1` |
| Blanks | 2 automatic, 2 manual | Automatic blanks by hand with the intake tip in milliQ and no spool move: `E1-B1-00M-R1` (4 Nov) and `E2-Blank` (6 Nov); see tutorial §2.4 |
| Prefilter | in the agreed configuration | 20 µm for the bacterial work, first in the line (F248), changed at every ethanol purge (user, 2026-11-06). Protocol in [archive](../archive/field-plans/lexplore-field-protocol-2026-11.md) |

## 33. Automatic versus manual sampling, E1 and E2 (2026-11-06)

**What was compared.** The manual samples (Niskin bottle at the platform, filtered by hand, protocol in [manual-sampling-protocol.md](../field/manual-sampling-protocol.md)) were taken to compare with the autosampler. Their times are in the sheet [manual-sampling-lexplore-2026-11.csv](manual-sampling-lexplore-2026-11.csv) (user, 2026-11-06). The labels: E1 and E2 are two 24 h runs on the LéXPLORE station. T1 to T7 are the time points: T1 16:00, T2 20:00, T3 00:00 (midnight), T4 04:00, T5 08:00, T6 12:00 (noon), T7 16:00. Manual samples were taken only at T6 and T7 (E1) and T1 and T2 (E2): there was no manual sample at 08:00 (user, 2026-11-06). 20M, 05M and 01M are the depths. R1 and R2 are replicates: both methods were run in duplicate at all three depths, at one time point per run (E1 T6 and E2 T1). The first day ran late on the schedule and was caught up later; the first manual samples were not exactly at the same time as the autosampler, and they were synchronised afterwards (user).

**Pairing.** Each manual sample is paired with the automatic sample of the same name and depth (its last PASS run). The automatic window is the filtration window: `Step sample through filter started` to `Step sample ended` in `session.log`. Offset = manual time minus the start of the automatic window, in minutes.

| Sample | Manual | Auto window | Offset (min) |
|---|---|---|---|
| E1-T6-20M-R1 | 13:20 | 13:16–13:28 | +3.5 |
| E1-T6-20M-R2 | 13:48 | 13:47–13:58 | +0.8 |
| E1-T6-05M-R1 | 14:15 | 14:18–14:27 | −3.6 |
| E1-T6-05M-R2 | 15:00 | 14:46–14:58 | +13.2 |
| E1-T6-01M-R1 | 15:24 | 15:18–15:29 | +5.9 |
| E1-T6-01M-R2 | 15:47 | 15:48–15:58 | −1.6 |
| E1-T7-20M-R1 | 16:13 | 16:15–16:28 | −2.7 |
| E1-T7-05M-R1 | 16:47 | 16:47–16:59 | −0.6 |
| E1-T7-01M-R1 | 17:15 | 17:19–17:30 | −4.4 |
| E2-T1-20M-R1 | 17:21 | 17:20–17:31 | +0.7 |
| E2-T1-20M-R2 | 18:00 | 17:50–18:06 | +9.7 |
| E2-T1-05M-R1 | 18:27 | 18:25–18:33 | +1.0 |
| E2-T1-05M-R2 | 18:52 | 18:52–19:08 | −0.5 |
| E2-T1-01M-R1 | 19:28 | 19:32–19:45 | −4.3 |
| E2-T1-01M-R2 | 20:17 | 20:14–20:25 | +2.4 |
| E2-T2-20M-R1 | 20:50 | 20:42–20:56 | +7.0 |
| E2-T2-05M-R1 | 21:30 | 21:18–21:30 | +11.3 |
| E2-T2-01M-R1 | 22:08 | 21:53–22:05 | +14.7 |

- All 18 manual samples have an automatic PASS run to pair with. Three E1 T5 entries that were on the first draft of the sheet were never taken and are discarded (user, 2026-11-06). The manual blank `E2_manual_blank` (5 Nov, 15:45) is kept with the manual samples, labelled as a blank (user).
- The 18 pairs from E1 T6 onwards are within 15 min of the automatic window (mean absolute offset 4.9 min).
- The three E1 T5 pairs were 43 to 75 min later, the first manual samples of the campaign (user: not synchronised at first).
- The manual blank `E2_manual_blank` (5 Nov, 15:45) has no automatic blank on that day. The CoWaS blanks were `E1-B1-00M-R1` (4 Nov) and `E2-Blank` (6 Nov).
- The sheet has times only: no volumes and no DNA results. The comparison of the samples themselves is for the lab.

**Correction (2026-11-11):** the sample lost to an operator entry error is **E1_T5_01M_R1** (4 Nov, F244): the start list typed `5-6,8-11` instead of `4-5,8-11`, so the run went to slot 6, which holds no filter, and the water went to the drain. Its record (slot 6, 1001 mL) is not a valid sample. The paragraph below guessed the prefilter case of 5 Nov (F248) and is wrong about which sample it is; the prefilter case lost no sample. With this, the CoWaS has 46 valid samples and 2 blanks at LéXPLORE (the earlier count of 47 included the empty-slot run), and 67 samples with the 19 manual ones.

**Automatic sample with a human error (user).** The user reported one automatic sample lost to a loading error. In the logs, the prefilter error of 5 November (F248) caused five failed runs of `E2_T1_20M_R1` (16:06 to 16:59). The sample itself passed on its sixth run: 17:20 to 17:31, 1001 mL, 865 mL trusted, paired with the manual sample at 17:21. The only automatic name with no PASS run is `E1_T1_05M_R1` (3 Nov): its sample was clogged and valid, but the spool roll-up timed out after it (F224). It has no manual pair.

## 34. E1_T1_20M_R1: real volume against the flowmeter (2026-11-06)

- **Record:** `163055269_E1_T1_20M_R1.json`, 3 Nov 16:10 to 16:36, end `volume`. The flowmeter counted **1001 mL** (944 mL trusted); the sample stopped at the 1000 mL target.
- **Bucket check (user, 2026-11-06):** the 1250 mL is the real volume, measured in a bucket to compare with the flowmeter. The flowmeter counted 1001 mL, so it **underestimated** the real flow by about 20 % at this flow (about 60 to 80 mL/min). This matches the known low-flow under-reading (F150).
- **Why it matters:** the sample stop uses the flowmeter. At this flow the sample was about 1.25 L, not the 1 L target. The trusted-volume figure (944 mL) is low for the same reason. Not very important for the analysis, but the CoWaS volumes at low flow are underestimated.
- **Next:** more bucket checks at other flows, to set the flowmeter correction (F254).

## 35. DIEL study status at 16:47, 6 November (E2 schedule 3)

- **Schedule 3** (tools/DIEL-E2-schedule3.csv, T7 at 16:00): E2_T7_20M_R1 PASS (slot 2, 16:00 to 16:30, 806 mL, 578 mL trusted, `clogged`); E2_T7_05M_R1 running (slot 3, from 16:30); E2_T7_01M_R1 waiting (slot 14).
- **Slots predicted before the start:** 20 m slot 2, 5 m slot 3, 1 m slot 14. The first two were as predicted.
- **Not counted** in the field report until the schedule ends.
- **Records to do:** the E2 T7 filters on the paper sheet (ID, batch, expiry).

## 36. Problems of the DIEL study, counted as problems (2026-11-06)

A problem is one failure, or a run of failures from one cause with no success in between, until it is resolved (D97). The runs behind them are in the session logs. Automatic samples only; the air/water runs and the test are not part of this count.

| # | When | Cause | Failed runs | Samples | How it ended |
|---|---|---|---|---|---|
| 1 | 3 Nov 16:45–17:35 | Spool lost its position (F214); the schedule went on (F225); roll-up timeout (F224) | 3 | E1_T1_05M, E1_T1_01M | Both samples by hand (17:13, 18:05) |
| 2 | 3 Nov 20:15 | Prime found no flow, cause not recorded; the operator stopped it | 1 | E1_T2_20M | Operator restart, 20:24, PASS |
| 3 | 3 Nov 21:22–22:10 | Spool lost its position again (false top press; press not confirmed after the reflash) | 2 | E1_T2_01M | PASS at 22:49, after the reflash (operator restart) |
| 4 | 4 Nov 04:19–04:34 | Pressure sensor out of its tubing; its short disturbed the encoder (F234, F235) | 2 | E1_T4_20M, E1_T4_05M | Sensor dried and refitted; both rerun by the operator, PASS |
| 5 | 4 Nov 08:52 | Spool zero taken from a top-switch press not at the top (F241) | 1 | E1_T5_01M | Retried by the schedule 5 min later, PASS 09:14 |
| 6 | 5 Nov 16:06–16:59 | Prefilter replaced by a spacer (F248): primes found no flow; homing stall at 16:27 (F250); two runs stopped with `x` | 5 | E2_T1_20M_R1 | Prefilter replaced; PASS on the sixth run, 17:20 (1001 mL) |
| 7 | 5 Nov 19:20 | Spool zero taken from a top-switch press not at the top (F241) | 1 | E2_T1_01M_R1 | Retried by the schedule 2 min later, PASS 19:52 |
| 8 | 5 Nov 19:53 | Spool drum stalled (F250) | 1 | E2_T1_01M_R2 | List stopped; repaired; PASS at the operator's restart, 20:32 |

- Before the spool fixes (first run to 3 Nov 22:49): problems 1 to 3, 6 samples. Two samples (E1_T1_05M, E1_T1_01M) were finished by hand.
- After the spool fixes: problems 4 to 8, 39 samples. Problems 5 and 7 were retried by the schedule; 4, 6 and 8 stopped the list and the operator restarted it. No failed run after the fixes used a filter.
- In all: 16 failed runs, 8 problems, all resolved.

## 37. E2_T7_05M_R1: second bucket check (2026-11-06)

- **Sample:** E2_T7_05M_R1 (DIEL study, E2, 6 Nov, slot 3, from 16:30).
- **Bucket check (user, 2026-11-06):** the flowmeter counted **549 mL**; the real volume through the filter was **800 mL**. The flowmeter read about 31 % low (549 / 800 = 0.69). The sample stopped on the flowmeter, so about 800 mL went through, not the target.
- **With §34:** two bucket checks, two under-readings: E1_T1_20M_R1 1001 counted for about 1250 real (80 %), E2_T7_05M_R1 549 counted for 800 real (69 %). The under-reading is not a constant factor; it is larger at the lower flow of a clogging filter. Consistent with F150 and F254.
- **Not in this entry:** the end reason, the trusted volume and the flow rate of this run (read them from the record when the schedule ends).


## 38. Manifold over-travel, lost link and a possible burnt smell after E2_T7_05M_R1 (2026-11-06)

**Status: cause unknown. Everything in the hypothesis list is a hypothesis, not a finding.** **Read §39 (2026-11-10) next: the final log lines, the flash result of 10 Nov (COM6 is seen, the SAM3X does not answer: this corrects the "no COM6" below) and the weighing.** Recorded from the user's account in the field, so times, console output and the state of the board are not yet known. Register: F257.

### What was observed (user, 2026-11-06, in the field)

- **Where in the run:** schedule 3, E2_T7_05M_R1 (slot 3). The sample itself was finished and the CoWaS was about to start the DNA Shield step.
- **The manifold kept turning** until it was tangled in the tubing and the cables. This is the first time: all week it turned in both directions and never tangled. The user believed the safety was in place (the 15 s rotation timeout and the lost-encoder rule; see the note on the guard below).
- **The computer lost the connection to the CoWaS** and cannot reconnect. Windows does not detect anything on COM6 (the Programming port, 9600).
- **A burnt smell** may have been present. The user is **not sure**. It is not confirmed that it came from the CoWaS.
- **The LéXPLORE platform started its generator** at the same time, or a few seconds before or after. The platform battery may have dropped and cut the power outlets for a moment. The user wonders if this made the CoWaS lose power for a second.
- The user could not bring the link back and **stopped the field work**.

### State of the samples (user)

| Sample | State |
|---|---|
| E2_T7_20M_R1 (slot 2) | Collected by the CoWaS, secured |
| E2_T7_05M_R1 (slot 3) | Collected by the CoWaS; **the DNA Shield was added by hand**; secured |
| E2_T7_01M_R1 (slot 14) | **Never taken.** Lost. The E2 series ends with no 1 m sample at T7 |
| Ethanol purge of the three slots used in schedule 3 | **Not done.** To do later. The three are probably the blank (slot 1), the 20 m (slot 2) and the 5 m (slot 3); check against the sample files |

### Hypotheses (none confirmed)

**H1. A power dip while the manifold was turning (the user's idea).** The generator start made the platform battery or the transfer switch drop the outlets for a moment.
- The 24 V supply, the Due or the motor driver lost power briefly and came back.
- If only the motor supply or the encoder dipped while the Due kept running, the encoder could read a wrong angle after the dip (a jump, a zero, or the same value for ever). The rotation loop would then drive the motor towards an angle it never reaches.
- A software loop would still stop at the 15 s timeout, so this needs the Due to start rotations again and again (see H3), or the guards not to run (H2).

**H2. The processor stopped (hang or brown-out) while the motor driver output stayed on.**
- The 15 s timeout, the 50 bad-read rule and the stall guard all run **inside the Due**. If the Due is frozen, none of them can stop the motor.
- A frozen Due leaves INA, INB and PWM at their last level, so the VNH5019 keeps driving until something else stops it. This fits "kept turning until tangled" better than a software loop, which would have stopped at 15 s.
- It would also fit the lost USB link.
- To check: whether the VNH5019 inputs have a pull-down on the shield, and what the pins do during a reset. A reset alone should drop them to inputs (no drive); a freeze would not.

**H3. The Due reset or restarted and something moved the manifold again.**
- After a brown-out reset the firmware starts in the test menu (F23). The lock state is UNKNOWN (rotation refused), so a clean reset should not turn the manifold.
- Check if the host, a retry or the legacy Pi link could have sent a command that rotated it (a reconnecting COM port, a 1200 baud touch, a line repeated by the host). Never 1200 baud (CLAUDE.md).
- Check which firmware was on the board, with or without the motion guard (see the note below).

**H4. A fault of the manifold motor circuit itself, independent of the generator.**
- The motor pushed against the tangle, stalled, and drew a high current. The tangle may have been the cause of the burn, not the result of a power event.
- The VNH5019 has a thermal shutdown and a current limit, but the motor, the connector, the wires or the shield could still have been damaged. Inspect the 24 V wiring, the connector and the motor winding for heat marks.
- A stalled motor alone would not explain the lost USB link, unless a short or a supply sag took the Due down with it.

**H5. A short or surge damaged the board, which would explain the missing COM6.**
- COM6 is the Programming port: the ATmega16U2 USB bridge. It disappears if the bridge or the Due is dead, the cable or the laptop port is damaged, or the board has no 5 V.
- A surge, a short on the 24 V side, or a ground difference between the laptop (on platform power or on its battery) and the CoWaS supply could damage the USB bridge, the regulator or the Due. The same path could explain the smell.
- To separate the cases: board power LED, another USB cable and another computer, the Native USB port, voltage at the 5 V and 3.3 V rails, and the current drawn by the board alone with the 24 V disconnected.

**H6. The generator start caused electrical noise, not a power cut.** A switching transient on the supply or the ground could disturb the SPI encoder (AMT22, CS D38) or the Due with no loss of supply. A noisy encoder gives bad frames (the 50-failure rule stops the motor) or plausible wrong angles (it does not). This is close to F234 and F235, where a short at the pressure sensor disturbed the encoder.

**H7. The laptop side failed, not the CoWaS.** The laptop lost power or its USB port reset when the platform power changed, so COM6 vanished on the laptop only. This does not explain the manifold turning, so at most it is a second event. Check at once with another computer.

**H8. A software state error after the sample (nothing of this was seen).** The manifold turned after the sampling step and before the shield. A wrong target angle or a wrong direction would turn it further than a slot change. The over-travel rule of the guard (400 degrees) is meant for this. The sample file and the step log of E2_T7_05M_R1 (D96) should show the last step done and the last angle.

**H9. The O-ring state at the moment of the failure is unknown.** `rotateMotor` unlocks the O-ring and relocks it on return. If the Due stopped or reset during a rotation, the O-ring may have been left **unlocked**, with the lock state lost in RAM. If the manifold was turned while tangled, the seal and the slot positions cannot be trusted. The samples already in the filters are not affected if their filters were closed before (the user says both are secured).

**H10. Rain (user, 2026-11-06).** There were some drops of rain at the time. The CoWaS is sometimes a bit resistant to rain, so this is only noted. Water on the motor driver shield, the connectors, the 24 V terminals or the USB port could short or leak a current. It could explain a burnt smell, a frozen or dead Due and the lost COM6, and it would not need the generator at all. Check the enclosure, the cable entries and the connectors for water marks, corrosion or drops, and note where the rain fell on the machine.

### Update 2026-11-09: leading hypothesis, the driver question, and the plan

**The user's assessment.** The most likely source is the power shutdown of the platform (generator start). If so, the cause is outside the CoWaS. **A machine used on a platform must still survive a power dropout at any moment**, so the CoWaS has a gap either way: it should stop safely and restart in a checked state. This is written up as PW1 in [stage 2 §7.0](../roadmap/stage2-pilots.md) and as F258. The hypotheses H1 to H10 stay open until the bench work.

**The fact, kept apart from the hypotheses.** The manifold turned further than a slot change needs and entangled itself in the tubing and the cables. This had not happened in the week before. Nothing else about the cause is known.

**Question (user): could a part of the manifold motor driver overheat or burn in a way that stops the COM6 communication of the Due?** Analysis only, nothing measured.
- **Heat alone: unlikely.** The VNH5019 is on the motor path, not on the USB path, and it has a thermal shutdown and a current limit. A hot driver shuts itself off; it does not by itself take the USB link down.
- **Electrical paths that are possible:**
  1. *The driver or its connector fails as a short, or a burnt wire touches the shield.* Motor voltage (24 V) can then reach INA, INB, PWM, EN/DIAG or CS. The Due is 3.3 V only and no pin is 5 V tolerant (CLAUDE.md), so the SAM3X could be damaged, and the shield also shares the 5 V and ground with the Due.
  2. *A stalled motor draws a large current* (the manifold pushing on the tangle) and the supply sags or trips. If the Due or its 5 V sits on the same source, the board resets or goes dark. This is the same family as H1/H2, with the tangle as the trigger.
  3. *A short on the shield's 5 V* pulls down the 5 V of the Due, including the USB bridge.
  4. *A ground difference* between the laptop and the CoWaS supply (generator, platform) puts a surge on the USB shield and ground.
- **What the missing COM6 tells us (to verify on the bench).** On a standard Due the Programming port is an ATmega16U2 bridge that appears in Windows on its own, even when the SAM3X is crashed or dead. If so, "no COM6 at all" is less likely to be only a dead or frozen SAM3X. It points to the USB cable or port, the laptop, no 5 V at the bridge (the rail collapsed or a protection tripped), or a dead bridge. A dead SAM3X with a living bridge would still show a port.
- **Cheap tests, in this order:** (1) another USB cable and another computer; (2) the Native USB port; (3) the board alone on USB with the motor shield removed and the 24 V off: if COM6 comes back, the fault is on the shield or motor side; (4) the 5 V and 3.3 V rails with a meter, then the shield off and on; (5) look at the shield, the VNH5019 and the motor connector for burn marks (photos first); (6) the driver inputs to ground and 24 V with the power off, to look for a short.

### Note on the motion guard in the working tree

- `src/Core/manifold_guard.*`, `Manifold.cpp` and `Settings.h` (`MANIFOLD_STALL_MS`, `MANIFOLD_STALL_MIN_DEG`, `MANIFOLD_MAX_TRAVEL_DEG`) contain a stall and over-travel guard that refers to F256. It is **not committed**, and nothing in the logs says it was flashed. If the board ran the earlier build, the only limits were the 15 s timeout and the 50 bad reads.
- Whichever build was flashed, a guard in the Due cannot help if the Due itself is frozen or off (H2). The hardware needs a stop that does not depend on the Due.

### What to collect before any new power-up or repair

1. The serial log or the session log of the host up to the last line (the last step, the angle, any `ERROR | rotateMotor` line).
2. The sample file and the step log of E2_T7_05M_R1 and E2_T7_20M_R1.
3. The time of the generator start (platform log) against the time of the last log line.
4. Which firmware was on the board (build time, with or without the guard).
5. Photos of the tangle, the cables and the connectors before anything is untangled; heat marks on the motor, the shield and the 24 V connector.
6. The behaviour of the board alone: LED, USB enumeration on another computer, current drawn with the motor and the 24 V disconnected.
7. The Windows device log of the laptop for COM6 (time of the disconnect).

### Debug plan for the bench (not done)

- Do not run the pump or the manifold motor until the cause is known. Treat the board as damaged until checked.
- Reproduce a supply dip on the bench with the manifold free and the motor running: cut the 24 V alone, then the 5 V alone, then both for 1 s. Record what the motor, the encoder and the Due do. Also test an angle that never changes and an angle that jumps.
- Put a stop outside the Due: for example the motor supply behind a relay or contactor that the Due must keep energised (so a freeze or a reset drops it), a fuse or current limit on the motor line, and a slip clutch or a turn limit on the manifold.
- Add a supply-voltage or power-good line so that a dip is logged.

### Left to do from this event

- E2_T7_01M_R1 is **lost**. Note it in the field report (E2 T7 has two of three depths).
- Do the **ethanol purge of the three slots** used in schedule 3 (and check that the slots of the earlier runs are all done, §25).
- Debug the CoWaS as above before the next field run.
- Field report and paper sheet: the DNA Shield of E2_T7_05M_R1 was added by hand (record the time and the volume).


## 39. The incident reviewed against the final log, and the hypotheses weighed (2026-11-10)

Status: **cause still unknown.** This section adds the evidence from the last lines of the log and the result of a flash attempt on 10 Nov, and weighs the hypotheses of §38. The weights are my judgement from the evidence (Claude, for the user), not measurements.

### 39.1 Facts from the final log (`2026-11-06/01_E2-DIEL3-session6-2026-11-06/session.log`)

| Time | Line | Meaning |
|---|---|---|
| 16:54:08 | `Step sample ended: clogged, pauses 86` | E2_T7_05M_R1 finished, record received 16:54:09 (549 mL, 351 trusted) |
| 16:54:34 | `Vent: ... released` | normal vent |
| 16:54:41.653 | `O-ring unlocked` | the unlock before a manifold rotation |
| 16:55:45.511 | `DISCONNECTED: ClearCommError failed (PermissionError 13, The device does not recognize the command)` | the USB device vanished from Windows |

- **Normal pattern:** `O-ring unlocked` is followed by `O-ring locked` 7 to 9 s later (16:30:09 to 16:30:16, 16:35:46 to 16:35:54, 16:44:01 to 16:44:10). This time **no `O-ring locked` and no error line came.**
- **Silence of 64 s.** If the Due had been running its rotation loop, the 15 s timeout (or the stall guard, if it was flashed) would have printed an `ERROR | rotateMotor` line. A Due that had reset would have printed its boot text on a live USB link. Neither appears. **So the Due most likely stopped executing code within the first seconds of the rotation, and its USB bridge stayed alive for about a minute.**
- The rotation was probably driven for the whole 64 s. At the normal speed that is plausibly enough to wind the manifold more than a turn into the tubing (not measured).
- **Update 10 Nov (flash attempt).** `pio run -t upload`: `Auto-detected: COM6`, `Forcing reset using 1200bps open/close on port COM6`, `No device found on COM6`. **So COM6 exists in Windows now** (my §38 text and the first report said "no COM6": that was the situation in the field; today the port is seen). The 1200 bps touch makes the USB bridge (ATmega16U2) pull ERASE and RESET on the SAM3X, and the SAM-BA monitor of the SAM3X then has to answer. It does not. **The USB bridge is alive; the SAM3X or its 3.3 V supply is not answering.**

### 39.2 Weighing of the hypotheses

The user's current hypothesis: low voltage of the platform batteries and a power shutdown while the generator started; the CoWaS had the order to turn the manifold and never received the order to stop when power came back; the manifold tangled; the driver was overpowered and something burnt; and the CoWaS can no longer be connected to.

| # | Hypothesis | Weight | Why |
|---|---|---|---|
| A | A **supply disturbance** from the platform (low battery, outage, generator start) as the **trigger** | **High** | The only external event, at the right moment (the user). Cannot be proven from the logs; the platform's own log would give the time |
| B | The **Due froze or lost its 3.3 V (brown-out, latch-up, surge)** during the rotation, and the motor driver kept the last command | **High** | Best fit to the 64 s of silence, no timeout line, no boot text, the over-travel, and a SAM3X that does not answer now. Output pins stay at their last level when the core freezes |
| C | The Due **reset and restarted, and the motor was still driven or was driven again** ("did not receive the stop order") | **Low to medium** | A reset would print the boot text on a live link (absent) and leaves the lock state UNKNOWN, so the firmware refuses to rotate. Only a driver-side latch (inputs floating high) could keep it turning; to check on the shield |
| D | **The tangle stalled the motor and burnt something** (motor winding, driver, wiring: the smell) | **Medium** for a burnt motor or wire, **low** for killing the Due | A stall draws a high current for a minute. The VNH5019 has thermal shutdown and a current limit, so the chip itself should protect. The tangle is a consequence of B, not its cause |
| E | **An overvoltage or a short killed the SAM3X** (a supply spike from the generator start or the motor's switch-off, motor voltage on a driver logic pin, a ground difference through USB) | **Medium to high** | Fits the live bridge and the dead SAM3X. A power event alone can also explain B and E together: one cause, two damages |
| F | Rain on the electronics | **Low** | A few drops, the box is somewhat resistant, no evidence. Check for water marks |
| G | A firmware logic error (wrong direction, wrong target) | **Low** | The same code rotated about 70 times that week; a logic error would not stop the Due or kill the SAM3X |
| H | A laptop or USB cable fault | **Low** | The bridge answers on COM6 today. Another computer would confirm |
| I | Electrical noise on the encoder (generator) | **Low** | Bad frames stop the motor within 50 ms. It would not stop the Due |

**Answers to the user's questions**
- *Could the entanglement really burn the driver or something else?* It could burn the **motor or its wiring**, and could stress the driver (high stall current for about a minute; the VNH5019 should shut down thermally first). It is much less likely to explain the dead Due by itself.
- *Could one of these steps make the Arduino impossible to connect to?* Yes, but through the **supply or a short, not through heat**: a spike or a motor-voltage short on a Due pin or its regulator can kill the SAM3X while the USB bridge keeps showing COM6. This is what the 10 Nov upload result looks like.
- *How could an overpower in the driver affect the COM6 communication?* A failed driver or a burnt connector can feed the motor voltage into INA, INB, PWM, CS or EN/DIAG (the Due is 3.3 V only), or a stalled motor can pull the shared supply down. The USB bridge itself sits on the 5 V rail and survives more easily; the SAM3X is the weak part.
- *The "no stop order after power came back" idea:* the firmware cannot send the motor on after a reset (rotation refused, lock UNKNOWN). If the motor kept turning, it was because **nothing was running to stop it**, not because the stop order was missed. That is why PW1 puts a stop that does not need the Due.

### 39.3 Tests that separate the hypotheses (not done)

1. Remove the motor shield and all wiring. Power the Due from USB only and try the upload again (and the Native USB port). **If it flashes: the fault is on the shield or the wiring side. If not: the SAM3X or its regulator is damaged** (replace the Due; the board is cheap).
2. Press the ERASE button for a few seconds, reset, then upload. Measure the 3.3 V and 5 V rails and the current drawn by the board.
3. With the Due replaced and the shield not fitted, check the shield, the VNH5019, the motor connector and the motor for burn marks and shorts (inputs to ground and to 24 V, power off). Photograph first.
4. Get the platform power log: the time of the outage and of the generator start against 16:54:41 to 16:55:45.
5. Try another computer and cable (H).
6. Find which firmware was on the board (with or without the motion guard).

### 39.4 For the report

Only the main hypothesis (A with B) is kept in the [field report](../archive/reports/field-report-2026-11.md), marked as in progress. This section and §38 hold the rest.
