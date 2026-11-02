# CoWaS operator tutorial

**Safety:** someone stays at the 24 V switch for every sample. If you see `SAFETY`, cut the 24 V switch and call the engineer. Never type `stop`.

Open a terminal in the **project folder** (the one with `tools` and `docs`).

---

## 1. Start of the day

1. Plug in the Due. Close other programs that use its COM port (Arduino IDE, PlatformIO monitor). The COM port is in Device Manager (COM6 here).
2. Start the log and keep the window open all day:
   ```
   tools\session_log.cmd --port COM6 --tag field-<site>-<date>
   ```
   Wait for `=== Test menu READY`.
3. Type `actuator_home`. Check: `state=UNLOCKED`.
4. Type `session` and answer:
   - site
   - date (for example `2026-10-23`)
   - experiment name
   - operator
   - **spool module on the machine? (y/n)**
5. **Clean the tubing with ethanol** (section 3b), filters still out. Do it before loading the shield and before the first sample.
6. Type `fill_DNA` to load the DNA shield until the check valve.
7. **Spool on the machine:** type `spool_height <cm>` (cm from the spool's top switch to the water surface), then `spool_status`. Check: `origin switch PRESSED` when the intake is at the top, `released` when it is not. Details in section 5.

Steps 3 and 4 (and 7 with the spool) again after any reset or flash. After a reset the spool does not know where the tube is: its next move first rolls up to the top switch (slow, up to a few minutes). **Unplug and plug the USB cable or closing and reopenng the serial port** is also a reset.

---

## 2. One sample

1. Put a new, labelled filter in the next slot.
2. Type `run_sample` and the slot, for example `run_sample 7`. With the spool, add the intake depth in metres: `run_sample 7 depth=2.5`.
3. Answer: sample name (no spaces), condition, notes (optional, ENTER to skip).
4. Send nothing else. The machine purges, samples, adds the shield and unlocks by itself. With the spool it first unrolls to the depth, and rolls up after the shield. Time: about 5–8 min without the spool, about 20 min with it plus the spool moves (the purge alone is about 11 min). Someone stays at the switch.
   With `depth=`, the machine ends by asking `spool: next sample depth`. Type ENTER (tube rolls up and drains), or the depth of a sample that starts right after (tube stays out and goes straight there). The schedule tool answers this by itself.
5. Read the last line, `TEST|run_sample|...`. Check: `PASS`.

| `sample_end=` | Meaning | Do |
|---|---|---|
| `volume` | 1000 mL reached | Next sample |
| `clogged`, `stalled`, `pressure`, `timeout` | Filter stopped early (normal) | Note it. Next sample |
| `Vent: ... NOT released` | Line still above 0.3 bar after the vent. The run went on | Note it on the sheet. Tell the engineer |
| `SAFETY`, or pressure at 3.6 bar | Stop | Cut the 24 V switch. Call the engineer |

6. On the paper sheet, write: slot, filter ID, sample name, `sample_end=`, `trusted_ml=`, `sampled_ml=`.

- To stop a sample now: `x` + ENTER.
- To skip only the current pump step: `s` + ENTER.
- Use `trusted_ml`. `sampled_ml` is the flowmeter total and is not reliable at slow flow.

---

## 3. End of the day

1. Clean the tubing with ethanol again (section 3b), after the last filter is out and before the log is closed.
2. Type `:quit` + ENTER. The log closes.
3. Copy the run folder `docs\logs\test-logs\<date>\<NN>_<tag>\` (it holds `session.log`, `tests.csv` and `samples\`). Add the paper sheet.
4. Send them to the engineer.

### 3b. Ethanol cleaning of the tubing (start and end of the day)

**No filter may be in the manifold.** The pump runs at **100 %** here (an exception, like the purge), which a filter would not survive.

1. Take every filter out. Put the inlet and the outlet tubes in 70 % ethanol (they stay there), outlets draining to a waste container. Ethanol needed: 0.5 L per slot to clean, plus 3 L with the spool (0.5 L without).
2. `actuator_home` if the board was reset since the last sample (an `UNKNOWN` lock state is refused). `session` first, so the spool answer is right.
3. Type `ethanol_purge`. To skip slots you did not use, answer `y` and type their numbers (`3,7,12`, `3 7 12`, `2-5,9`), or give them at once: `ethanol_purge skip=3,7,12`. Check the printed slot list and ethanol quantity, then answer `y`.
4. The machine flushes the purge slot (prime), then 0.5 L through each slot, then blows air 30 s through each slot and 1 min at the purge slot. Roughly 1–2 min per slot. Stay with it. `x` + ENTER aborts (pump off, valves closed, O-ring locked).
5. Check the last line: `Ethanol cleaning complete: ... 0 warning(s)`. A `WARNING` line (skipped run, air that never reached air level) means a slot may still be wet: look at its outlet and rerun with `skip=` listing the slots that are fine.

The intake tube before the pump is not emptied (the air comes in by the air valve). The purge at the start of each `run_sample` flushes it before the sample.

---

## 4. Optional: a timed list of samples

Use this to run several samples at set times, for example over a night. The tool runs them, you do not type each `run_sample`. If you do not use it, skip this section.

**Write the list.** Copy `tools\schedule_example.csv` into the **project folder** (the one you open the terminal in) and rename it, for example `schedule.csv`. Keep its first line (the column names) and fill one row per sample. Save it as CSV in Excel (comma or semicolon, both work). Any spreadsheet or text editor works.

| Column | What to write | Example |
|---|---|---|
| `date` | `2026-10-27` (or `27/10/2026`), or `+0` for the day you start the schedule, `+1` for the next day, `+2`… | `+1` |
| `time` | clock time of the **PC**, 24 hours, `HH:MM`. Required | `00:30` |
| `name` | sample name, no spaces: a space is changed to `_` by the tool. Empty = the date and time | `lake A` becomes `lake_A` |
| `condition` | `control`, `blank`, `filter A`… no spaces: a space is changed to `_` by the tool. Empty = `not_given` | `filter A` becomes `filter_A` |
| `spool` | `on` or `off`. Empty = `off` | `on` |
| `depth_m` | depth in metres, **only** when spool is `on` | `2.5` |

- Put the rows in any order. The tool sorts them by date and time. Rows with the **same** time keep their order in the file and run one after the other.
- Times are the PC clock: check it before you start.
- A sample needs room: about 5–8 min without the spool, about 20 min with it. A row that is due while the previous sample is still running waits for it. Nothing overlaps.
- A row whose time has already passed runs as soon as the sample before has finished. The table marks it.
- A wrong row (bad date, time, spool or depth) stops the whole file: the tool does not read it and shows the line number. Fix the file, then start the tool again with the same command.
- **Spool `on`:** the tool sends `run_sample <slot> depth=<depth_m>`. The depth is 0 to 40 m. The session must have the spool answer `y`, and `spool_height` must be set (section 5). Spool `off` does not move the spool: the intake stays where it is.
- **Next sample waiting:** when a sample ends, the tool looks at the next row. If it has the spool on and is due within 10 minutes, the tube stays out and goes straight to the new depth. Otherwise the tube rolls up. After `schedule stop`, it always rolls up.
**Start the tool with the list** (the path is relative to the project folder):
```
tools\session_log.cmd --port COM6 --tag <run name> --schedule schedule.csv
```
The tool shows your list. **Nothing runs yet.**

1. Check the list on the screen (times, names, spool).
2. Type `session` and answer its questions. Do it before the first sample.
3. Type `schedule start`. The tool shows the list with the full dates (`+N` becomes a date now). Answer `y` to run it.

**While it runs:**
- `schedule` shows each sample: `[waiting]`, `[running]`, `[PASS]`, `[FAIL]`, `[refused]`.
- `schedule stop` stops the list after the sample on the device now. `schedule start` runs the samples left.
- Do not type anything else during a sample, except `s` or `x` (see section 2).
- To run one sample by hand, type `run_sample`. The tool asks *stop the schedule?*. Answer `y`. When your sample has ended, the tool says to type `schedule start` to go on.

**If the tool was closed** (to work on the code, or after a PC restart): start it again with the **same `--tag`** and without `--schedule`. The list is loaded from the run folder, with what already ran. Type `schedule start` and `y` to run the samples left. A sample that was running when the tool closed is **not** run again: check it on the device, and run it by hand if needed.

**Afterwards:** the run folder has `schedule.csv` with the real start and finish of each sample, and `session.log`. Send them with the other files (section 3).

---

## 5. The spool (depth)

**One sample at depth.** `run_sample 7 depth=2.5` does: unroll to 2.5 m (the air valve is open so the tube fills) → purge 3 L → sample → shield → roll up with the air and water valves open (the tube empties by itself as it leaves the water) → unlock. A move stops by itself on a stall, a wrong-way encoder, 15 min, an end switch, or `x`. If the unroll fails, nothing is pumped.

**Several samples.** Use the schedule (section 4) with `spool` and `depth_m`. Same-time rows run one after the other, in file order.

**Minimum check before the first sample** (3 min, intake at the top, tube not in water):
1. `spool_status`. Check: `origin switch PRESSED` with the intake at the top.
2. `spool_height <cm>`: distance from the top switch to the water surface. Depth = tube out − height.
3. `spool_goto length=1`. Check: `PASS`, and a tape says **1.05 m ± 8 cm** from the origin mark (the code reads about 5 % short near the top because the outer turns of the roll are wider; expected). Then `spool_home drain=n`: it returns by itself.
   If the tape is outside that range: `spool_calibrate quick`, type the tape reading at the 2 m stop, answer `y`. Do 3 again.

**Calibrate the length** (once, dry, on the bench, 15 m of free space and a tape). It covers 0 to 15 m. Deeper than 15 m the tube comes out shorter than commanded (roughly 5 % short at 20 m and 14 % at 40 m, from the layer model): ask for a bit more depth, or check one deep point with a tape.
1. Type `spool_calibrate`. The tube goes to 2 m, 5 m, 10 m, 15 m.
2. At each stop, measure the tube out from the origin mark and type it in metres (`4.93`).
3. The tool prints the drum diameter. `PASS` needs every stop within 6 % of one straight line (the layers of tube on the drum make about 5 % normal). Answer `y` to use it until the next reset.
4. To keep it: put `SPOOL_DRUM_DIAMETER_CM = <value>` in `include/Settings.h` and flash.
5. Check: `spool_goto length=5`, measure (within 5 % of 5 m), `spool_home drain=n`.

**If a move fails** (`ERROR | ... did not get ...`): read the reason. `stalled`: encoder, motor or jam. `wrong way`: encoder wiring. Free the tube by hand, then `spool_home drain=n`. A failed roll-up ends the sample as `FAIL` (`failed_at=rewind`); the filter is still good. Note it and call the engineer.

**End of the day:** `spool_home`. It also empties the tube. Do it after any `x`: an aborted sample leaves the tube out.

| Command | Does |
|---|---|
| `spool_status` | position, height, diameter, switches |
| `spool_height <cm>` | top switch to water surface (RAM only) |
| `spool_goto <m>` / `spool_goto length=<m>` | go to a depth / a tube length |
| `spool_home [drain=n]` | roll up; `drain=n` = no valves, no manifold move |
| `spool_calibrate` | dry calibration at 2, 5, 10, 15 m |
