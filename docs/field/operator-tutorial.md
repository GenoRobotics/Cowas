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

Steps 3 and 4 again after any reset or flash.

---

## 2. One sample

1. Put a new, labelled filter in the next slot.
2. Type `run_sample` and the slot, for example `run_sample 7`.
3. Answer: sample name (no spaces), condition, notes (optional, ENTER to skip).
4. Send nothing else. The machine purges, samples, adds the shield and unlocks by itself. Time: about 5–8 min without the spool, about 20 min with it (the purge alone is about 11 min). Someone stays at the switch.
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

1. Type `:quit` + ENTER. The log closes.
2. Copy the run folder `docs\logs\test-logs\<date>\<NN>_<tag>\` (it holds `session.log`, `tests.csv` and `samples\`). Add the paper sheet.
3. Send them to the engineer.

---

## 4. Optional: a timed list of samples

Use this to run several samples at set times, for example over a night. The tool runs them, you do not type each `run_sample`. If you do not use it, skip this section.

**Write the list.** Copy `tools\schedule_example.csv`, keep its first line, and fill one row per sample. Save it as CSV in Excel (comma or semicolon, both work). Any spreadsheet or text editor works; a coloured CSV viewer is optional.

| Column | What to write | Example |
|---|---|---|
| `date` | `2026-10-27`, or `+0` for the day you start the schedule, `+1` for the next day, `+2`… | `+1` |
| `time` | clock time, 24 hours, `HH:MM` | `00:30` |
| `name` | sample name. Empty = the date and time | `lake A` |
| `condition` | `control`, `blank`, `filter A`… Empty = `not given` | `control` |
| `spool` | `on` or `off`. Empty = `off` | `on` |
| `depth_m` | depth in metres, **only** when spool is `on` | `2.5` |

- Put the rows in any order. The tool sorts them.
- Keep the file in the project folder. Its name does not matter; `schedule.csv` is a good one.

**Start the tool with the list:**
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
