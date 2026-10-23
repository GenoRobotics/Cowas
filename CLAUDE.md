# CoWaS: Claude Code guide

## Project

CoWaS (GenoRobotics) is an autonomous aquatic eDNA sampler heading to a supervised pilot on Lake Geneva.
A 24 V diaphragm pump (speed from DAC1, power through a relay on D42) pushes lake water through Sterivex filters on a rotating manifold.
Manifold slot 0 = purge and slots 1..14 = samples, used in `FILL_ORDER` (Settings.h). An AMT22 SPI encoder (CS D38) and a DC motor on a VNH5019 shield position it.
An O-ring seals the manifold. It is locked and unlocked **open-loop** by a NEMA17 + A4988 linear actuator (STEP/DIR/EN = D9/D10/D11).
A micro pump (D48) adds DNA/RNA Shield after sampling. A turbine flowmeter (D13) sets the sampled volume.
An Arduino Due (PlatformIO env `due`) runs everything. A legacy Raspberry Pi script (`computer_interface/`, PyQt5 + ZMQ) sends text commands over the same USB serial port as the console.
The spool (depth) is broken and disabled (`SPOOL_USE = false`); the intake is at a fixed depth.
The roadmap has three stages (2026-10-06):
**stage 1** = [roadmap/stage1-FINISHED-prototype.md](docs/roadmap/stage1-FINISHED-prototype.md), the operator test (session, one `run_sample` per sample). **Read its status and "Left to do" first. Deferred work is in stage 2 §7b.** Keep what works, no new tools or tuning. Earlier stage 1 plans: docs/archive/roadmap-stage1-2026-10-23.md and docs/archive/roadmap-stage1-2026-10-15.md. Removed code: docs/archive/code/README.md;
**stage 2** = [roadmap/stage2-pilots.md](docs/roadmap/stage2-pilots.md), the lake pilots: the design decisions, findings, test strategy, rules and the step cards (FW/HO/HW/LA/VA);
**stage 3** = docs/roadmap/stage3-full-system.md, the P2 work after the first data.
"ROADMAP §N" means a section of the stage 2 file. Read ROADMAP §3 (design decisions), §4 (findings) and §6 (rules) before any change.

## Hardware and pins (details: [hardware/pinout.md](docs/hardware/pinout.md))

| Item | Pins | Notes |
|---|---|---|
| Diaphragm pump | DAC1 (speed), D42 (24 V relay) | DAC gives ~0.55–2.75 V, the relay is the real stop (F25) |
| Valves | V1 D44 (air inlet), Vman D46 (deployment line); V23 D40 = **valve removed**, relay only (F51) | |
| Micro pump (shield) | D48 | time-based |
| Manifold motor | VNH5019 M2: INA D6, INB D7, PWM D3, EN/DIAG D5, CS A11 | |
| Manifold encoder | AMT22 on SPI, CS D38 | |
| O-ring actuator | A4988 STEP D9, DIR D10, EN D11 (active LOW) | open-loop, lock state RAM-only (F18) |
| Pressure | **Honeywell ABP on SPI, CS D8** (ordered and supplied as `ABPDANV150PGSA3`; against the bench manometer it reads 2.4 times too high with the 150 psi constant, so `ABP_FULL_SCALE_PSI = 60`, the value that agrees with the manometer: PLACEHOLDER, unresolved, F126) = `pressure1` in the code (becomes `p_in` in FW3). Since S1-1 it backs the pump cap and the PID loops (`read_p_in()`), `PRESSURE_GAIN` / `PRESSURE_ZERO_OFFSET_BAR` in Settings.h. The analog A2 sensor (`pressure2`) was removed from the machine and its code deleted (2026-10-12, user request) | |
| Flowmeters | `flow_sensor_small` D13 (calibrated, factor 77, wired at 3.3 V: user, 2026-10-23; F6, F26 closed); `flow_sensor_big` D12 | |
| Panel | START D24, LEFT D25, RIGHT D26; LEDs D22/D23 | |
| Spool (disabled) | VNH5019 M1 (D2, D30, D32, D34, A10), encoder D31/33/35, end-stops D28/D29, container switch D27 | still configured at boot |
| Serial | `Serial` (Programming port, 9600): console **and** legacy Pi link | Native USB unused until FW7 |

The Due is 3.3 V only; no pin is 5 V tolerant.

## Build and test

```sh
pio run -e due                  # firmware build (must pass)
pio test -e native              # host unit tests for src/Core/ (must pass)
pio run -e due -t upload        # flash: only when the user asks
pio device monitor              # console (9600, LF, send on ENTER: set in platformio.ini)
bash tools/golden/capture_manifold_golden.sh   # regenerate manifold goldens: only on purpose
cd host && .venv/Scripts/python -m pytest      # host tests (must pass); ruff check . && ruff format --check .
pytest tools/                   # tests of the analysis scripts in tools/ (e.g. analyze_la3.py; stdlib + pytest)
```

- `native` needs gcc/g++ on PATH. On the Windows laptop: WinLibs from winget (`BrechtSanders.WinLibs.POSIX.UCRT`, installed under `%LOCALAPPDATA%\Microsoft\WinGet\Packages\...\mingw64\bin`). `pio` itself is `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`.
- If `pio` downloads fail with `CERTIFICATE_VERIFY_FAILED` (TLS inspection on the network), point `SSL_CERT_FILE` and `REQUESTS_CA_BUNDLE` at a PEM bundle that includes the Windows root store.
- `pio run -e uno` does not build (Due-only code; see F12). Don't use it.
- CI (`.github/workflows/ci.yml`): `pio run -e due`, `pio test -e native`, ruff + `pytest host/` (Ubuntu and Windows), and `pytest tools/`.
- Host venv: `host/.venv`, created from `%USERPROFILE%\.platformio\python3\python.exe` (no system Python on the laptop); `pip install -e ".[dev]"` needs the same CA bundle as `pio`.

## Code layout

- `src/main.cpp`: globals, `setup()`, `loop()` (button panel + legacy Pi protocol).
- `src/Tests.cpp`: the serial test menu `test_all_components()` and all calibrations. `setup()` ends inside it (F23).
- `src/Step_functions.cpp`: production procedures (`step_purge`, `step_sampling`, `step_DNA_shield`, `step_empty`, `sample_process`, `load_slot*`).
- `src/Hardware/`: thin drivers. `Oring_lock.cpp` = lock/unlock. `Pump.cpp` = pump + PID loops + the pressure cap.
- `src/Core/`: **pure logic, no Arduino.h**, unit-tested natively. Today: `manifold_geometry` (slot-angle table, clock labels, `scale_angle`), `shield_calibration` (LA3: micro pump rate fit, fill time), `abp_convert` (S1-1: ABP frames, counts → bar, the dead-sensor rule), `pressure_calibration` (S1-1: gain + offset fit).
- `test/test_*/`: native Unity tests. `test/stubs/` = a fake Arduino.h/TimeLib.h so tests can read the real `include/Settings.h` constants (never used by the firmware).
- `include/Settings.h`: all tunables and pins. `include/Manifold.h`: manifold calibration (`purge_angle`, ...).
- Host protocol v1 (the contract for FW7 and the host track): [software/protocol.md](docs/software/protocol.md) + `protocol/vectors.json` (shared C++/Python test vectors).
- `host/`: the Python host (package `cowas`, HO1): protocol models, serial transport (Native USB by VID/PID, never 1200 baud), client, `FakeController` simulator, the `cowas` CLI (`--sim` everywhere). See host/README.md.
- Every command, with its hardware, blocking and pressure dependence: [software/commands.md](docs/software/commands.md). Tests waiting for hardware: [stage 2 §7a](docs/roadmap/stage2-pilots.md). Doc index: [docs/README.md](docs/README.md).

## Rules (ROADMAP §6, verbatim)

1. **Behavior changes only in steps that say so**, each listed in the PR description.
2. **Nothing is deleted without evidence and the user's approval.** Git tag `baseline-v0` keeps everything.
3. **Pure logic in `src/Core/`**, unit-tested with `pio test -e native`. Hardware code stays thin.
4. **Safety invariants are never weakened:**
   - The pump never runs above `PRESSURE_CAP`, and an implausible `P_in` stops it.
   - No manifold rotation while locked, or while the lock state is unknown.
   - Every procedure has a timeout.
   - STOP and E-stop are always accepted.
   - Safe state = pumps off, valves closed, O-ring stays locked.
   - Recovery is required after every stop and every boot (from FW6).
   - Placeholders stay marked `PLACEHOLDER`.
   - The only exception is FW2's sensorless pump test mode: explicit, test-only, low power, time-limited, confirmed at every run, unreachable from production code.
5. **Degraded modes are explicit.** When a non-safety sensor fails, keep working, raise a warning, and flag the sample record. Never fail silently.
6. **No hard-coded slot counts in new code.**
7. **Every step ends with:** `pio run -e due` and `pio test -e native` (and `pytest` once `host/` exists) passing; a hardware checklist with numeric pass criteria; the pending checks in the roadmaps updated.
8. **eDNA quality rules (§3.7)** are requirements, not options: purge volume, blanks, and recorded `end_reason`.
9. Claude does not flash, commit, push or delete files without asking.

## Known hazards (as of FW0)

- **Placeholders:** `LINEAR_ACT_LOCKED_STEPS = 210` was measured on 2026-10-12 after the potentiometer change and confirmed good on 2026-10-20 (redo `calibrate_actuator` after any change of the A4988 Vref). Still placeholders: `PURGE_MILLILITERS = 100` (below the dead volume, F30), `STX_SAMPLE_MILLILITERS = 100` (F15), the shield time `FILL_STERIVEX_TIME / 10.` (a test shortcut, F4), `ABP_FULL_SCALE_PSI = 60` (F126), the `PRESSURE_RAMP_*` values (S1-1b). The pressure calibration is pasted since 2026-10-12 (`PRESSURE_GAIN = 1.0413`, offset `-0.0311`, valid to 2.0 bar). `STX_MAX_PRESSURE` is **3.6** (user decision 2026-10-19, was 3.2 from 2026-10-15; the `fill_test` pause, resume and highest target derive from it in Settings.h; droplets leaked at the manifold seal around 3 bar in earlier tests, F144; the sensor is calibrated only to 2.0 bar, F136; the weakest part's rating and the relief valve plan are open, F143). The PID setpoint 2 is still hard-coded (F3).
- **Flowmeter (user, 2026-10-23):** `calibrationFactor_small = 77.0` (Flow_sensor.h) is the calibrated factor, and D13 is wired at 3.3 V (F6, F26 closed). It reads almost nothing below about 25 mL/min and about 32 % low at about 50 mL/min (F150), so the confident volume `trusted_ml` (windows at 50 mL/min or more, `FLOW_TRUST_MIN_ML_PER_MIN`) is the volume to use. The plain total is truncated to an integer (F31).
- **The ABP gave garbage frames at an 800 kHz SPI clock (F124, fixed 2026-10-09 with `ABP_SPI_HZ = 100000`)**, with a "valid" status that the firmware can't tell from a real reading. After any rewiring, `abp_raw` must show a steady reading at rest (20 lines within about ±20 counts, room temperature) before any calibration or pumping. **Sensor and manometer disagree by 2.4 (F126):** the firmware follows the manometer. User decision (2026-10-09): trust the manometer. Don't change `ABP_FULL_SCALE_PSI` without checking against it. The same session saw the USB link drop and the Due reset when `actuator_home` was typed (F125).
- **Pressure cap (S1-1, bench-checked 2026-10-12):** the pump cap and every PID loop read the ABP on SPI D8 (`read_p_in()`). A fault frame, no answer, or 500 ms of stale frames makes the reading implausible (−99 bar), which stops the pump and refuses a start. Calibration, dead-sensor stop and cap trip (at a temporary 1.0 bar) passed. **But the control loop in front of the cap is not reliable on a clogged filter (F132–F135):** `CtrlPump` starts open-loop at 50 %, its PID is untuned and blocks for up to 4 s after a big step, and it reads the cap only every 100 ms. On 2026-10-12 `leak_hold` on clogged filters hit the cap almost at once and leaked. **Don't run `leak_hold`, `full_cycle`, `multi_sample` or `load_slot*` on a clogged or nearly clogged filter until S1-C is done** (since 2026-10-15 the production loops start at once and no longer block; the gains are 200 / 100 / 100, like the tests since 2026-10-19, F147).** For pressure checks use `pressure_ramp` (PID by pressure, slow setpoint ramp, cap checked every pass). The software cap is still the only pressure protection (no relief valve, no E-stop); never unattended.
- **`setup()` ends in the blocking test menu** (`test_all_components()`, main.cpp:219). The Pi protocol and the button panel only run after someone types `stop`, so after any reset the machine waits for a human (F23).
- **The spool is disabled** (`SPOOL_USE = false`), but its pins are still configured and its end-stop interrupt stays attached.
- **O-ring lock tri-state (FW2):** LOCKED / UNLOCKED / UNKNOWN, **UNKNOWN at every boot** and after `linear_actuator` / `calibrate_actuator`. `rotateMotor()` and every direct-motor test refuse unless UNLOCKED; `lock_oring`/`unlock_oring` refuse while UNKNOWN. After a reset, run `actuator_home` (needs the manifold within ±3° of a slot, PLACEHOLDER). **No escape** if the state is UNKNOWN and the rotor is not aligned (user decision, F60).
- **`PRESSURE_SENSOR_AVAILABLE` is `true` (Settings.h, since 2026-10-12):** the ABP is calibrated and the dead-sensor stop passed, so the procedures that need pressure run (`sample_process`, `step_*`, `load_slot*`, `pump`, `full_cycle`, ...), and the sensorless test mode (NOSENSOR) refuses. The panel Pump mode, `calibrate_flow` and `fillContainerFunction` poll the cap on every pass since 2026-10-23 (F201). `purgePipesFunction` and `cal_DNA` still do not poll it while pumping (F202): don't use them.
- The V23 valve (D40) is physically removed, but the code still switches its relay (F51, cleaned up in FW4).
- **Rest state decided in FW1 (2026-09-25): the O-ring is LOCKED at rest, unlocked only to rotate** (ROADMAP §3.2, docs/software/protocol.md §5.1). Implemented in FW4; until then `sample_process()` and `load_slot*()` still end unlocked.
- `scale_angle()`'s loop guard is dead (inherited, kept in FW0): ±inf would hang it. `go_to_zero` (`zero`) has no timeout.

## Working here

- **Read [LOG.md](docs/logs/LOG.md) at the start of every step.** It holds what previous steps changed. Findings (F37+) are in [findings.md](docs/logs/findings.md), decisions in [decisions.md](docs/logs/decisions.md). At the end of the step, add an entry (changes, behavior changes, tests, hardware checklist results) and log any new finding with the next free F-number; mark fixed findings with the step and date. Stage 1 steps: log each bench result in LOG.md and update the status in stage 1 §0. Bench transcripts go in `docs/logs/test-logs/`.
- One ROADMAP step = one branch/PR. **Branch from `cowas-v2` and merge back into `cowas-v2`, never into `main`** (main stays the old code until Pilot A; ROADMAP §0). List every behavior change in the PR description.
- Moving math out of a hardware file: record golden outputs first, and assert them bit for bit (see `tools/golden/` + `test/test_manifold_geometry/`).
- Keep `src/Core/` C++11-compatible: the Due toolchain is arm-none-eabi-gcc 7.2.1 with `-std=gnu++11`.
