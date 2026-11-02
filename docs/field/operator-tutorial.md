# CoWaS operator tutorial

**Safety:** someone stays at the 24 V switch for every sample. If you see `SAFETY`, cut the 24 V switch and call the engineer. Never type `stop`.

Open a terminal in the **project folder** (the one with `tools` and `docs`). Type the commands in the log window (section 1).

**The day:** 1 start of the day → 2 samples (2a one sample by hand, or 2b several from a schedule; 2c the spool) → 3 if something goes wrong → 4 end of the day. Section 5 is the ethanol cleaning of the tubing, done at the end of the day (or the start, if the last cleaning was missed).

---

## 1. Start of the day

1. Plug in the Due. Close other programs that use its COM port (Arduino IDE, PlatformIO monitor). The port is in Device Manager (COM6 here).
2. Start the log and keep its window open all day:
   ```
   tools\session_log.cmd --port COM6 --tag field-<site>-<date>
   ```
   Wait for `=== Test menu READY`.
3. `actuator_home`. Check: `state=UNLOCKED`.
4. `session`, and answer:
   - site, date (for example `2026-10-23`), experiment name, operator (optional);
   - **spool module on the machine? (y/n)**. If `y`: **the deepest depth of this session, in m** (ENTER = 40 m, the full tube). Every sample at depth starts from this depth, so give a depth the tube can reach without touching the ground.
5. **Tubing:** clean it with ethanol if it was not done at the end of the last day (section 5). Filters out.
6. `fill_DNA`: loads the DNA shield until the check valve.
7. **Spool (only if `session` said `y`):**
   - `spool_height <cm>`: distance from the top switch to the water surface. Type what you measured.
   - `spool_status`: `origin switch PRESSED` when the intake is at the top, `released` when it is not.
   - `spool_goto length=1`: `PASS`, and a tape from the origin mark reads **0.98 m ± 5 cm**.
   - `spool_home drain=n`: rolls the tube back up. Wait for it.

Redo steps 3, 4 and 7 after any reset or flash. A reset is: the Due unplugged, reset, or the serial port closed and reopened. After a reset the spool does not know where the tube is: its next move first rolls up slowly to the top switch (a few minutes).

---

## 2. Samples

Two ways to run samples. Both use the same procedure and the same paper sheet: **2a** one sample, typed by hand; **2b** several samples at set times, from a schedule file. **2c** explains the spool (depth) for both.

### 2a. One sample, by hand

1. Put a new, labelled filter in the next slot.
2. Type `run_sample` and the slot, for example `run_sample 7`. With the spool, add the intake depth in metres: `run_sample 7 depth=2.5`.
3. Answer the questions: sample name (no spaces), condition, notes (ENTER to skip).
4. Send nothing else. The machine purges, samples, adds the shield and unlocks by itself. Someone stays at the switch. Time: about 5–8 min without the spool; about 20 min with it (the purge alone is about 11 min).
5. Read the last line, `TEST|run_sample|...`. Check: `PASS`.
6. On the paper sheet, write: slot, filter ID, sample name, `sample_end=`, `trusted_ml=`, `sampled_ml=`.

- Use `trusted_ml`. `sampled_ml` is the flowmeter total, which is not reliable at slow flow.
- With the spool, the machine ends by asking `spool: next sample depth`. Type ENTER (the tube rolls up and drains), or the depth of the sample that starts straight after (the tube stays full and goes there). The schedule answers this by itself.
- To stop the sample now: `x` + ENTER. To skip only the current pump step: `s` + ENTER.

### 2b. Several samples, from a schedule

The tool runs the samples at their times. You do not type each `run_sample`.

**Write the list.** Copy `tools\schedule_example.csv` into the project folder and rename it, for example `schedule.csv`. Keep the first line (the column names) and fill one row per sample. Save as CSV (comma or semicolon, both work).

| Column | What to write | Example |
|---|---|---|
| `date` | `2026-10-27` or `27/10/2026`; `+0` = the day you start the schedule, `+1` the next day… | `+1` |
| `time` | clock time of the **PC**, 24 h, `HH:MM`. Required | `00:30` |
| `name` | sample name. A space becomes `_` (`lake A` → `lake_A`). Empty = the date and time | `lake A` |
| `condition` | `control`, `blank`, `filter A`… A space becomes `_`. Empty = `not_given` | `filter A` |
| `spool` | `on` or `off`. Empty = `off` | `on` |
| `depth_m` | depth in metres, **only** when `spool` is `on` (0 to 40) | `2.5` |

**How the rows run:**
- Rows are sorted by date and time. Rows at the **same** time run the **deepest first** (spool off last), then in file order.
- A row due while the previous sample still runs waits for it. Nothing overlaps. Allow about 5–8 min per sample without the spool, about 20 min with it.
- A row whose time has passed runs as soon as the sample before it has finished. The table marks it.
- A wrong row (bad date, time, spool or depth) stops the whole file. The tool shows the line number. Fix the file and start the tool again.
- Times are the PC clock. Check it before you start.

**Start the tool with the list** (path relative to the project folder):
```
tools\session_log.cmd --port COM6 --tag <run name> --schedule schedule.csv
```
The tool shows your list. **Nothing runs yet.**

1. Check the list on the screen (times, names, spool).
2. Type `session` and answer its questions, with the spool answer and the deepest depth (section 1, step 4). Do this before the first sample.
3. Type `schedule start`. The first thing you see, before the list, are the warnings: the samples that need your answer (a depth deeper than the session, or a tube total over 40 m). Then the list with the full dates. Then:
   - **Depth conflict:** if a spool sample is deeper than the session's deepest point, the tool prints `CONFLICT` and lists those samples. Answer `y` to run them anyway (each goes to its depth), or `n` to stop. On `n`, type `session` with the deeper depth, then `schedule start` again.
   - Without a spool depth in the session, the tool does not start the spool samples (`BLOCKED`): type `session` with the spool answer `y` first.
   - Answer `y` to start the list.

**While it runs:**
- `schedule` shows each sample: `[waiting]`, `[running]`, `[PASS]`, `[FAIL]`, `[refused]`.
- `schedule stop` stops the list after the sample now on the device. `schedule start` runs the samples left.
- Type nothing else during a sample, except `s` or `x`.
- To run one sample by hand, type `run_sample`. The tool asks *stop the schedule?*: answer `y`. When your sample has ended, the tool says to type `schedule start` to go on.

**If the tool was closed** (PC restart, or work on the code): start it again with the **same `--tag`** and without `--schedule`. The list is loaded with what already ran. Type `schedule start` and `y`. A sample that was running when the tool closed is **not** run again: check it on the device, and run it by hand if needed.

**Afterwards:** the run folder holds `schedule.csv` (the real start and finish of each sample) and `session.log`.

### 2c. The spool (depth)

With `session` answered `y`, a sample with a depth goes like this:

1. **Fill.** If the tube is dry (after a roll-up or a reset), the spool unrolls to the session's deepest point with the air valve open, so the tube fills with lake water.
2. **Flow check.** The purge slot is primed until the flowmeter counts 100 mL (about 2 min at the slowest confident flow). No flow in 4 min: the sample stops, nothing more is pumped.
3. **Roll up to the depth.** The pump is off and the deployment valve is shut, so the water stays in the tube. The spool moves to the sample depth.
4. **Purge, sample, shield** as in 2a.
5. **Roll up or keep.** A sample at the **same timepoint** that follows: the tube stays full and goes straight to its depth, with no emptying in between (the next purge flushes it). Otherwise the tube rolls up with the valves open.
6. **Empty.** After the last sample of a timepoint, and after a failed sample, the spool is rolled up with the valves open, and the spool tubing is emptied: air is pulled through the purge slot until the line is empty. The `TEST` line shows `spool_emptied=y`. Water left in the tube would deposit organic matter before the next timepoint, so it is never left there. Only a sample at the **same** timepoint keeps the tube full (step 5). After any emptying, the next sample fills the tube again: it dives to the session's deepest point, then rises to its depth.

The deployment valve is shut whenever the tube is out and nothing pumps through it (manifold turns, air drying, DNA shield). That keeps the water in the tube.

- **The 40 m total:** the tube (depth + spool height) never goes beyond 40 m, whatever the height. A deeper depth is cut at the limit (40 m minus the height) with a warning, and the machine asks `run_sample: solve it or proceed?`. `y` goes to the cut depth; anything else stops with nothing started. The same question comes for a depth deeper than the session's deepest point. From a schedule, `schedule start` lists these rows as `CONFLICT` first and answers the machine `y` only for them (2b).
- Every spool move logs its speed (`cm/s`). Send the `spool` lines of the first run with the run folder: the speed is measured for the first time on the bench.

---

## 3. If something goes wrong

| What you see | Meaning | Do |
|---|---|---|
| `sample_end=volume` | 1000 mL reached | Next sample |
| `sample_end=clogged`, `stalled`, `pressure`, `timeout` | Filter stopped early (normal) | Note it. Next sample |
| `Vent: ... NOT released` | Line still above 0.3 bar after the vent. The run went on | Note it on the sheet. Tell the engineer |
| `ERROR \| step_dive: the spool did not get to the depth` | A spool move stopped: `stalled` = encoder, motor or jam; `wrong way` = encoder wiring; `timeout` = slower than 1 cm/s | The sample is `FAIL` (`failed_at=dive`) but it is still rolled up and emptied (`spool_end=rolled_up_after_failure`, `spool_emptied=y`). Only if the log says `spool_end=FAILED`: free the tube by hand, then `spool_home`. The filter is still good. Note it, call the engineer |
| `ERROR \| spool fill: no flow` | The flowmeter counted no 100 mL in 4 min | Check the intake and the tube, then `spool_home` |
| `ERROR \| run_sample: the spool tubing was NOT emptied` | The emptying did not reach air (`spool_empty_end=` says why). The sample is `FAIL` (`failed_at=empty`) | Check the tubing at the purge outlet, then `spool_home`. Tell the engineer |
| `SAFETY`, or pressure at 3.6 bar | Stop | Cut the 24 V switch. Call the engineer |

`x` + ENTER aborts a command (pump off, valves closed, O-ring locked). After an `x` the spool is still rolled up (a move, no pump), but the emptying is skipped: the log says `spool_emptied=NO`. Then run `spool_home` (it empties).

---

## 4. End of the day

1. **Spool (if on):** the last sample of the day empties it (2c, step 6). Check with `spool_status`: `origin switch PRESSED` means it is rolled up. If it is not (after an `x`, or a roll up that failed), run `spool_home`: it rolls the tube up and empties it. Do it before the ethanol cleaning.
2. **Tubing:** clean it with ethanol (section 5), with every filter out.
3. Type `:quit` + ENTER. The log closes.
4. Copy the run folder `docs\logs\test-logs\<date>\<NN>_<tag>\` (it holds `session.log`, `tests.csv`, `samples\`, and `schedule.csv` if you used a schedule). Add the paper sheet. Send them to the engineer.

---

## 5. Cleaning the tubing with ethanol

**When:** at the end of the day (section 4). At the start of the day only if the last cleaning was missed. **No filter may be in the manifold:** the pump runs at **100 %** here, which a filter would not survive.

1. **Take every filter out.** Put the inlet and the outlet tubes in 70 % ethanol (they stay there), outlets draining to a waste container. Ethanol needed: 0.1 L per slot to clean, plus 3 L with the spool (0.1 L without).
2. `actuator_home` if the board was reset since the last sample (an `UNKNOWN` lock state is refused). `session` first, so the spool answer is right.
3. Type `ethanol_purge`.
   - The machine asks whether some slots were not used. To skip them, answer `y` and type their numbers (`3,7,12`, `3 7 12`, `2-5,9`). Or give them at once: `ethanol_purge skip=3,7,12`.
   - Then it asks **Filters removed, inlet and outlet in 70 % ethanol?** Answer `y` only when every filter is out.
   - Check the printed slot list and the ethanol quantity, then answer `y` to start.
4. The machine flushes the purge slot (prime), then 0.1 L through each slot, then blows air 30 s through each slot and 1 min at the purge slot. Roughly 1–2 min per slot. Stay with it. `x` + ENTER aborts (pump off, valves closed, O-ring locked).
5. Check the last line: `Ethanol cleaning complete: ... 0 warning(s)`. A `WARNING` line (skipped run, air that never reached air level) means a slot may still be wet: look at its outlet, then rerun with `skip=` listing the slots that are fine.

The intake tube before the pump is not emptied (the air comes in by the air valve). The purge at the start of each sample flushes it.

---

## 6. Commands used

| Command | Does |
|---|---|
| `session` | site, date, experiment, operator; spool y/n and the deepest depth |
| `actuator_home` | finds the manifold; needed after a reset |
| `fill_DNA` | loads the DNA shield |
| `run_sample <slot> [depth=m]` | one sample (2a) |
| `schedule`, `schedule start`, `schedule stop` | the list (2b) |
| `ethanol_purge [skip=…]` | cleaning of the tubing (section 5) |
| `spool_status`, `spool_height <cm>`, `spool_goto <m>` / `length=<m>`, `spool_home [drain=n]`, `spool_calibrate` | the spool (sections 1 and 2c) |
| `x` + ENTER / `s` + ENTER | stop the command / skip the current pump step |
| `:quit` | closes the log |
