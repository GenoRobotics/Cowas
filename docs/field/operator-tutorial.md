# CoWaS operator tutorial

**Safety.** Someone stays at the 24 V switch for every sample. On `SAFETY`, or a pressure of 3.6 bar: cut the 24 V switch and call the engineer. Never type `stop`.

**The day:** 1 Start → 2 Samples → 3 End of the day (with the ethanol cleaning, section 4). Problems: Appendix A.

Type every command in the **log window** (section 1), never in PowerShell, and never paste: read the line before ENTER. `x` + ENTER stops the running command (pump off, valves closed, O-ring locked). `s` + ENTER skips only the current pump step.

---

## 0. Before you leave

- **20 µm prefilter first in the inlet line, then the Sterivex filters. Nothing between them.** On Lexplore (bacterial work) the prefilter is **changed at every ethanol cleaning**: fit a new one before the next filter set (a plastic spacer there clogged the prefilter on 5 Nov: flow fell to a quarter, dives and primes failed). The first purge of the day should give about 450 mL/min.
- Filters labelled and in date (check the expiry date), the paper sheet ready (slot, filter ID and batch, sample name, planned time).
- Ethanol (70 %): 0.1 L per slot plus 3 L with the spool. MilliQ for blanks: 4 to 5 L.
- Laptop on AC, USB cable locked, Windows updates paused.
- Spool: tube, drum, level-wind guide and top switch checked by hand (nothing wooden or loose).

---

## 1. Start of the day

1. Plug in the Due. Close every program that uses its COM port (Arduino IDE, PlatformIO monitor). The port is in Device Manager (COM6 here).
2. In a terminal in the **project folder**:
   ```
   tools\session_log.cmd --port COM6 --tag <site>-<date>
   ```
   Keep the window open all day. Wait for `=== Test menu READY`.
3. The tool asks **which samples are AVAILABLE** (the filters you loaded): `all`, `1-14`, `4-7,9,12-14`, or ENTER for no change. Read the answer back and confirm. Slots outside the list are never used.
4. `actuator_home`. Check `state=UNLOCKED`. (Needs the manifold within ±3° of a slot.)
5. `session`: site, date, experiment, operator, **spool module (y/n)**, and with `y` the **deepest depth of the session** in m (ENTER = 40 m; use a depth the tube reaches without touching the ground).
6. Tubing cleaned with ethanol (section 4) if the last cleaning was missed, then filters in.
7. `fill_DNA`: loads the DNA shield up to the check valve.
8. **Spool only:** `spool_height <cm>` (intake at the top to the water surface; how to measure it: Appendix B), then `spool_status` (origin switch `PRESSED`), `spool_home drain=n`.

**Redo steps 4, 5 and 8 after any reset** (Due unplugged or reset, port closed and reopened). A reset forgets the lock state, the spool height and the tube position; the spool's next move first rolls up to the top (a few minutes, up to 30).

---

## 2. Samples

| Mode | Use | Time per sample |
|---|---|---|
| 2.1 by hand | one sample | about 20 min (about 30 with the spool: dive, 3 L purge, emptying) |
| 2.2 schedule | several samples at set times | same, no overlap |

### 2.1 One sample

1. New labelled filter in the next slot.
2. `run_sample <slot>` (with the spool: `run_sample 7 depth=2.5`, depth in m). Answer: name (no spaces), condition, notes (ENTER to skip).
3. Send nothing else. The machine purges, samples, adds the shield and unlocks by itself.
4. Last line `TEST|run_sample|...`: check `PASS`. On the sheet: slot, filter ID, name, `sample_end=`, `trusted_ml=`.

**Use `trusted_ml`**, not `sampled_ml` (the flowmeter total is unreliable at low flow).
`sample_end=volume` is a full 1000 mL; `clogged`, `stalled`, `pressure`, `timeout` mean the filter stopped early. That is normal on turbid water: note it and go on.

### 2.2 Schedule

**The file.** Copy `tools\schedule_example.csv` to `schedule.csv`; keep the header line, one row per sample, CSV with comma or semicolon.

| Column | Value |
|---|---|
| `date` | `2026-11-06`, `06/11/2026`, or `+0` (today), `+1` (tomorrow). **Use full dates for runs past midnight**: `+N` counts from the day you type `schedule start` |
| `time` | PC clock, 24 h `HH:MM` (required) |
| `name` | spaces become `_`; empty = date and time |
| `condition` | `control`, `blank`, ...; empty = `not_given` |
| `spool` | `on` / `off` (empty = off) |
| `depth_m` | 0 to 40, only with `spool on` |
| `force_slot` | empty = next available slot in the loading order; 1 to 14 = that slot |

Rows run in date/time order, same time = file order. A row never starts while the previous sample runs. **A row whose time has already passed runs at once**: delete rows you do not want. One wrong row stops the whole file (the tool gives the line number).

**Run it**
```
tools\session_log.cmd --port COM6 --tag <run name> --schedule schedule.csv
```
1. Answer the AVAILABLE question. Check the printed list (times, names, spool, slots).
2. `session` (spool answer and deepest depth).
3. `schedule start`. Warnings come first: `CONFLICT` (a depth deeper than the session's, or a tube total over 40 m) and `BLOCKED` (spool rows without a spool session: run `session` with `y`). `y` accepts, `n` stops.

**While it runs:** type nothing except `s` / `x`. `schedule` shows `[waiting] [running] [PASS] [FAIL] [refused]`; `schedule stop` stops after the current sample. The reminder `still waiting ... no TEST line yet` every 10 min is normal.

**A failure before the filter was loaded** (`failed_at=dive` or `purge`): the same row is retried on the same filter, twice at most; the filter stays free.
**The list stops by itself** (`schedule STOPPED` and the reason) after an `x`, a spool left out (`spool_end=FAILED`) or a refusal. Fix the cause, then see Appendix C.

**Manual sample during a schedule:** `run_sample`, answer `y` to *stop the schedule?*; afterwards `schedule start`, `y`.

**Tool closed (PC restart):** start again with the **same `--tag`** and no `--schedule`, `schedule start`, `y`. The sample that was running is not repeated: check it on the device.

### 2.3 With the spool (depth)

Per sample the machine: dives (a dry tube fills with lake water), primes the purge slot (flow check, 100 mL), rises to the depth, purges 3 L, samples, adds the shield, rolls up and **empties the tube** (`spool_emptied=y`). Nothing stays out between samples. The tube total (depth + height) never exceeds 40 m: a deeper depth is cut with a warning and `solve it or proceed?` (`y` = go to the cut depth).

**After each sample at depth, read the `Spool move:` lines.** The rise must end at the depth plus the height (20 m, 90 cm: about 2090 cm). `length_cm= 0` or anything unexpected: `schedule stop` and call the engineer.

If the prime finds no flow it retries (up to 3 tries, 10 s apart). Three failures stop with `no flow at the purge slot`: check prefilter, tube and intake, then `spool_home`.

### 2.4 Blank (negative control)

A blank gets the same 3 L purge as a spool sample but **the spool does not move**: tip of the intake in milliQ by hand.

1. Tip in the milliQ, **kept there until the water runs out** (the flow stops changing). Write the time it leaves on the sheet. At least 4 L, ideally 5.
2. After a reset: `spool_height <cm>`. `session` with spool `y` (3 L purge).
3. In a schedule: `schedule stop` first, wait for the current sample.
4. `run_sample <slot>` with **no `depth=`** (never `depth=0`: it still dives). Name e.g. `BLANK`, condition `blank`. Give the slot explicitly unless it is the first free one.
5. Expect `WARNING | run_sample: ... no depth= was given: it is NOT moved`, then `PASS` with `purge_ml=3000`. A blank primes more slowly than a sample: do not stop it.
6. Take the tip out of the milliQ, then `schedule start`, `y`. The next scheduled sample rolls the tube up first; that is expected.

Without the spool, a blank is a plain `run_sample` with condition `blank` (500 mL purge).

---

## 3. End of the day

1. `python tools\run_report.py <run folder>`: pauses, purge flow, prime tries, spool warnings, errors. Read-only.
2. **Spool:** `spool_status` must read origin `PRESSED` (rolled up). If not, `spool_home` (rolls up and empties). Never leave the tube out.
3. Ethanol cleaning (section 4), every filter out.
4. `:quit` + ENTER.
5. When the filters are out, check that each Sterivex is nearly full of DNA Shield (about 2.2 mL); cap both ends, bag and label them. A visibly emptier filter: note it on the sheet.
6. Send the run folder `docs\logs\test-logs\<date>\<NN>_<tag>\` (`session.log`, `tests.csv`, `samples\`, `schedule.csv`) and the paper sheet to the engineer.

---

## 4. Ethanol cleaning

At the end of the day, or at the start if the last cleaning was missed. **No filter in the manifold**: the pump runs at 100 %, which a filter would not survive.

1. Filters out. Inlet and outlet tubes in 70 % ethanol, outlets draining to a waste container. Take the 20 µm prefilter out: it is replaced after the cleaning (Lexplore, bacterial work).
2. `session` (right spool answer). `actuator_home` only after a reset.
3. `ethanol_purge` (or `ethanol_purge skip=3,7,12`). Answer: all slots need cleaning `y`, or `n` and the numbers that do **not**; filters removed `y`; check the list and quantity, `y`.
4. About 1 to 2 min per slot: prime, 0.1 L per slot, then air. Stay with it.
5. Last line `Ethanol cleaning complete: ... 0 warning(s)`. A `WARNING` means a slot may still be wet: check its outlet, rerun with `skip=` for the slots that are fine.

---

## Commands

| Command | Does |
|---|---|
| `session` | site, date, experiment, operator, spool y/n, deepest depth |
| `actuator_home` | finds the manifold; after every reset |
| `fill_DNA` | loads the DNA shield |
| `run_sample <slot> [depth=m]` | one sample |
| `schedule`, `schedule start [row]`, `schedule stop` | the list |
| `ethanol_purge [skip=...]` | cleaning |
| `spool_status`, `spool_height <cm>`, `spool_goto <m>` / `length=<m>`, `spool_home [drain=n]` | the spool |
| `oring_state`, `encoder_read [n]`, `abp_raw` | read-only checks of the lock, the manifold encoder, the pressure sensor |
| `x` / `s` + ENTER, `:quit` | stop / skip a step, close the log |

---

# Appendices

## A. Problems

| You see | Do |
|---|---|
| `could not open port ... Access is denied` | Another program holds the port (PlatformIO monitor, Arduino IDE). Close it, restart the tool |
| `actuator_home: not recognized` | Typed in PowerShell. Type device commands in the log window |
| `actuator_home` FAIL, error above 3° | Manifold not within tolerance (it still unlocks). Align with `slotN` / `cal` before relying on it; see the manifold note below |
| `REFUSED ... lock state is UNKNOWN` | `actuator_home` first; or `spool_home drain=n` (no manifold move) |
| `SAFETY \| implausible pressure reading` | Pump stopped. Do not restart it. Call the engineer |
| `abp_raw` shows `00 00 00` or one fixed value; `oring_state` shows `no valid reading` | Pressure sensor shorted (water on its pins also corrupts the manifold encoder). Do not pump, do not rotate. Unplug the sensor, call the engineer |
| Pressure falls to about 0.04 bar while the pump runs | Sensor loose or dead (no automatic stop for this). Stop, check its mount |
| `Vent: ... NOT released` | Line still above 0.3 bar; the run went on. Note it, tell the engineer |
| `PAUSE n in this run (cap margin, normal)` | Pump pauses near the cap. Many per sample is normal at 3 bar. Note only if the flow stops for minutes |
| `WARNING \| pause of only ... ms` | Suspect pressure reading. Note it |
| `WARNING \| spool: origin switch held at ... ignored` | Normal once. If it repeats, call the engineer (switch or wiring) |
| `Spool: depth in doubt ...: rolling up to the origin` | Nothing: the sample goes on after the roll up |
| `Spool move: ... stalled (encoder not counting)` | Check tube and drum by hand and `spool_status`, then `spool_home`. No spool sample until fixed |
| `spool_home` times out with the tube out | Do not repeat more than twice. Free the tube by hand, check `spool_status`, call the engineer |
| Run says `origin switch reached before the goal`, or the tube is down when it should be up | Zero not trusted. No dive. `actuator_home`, `spool_home` (up to 30 min), check origin `PRESSED` and tube out `0`, then `schedule start <row>` |
| `Sample N erased: available again` | You reused a used slot and said yes. Nothing |
| A stray line was sent in front of a command (paste, typo) | Check `session.log`; type, do not paste |

**Manifold drift.** The manifold has twice read 3 slots (67.5°) off in one day. If a slot is visibly wrong, do not sample: tell the engineer, who redoes `cal` on slot 7 with the O-ring locked.
**Leaks and joints.** Check the DNA shield inlet (check valve and T connector) for a sharply bent tube; a leak lowers the shield dose.

## B. Measuring the spool height

Intake at the top, tube not in the water.
1. `spool_status`: origin switch `PRESSED`.
2. Unroll in steps (`spool_goto length=0.5`, `1`, `1.5`, ...) until the intake **touches the water surface**.
3. `spool_status` prints `tube out = X cm`. Type `spool_height X` (whole cm).
4. Check: `spool_goto 0` prints the same X. Then `spool_home drain=n`.

About ±2 cm. RAM only: repeat after every reset. Check: `spool_goto length=1` reads 0.98 m ± 5 cm on a tape. Depths are estimates: current pulls the tube sideways.

## C. Restarting a schedule after a failure

1. Fix the cause. Check the tube: `spool_status`, origin `PRESSED` when rolled up.
2. `schedule`: rows are numbered. Note the row to run again (say `3`).
3. `schedule start 3`: waiting rows before 3 are **skipped**; row 3 and later rows that did not pass run. Rows that passed never rerun. Answer `y`.
- A `[FAIL]` row is not "waiting": plain `schedule start` skips it. Use the row number.
- A run stopped before its filter was loaded frees its slot; one stopped during or after sampling keeps it used.
- The re-run takes the next available filter. The schedule file is never changed; the run's list is `schedule.csv` in the run folder.
- Times and depths of a saved list cannot be changed in the tool. Closing the tool and starting a new file is a stopgap that has caused errors: avoid it, and never reuse an old file with passed rows.
- After an `x` the spool is rolled up but not emptied (`spool_emptied=NO`): run `spool_home`.
