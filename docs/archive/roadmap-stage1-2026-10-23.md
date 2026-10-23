# CoWaS roadmap, stage 1: a working prototype

**Goal:** finish the prototype in the next few days. Water flows through a 1 L Sterivex, the shield is added, the lines are emptied, and the next filter is run. Three filters in a row, from the console, with nothing fixed by hand.

Rewritten 2026-10-19 ("it needs to work, not be perfect"). Restructured 2026-10-21. The previous version (S1-0 to S1-3) is in [archive/roadmap-stage1-2026-10-15.md](../archive/roadmap-stage1-2026-10-15.md).

Stage 2 ([stage2-pilots.md](../roadmap/stage2-pilots.md)) and stage 3 are unchanged except for the items deferred here (§8). "ROADMAP §N" means the stage 2 file.

**Sections:** §0 status · §1 goal · **§2 plan (start here)** · §3 step procedures · §4 rules and settled values · §5 fallbacks · §6 tools and session records · §7 status detail · §8 later

---

## 0. Where we are

**HOLD LIFTED (2026-10-23, F191, D40):** the 3-slot offset is solved after the reflash (`slot7` lands on slot 7). **P1 PASS** (max 3.53 bar, 68 pauses, clogged at 38 mL; LOG). Open from P1: F192 (O-ring unlocks at about 2.9 bar), F193 (the 0.8 bar sterivex threshold reads a wet filter as dry). F194 (clog floor, 50 mL per minute, PLACEHOLDER) and F192 (wait for the line to vent to 0.3 bar before the O-ring unlocks, 20 s timeout, PLACEHOLDER) are built, not flashed: reflash first. F192 is untested on the bench, and a timeout is the first thing to check on a clogged filter. Next: S1 (shield grams). Old hold text kept for the record: **HOLD (2026-10-23, F191):** `slot7` still lands on the physical slot 10 after the reflash with `purge_angle` 276.77 (D39). D39 is **not** a fix (0.35° change against a 67.5° offset). Candidate: `purge_angle` = **344.27** (276.77 + 67.5), to decide with the user and check by eye. Until `slot7` lands on the right mark (user decision: `encoder_repeat` and `rotate_all` not rerun, LOG session 2026-10-23 late). Until that passes, do not run P1 or any seal test. The loose coupling is a future mechanical fix (§8.1). Details: LOG, session 2026-10-23 (late).

**Next action (2026-10-23):** K2, K3, E1 and E2 PASS (LOG, session 2026-10-23 afternoon). Next, after the HOLD: **P1** (`slot12`, `full_cycle 12 purge=n shield=n empty=n volume=1000` on the used slot 12 filter, no new filter), then S1 (shield grams, bare tube), then S1-B + S1-C (`full_cycle 11 purge=y shield=y empty=n volume=1000` on one new filter, weighed dry and wet), then S1-D (`full_all purge=y shield=y empty=n volume=1000 order=index from=1 count=3 auto`, three new filters in slots 1–3). Full list: [§0.1](#01-ledger-2026-10-23).

**Status (2026-10-23):**
- Branch `s1-1-pressure-abp`, **uncommitted**. The build passes (`pio run -e due`), `pio test -e native` 163/163.
- **Flashed and checked on the bench this session:** A1 (abort), A2 (homing), A3 (`slot1`), `encoder_repeat` (spread 0.09°, F165 closed), `rotate_all` 15/15 (worst 0.35°), `lock_cycle 5` (unlock 6372 ms), pressure path 1.0 and 2.5 bar (overshoot accepted), seal at 3.0 bar (no leak), K1 (skip) PASS, one full sample (1001 mL, no weighing yet).
- **Flashed 2026-10-23 (user, the latest flash included everything below):** air hold 20 s (F188, D38), `actuator_home` runs at any angle (D36), purge reference 276.42 (D37), offsets 2.3 / 2.5, `rotate_all` unlocks before each move.
- **Not flashed:** nothing known. Check that the bench build matches the working tree before a test.
- **Still open:** K2, K3, E1, E2, P1, S1, S1-B, the full cycle, S1-D, the session log checks 1–10 (the port was not given), the outlet leak (PTFE tape, low priority), the commit (only when asked).
- The manifold encoder came loose on 2026-10-23 (F187), and the reference was re-measured (D33–D37). The current reference is 276.42 (encoder 3145).

### 0.1 Ledger (2026-10-23)

| Step | What | Result |
|---|---|---|
| 1 | Flash, flash-day checks | A1 PASS, A2 PASS, A3 PASS, `encoder_repeat` PASS (0.09°), `rotate_all` PASS (15/15), `lock_cycle 5` PASS (unlock 6.37 s). **Session log checks 1–10 not run** (port placeholder) |
| 2 | Outlet reprint | Done (F186: the outlet was out of square; realigned by hand) |
| 3 | Pressure path and seal | 1.0 bar PASS (overshoot 0.33, accepted). 2.5 bar PASS (overshoot 0.39, accepted). 2.0 and 2.8 bar not run (bracketed). 3.0 bar seal: no leak. The firmware line reads FAIL only because `reached=no` (overshoot, accepted) |
| 4 | Filter alignment | Done (user, "aligned") |
| 5 | S1-C checklist | K1 PASS (skip in the purge and in the sample). K2 PASS (abort, O-ring LOCKED). K3 PASS (shield skip). E1 PASS and E2 PASS (emptying `cond`). **P1, S1 not run** |
| 6 | S1-B, one filter, 1 L | **Not run.** The first full sample ran (1001 mL, flowmeter, not weighed) |
| 7 | `calibrate_all` purge check, one full cycle | **Not run** |
| 8 | Emptying time review | Air hold 20 s (F188). E1 and E2 PASS with it (`cond` on bare tube, pressure 0.01–0.13 bar, the hold ends each phase). Check on a filter at S1-D |
| 9 | S1-D, three in a row | **Not run** |
| 10 | Commit and merge | Only when asked |

**Open findings from this session:** F188 (air hold 1 s, emptying too short; fixed in code, not flashed), F187 (encoder came loose; fixed), F186 (outlet out of square; realigned), F164 (frame realigned again 2026-10-22), F165 (encoder repeat PASS; closed), F166 (abort, done). The fill-loop power is not printed, so the pump's regulation to 3 bar is not visible in the log (question, not blocking).

**Decisions this session:** D33–D37 (purge reference, reference history), D36 (`actuator_home` at any angle, overrides D3/F60), D38 (air hold 20 s), offsets 2.3 CW / 2.5 CCW (rotate_all 15/15).

---

**Status (2026-10-21, kept for history):**
- Branch `s1-1-pressure-abp`, **uncommitted**. Not flashed since the shield check.
- Firmware builds. `pio test -e native` 163/163. `pytest tools/` 101.
- **S1-A passed:** water flows and is measured.
- **S1-B not met:** the filter clogged with field water at 673 mL (F153). Accepted by the user. S1-B is repeated on a new filter.
- **S1-C and S1-D not passed.** Their code is written and builds, but it has not run on the bench since the S1-C changes.
- The manifold was realigned by hand on 2026-10-20 (F164). Its calibration was redone (F165).

---

## 1. Goal and done criteria

**Water flows from the intake through a Sterivex (about 1 L). The DNA shield is added. The lines are emptied. The manifold moves to the next filter. This repeats for several filters. Supervised, from the console. Nothing else.**

**Done when:** `full_all count=3 volume=1000` runs three filters in a row without a manual fix. Each ends `PASS`. Each filter holds about 1 L (weighed, ±10 %). No `SAFETY` line. No leak.

| Step | What | Pass line | Status |
|---|---|---|---|
| S1-A | Water flows | 100 mL through a bare tube (slot 7), then through a filter (slot 11), weighed | **PASS** 2026-10-19 |
| S1-B | One filter, 1 L | `fill_test` (1000 mL, **new** filter, 3.0 bar highest target) ends `volume`, 900–1100 g weighed, no leak | Not met 2026-10-19; repeat |
| S1-C | One real cycle | `full_cycle 1 volume=1000` ends PASS: purge, 1 L, shield, empty (or `empty=n`) | Code done, not flashed |
| S1-D | Several in a row | `full_all count=3 volume=1000`: three PASS, no manual fix | Not started |

---

## 2. Plan: the test sequence

**Read this section first in every session.** Work through the steps in order. Mark a step `[x]` when its pass line is met, and write the result in [LOG.md](../logs/LOG.md). Do not start step N+1 before step N passes, or is stopped on purpose.

**Owner:** who acts. **Prompts:** messages to Claude, as an estimate. **Time:** about 4 bench days if nothing fails (half a day each for S1-A and S1-B, one day each for S1-C and S1-D).

| # | Owner | Step | Pass line | Prompts |
|---|---|---|---|---|
| 1 | user + Claude | Flash the working tree, then the flash-day checks | A1–A3 pass; session log checks 1–10 pass | 2–3 |
| 2 | user | Reprint the outlet manifold | Fitted, reported to Claude | 1 (plus print time) |
| 3 | user + Claude | Pressure path checks (`pressure_ramp` at 1.0, 2.0, 2.8 bar), then the seal check at 3.0 bar | No leak at 3.0 bar for 30 s | 2–4 |
| 4 | user | Filter cartridge alignment | Square and centred | 1 |
| 5 | user + Claude | S1-C fixes checklist (K1–K3, E1–E2, P1–P2, S1) | Each row's pass line | 2–4 |
| 6 | user + Claude | S1-B: one filter, 1 L (new filter, slot 11) | `end=volume`, 900–1100 g, no leak, no cap trip | 1–2 |
| 7 | user + Claude | `calibrate_all` purge check, then S1-C: one full cycle | `PASS`, 1 L weighed, shield done, emptying ends by itself | 2–4 |
| 8 | user + Claude | Review the emptying time with the data (60 s and 1 s hold) | Decision recorded in LOG and §4 | 1 |
| 9 | user + Claude | S1-D: three in a row | Three `PASS` in a row, no hand fix. **Stage 1 done** | 1–3 |
| 10 | user, when asked | Commit and merge | Commit made. `pio test -e native` and `pio run -e due` green | 1 |

**Totals:** the 2026-10-19 estimate (14 to 22 prompts) no longer holds, because the realignment and the reprint were added. Re-estimate after step 3.

**Stop rules:**
- If A1 does not stop within 2 s, do not use `actuator_home` on the bench until it is fixed. Write it down in LOG first.
- If the seal leaks at 3.0 bar (step 3), do not run step 6 until it is fixed.

### Step 1: flash the working tree

- **What is flashed:** the S1-C changes, the purge angle 270.44, the `actuator_home` abort, the session log, and the emptying time of 60 s (D32).
- **Who:** Claude asks for the go-ahead, then flashes (`pio run -e due -t upload`).
- **Then, on the bench:**
  - **A1:** `actuator_home` with the rotor on a slot. Type `x` + ENTER about 2 s in. Pass: `ABORT | actuator_home: stopped` within 2 s, `reason=aborted`, state UNKNOWN, and `slot1` refused (no rotation).
  - **A2:** `actuator_home` with no key. Pass: `done, O-ring UNLOCKED (known)`, PASS.
  - **A3:** `slot1`. Pass: the rotor moves to slot 1.
  - **Lock time (O-ring 210 check):** `lock_cycle 5`. The unlock should take about 6.4 s (210 steps at 33 steps/s). About 5.3 s would mean the 160 build (LOG, FW2).
  - **Session log checks 1–10:** see §6.2. Check 2 decides whether opening the port resets the Due.

### Step 2: reprint the outlet manifold

3D print the new outlet part and fit it (F144, F162). Report to Claude when it is fitted.

### Step 3: pressure path, then the seal check

1. **`pressure_ramp` at 1.0, 2.0 and 2.8 bar** (checks R1–R3). These three targets are not recorded yet. 2.2 bar was held on slot 11 on 2026-10-19 (§7).
   - Setup: slot 12 (clogged) or 11, manometer in view, someone at the 24 V switch.
   - Commands: `actuator_home`, `slotN`, `pressure_ramp target=1.0 hold=10 decay=20`, then `target=2.0`, then `target=2.8`.
   - Pass: `end=completed`, `reached=yes`, no SAFETY line, `p_max` ≤ target + 0.15, manometer within 0.1 bar of the reading. At 2.5 bar, note the manometer (F136).
2. **Seal check at 3.0 bar.** After step 2: `actuator_home`, then `pressure_ramp target=3.0 hold=30` on slot 12. If it leaks, repeat with the O-ring unlocked, for comparison.
   - Pass: `leak=no`, no droplet at the exit for 30 s at 3.0 bar. On a leak: stop, report the slot and the pressure, do not go on.

### Step 4: filter cartridge alignment

Check the cartridge in its holder before a new filter goes in: square and centred. Send a photo (F163).

### Step 5: S1-C fixes checklist

Run the rows in order. Results go in LOG (2026-10-19 entry).

| Row | Setup | Command | Pass |
|---|---|---|---|
| K1 | Bare tube (slot 7) | `full_cycle 11 purge=y shield=n empty=n volume=1000`; type `s` + ENTER 3 s into the purge | `SKIP \| pump run skipped by s` within 2 s; `end=skipped`; sampling starts; TEST line `skipped=y`, result SKIP |
| K2 | New filter (slot 11) | Same run; type `x` + ENTER during the sampling | `ABORT` within 2 s; pump off; valves closed; O-ring LOCKED (`oring_state`); TEST line `aborted=x`; no further pumping in that command |
| K3 | Bare tube (slot 7) | `full_cycle 11 purge=n shield=y empty=n`; `s` + ENTER during the 25 s micro pump | `shield pump skipped by s`; the air push still runs |
| E1 | Bare tube (slot 7) | `full_cycle 11 purge=y shield=n empty=y volume=1000` | `Step empty: end=cond` within 10 s of air arriving. If `end=timeout`, send the pressure log: the 0.4 bar threshold is wrong |
| E2 | Bare tube (slot 7) (LOG). The earlier wording also asks for a filter run: decide at the bench | `full_cycle 11 purge=y shield=n empty=n volume=100`, then `purge=y ... empty=y` | The emptying through the purge ends with `cond` in under 60 s, not `timeout` |
| P1 | Used filter (slot 12, clogged; no new filter, to save filters, 2026-10-23) | `full_cycle 12 purge=n shield=n empty=n volume=1000` | Possible `PAUSE at ...` lines with P between 3.35 and 3.6 bar; the pressure never shows above 3.6 bar; `pauses=N` printed |
| P2 | Build only (no flash needed to read the number) | Set `SPOOL_INSTALLED = true`, then `full_cycle 11 purge=y ...` | `Step purge: 7710 mL, spool installed`; TEST line `purge_ml=7710;spool=installed`. Then set it back to `false` |
| S1 | New filter (slot 11), bottle weighed | `full_cycle 11 purge=n shield=y empty=n volume=100` | The grams are written in LOG; the micro pump time is in the log (`ran 25000 ms`). F156 closes when the grams are recorded |

If E1 or E2 fails, the 0.4 bar emptying threshold is wrong (§3.3, item 5).

### Step 6: S1-B, one filter, 1 L

Only after steps 3, 4 and 5. New filter on slot 11: `fill_test volume=1000 label=b1`. Weigh the water. Pass and the `end=` table: §3.2.

### Step 7: `calibrate_all` purge check, then one full cycle

1. **Purge size (F130): `calibrate_all`** before the cycle. Answer "spool connected?" as the machine is (default yes). Pass: the printed purge matches `purge_volume_ml()` for that answer (3 × dead volume). Paste the line into LOG.
2. **Full cycle:** `full_cycle 11 volume=1000 purge=y shield=y empty=y`. Send the TEST line and the weight. Fix the first failure, then rerun.
   - Pass: `PASS`, 1 L weighed, shield done, emptying ends by itself.

### Step 8: review the emptying time

The value in the code is 60 s (D32, flashed in step 1). The 1 s air hold is still to confirm (D22). Use the E1 and E2 results and the cycle data to confirm both, or change them. Record the decision in LOG and §4.

### Step 9: S1-D, three in a row

`full_all count=3 volume=1000` on three new filters, after step 8. Pass: three `PASS` in a row, no hand fix. **Stage 1 done.** Then the handover in §8.3.

### Step 10: commit and merge

Only when the user asks. Claude commits on `s1-1-pressure-abp` and merges into `cowas-v2`, never `main`. Pass: commit made, `pio test -e native` and `pio run -e due` green.

---

## 3. Step procedures

### 3.1 S1-A: water flows (PASS)

The pressure builds but no water comes out at first (F148). The decay after the pump stops is slow (1.9 → 0.8 bar in 10 s). That looks like air in the line. Water is nearly incompressible, so the pressure would drop at once. Two suspects: no water reaches the pump, or the flowmeter shows nothing. Order of checks:

1. **Slot 7, bare tube, outlet into a jug on a scale.** `slot7`, then `fill_test volume=100 label=a1 quick`.

   | You see | It means | Do |
   |---|---|---|
   | Water comes out, printed mL within 20 % of the weight | Water path and flowmeter fine | Go to 2 |
   | Water comes out, printed mL stays about 0 | The flowmeter does not count: D13 voltage (3.3 V max, F26), wiring or sensor | Fallback: take the volume from the weighing (§5), carry on |
   | **No water** | Nothing reaches the pump, or the pump runs dry | Intake in the water, `valve_check` (V1 closed, Vman open), listen for the pump, prime by hand with the intake in the water, check for air leaks on the suction side. Fix this first |
2. **Slot 11, new filter.** `slot11`, `fill_test volume=100 label=a2 quick`. If pressure builds and nothing comes out, the line after the pump still holds air against a wetted membrane. Prime with water through slot 7 until the outlet runs full. Keep the intake under water. Then go back to slot 11. A dry Sterivex passes water, so the filter is not the suspect.
3. **Pass:** 100 mL weighed through slot 11 in 5 minutes or less, at 2.2 bar or less, printed mL close to the weight. The time gives the time for 1 L.

**Result (2026-10-19): PASS** (user). Slot 7: 1000 mL printed, 1006 g weighed (0.6 %). Slot 11, new filter: 100 mL printed in 52 s at 2.2 bar, no pause. The runs on the used filter are in the same log. They show the stop criteria, not the flow. **Open:** the weight of the slot 11 run is not in the notes. Record it in LOG (S1-A) and `docs/test-logs/2026-10-19_s1a.log`.

### 3.2 S1-B: one filter, 1 L (NOT MET; to repeat)

The filter clogged with field water at 673 mL counted (F153). The user accepted this and moved on to S1-C. The pass still needs a **new** filter on slot 11. A used filter stops early (F152), so `end=volume` on it means nothing.

1. **Weigh the water** in a tared jug or bucket. The flowmeter reads low at low flow (F150), so the weight is the volume.
2. `slot11`, `fill_test volume=1000 label=b1`. Defaults: target **3.0 bar** (highest allowed), pause 3.35 bar, resume under 3.15 bar, cap 3.6 bar, timeout 30 min, stall window 60 s. Someone at the 24 V switch.
3. Read `end=` on the TEST line:

   | `end=` | Meaning | Do |
   |---|---|---|
   | `volume` | Flowmeter counted 1000 mL | Weigh. 900–1100 g is the pass |
   | `stalled` | Under 3 mL in a 60 s window of pumping time | Clogged. The weighed water is the sample, flagged (§5). Not a pass |
   | `clogged` | Mean of the last two windows under 30 % of the first | Same as `stalled` |
   | `pressure` | Paused, and pressure does not come back under 3.15 bar for 20 s | The filter holds the pressure. Note the pressure, weigh, stop (§5) |
   | `timeout` | 30 min | Weigh the partial volume. Flag it. Not a pass |
   | `cap` | Hard cap (3.6 bar) reached | Not expected. Stop. Write down the log |
   | `aborted` | A key was pressed | Weigh. Note |

4. **If the weight is off by more than 10 % at `end=volume`:** correct the flow factor once. New `calibrationFactor_small` = 77 × printed mL / weighed g (`Flow_sensor.h`). Reflash with the user's OK, and repeat. Take the factor from a normal-flow run (slot 7 agreed to 0.6 %), not from a clogged run (F150).
5. Then `slot12` (clogged), `fill_test volume=200`. It must end `stalled`, `clogged`, `pressure` or `volume`, never `cap`.

**Pass:** on a new filter, `end=volume`, 900–1100 g weighed, no droplet at the manifold seal, no `SAFETY` line. The slot 12 run ends by one of its stop reasons.

### 3.3 S1-C: one real cycle (code done, not flashed)

**Goal:** `full_cycle 1 volume=1000 purge=y shield=y empty=y`. **Pass:** a `PASS` line, 1 L weighed, shield added, filter capped and labelled. If the emptying fails (air through a wet filter), run `empty=n`: a filled, shielded filter is already a sample.

**Sequence of one `full_cycle <slot> purge=y shield=y empty=y volume=mL`** (code: `src/Step_functions.cpp`; the TEST line shows each step). Sequence (user, 2026-10-19): purge (spool or no spool) → sample: fill the filter → purge: air emptying → sample: air emptying → DNA-shield pump (primed first with `prime_shield`) → a little air (4.8 s) to push the shield into the filter → back at the purge, ready for the next sample. No deployment-line emptying.

1. **Purge** (`step_purge`): pumps water from the intake through the **purge slot** (slot 0) until the purge volume is counted. Without the spool it is `PURGE_MILLILITERS` (100 mL, placeholder, F30). With the spool it is 3 × (intake + spool), about 7.7 L (`SPOOL_INSTALLED`). The 2026-10-19 purge took 9 s.
2. **Sample** (`step_sampling`): rotates to the filter slot and locks. Then:
   - pumps water through the filter until the volume is counted (flowmeter, uncalibrated, F6), or a stop: stalled, clogged, timeout (30 min, `SAMPLE_MAX_DURATION_MS`), cap, sensor (same rules as `fill_test`);
   - **air emptying through the purge slot:** up to `EMPTY_MAX_TIME_MS`; stops when the pressure falls under `EMPTY_WATER_PRESSURE_PURGE_THRESHOLD` = 0.4 bar (placeholder);
   - **air emptying of the filter:** the same, with `EMPTY_WATER_PRESSURE_STX_THRESHOLD` = 0.8 bar.
   Operation pumping: at most 60 % with water, 100 % with air. Target 3 bar (D21).
3. **Shield** (`step_DNA_shield`): the micro pump (D48) runs for `FILL_STERIVEX_TIME / 10` (25 s, a test shortcut, F4). Then the valves switch and the big pump pushes air at 13 % for `PUMP_SHIELD_TIME` (300 ms as set). The log shows 4.8 s, because the 4 s soft ramp of `Pump::start(ms)` is included. Both are placeholders. Neither is measured (F156).
4. **Deployment-line emptying removed** from the cycle (D23). `step_empty()` stays for `abort_sample()`. `full_cycle empty=y` only records `empty_step=removed`.
5. **Unlock** the O-ring. The test ends unlocked. The rest state (locked) comes in FW4.

The TEST line of `full_cycle` reports `sample_end=` (volume, timeout, cap, sensor, clogged; `rule=` for the clog criterion) and `placeholders=` (purge_ml, shield_time, flow_factor).

**Changes made in this stage** (in the working tree, not flashed):
1. **PID:** nothing to do. 200 / 100 / 100 in tests and production since 2026-10-19.
2. **Sample timeout:** the 3 minute limit in `step_sampling` (F15) became `SAMPLE_MAX_DURATION_MS` (30 min). The TEST line says why the run ended (`end=volume|timeout|stalled`), so a clogging filter is never silent.
3. **Placeholders:** purge volume (intake hose about 30 mL; 150 mL is enough on the bench), shield time (25 s test value, or the lab's number), `volume=1000` on the command line. Printed on the TEST line.
4. **Skip and abort keys** (D25). Before: `x` aborted tests only. Now every pump loop reads the keys.
   - `s` + ENTER skips the current pump run. The step ends as `skipped` and the next phase runs. A skipped sample is flagged and is not a sample. Its shield and emptying still run.
   - `x` + ENTER aborts the command. The pump stops, the valves close, the O-ring locks.
   - Pure rule: `skip_or_abort(key)` in `src/Core/`, with native tests.
5. **Emptying threshold (F155).** Air-only lines read 0.02 to 0.09 bar. Water through the open tube reads 0.75 to 0.83 bar. The old threshold, 0.04 bar, was never met on a free tube, so the emptying ran to its maximum. It is now **0.4 bar** (placeholder). Checks E1 and E2 test it on the bare tube (§2, step 5); the earlier wording also asks for a filter run. A run that never sees the threshold ends at its maximum and says so, with the last pressure. If E2 fails, the fallback is a time per phase, or a flowmeter rule (a new rule in `src/Core/`, with its own test).
6. **Purge volume and spool.** The 2026-10-19 "6 L" was the spool. Its volume is about 2.5 L (F130). It is **not installed** (`SPOOL_INSTALLED = false`). The purge is max(`PURGE_MILLILITERS`, 3 × dead volume): 100 mL without the spool, 7710 mL with it (the rule of stage 2 §3.7). Native test: `test/test_run_keys/`.
7. **DNA-shield delivery not shown** (F156, F159). The log shows "started" and "stopped" with no time between them, and the air push lasts 300 ms. The "started" line is software: it prints when the pin is set.
   - Check first, with no code change: `shield_calibrate` (LA3), or weigh the bottle before and after `shield_only`. Write the grams in LOG.
   - If the pump did not move: check the reservoir, the tube and the pump on D48 (time-based, `Micro_Pump::start(ms)`). Do not change code yet.
   - After the measurement, set `PUMP_SHIELD_TIME` so the shield reaches the filter (LA3, FW4).
8. **Test menu:** an unknown line prints `UNKNOWN command` (F154, done). The shared-brick design is in stage 2 (FW4a).
9. **Pressure in operation** (F157, user: "pausing is important, implement it"). `CtrlPumpFlow` pauses at 3.35 bar and resumes at 3.15 bar with the `fill_test` rules. The pressure stop is on, in `step_sampling` and `load_slot_water`. The PID target is 3 bar (it was 2 bar until 2026-10-19).

### 3.4 S1-D: several in a row

`full_all count=3 volume=1000`, then `count=5`. Used slots go on a paper list (the Due forgets them at every reset). **Pass:** three `PASS` lines in a row, nothing fixed by hand. This ends stage 1. A lake-side run is the same command with a person at the machine.

---

## 4. Rules and settled values

**Focus rules**
1. A step ends when its pass line is true, not when it is perfect.
2. Keep what works. No new tuning, tools, analysis scripts or refactors during stage 1.
3. A step's record is one entry in [LOG.md](../logs/LOG.md) plus the bench transcript.
4. Found something else? One line in the deferred table (§8.1), with its stage 2 home. Then go on.
5. Stuck for half a day? Take the step's fallback (§5).
6. Safety does not change. The cap is 3.6 bar, and only the user changes it. Someone is at the 24 V switch for every run. Never unattended. The rule 4 invariants of CLAUDE.md apply.

**Settled: do not redo.** The values below are decided. Do not re-tune or re-decide them without the user. Working style: short, plain answers. Stop optimising. Get water flowing.

| What | Value |
|---|---|
| Pressure sensor | ABPDANV150PGSA3 (150 psi). `ABP_FULL_SCALE_PSI = 60` stays: it matches the manometer (F126). The manometer is the reference (user). The calibration absorbs the rest: `PRESSURE_GAIN = 1.0413`, `PRESSURE_ZERO_OFFSET_BAR = -0.0311`, valid to 2.0 bar |
| Cap and working pressure | `STX_MAX_PRESSURE = 3.6` bar (user, 2026-10-19). The only number to change by hand. Derived in `Settings.h`: pause 3.35 bar (cap − 0.25), resume 3.15 (pause − 0.2), highest fill target 3.0 (cap − 0.6, the default). S1-A ran at 2.2 bar. Calibration valid to 2.0 bar only (F136) |
| PID | **200 / 100 / 100** (user, 2026-10-19), slew 3 % per 100 ms, start 15 % at once, raw pressure. It ripples (F146) but drives real power. Start at once with `Pump::start_at(15)`: no 4 s soft start in a regulated loop (D16). Output through `Pump::set_power_live`, which never blocks. PID tuning by measurement is dropped for stage 1 (S1-1b part 1b, `tools/compare_tune.py`); stage 3 P2-4 keeps it |
| Pump | Nothing flows below about 10 %. On slot 11, flow starts near 12 % |
| Pumping limits | Operation pumping at most 60 % with water, 100 % with air. Target 3 bar (D21) |
| `fill_test` stops | Volume, timeout (30 min), `stalled`, `clogged`, `pressure`, aborted, cap. The reason is on the TEST line as `end=`. Stall windows count pumping time only (F149, fixed) |
| Emptying time | `EMPTY_MAX_TIME_MS` = 60 s (D32, 2026-10-21): the longest an emptying phase (an air push) may run. It ends earlier when the air is detected. The air hold is 20 s (D38, 2026-10-23, F188; was 1 s) |
| Purge angle | `purge_angle = 344.27` (D40, F191: 276.77 + 67.5, the 3-slot fix; user confirmed by eye). The reference is raw 0 = slot 7. In code, **not flashed yet**. Earlier values (345.06, 279.58, 275.45, 276.42; D33–D37) are superseded |
| O-ring | `LINEAR_ACT_LOCKED_STEPS = 210` (current value; confirmed 2026-10-20). A 2026-10-12 entry says 160: see the conflict note in LOG, FW2 |
| Hose | Intake 4 mm ID × 2.40 m, about 30 mL. Spool path 9 mm × 40 m, about 2.5 L, connected by default (F130) |
| Flowmeter | `flow_sensor_small` on D13, factor 77, uncalibrated, 5 V risk (F26) |
| Working style | The manometer is trusted. Short, plain instructions. Do not flash or commit without asking |

---

## 5. Fallbacks

| If | Then |
|---|---|
| The flowmeter does not work or cannot be trusted | Volume by weighing: run `fill_test` for a measured time at the usual pressure, weigh, and use the time for the volume. Fix the flowmeter in stage 2 (HW3) |
| The pressure oscillates | Expected with 200 / 100 / 100 (power bursts to about 18 %, ripple about 1 bar, F146). The water still flows. If the peaks reach the cap, lower the target to 2.0 bar. The calm `kp=3 ki=0.2 kd=20` is available on the command line but runs at 8–12 % of power and may not move enough water. No tuning session |
| The manifold leaks at 2.2 bar | Re-run `calibrate_actuator` for more lock force, or work at 1.8 bar |
| A filter clogs | `end=stalled`, `clogged` or `pressure`. The weighed partial volume is the sample, flagged. This is correct behaviour, not a failure |
| The emptying step fails | `empty=n` |
| Stuck on a step for half a day | Write what you saw in [LOG.md](../logs/LOG.md) and ask. Do not widen the scope |

---

## 6. Tools and session records

### 6.1 Console and session log (built, not flashed)

**The tool:** `tools/session_log.py`. It shows the Due's console like `pio device monitor`, and it sends what you type (one line, LF). It writes to `docs/test-logs/`, for a run tagged `<tag>` on `<date>`:
- `<date>_<tag>.log`: the full transcript. Each line in (`<`), out (`>`) or from the tool (`!`), with the PC time.
- `<date>_<tag>.csv`: one row per `TEST|<name>|<id>|<PASS/FAIL/SKIP>|<k=v;...>` line (the FW2 checklists).
- `<date>_<tag>_samples/`: one JSON file per sample or purge (the firmware's `{"t":"sample",...}` line plus `pc_received`), and `samples.csv`, one row per sample.

Run: `python tools/session_log.py --port COM6 --tag s1-1c`. Quit with `:quit` or Ctrl+C. `tools/hw_test_log.py` is the old name. It runs the same code. Operator tutorial: [reference/session-log.md](../software/session-log.md).

**Decisions:**
- **Clock:** the PC clock stamps every line and record. The Due has no RTC. The Due's `millis()` is in each record, to order events within a run.
- **Port:** opening the Programming port resets the Due (stage 2 §3.4). The tool holds DTR and RTS low before it opens, and refuses 1200 baud (it erases the flash). **Not verified on the Due:** check 2 below decides it.
- **No host:** a run needs the tool open. If it is not running, the sample is still done, but nothing is kept on the Due.
- **Disconnect:** a pulled cable prints `DISCONNECTED` and ends the session. Restart the tool when the port is back.

**The sample record** (`src/Core/sample_record.cpp`, printed by `src/Step_functions.cpp` when a purge or a sample pump run ends, also when it ends on a key):

| Field | Meaning |
|---|---|
| `fw`, `id` | Firmware version (`S1-1b-dev`, update per step). `id` is `auto-N`, counted from boot. The operator id is not in the firmware yet |
| `purpose`, `slot` | `purge` or `sample`; the manifold slot |
| `start_ms`, `end_ms`, `target_ml`, `vol_ml`, `purge_ml` | Timing; target and measured volume (flowmeter, uncalibrated, F6); purge volume for a purge |
| `end_reason` | `volume`, `pressure`, `timeout`, `stop_key`, or the control loop's names: `sensor`, `skipped`, `stalled`, `clogged`, `refused` |
| `p_mean_bar`, `p_max_bar`, `p_cap_bar` | Mean and peak of every plausible reading in the run (pauses included). Cap in force |
| `lock_before`, `lock_after` | O-ring state before and after: `LOCKED`, `UNLOCKED`, `UNKNOWN` |
| `shield_ml` | Always null here. The shield pump is a separate step |
| `calib` | Flow factor, pressure gain and offset, ABP full scale, locked steps |
| `flags` | `flow_uncal`, `placeholder:ABP_FULL_SCALE_PSI`, `placeholder:STX_MAX_PRESSURE`. Set by hand in `Step_functions.cpp`. Update when those are closed |

A missing value is `null`, never 0.

**Known limits:** nothing is kept if the tool is not running. The sample line starts with `{`, which `host/` routes as protocol: FW7 must route `{"t":"sample"` as a log line. The Python tool is for the laptop. The Pi version comes with HO4.

**Built and tested off the bench (2026-10-21):** `pio run -e due` passes. `pio test -e native` 163/163, including `test_sample_record` (a golden line that the Python tool parses). `pytest tools/` 101 passed. The serial parts (port, disconnect) are **not** tested without the Due.

### 6.2 Session log checks (step 1)

| # | Do | Pass |
|---|---|---|
| 1 | `python tools/session_log.py --port COMx --tag s1-1c`, Due on the Programming port | The session folder with the `.log` and `_samples/` exists within 2 s |
| 2 | Open the port, wait 10 s, type nothing | 0 boot-banner lines in the `.log`. If the Due resets anyway, the DTR/RTS-low open does not work. Write it down. The console moves to the Native port after FW7 |
| 3 | Type `stop` (any command with a reply) | Reply on screen within 1 s |
| 4 | One `purge` (100 mL, bench water), then one `sample1m` on a new filter | One JSON per run in `_samples/`, both parse. `samples.csv` has 2 rows. `end_reason`, volumes, `p_mean_bar` and `p_max_bar` not null. Values match the screen |
| 5 | Compare the screen with the `.log` | Same device lines, same count |
| 6 | Pull the Due's USB cable during a run, then put it back | `DISCONNECTED` within 2 s on screen and in the `.log`. The tool exits without a traceback. A restart opens a new session |
| 7 | Make the `_samples/` folder read-only during a run (or rename it) | `LOG ERROR` on screen. The console still answers. The next `.log` line starts with `LOG GAP` |
| 8 | Run `purge` and then `sample1m` (or a `fill_test`), then open the sample JSON | `calib`, `flags`, `end_reason` and `pc_received` present; `flags` lists `flow_uncal` and the two placeholders; `null` only where documented |
| 9 | Send `s` + ENTER during a pump run, then `x` + ENTER during another | `.log` shows the `>` lines and the `SKIP` / `ABORT` replies; the sample record has `end_reason` = `skipped` or `stop_key` |
| 10 | Two runs with different tags on the same day | Two separate `.log` files; nothing from one in the other |

Results go in LOG (the 2026-10-21 entry). Transcripts go in `docs/test-logs/`.

---

## 7. Status detail

### 7.1 What works (bench)

| Area | State |
|---|---|
| Manifold rotation, encoder, O-ring lock, `actuator_home` | FW2 rows 1–5 passed 2026-10-12. Lock at 210 steps (confirmed 2026-10-20). Purge angle 270.44 from `cal` on 2026-10-20 (F165). `actuator_home` has an `x` abort (F166). Both not flashed |
| Pressure sensor (ABP, SPI D8), calibration, dead-sensor stop, cap | Calibrated 2026-10-12, valid to 2.0 bar. Dead-sensor stop and cap trip done. Cap 3.6 bar (2026-10-19) |
| Pump regulated by pressure | `pressure_ramp` on slot 11 held 2.2 bar with the calm gains 3 / 0.2 / 20 (overshoot 0.23, ripple 0.38 bar), but at only 8–12 % of power, which may be too little to move water. **Defaults are back at 200 / 100 / 100** (user, 2026-10-19): power bursts to about 18 %, pressure ripples by about 1 bar, accepted for the prototype |
| `fill_test` (volume through the filter, pauses at the limit, stop criteria) | Run on the bench 2026-10-19 (S1-A, slots 7 and 11). Water flows and is measured. Stop criteria rebuilt from the logs (F149, F152) |
| `full_cycle`, `full_all` (purge, sample, shield, empty) | Coded. **Never run with real water** since the pressure sensor went in |

### 7.2 What does not work, or is unknown

| Issue | Why it matters |
|---|---|
| The flowmeter under-reads at low flow (F150): 8 mL counted for 125 g, 517 mL for 847 g. Within 3 % at 0.65 L/min | Weigh the water for any volume at low flow. Fix in HW3 |
| Pauses near the pause level (F151): 366 pauses in 383 s at 3.0 bar | Accepted for the prototype. A lower target gives fewer pauses |
| The manifold seal leaked droplets near 3 bar in earlier tests (F144). None reported at 3.0 bar on 2026-10-19 | Check the seal at 3.0 bar (step 3) |
| Production loops (purge, sample, empty) use the same 200 / 100 / 100 as the tests. They start at once and do not block (F147 is moot) | Nothing to switch |
| Flowmeter factor 77 never calibrated. 5 V level risk on D13 (F6, F26). Sample volume, purge and shield values are placeholders | Checked in S1-A and S1-B. Set by hand in S1-C |

### 7.3 Completed checks

| Check | Date | Result |
|---|---|---|
| S1-1 calibration against the manometer (5 points) | 2026-10-12 | PASS: gain 1.0413, offset −0.0311, R² 0.99897, max residual 0.032 bar |
| S1-1 dead-sensor stop | 2026-10-12 | PASS (user report) |
| S1-1 cap trip at a temporary 1.0 bar | 2026-10-12 | Done (user report). Manometer reading at the trip not reported |
| Manifold purge angle after the manual realignment (F165): encoder 3077.00 → 270.44 | 2026-10-20 | User report, in `Manifold.h`. Not flashed |
| **S1-A: water flows** (slot 7: 1000 mL printed, 1006 g; slot 11, new filter: 100 mL in 52 s at 2.2 bar) | 2026-10-19 | PASS (user). `docs/test-logs/2026-10-19_s1a.log` |
| Spare 150 psi ABP and the FW3 cap trip: superseded by the installed ABP | 2026-10-12 | Done. The spare sensor check (stage 2 §7a, HW4) is needed only for a second sensor |

---

## 8. Later, and not in this stage

### 8.1 Deferred to stage 2 and 3

Nothing is lost. Each item is also written at its step in the stage 2 and stage 3 files.

| Item | Goes to |
|---|---|
| Layered pressure controller (setpoint ramp, soft limit, rise-rate limit, gain scheduling), clog analytics | FW8 (minimal clog end condition) and [stage 3](../roadmap/stage3-full-system.md) P2-6 |
| PID tuning tools (`tools/compare_tune.py`, tune CSV), 10 Hz trace, `pump_step` test, the pump model (deleted 2026-10-19, F146) | Stage 3 P2-4 / P2-5 / P2-6 |
| One set of PID gains (`PUMP_PID_*` and `PRESSURE_RAMP_PID_*` are two names for one value until FW4), one `PRESSURE_TARGET_BAR` for the hard-coded 2 (F3), `Pump::set_power` blocking ramp (F132) | FW3 / FW4 |
| Stop by key press in the pump loops; recovery after a reset; watchdog | FW5 / FW6 |
| Purge volume from hose and dead volume (a tested function), shield dose, residual water | FW4 / LA3 |
| Flowmeter: a 3.3 V safe input and a proper calibration (F6, F26) | HW3 |
| Relief valve, E-stop, pressure rating of the weakest part, the leak at 3 bar (F144), the cap against the sensor range (F126, F143) | HW2 / HW7 |
| Laptop campaign runner (`tools/campaign.py`), slot inventory file, sample record columns | HO2 / HO3. Until then: the console, the session log and a paper list |
| Delete the A2 code, rename `pressure1` to `p_in`, SPI diagnostics | FW3 |
| Auto-unlock, locked rest state, coils off, V23 removal, float flow factor | FW4 |
| A pump and driver controllable from near zero (F137, F141, F142) | Stage 3, "Notes for the next machine version" |
| Manifold outlet leak at the 3D-printed thread (not the O-ring seal), F189. Not urgent, not for the prototype; PTFE tape for now | Stage 2, outlet part (HW) |
| Keys read during the O-ring move (the unlock/lock before the pump starts), F190 | FW5 |
| Manifold encoder coupling loosens (F187, F191: three times in two days). Needs a mechanical lock, not only tightening. Must be fixed before the pilot | Stage 2, mechanical (HW) |

### 8.2 What is knowingly missing

The prototype has a working cap, a dead-sensor stop and a supervised console cycle. These stage 2 layers (stage 2 §3.2) do not exist yet:
- **Relief valve and E-stop.** The 24 V switch is the E-stop: someone is at it for every run.
- Watchdog and supervisor; stop outside the pump loops (wait or cut power).
- Recovery after a reset (lock state UNKNOWN: `actuator_home` with the rotor on a slot, F60).
- Slot inventory in the Due (paper list).
- Sample-quality values: purge factor, shield dose, residual water. **Filters from stage 1 are engineering samples, not publishable eDNA data. Tell the lab. Include a blank.**

Do not use the panel pump mode, `calibrate_flow` or `fillContainerFunction`. They do not poll the cap (F2, F41, F42). The laptop must run on mains with sleep off. A sleeping laptop resets the Due (F125).

The shield dose check (LA3 Part A) is in [stage 2 §7a](../roadmap/stage2-pilots.md).

### 8.3 Handover to stage 2

When S1-D passes: commit (when the user says so). Then the deferred table (§8.1) is the first list of stage 2 work, in the order of [stage2-pilots.md](../roadmap/stage2-pilots.md) (FW3, FW4, FW5 …). The numbers measured here (time for 1 L, flow factor, purge and shield values) are inputs to FW4, HW3 and LA3.
