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
5. Type `fill_DNA` to load the DNA shield until the check valve.

Steps 3 and 4 again after any reset or flash. **Unplug and plug the USB cable or closing and reopenng the serial port** is also a reset.

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
