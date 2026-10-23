# Archived code

Code that is not in the firmware any more. Nothing here is compiled. Each item is kept for one of two reasons:

- **`reusable/`**: the logic may be useful again (a future feature, a hardware part that comes back, a campaign tool). Kept as it was when it was removed.
- **`superseded/`**: replaced by newer code. Kept to trace why the change was made, and to reuse parts of the logic if needed.
- **`snapshots/`**: whole files as they were before a clean-up, so any removal can be undone line by line.

Each item says where it came from (the last commit, or a snapshot). To bring something back: copy it into `src/` or `include/`, add the includes it needs, and build with `pio run -e due`. Then check the console commands that call it.

## reusable/

| Item | From | Removed | Why it was removed | Why keep it |
|---|---|---|---|---|
| `operation/DNA_shield_test.cpp.txt` | `Step_functions.cpp` | 2026-10-23 | No caller. Rotate to a slot and run the micro pump for the full 250 s, no air push | A timed shield test. Replaced in production by `step_DNA_shield()` |
| `operation/fill_DNA_shield_tube.cpp.txt` | `maintenance.cpp` | 2026-10-23 | Only a commented call. Panel-driven priming (RIGHT starts, LEFT stops) | A button-driven prime, if a panel is ever fitted. `prime_shield` does the same on the console |
| `operation/checkpoint-weighing/` | `Tests_mech.cpp` (`fill_test checkpoint=`) | 2026-10-23 | The pump restart after a checkpoint climbed past the cap (see its README). Replaced by manual weighing | The flow-versus-weight comparison for the flowmeter calibration (F6, F150). Read its README |
| `spool-procedures/test_1_depth_40m_direct`, `test_2_remplissage_container_1m` and `_40m`, `test_3_sterivex_1` and `_2` | `Tests.cpp` | 2026-10-23 | Spool procedures (`SPOOL_USE = false`), not in any command | Reuse when the spool returns (stage 3). `test_2` is the container-filling routine |
| `sample-bookkeeping/Sample.{h,cpp}` | `System/` | 2026-10-23 | Nothing included it. Its sample list and the next free filter place were replaced by the manifold slot states | Sample list, filter places and time display: a base for the campaign runner (stage 2 HO2/HO3) |
| `serial-device/Serial_device.{h,cpp}` | `Connectivity/` | 2026-10-23 | No object was ever created | A time-stamped line reader for the serial link. A base for the Pi or host link (stage 2 HO4) |
| `experiments/experiences.{h,cpp}` | `Misc/` | 2026-10-23 | Two experiment scripts (`cross_cont_exp`, `exp_explore_2905`), not called | Examples of full test sequences: useful when a new experiment is written |
| `utilities/ISR_encoder_z_signal.cpp.txt` | `Hardware/Encoder.cpp` | 2026-10-23 | Not attached to any interrupt. The Z index is not used | The Z-index handler, if an absolute encoder reference is ever needed |
| `utilities/reset_encoder.cpp.txt` | `Tests.cpp` | 2026-10-23 | No caller | Encoder reset helper |
| `utilities/slot_encoder_angle.cpp.txt` | `Hardware/Manifold.cpp` | 2026-10-23 | No caller | Table angle of a slot, as the encoder would read it. Handy for diagnostics |
| `utilities/timerStart.cpp.txt`, `timerStop.cpp.txt` | `Hardware/Timer.cpp` | 2026-10-23 | No caller | Hardware timer (TC) start and stop. Needed for a timer-driven pump or sampling |
| `utilities/timeToEpoch.cpp.txt`, `format_date_friendly.cpp.txt`, `get_system_state.cpp.txt` | `System/Settings.cpp` | 2026-10-23 | Used only by the removed `Sample` code | Clock conversion and a readable date for logs and records |
| `utilities/test_serial_device.cpp.txt` | `Tests.cpp` | 2026-10-23 | No caller | A serial echo test |

## superseded/

| Item | From | Removed | Replaced by | Notes |
|---|---|---|---|---|
| `pressure-sensor-A2/read_pressure2`, `calibrate_pressure2_zero` | `main.cpp`, `Tests.cpp` (last commit) | 2026-10-12 (user), before the clean-up of 2026-10-23 | The ABP sensor on SPI D8 (`pressure1`, `read_p_in()`) | The analog sensor A2 was removed from the machine |
| `pressure-ramp/ramp_soft_limit`, `test_soft_limit` | `Core/pressure_ramp.cpp`, `test/test_pressure_ramp` (last commit) | Before 2026-10-23, from the working tree (reason not logged) | The cap check on every pass and the pause at 3.35 bar | The setpoint shaping idea for stage 3 (P2-6). See its README |
| `power-ramp/power_ramp.{h,cpp}`, `test_main.cpp` | `src/Core/`, `test/test_power_ramp/` (last commit) | Before 2026-10-23 (the power-ramp controller failed on the bench, LOG S1-1b) | `pressure_ramp` and the PID loops (200 / 100 / 100) | The soft-limit and rise-rate limiters may be reused in stage 3 (P2-6 layered controller) |
| `operation-panel/SIC_control.cpp.txt` | `main.cpp` | 2026-10-23 | `run_sample` on the console | The panel sampling (START, LEFT, RIGHT). Its call was commented out in `loop()` |
| `multi-sample-old/test_multi_sample_loading.cpp.txt` | `Tests.cpp` | 2026-10-23 | `multi_sample` in `Tests_mech.cpp` | The old branch was shadowed by the new command |
| `reusable/manifold-flush/purge_pipes_manifold.cpp.txt`, `pi-purgePipesFunction.py.txt` | `maintenance.cpp` (`purge_pipes_manifold`, `run_pump`), `main.cpp`, `computer_interface/` | 2026-10-23 (user) | None (the operator never used it) | Fills the container, then purges all 15 manifold slots in turn (27 s each). A maintenance flush after the realignment could reuse it, with the cap polled (F202). The Pi button is in the .py.txt |
| `tests/test_pump_ramp_settings_window.cpp.txt` | `test/test_pump_ramp/` | 2026-10-23 | None | Checked a constant against an old request, not behaviour |

## snapshots/

| Item | What it is |
|---|---|
| `snapshots/2026-10-23-before-removal/` | Five files as they were before the clean-up of 2026-10-23 (`main.cpp`, `Tests_mech.cpp`, `Filter_watch.cpp`, `Filter_watch.h`, `Settings.h`). Use them to undo a removal line by line |

## Also removed by the clean-up, not archived

Single lines only, listed in the log: the `#include` and `extern` lines of `Serial_device`, `experiences` and `Sample` in four files; the commented `fill_DNA_shield_tube()` call and the commented `SIC_control()` call.
