# CoWaS: Claude Code guide

## Project

CoWaS (GenoRobotics) is an autonomous aquatic eDNA sampler heading to a supervised pilot on Lake Geneva.
A 24 V diaphragm pump (speed from DAC1, power through a relay on D42) pushes lake water through Sterivex filters on a rotating manifold.
Manifold slot 0 = purge and slots 1..14 = samples, used in `FILL_ORDER` (Settings.h). An AMT22 SPI encoder (CS D38) and a DC motor on a VNH5019 shield position it.
An O-ring seals the manifold. It is locked and unlocked **open-loop** by a NEMA17 + A4988 linear actuator (STEP/DIR/EN = D9/D10/D11).
A micro pump (D48) adds DNA/RNA Shield after sampling. A turbine flowmeter (D13) sets the sampled volume.
An Arduino Due (PlatformIO env `due`) runs everything. A legacy Raspberry Pi script (`computer_interface/`, PyQt5 + ZMQ) sends text commands over the same USB serial port as the console.
The spool (depth) is broken and disabled (`SPOOL_USE = false`); the intake is at a fixed depth.
The plan, findings (F1–F36), test strategy and step prompts are in **[docs/ROADMAP.md](docs/ROADMAP.md)**; P2 work is in docs/ROADMAP_LATER.md.
Read ROADMAP §3 (design decisions), §4 (findings) and §6 (rules) before any change.

## Hardware and pins (details: [docs/PINOUT.md](docs/PINOUT.md))

| Item | Pins | Notes |
|---|---|---|
| Diaphragm pump | DAC1 (speed), D42 (24 V relay) | DAC gives ~0.55–2.75 V, the relay is the real stop (F25) |
| Valves | V1 D44 (air inlet), Vman D46 (deployment line); V23 D40 = **valve removed**, relay only (F51) | |
| Micro pump (shield) | D48 | time-based |
| Manifold motor | VNH5019 M2: INA D6, INB D7, PWM D3, EN/DIAG D5, CS A11 | |
| Manifold encoder | AMT22 on SPI, CS D38 | |
| O-ring actuator | A4988 STEP D9, DIR D10, EN D11 (active LOW) | open-loop, lock state RAM-only (F18) |
| Pressure | **none installed** (F50). Code: `pressure1` = ABP driver on SPI, CS D8 (becomes `p_in` in FW3); `pressure2` = analog A2, sensor removed, **but it is what safety reads** (F7/F22) | |
| Flowmeters | `flow_sensor_small` D13 (the volume stop, uncalibrated, 5 V level risk F26); `flow_sensor_big` D12 | |
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
```

- `native` needs gcc/g++ on PATH. On the team's Windows laptop: WinLibs from winget (`BrechtSanders.WinLibs.POSIX.UCRT`, installed under `%LOCALAPPDATA%\Microsoft\WinGet\Packages\...\mingw64\bin`). `pio` itself is `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`.
- If `pio` downloads fail with `CERTIFICATE_VERIFY_FAILED` (TLS inspection on the network), point `SSL_CERT_FILE` and `REQUESTS_CA_BUNDLE` at a PEM bundle that includes the Windows root store.
- `pio run -e uno` does not build (Due-only code; see F12). Don't use it.
- CI (`.github/workflows/ci.yml`): `pio run -e due`, `pio test -e native`, and `pytest host/` once `host/` exists.

## Code layout

- `src/main.cpp`: globals, `setup()`, `loop()` (button panel + legacy Pi protocol).
- `src/Tests.cpp`: the serial test menu `test_all_components()` and all calibrations. `setup()` ends inside it (F23).
- `src/Step_functions.cpp`: production procedures (`step_purge`, `step_sampling`, `step_DNA_shield`, `step_empty`, `sample_process`, `load_slot*`).
- `src/Hardware/`: thin drivers. `Oring_lock.cpp` = lock/unlock. `Pump.cpp` = pump + PID loops + the pressure cap.
- `src/Core/`: **pure logic, no Arduino.h**, unit-tested natively. Today: `manifold_geometry` (slot-angle table, clock labels, `scale_angle`).
- `test/test_*/`: native Unity tests. `test/stubs/` = a fake Arduino.h/TimeLib.h so tests can read the real `include/Settings.h` constants (never used by the firmware).
- `include/Settings.h`: all tunables and pins. `include/Manifold.h`: manifold calibration (`purge_angle`, ...).
- Every command, with its hardware, blocking and pressure dependence: [docs/COMMANDS.md](docs/COMMANDS.md). Tests waiting for hardware: [docs/TESTS_PENDING.md](docs/TESTS_PENDING.md).

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
7. **Every step ends with:** `pio run -e due` and `pio test -e native` (and `pytest` once `host/` exists) passing; a hardware checklist with numeric pass criteria; `docs/TESTS_PENDING.md` updated.
8. **eDNA quality rules (§3.7)** are requirements, not options: purge volume, blanks, and recorded `end_reason`.
9. Claude does not flash, commit, push or delete files without asking.

## Known hazards (as of FW0)

- **Placeholders:** `LINEAR_ACT_LOCKED_STEPS = 210` is not calibrated (run `calibrate_actuator`). Also `PURGE_MILLILITERS = 100` (below the dead volume, F30), `STX_SAMPLE_MILLILITERS = 100` (F15), the shield time `FILL_STERIVEX_TIME / 10.` (a test shortcut, F4), and `STX_MAX_PRESSURE = 3.0` (F3).
- **Uncalibrated flowmeter:** `calibrationFactor_small = 77.0` (Flow_sensor.h) has not been calibrated on this machine, and it is the volume stop for every sample (F6). It is truncated to an integer (F31), and the D13 input may see 5 V (F26).
- **No pressure sensor is installed (F50), and safety reads a floating pin:** the pump cap and every PID loop use `read_pressure2()` = analog A2, whose sensor was removed, so **the cap can never trip**. No ABP answers on SPI D8 either. Don't pump against a closed or clogged outlet, and never unattended, until FW3 + HW4 (F7/F22, F17).
- **`setup()` ends in the blocking test menu** (`test_all_components()`, main.cpp:219). The Pi protocol and the button panel only run after someone types `stop`, so after any reset the machine waits for a human (F23).
- **The spool is disabled** (`SPOOL_USE = false`), but its pins are still configured and its end-stop interrupt stays attached.
- **Rotation ignores the O-ring lock** in `slotN`, `clockMM`, `slot0`, panel Man Slot, `cal`, `zero`, `manifold`, `encoder_manifold` (F5), and the lock state is `false` at every boot (F18).
- The V23 valve (D40) is physically removed, but the code still switches its relay (F51, cleaned up in FW4).
- `scale_angle()`'s loop guard is dead (inherited, kept in FW0): ±inf would hang it. `go_to_zero` (`zero`) has no timeout.

## Working here

- **Read [docs/FIRMWARE_LOG.md](docs/FIRMWARE_LOG.md) at the start of every step.** It holds what previous steps changed and the findings F37+ (new since ROADMAP §4). At the end of the step, add an entry (changes, behavior changes, tests, hardware checklist results) and log any new finding with the next free F-number; mark fixed findings with the step and date.
- One ROADMAP step = one branch/PR. **Branch from `cowas-v2` and merge back into `cowas-v2`, never into `main`** (main stays the old code until Pilot A; ROADMAP §0). List every behavior change in the PR description.
- Moving math out of a hardware file: record golden outputs first, and assert them bit for bit (see `tools/golden/` + `test/test_manifold_geometry/`).
- Keep `src/Core/` C++11-compatible: the Due toolchain is arm-none-eabi-gcc 7.2.1 with `-std=gnu++11`.
