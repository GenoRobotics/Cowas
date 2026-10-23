# Tutorial: the console with a session log

`tools/session_log.py` does three things at once:
- it shows the Due's console, like `pio device monitor`;
- it sends what you type to the Due;
- it saves everything: the transcript, the TEST results, and one file per sample.

Use it instead of `pio device monitor` for any bench run that should be kept.

## 1. Before you start

- Only one program can use a COM port. **Close `pio device monitor` and any other terminal on that port** first.
- Find the port number in Device Manager (Ports (COM & LPT)), for example `COM6`. The Due's **Programming port** is the one used by the console.
- Do not use 1200 baud. The tool refuses it, because it erases the Due's flash.
- Run the tool from the project folder (the one with `tools` and `docs`). The runs go to `docs\logs\test-logs\<date>\` in that folder, whatever the working folder is (`--dir` changes it).

## 2. Start a session

```
tools\session_log.cmd --port COM6 --tag s1-1c
```

`tools\session_log.cmd` runs the script with the PlatformIO Python (`%USERPROFILE%\.platformio\penv`), because the laptop has no other Python. It works from any Windows account that has PlatformIO installed.

- `--tag` names the run: `field-<site>-<date>`, `s1a`, `tune`. The run folder is numbered (`01_`, `02_`) in order of the day.
- The first line tells you where the files are, for example `Logging to docs/logs/test-logs/2026-10-23/02_field-test/session.log`.
- The tool prints `open COM6 9600 baud, DTR/RTS low` into the log. This is normal.
- The Due prints its boot lines and the test menu as usual. If it reboots when the port opens, write it down (the session-log checks, [archive copy §6.2](../archive/roadmap-stage1-2026-10-23.md), check 2).

## 3. Send commands

Type a command and press ENTER, the same as in the console:

```
slot11
fill_test volume=100 label=a2 quick
```

- Every line you type is shown on screen and written to the log with `>`.
- Stop a pump run: `s` then ENTER skips the current pump run. `x` then ENTER aborts the whole command. Any other typed line during a run is ignored with a message.
- Leave the tool with `:quit` or Ctrl+C. The session is closed cleanly.

## 4. What you see and what is saved

| On screen | Meaning | Saved in |
|---|---|---|
| Device lines | What the Due prints | `session.log` with `<` |
| Your typed lines | What you sent | `session.log` with `>` |
| `TEST\|...` lines | A test result | also `tests.csv`, one row each |
| `-> sample saved: ...` | A sample record was received and saved | `samples/NNN.json` and a row in `samples/samples.csv` |
| `!` lines | The tool itself (open, closed, errors) | `session.log` only |
| `STEP TIMES | purge … | sample … | shield … | unlock … | total …` | How long each step of a `run_sample` took (seconds) | the same numbers as `t_purge_s`, `t_sample_s`, `t_shield_s`, `t_unlock_s`, `t_total_s` on the `TEST` line (`tests.csv`) |

The folder `docs\logs\test-logs\2026-10-23\02_field-test\` then holds, for a run tagged `field-test` on 23 October (the second run of that day):
- `session.log`: the full transcript;
- `tests.csv`: the TEST results, one row each;
- `samples/`: one JSON per sample or purge, and `samples.csv`.

## 5. Reading a sample record

Each purge and each sample writes one record. Open the JSON file in any editor. The main fields:

| Field | Read it as |
|---|---|
| `purpose`, `slot`, `id` | What was run, where, and its id (`auto-N` counts from boot; a `run_sample` sample has its sample name) |
| `site`, `date`, `condition` | From `session` and `run_sample`: the site, the date and the sample's condition. Written in the purge and the sample of that run (missing when the record is from another command) |
| `run_start_pc`, `run_finish_pc` | The PC time when the `run_sample` started (its `=== run_sample ===` line) and when it finished (its `TEST\|run_sample` line). Written into every record of the run. Empty for a record from another command |
| `end_reason` | Why it stopped: `volume`, `pressure`, `timeout`, `stop_key`, `stalled`, `clogged`, `sensor`, `skipped`, `refused` |
| `target_ml`, `vol_ml`, `purge_ml` | Target volume, measured volume (flowmeter; `trusted_ml` is the fast-flow part, F150), purge volume |
| `p_mean_bar`, `p_max_bar`, `p_cap_bar` | Pressure over the run, the peak, and the cap in force |
| `lock_before`, `lock_after` | O-ring state before and after |
| `calib` | The calibration values the run used |
| `flags` | Placeholders in use (`placeholder:…`). Read them: a value listed here is not final |
| `pc_received` | When the PC received it (the trusted clock) |

`null` means "not available", never 0. The CSV `samples.csv` has the same main fields, one row per sample, for a table.

## 6. When something goes wrong

| Message | What it means | What to do |
|---|---|---|
| `LOG ERROR: ... not written` | A file could not be written (disk, folder removed, file open elsewhere) | The console still works. Fix the cause. The next line in the `.log` starts with `LOG GAP`: a gap is recorded |
| `DISCONNECTED: ...` | The cable was pulled or the port is gone | The session ends. Check the cable and restart the tool. A restart with the same tag and day appends to the same folder, so nothing is lost |
| `BAD SAMPLE LINE` | A sample line was received but is not valid JSON (cut or corrupted) | It is not saved. The `.log` keeps the raw line. Send the log to the developer |
| Nothing after typing | The Due is busy, or the line did not reach it | Wait a second. If the Due is in a pump run, `s` or `x` first |
| `SAMPLES CHECK: n JSON files ..., m saved` (at `:quit`) | A sample file is missing from `samples/` | Rebuild it (below) and tell the engineer |

## 7. Good habits

- One tag per run. A restart with the same tag on the same day writes into the same numbered folder; use a new tag for a different test.
- Write the bench notes (who, filter, water, weight) in LOG, not in the console. The console is for the Due.
- Keep the `.log`. It is the only full record of a run. Commit it with the checklist results when a step passes.
- If you need the Pi or a laptop without the tool, the record is not kept. The sample is still run.

## 8. Rebuild a lost samples folder

If the `samples/` folder of a finished run is empty or missing its files, rebuild it from the transcript. No port is needed:

```
tools\session_log.cmd --rebuild docs\logs\test-logs\<date>\<NN>_<tag>
```

The records are the same as the live tool writes, with the times of their transcript lines. Nothing is overwritten: if the run folder already has sample files, the transcript is copied into the next numbered folder of the day (`<NN+1>_<tag>`) and the samples are rebuilt there. The command prints the folder it wrote.
