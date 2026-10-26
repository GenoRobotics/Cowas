# CoWaS roadmap, stage 1: the prefield test

**Goal:** the operator runs samples, one command per sample, and the records come back.
**Status (2026-10-23):** the prefield tests have passed. Stage 1 is done apart from the items below. Done steps are in [LOG.md](../logs/LOG.md).

---

## Left to do

1. **Felix: clean the tubing** before his acquisition campaign.
2. **Felix: DNA shield dose and air timing** ([for-felix.md](../for-felix.md)).
3. **Commit** the stage 1 work.
4. **Weigh one spool-on purge** to check that the 3 L purge covers the 2.54 L spool volume (the spool volume is still geometry, F130; the purge is 3 L since 2026-10-26, D50). Weigh the outlet water, and check the purge with `pressure_max` on the record (F207).
5. **Field.** Operator guide: [field/operator-tutorial.md](../field/operator-tutorial.md).
6. **Timed sample list test** (two real samples, before the night run): [section below](#quick-test-the-timed-sample-list-field).

---

## Quick test: the timed sample list (field)

**Why:** the night run uses the timed list ([operator tutorial, section 4](../field/operator-tutorial.md#4-optional-a-timed-list-of-samples)). This checks it once on the field, with two real samples of the plan (no extra samples). For this test the two samples are 15 minutes apart.

**Before:** the Due is on and `session` is not typed yet. The filters of the two samples are on the manifold. Look at the PC clock.

**Steps:**
1. In the project folder, make `schedule.csv` (Excel, save as CSV). `+0` is the day you type `schedule start`. Sample A is at a time about 5 minutes from now; sample B is 15 minutes after A:
   ```
   date,time,name,condition,spool,depth_m
   +0,14:05,<sample A name>,<condition>,<on or off>,<depth if on>
   +0,14:20,<sample B name>,<condition>,<on or off>,<depth if on>
   ```
2. Start the tool: `tools\session_log.cmd --port COM6 --tag schedule-test --schedule schedule.csv`. Answer the loaded questions as usual.
3. Check the list on the screen. Type `session` and answer its questions.
4. Type `schedule start`, then `y`, **at least 2 minutes before sample A's time**.
5. Wait. Type nothing except `s` or `x`. When sample B has finished, type `schedule`, then `:quit`.
6. Start the tool again with the same `--tag` and without `--schedule`. Type `schedule`.

**Pass (all four):**
- Sample A starts within 10 s of its time (`schedule.csv`: `started_pc` against `date` and `time`).
- Sample B does not start before sample A has finished (`started_pc` of B is not before `finished_pc` of A). If A has finished before 14:20, B starts within 10 s of 14:20.
- Both rows have status `PASS` or `FAIL`. Not `refused`, not `waiting`.
- After step 6 the list shows both samples with the same status (the restart works).

**If it fails:** write the times and the status in [LOG.md](../logs/LOG.md). Type `schedule stop`, run the rest by hand with `run_sample` (the tool still records them), and send the run folder.

---

## Limits to know in the field

- **Stop rules:** a `SAFETY` line, pressure above 3.6 bar, or a seal leak stops the run. Cut the 24 V switch, then send the log. Someone stays at the 24 V switch for every sample.
- **The vent never stops a sample.** If the line stays above 0.3 bar after the vent, the log says `NOT released` and the run goes on.
- **The flowmeter reads almost nothing at slow flow.** Use `trusted_ml`, not `sampled_ml`.
- **The manifold sometimes does not turn** (F203, open). Check the cable connector first. Details in stage 2 §7b.
- **Not in the record:** filter ID, weights and shield amount. These go on the paper sheet. The shield is timed, not measured.
- **Use the session tool for every sample.** A `run_sample` typed in `pio device monitor` skips its record, the loading order and the timed list (stage 2 §7b).
- **The records need the session tool:** `tools\session_log.cmd` ([software/session-log.md](../software/session-log.md)).
