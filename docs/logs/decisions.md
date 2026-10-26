# Decisions

Decisions are `PH<n>` (before the first firmware step, PH = prehistory) and `D<n>`. Keep the numbers. A superseded decision stays in the table and says what replaced it. Each row gives the date, who decided, the reason or finding, and the log entry where the decision is recorded.

**Register.** This file is the register of decisions. A decision is changed here (superseded or overridden) and recorded in the log with its date.

**By:** `user` = the operator's decision. `code default` = a value chosen in the code and not yet confirmed by the user (see the row).

## Numbered decisions
| # | Date | Decision | By | Why / step | Entry |
|---|---|---|---|---|---|
| PH1 | 2026-09 | Manifold needs a cycle: loosen to rotate, tighten to seal (O-ring). The linear actuator does this | user | Rotation and sealing trade off  | — |
| PH2 | 2026-09 | Force sensor used only for contact detection. Partial unlock by a fixed step count | user | Partial-unlock signal too close to the noise floor  | [2026-09](LOG.md#force-sensor-added-then-removed) |
| PH3 | 2026-09 | Remove the force sensor. Lock open-loop from a known unlocked reference. Keep a button seam for the future | user | FSR not accurate: temperature-sensitive, not sensitive enough (F179)  | [2026-09](LOG.md#force-sensor-added-then-removed) |
| PH4 | 2026-09 | Purge moved from the top (calibration point) to 180°. One index list | user | Contamination: a leak drips down, so the upper samples are never reached  | [2026-09](LOG.md#manifold-layout-and-fill-order) |
| PH5 | 2026-09 | `FILL_ORDER` low to high by quadrant. Highest slot last | user | A leak only reaches already filled slots. Limits rotation  | [2026-09](LOG.md#manifold-layout-and-fill-order) |
| PH6 | 2026-09 | Pressure cap centralised in `Pump` (3 bar at the time) | user | Covers every call site, tests included  | [2026-09](LOG.md#pressure-safety-and-test-procedures) |
| PH7 | 2026-09 | Production `step_sampling` keeps its own emptying. Not switched to `load_slot` | user | Avoid destabilising working code  | [2026-09](LOG.md#firmware-linear-actuator-and-sample-cycle) |
| PH8 | 2026-09 | Encoder calibration asks for a direction. Speed 20 (`MANIFOLD_CAL_SPEED`) | user | Safe direction from the current position  | — |
| PH9 | 2026-09 | PTFE seal made with replaceable O-rings. Gluing after O2 plasma activation rejected | user | Replaceable instead of a permanent bond  | — |
| PH10 | 2026-09 | Force sensing removed from the prototype. The full system needs a new sensor type | user | FSR not accurate (F179)  | — |
| D1 | 2026-09-25 | A failed or refused rotation stops the whole procedure | user | Safety. FW2  | [2026-09-25](LOG.md#la1-lab-alignment-meeting-template-2026-09-25) |
| D2 | 2026-09-25 | Sensorless test mode: 30 % power limit (placeholder) | user | FW2  | [2026-09-28](LOG.md#fw2-mechanical-test-suite-lock-tri-state-test-logging-2026-09-28) |
| D3 | 2026-09-25 | No escape when the lock is UNKNOWN and the manifold is not aligned | user | F60  | [2026-09-28](LOG.md#fw2-mechanical-test-suite-lock-tri-state-test-logging-2026-09-28) |
| D4 | 2026-09-25 | Rest state: O-ring locked. Unlock only to rotate | user | FW1  | [2026-09-25](LOG.md#fw1-protocol-v1-specification-2026-09-25) |
| D5 | 2026-09-25 | Protocol decisions (§14 of the protocol file): duration = water phase only; `duration_reached` end; GET_INFO allowed while busy; STOP latches; purge below minimum is raised with a warning; dedupe keeps 16 ids and the last 4 records; `device_id` = `cowas-V1` | user | FW1  | [2026-09-25](LOG.md#fw1-protocol-v1-specification-2026-09-25) |
| D6 | 2026-10-06 | Three stages. Stage 1 = working prototype first | user | Water tests in the week of 2026-10-12  | [2026-09-25](LOG.md#la2-sops-for-filters-cleaning-biosecurity-chain-of-custody-2026-09-25) |
| D7 | 2026-10-09 | The manometer is the reference. `ABP_FULL_SCALE_PSI = 60` (placeholder). Do not re-check against the sensor body | user | F126  | [2026-09-25](LOG.md#la2-sops-for-filters-cleaning-biosecurity-chain-of-custody-2026-09-25) |
| D8 | 2026-10-09 | Calibrate with a syringe of air on a tee, not with the pump. The pump-driven water-column check was written and removed | user | F126  | [2026-10-06](LOG.md#planning-three-stages-stage-1--working-prototype-2026-10-06) |
| D9 | 2026-10-09 | `calibrate_pressure`: ask, wait for ENTER, then measure 3 s. Nothing is measured before ENTER | user | Bench bug, S1-1  | — |
| D10 | 2026-10-09 | `PRESSURE_SENSOR_AVAILABLE` back to `false` until the sensor is steady | code default | F124  | [2026-10-09](LOG.md#s1-1-pressure-on-the-abp-calibration-command-2026-10-09) |
| D11 | 2026-10-12 | `STX_MAX_PRESSURE` = 3.0 bar for the seal test | user | F136  | — |
| D12 | 2026-10-12 | `calibrate_all` asks the spool question and prints the purge the rule needs. Production purge unchanged | user | F130  | — |
| D13 | 2026-10-12 | D13 check deferred. Alignment check (F131) deferred to the pilot | user | Time  | [2026-10-12](LOG.md#s1-0-bench-session-of-2026-10-12-values-reported-by-the-user-no-log) |
| D14 | 2026-10-12 | Stop offsets 1.2 / +1.2. Wrap point of `scale_angle` at the no-hole position | code default | F128, F129  | — |
| D15 | 2026-10-12 | Dead sensor and cap trip checks accepted | user | S1-1  | — |
| D16 | 2026-10-15 | No 4 s soft start in any regulated loop. Start at once | user | F132, F133  | — |
| D17 | 2026-10-15 | Cap 3.5 bar, then 3.2 bar | user | F143, F144  | — |
| D18 | 2026-10-15 | Gentle controller dropped for a PID again | user | F138, F146  | — |
| D19 | 2026-10-19 | Cap 3.6 bar. Pause, resume and target derive from it (3.35 / 3.15 / 3.0 bar) | user | Only the cap is set by hand  | — |
| D20 | 2026-10-19 | S1-B not met (dirty water). Accepted. Go to S1-C | user | F153  | [2026-10-19](LOG.md#s1-b-and-s1-c-one-filter-at-1-l-the-shared-filter-stop-full_cycle-end-reasons-2026-10-19) |
| D21 | 2026-10-19 | Operation: 60 % power with water, 100 % with air. Target 3 bar. Each emptying phase has a maximum time (value: see D32) | user | F160, F161  | — |
| D22 | 2026-10-19 | Emptying: air hold shortened to 1 s, so a 5 s emptying can end early. **To confirm** (interpretation) | user (interpreted) | S1-C  | — |
| D23 | 2026-10-19 | Deployment-line emptying removed from the cycle | user | F160  | [2026-10-19](LOG.md#session-close-2026-10-19-what-was-learnt-and-where-it-stands) |
| D24 | 2026-10-19 | Spool not installed (`SPOOL_INSTALLED = false`). Spool volume 2.54 L | user | F130  | — |
| D25 | 2026-10-19 | A skip ends only the current pump run. The shield still runs after a skipped sample | user | S1-C  | [2026-10-19](LOG.md#s1-c-fixes-skip-and-abort-keys-emptying-rule-purge-volume-pause-in-operation-shield-timing-2026-10-19) |
| D26 | 2026-10-19 | PID gains back to 200 / 100 / 100 for tests and production | user | F146, accepted ripple  | [2026-10-19](LOG.md#update-6-2026-10-19-the-original-pid-again-a-fill_test-bug) |
| D27 | 2026-10-19 | Dead code deleted: `power_ramp`, `gentle_ctrl`, the pump model test | user | Clean-up  | [2026-10-19](LOG.md#update-7-2026-10-19-dead-code-removed-d27) |
| D28 | 2026-10-19 | Stage 1 rewritten around one goal. Focus rules apply | user | "It needs to work, not be perfect"  | — |
| D29 | 2026-10-20 | `purge_angle` 270.44 from the new `cal`. `LINEAR_ACT_LOCKED_STEPS` 210 kept | user | F165  | [2026-10-20](LOG.md#recalibration-after-the-manual-realignment-and-the-actuator_home-abort-2026-10-20) |
| D30 | 2026-10-21 | One console tool ([`session_log.py`](../../tools/session_log.py)); [`hw_test_log.py`](../../tools/hw_test_log.py) kept as a shim | user | Merge  | [2026-09-25](LOG.md#hw1-electrical-audit-2026-09-25) |
| D31 | 2026-10-21 | Docs restructured. LA4 protocol dropped (never written). Pointer files and HTML reports removed | user | Docs  | — |
| D32 | 2026-10-21 | Emptying maximum time = 60 s (every emptying phase, which is an air push). Closes the 15 / 30 / 60 s conflict. Set in [`Settings.h`](../../include/Settings.h), not yet flashed | user | S1-C, F155  | — |
| D33 | 2026-10-22 | `purge_angle` = 345.06 from `cal` with the rotor on **slot 7** (encoder 3926.00). Slot 7 is raw 0, the reference, so the value is the reference, not the purge hole. Replaces 270.44 (F165). Not flashed | user | F165, F186  | [2026-10-23](LOG.md#session-2026-10-23-bench-summary-stage-1-step-1-to-step-7-part) |
| D34 | 2026-10-23 | `purge_angle` = 279.58 from `cal` on slot 7 (encoder 3181.00). Replaced 345.06 (D33). Superseded by D35 | user | F165, F186  | [2026-10-22](LOG.md#outlet-reprint-frame-realignment-and-the-reference-2026-10-22) |
| D35 | 2026-10-23 | **The manifold encoder was loose.** It was re-tightened, and the manifold had been lost. `purge_angle` = **275.45** from `cal` on slot 7 (encoder 3134.00). The earlier values (3077, 3926, 3181) were all read with the encoder slipping. This also explains the 75° shift (F187). Not flashed | user | F165, F186, F187  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D36 | 2026-10-23 | `actuator_home` runs whatever the manifold angle (user). It moves only the O-ring (overdrive lock to the stop, then unlock). It never moves the manifold. Off-slot it prints `WARN` and the homing test records FAIL with `err_deg`. Overrides D3 and F60 ("no escape when UNKNOWN and not aligned"). Built, not flashed | user | F60, F187  | [2026-10-23](LOG.md#session-2026-10-23-bench-summary-stage-1-step-1-to-step-7-part) |
| D37 | 2026-10-23 | `purge_angle` = 276.42 from `cal` on slot 7 (encoder 3145.00). Replaces 275.45 (D35). The encoder read 3077, 3926, 3181, 3134 and 3145 today, so it is not yet proven stable: `encoder_repeat` decides. Not flashed | user | F187  | [2026-10-23](LOG.md#session-2026-10-23-bench-summary-stage-1-step-1-to-step-7-part) |
| D38 | 2026-10-23 | `EMPTY_AIR_HOLD_MS` = **20 s** (was 1 s). The 1 s hold ended every air phase at once and did not empty the line (F188). User decision "ok for 20s". Built and native tests pass; not flashed | user | F188, S1-C step 8  | [2026-10-23](LOG.md#session-2026-10-23-bench-summary-stage-1-step-1-to-step-7-part) |
| D39 | 2026-10-23 | `purge_angle` = 276.77 from `cal` on slot 7 (encoder 3149.00; 276.77 = 3149 × 360 / 4096). Replaces 276.42 (D37). Did not fix the 3-slot offset (see D40) | user | F191  | [2026-10-23](LOG.md#session-2026-10-23-late-slot-offset-f191) |
| D40 | 2026-10-23 | `purge_angle` = **344.27** (276.77 + 67.5°, three slots). The rotor was aligned with the table but 3 slots off. Replaces 276.77 (D39). Confirmed after the reflash | user | F191  | [2026-10-23](LOG.md#session-2026-10-23-late-after-the-reflash-d39-did-not-fix-it) |
| D41 | 2026-10-23 | **Superseded by D44.** Purge: factor 5, and the spool question. `PURGE_DEAD_VOLUME_FACTOR` 3 → 5 (F110): spool off 100 → **150 mL**, spool on 12.85 L. `session` asks "spool module on the machine? (y/n)" and sets the purge for the session (`spool_installed_session`); the `TEST` and META lines print `spool=`. The factor 5 is the pick from F110 (the user asked for more than 100 mL). Built, not flashed; 172 native tests pass, Due build passes | user (factor 5 chosen in the code from F110) | F110, F130  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D42 | 2026-10-23 | **The prefield test replaces T2** and the multi-slot test: `run_sample 11` (new filter), `12` (clogged), `10` (empty tube), after the flash. Pass, then the field. Stage 1 §1 | user | stage 1 §1  | — |
| D43 | 2026-10-23 | **Clog floor 50 → 30 mL per 60 s window** (`FILL_CLOG_MIN_WINDOW_ML`). The user: 400 mL is low for a new filter; be a bit more tolerant, not too much. Slot 11 stopped at 382 mL on its 4th window (36 mL, under the 50 mL floor; windows 146, 131, 69, 36). The relative rule (`FILL_CLOG_FRACTION` 0.3) still stops a slow decline. The clogged filter (slot 12, 3 mL first window) still stops at once. Built, not flashed; 172 native tests pass, Due build passes | user (value 30 chosen in the code, to confirm) | F194, F150  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D44 | 2026-10-23 | **Purge: 500 mL without the spool, 7 L with the spool** (user). *The spool value is superseded by D50 (3 L).* Chosen by the spool answer in `session`. Replaces D41's factor 5 (the factor is no longer used for the purge). The 7 L is 10 % below the F110 rule of 3 × dead volume (7.7 L), a user choice. The 3-minute purge timeout is not checked for 7 L. Built, not flashed | user | F30, F110, F130  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D45 | 2026-10-23 | **Test logs in one folder per run, numbered per day:** `docs/logs/test-logs/<date>/<NN>_<tag>/` with `session.log`, `tests.csv`, `samples/`. A restart with the same tag on the same day reuses its folder. Built in [`tools/session_log.py`](../../tools/session_log.py) (106 tool tests pass); today's two runs moved into it. Older runs (2026-10-09 to 2026-10-19) stay flat | user | F206  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D46 | 2026-10-23 | **Purge timeout with the spool: 13 min** (`PURGE_SPOOL_MAX_RUNTIME_MS`, user-confirmed; the first build said 15 min); without the spool it stays 3 min (`PURGE_MAX_RUNTIME_MS`). The user asked 5 min; the estimate says 5 min stops at about 3 L. The estimate: slot 10 pumped 1001 mL in 97 s (sample record start_ms 1146489, end_ms 1243438), so 7 L takes about 11 min. Built, not flashed | user (13 min confirmed) | F110, F30  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D47 | 2026-10-23 | **Older test logs archived**: the flat runs of 2026-10-09 to 2026-10-19 moved to [`docs/archive/test-logs-2026-10/`](../archive/test-logs-2026-10) (files unchanged, README lists them). LOG.md references to `docs/logs/test-logs/<old name>` now point to the archive | user | D45  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |
| D48 | 2026-10-23 | **Step times logged:** each step of `run_sample` (purge, sample with its emptying, DNA shield, unlock) is timed, printed as `STEP TIMES` and written on the `TEST` line as `t_*_s`. Built, not flashed | user | —  | [2026-10-19](LOG.md#session-close-2026-10-19-what-was-learnt-and-where-it-stands) |
| D49 | 2026-10-23 | **Rebuild never refuses:** if the run folder's `samples/` already has sample files, `session_log.py --rebuild` writes the transcript and the samples into the next numbered folder of the day. Nothing is overwritten | user | F206  | [2026-10-23](LOG.md#session-2026-10-23-fourteenth-to-nineteenth-summary-of-the-afternoon) |

## Unnumbered decisions (2026-10-23 and 2026-10-19)

These were user decisions stated in the log without a number. They are kept here so that none is lost.

| Date | Decision | Why / finding |
|---|---|---|
| 2026-10-19 | One stop-criteria set for every filter run (`Filter_watch`, `fill_stop`) | S1-B and S1-C |
| 2026-10-23 | Overshoot of the pressure path accepted: 1.0 and 2.5 bar pass with 0.3–0.4 bar overshoot. 2.0 and 2.8 bar not run | Stage 1 §2 step 3 |
| 2026-10-23 | `encoder_repeat` and `rotate_all` not rerun after the reflash (the morning calibration accepted) | Recorded as the user's call, not as a test result |
| 2026-10-23 | Flowmeter confidence threshold `FLOW_TRUST_MIN_ML_PER_MIN` = 50 mL/min (was 110 by interpolation, 100 placeholder) | At about 50 mL/min the flowmeter read 32 % low (F150): the lower windows under-count |
| 2026-10-23 | Field volume 1000 mL, or less when the clogging criteria stop the run. Volume criterion is the weighed volume, flagged with its stop reason | F150, filter capacity |
| 2026-10-23 | Do not flash or commit without asking | Working rule |
| 2026-10-23 | Shield grams not measured in stage 1. `calibrate_all` not run in stage 1 | Stage 1 scope |
| 2026-10-23 | Operator command is `run_sample`, not `sample`: a typed `sample…` line after `stop` goes to the legacy Pi protocol | Operator safety |
| 2026-10-23 | `run_sample` asks name, condition and notes. Start and finish times come from the PC clock | Simpler operator flow |
| 2026-10-23 | `session` checks the date is YYYY-MM-DD | The 2026-10-23 records had a typo in the date |
| 2026-10-23 | Sterivex air drying 40 s; purge emptying keeps 20 s | F188 follow-up |
| 2026-10-23 | `flow_uncal` flag removed from sample records (flowmeter is calibrated) | Record format |
| 2026-10-23 | Flowmeter is calibrated (factor 77, wired at 3.3 V). F6 and F26 closed. The 3.3 V wiring is the user's statement; the HW3 check was not done | F6, F26 |
| 2026-10-23 | F193 dropped (no weighing of wet filters) | F193 |
| 2026-10-23 | F144 and the lack of a relief valve and E-stop accepted for now: the software cap at 3.6 bar is the protection. Not to be raised again | F143, F144 |
| 2026-10-23 | The vent may unlock the O-ring at pressure (accepted) | F192 |
| 2026-10-23 | Shield dose calibration and the sterivex air drying time belong to the helper. Production keeps the 25 s shield time until the helper sets it | F156 |
| 2026-10-23 | `cal_DNA` cap check belongs to the helper. `purge_pipes_manifold` archived (no console command) | F202 |
| 2026-10-23 | Checkpoint feature of `fill_test` removed (unused after the restart incident) | Incident 2026-10-23 |
| 2026-10-23 | Stage 1 is for the operator test only. Calibrations are not stage 1 | Stage scope |
| 2026-10-23 | No more unit tests for now (vent and cap polls checked on the bench only) | Scope |
| 2026-10-23 | Operator test layout: new filters in slots 7 and 8, clogged filters in slots 9 and 10, empty tubing in the other slots. One `run_sample` per slot, not `full_all` (it stops at the first failure) | Replaces the earlier plan (D42) |
| 2026-10-23 | Realign the manifold after the stage 1 tests (step A of stage 1 after the operator test) | F164, F186 |
| 2026-10-23 | Encoder coupling mechanical lock before the pilot (not for the prototype) | F187, F191 |
| 2026-10-23 | Unused code archived ([`docs/archive/code/`](../archive/code)), nothing deleted without a record | Clean-up |
| D50 | 2026-10-26 | **Purge with the spool: 3 L** (`PURGE_SPOOL_ML` = 3000; was 7 L, D44). Replaces the 3 × dead volume rule of §3.7 rule 1 for stage 1: dead volume 2.57 L, so 3 L is about 1.2 ×. The spool timeout stays 13 min as a safety (D46). Without the spool: 500 mL, unchanged | user | F30, F110, F130 | [2026-10-26](LOG.md#session-2026-10-26-purge-3-l-with-the-spool-and-full-power-for-the-purge) |
| D55 | 2026-10-26 | **Proposed (spec only, not decided in code): sample durations come from the field records, never from a typed value.** The host (HO6) reads the last 10 session folders at each start, takes the 90th percentile of the repeatable group per key, and adds a 5 % margin. The values are kept in `timings.json`, and each start checks the measured times against them. The mission runner warns; it does not block. The firmware keeps its own timeouts. Confirmed by the user: the 90th percentile. Placeholders until field data: 3 samples per group, ±15 % spread. Decided 2026-10-26 by the user: the stored value never decreases (the largest candidate value ever seen per key); outliers are excluded by the ±15 % repeatability rule. The window bounds what is read at each start | user (proposal, to confirm) | F208 | [2026-10-26](LOG.md#session-2026-10-26-stage-2-spec-sample-duration-from-field-logs) |
| D54 | 2026-10-26 | **`+N` dates count from the day `schedule start` is first typed**, not from the day the file is read. The dates are fixed then, and saved. The file name is free (`schedule.csv` in the docs). No "15 minutes between samples" rule is written: the user's estimate is not a rule. Supersedes the base day of D53 | user | F208 | [2026-10-26](LOG.md#session-2026-10-26-timed-sample-list-third-pass) |
| D53 | 2026-10-26 | **The timed list runs only after `schedule start`, typed and confirmed.** Loading the file runs nothing. The dates are full dates in the saved list (`+N` converted once, at the first read). A `run_sample` typed by hand during the list asks to stop the list first. Restarting with the same tag goes on from the saved list. Supersedes the 6-hour next-day rule of D52 | user | F208 | [2026-10-26](LOG.md#session-2026-10-26-timed-sample-list-second-pass) |
| D52 | 2026-10-26 | **Timed sample list: `session_log.py --schedule FILE.csv` runs each sample at its time** (`run_sample`, answers from the row). A sample due while the one before runs waits for it to finish. No automatic check that the samples fit the timeline: the operator confirms the list (cycle time not known yet). The spool depth is recorded only (spool disabled, `SPOOL_USE = false`). The real start and finish times go to `schedule.csv` and the log | user | F208 | [2026-10-26](LOG.md#session-2026-10-26-timed-sample-list-in-the-session-tool) |
| D51 | 2026-10-26 | **Exception to the 60 % water limit: the purge runs at 100 % power** (`PUMP_PURGE_MAX_POWER_PERCENT`, `step_purge` only). The 3 bar target and the pressure cap stay, so the pump saturates at 100 % for the whole purge. Every other water run keeps 60 % (D21). Needs a bench check of the manifold seal | user | F161, F162, F207 | [2026-10-26](LOG.md#session-2026-10-26-purge-3-l-with-the-spool-and-full-power-for-the-purge) |
