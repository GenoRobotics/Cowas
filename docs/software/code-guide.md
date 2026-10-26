# Code guide

The firmware of the sampler: a pump pushes lake water through a Sterivex filter, and a rotating manifold with 15 positions (14 samples, 1 purge) brings each filter in turn. A motor with an encoder turns the manifold; an O-ring seals it, moved by a linear actuator. Target: Arduino Due (PlatformIO env `due`).

Project rules and safety invariants: [CLAUDE.md](../../CLAUDE.md). Status: [stage 1](../roadmap/stage1-FINISHED-prototype.md). Decisions: [decisions.md](../logs/decisions.md). Findings: [findings.md](../logs/findings.md). History: [LOG.md](../logs/LOG.md). Commands: [commands.md](commands.md).

## 1. Where things are

| Folder or file | What it holds |
|---|---|
| `include/Settings.h` | Every tunable value and pin. `PLACEHOLDER` marks values not measured yet |
| `include/Manifold.h` | `purge_angle`, the slot 7 reference |
| `src/Step_functions.cpp` | The production steps: `step_purge`, `step_sampling`, `step_DNA_shield`, `run_sample` helpers |
| `src/Tests_mech.cpp` | Operator commands (`session`, `run_sample`) and the test commands |
| `src/Tests.cpp` | The serial menu and the calibrations |
| `src/Hardware/` | Drivers: pump, valves, O-ring, manifold, encoder, flowmeter, pressure sensor, micro pump |
| `src/Core/` | Pure logic with no Arduino code: stop rules, lock rules, trusted volume, purge volume, record format. Each has a native test in `test/` |
| `src/main.cpp` | `setup()`, `loop()`, the button panel, the legacy Pi protocol |
| `tools/session_log.py` | The console with the log (see [session-log.md](session-log.md)) |
| `tools/` tests | `pytest tools/` |
| `host/` | Python host package for the future protocol. Not used on the bench |

Removed code and how to restore it: [archive/code/README.md](../archive/code/README.md).

## 2. Typed commands

`setup()` starts the drivers, then the serial menu runs (`test_all_components()`, `Tests.cpp`). A typed line goes to that menu, then to `mech_tests_dispatch()` (`Tests_mech.cpp`), then to the older commands in `Tests.cpp`. Typing `stop` leaves the menu and starts the panel and the Pi protocol (`loop()`). Full list: [commands.md](commands.md).

## 3. The main parts

- **Manifold** (`Manifold.cpp`, `src/Core/manifold_geometry.*`): slot N sits at an angle set from `purge_angle`. Slot 7 is the reference. `rotateMotor()` refuses unless the O-ring is UNLOCKED.
- **O-ring** (`Oring_lock.cpp`, `src/Core/lock_rules.*`): LOCKED, UNLOCKED or UNKNOWN. UNKNOWN at every boot, cleared by `actuator_home`. The moves are open loop (fixed step counts).
- **Pump control** (`Pump.cpp`): a PID on the inlet pressure. The pressure cap (`STX_MAX_PRESSURE`, 3.6 bar) is checked on every pass. A pause cuts the pump at 3.35 bar and resumes at 3.15 bar.
- **Stop rules of a filter run** (`src/Core/fill_stop.*`): the run ends on volume, timeout, stalled (under 3 mL in 60 s of pumping), clogged (a window under 30 mL, or two windows under 30 % of the first), or pressure (paused and stuck for 20 s). Stalled and clogged read the flowmeter.
- **Trusted volume** (`src/Core/trusted_volume.*`): the part of the flowmeter volume measured at 50 mL/min or more. Logged as `trusted_ml`. The plain total is `sampled_ml`.
- **Purge and sample times**: `run_sample` times each step and prints `STEP TIMES`.
- **Sample record** (`src/Core/sample_record.*`): one JSON line per purge or sample. `tools/session_log.py` saves it as a file when the run ends.

## 4. A field sample, step by step

`run_sample <slot>` (`Tests_mech.cpp`, `run_full_cycle()`):

1. **Purge:** the volume is 500 mL, or 3 L with the spool (answered at `session`). The timeout is 3 min, or 13 min with the spool. The purge runs at full power (100 %, `PUMP_PURGE_MAX_POWER_PERCENT`); every other water run is limited to 60 %.
2. **Sample:** water through the filter until a stop rule ends it.
3. **Shield:** the micro pump adds the DNA shield (timed), then air pushes it in (`step_DNA_shield`).
4. **Unlock:** the O-ring unlocks at the end.

The sterivex air drying (`EMPTY_STX_AIR_HOLD_MS`, 40 s) and the purge drying (`EMPTY_AIR_HOLD_MS`, 20 s) are in `Settings.h`.

## 5. DNA shield, what is timed

- The micro pump runs for `FILL_STERIVEX_TIME / 10` (25 s). This is a test shortcut and a placeholder.
- The air push: `PUMP_SHIELD_POWER` 13 % for `PUMP_SHIELD_TIME` (300 ms), after a 4 s ramp. About 4.8 s in total.
- `shield_calibrate` prints the dose time `DNA_SHIELD_FILL_MS`. Production does not read it yet. Calibration is Felix's task ([for-felix.md](../for-felix.md)).

## 6. Conventions

- Every behaviour change goes in the log, with its reason and its test.
- Placeholders are marked in `Settings.h` and printed on each `TEST` line (`placeholders=`).
- Findings are `F…` (findings.md) and decisions `D…` (decisions.md). A fix names the finding it closes.
- `src/Core/` must build as C++11 (`arm-none-eabi-gcc`, `-std=gnu++11`).

## 7. Known weak spots

- Stop rules read the flowmeter, which reads almost nothing below about 25 mL/min (F150, F194).
- The vent does not stop a sample. The O-ring unlocks after it anyway (F192).
- Two sample paths exist: `sample_process` (Pi line) and `run_sample` (console). They share the steps, not the wrapper.
- `fill_test` has its own pump loop, separate from `CtrlPump`. A change must be made in both.
- Legacy code still compiled: `maintenance.cpp` (`calibrate_DNA_pump`), `demo_sample_process`, `step_dive` (spool). See stage 2 §7b.
- Keys typed during an O-ring move are ignored (F190).
- Encoder coupling can slip, and software cannot see a shift of the whole table (F187, F191, F203).
