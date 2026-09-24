# CoWaS command list (firmware as of FW0)

Every way to make the firmware do something today: the serial test menu, the button panel, and the legacy Raspberry Pi text protocol. It was generated in FW0 from the code at commit `d508f41`; line numbers are for the FW0 tree. The legacy Pi commands are mapped to protocol v1 in FW1 (`docs/PROTOCOL.md`).

## How the three interfaces are reached

```
setup()  ── ends in test_all_components()  (Tests.cpp:75): the serial TEST MENU, an endless loop
   │          └─ type `stop` ──► setup() returns
   ▼
loop()   every 50 ms: button_control() (main.cpp:357)  +  main_program() (main.cpp:250)
                       = BUTTON PANEL                        = LEGACY PI PROTOCOL
```

- **The Pi protocol and the button panel only work after someone types `stop` in the test menu.** After any reset the machine waits in the menu (F23).
- **All three share one port:** `Serial` (Programming port, 9600 baud). Both the menu and `main_program()` read with `readStringUntil('\n')` (1 s timeout) and compare **exact** strings.
- **Line endings:** a trailing `\r` breaks every exact-match command (`abort\r` ≠ `abort`). Send **LF only**: `pio device monitor` does this with the settings in platformio.ini (`monitor_eol = LF`, send on ENTER). Prefix commands (`slotN`, `clockMM`) happen to tolerate `\r`.
- The menu echoes each line. Unknown commands are silently ignored. Several `if`s are not `else if`, but no two exact names collide.

**Column meanings**

- **Hardware touched:** P = diaphragm pump (DAC1 + relay D42), µP = micro pump (DNA shield, D48), V1 / V23 / Vman = valves (D44 / D40 / D46), M = manifold motor (VNH5019 M2) + AMT22 encoder, L = O-ring linear actuator (A4988), S = spool (disabled), F = flowmeter (D13), P1 / P2 = pressure sensors (SPI D8 / analog A2), BTN = panel buttons, LED.
- **Blocking?:** how long the call keeps the loop (and so every other interface) busy, and how to get out.
- **Needs P?:** does it depend on the pressure reading? Today the pump safety cap and all PID loops read **`pressure2` on A2, whose sensor was removed** (F7/F22). **No pressure sensor is installed at all** (F50), so the cap cannot trip. Every big-pump start checks that cap, so with a floating A2 the pump can refuse to start or be cut at random (F17). "cap" = only the start check / cap; "PID" = pressure-controlled loop.
- **Rotates while locked?:** marked **F5** when the command rotates the manifold without unlocking the O-ring first. The lock state is RAM-only and `false` at boot (F18).

## 1. Serial test menu: `test_all_components()` (Tests.cpp:75)

| Command | Function (file:line) | Hardware touched | Blocking? | Needs P? | Notes |
|---|---|---|---|---|---|
| `abort` | `abort_sample()` Step_functions.cpp:712 → `step_purge`, `step_rewind` (skipped), `step_empty`, `unlock_oring` | M, L, V1, Vman, P, F | yes: purge ≤ 3 min + empty ≤ 5 min + moves | yes (PID) | Ends unlocked |
| `temp` | inline loop, Tests.cpp:97-129 | M | **forever**: never leaves; only a reset gets out | no | Debug: typed float → `angle_offset_pos`, jogs M until a key, then `rotateMotor(0)` (F5). A `0` prints "zerrooooooooooo" |
| `sample1m` | `sample_process(80)` Step_functions.cpp:498 after a START press | M, L, V1, V23, Vman, P, µP, F | yes: whole cycle (purge, sample, shield, empty) | yes (PID) | Takes the next free slot in `FILL_ORDER` (RAM state). Depth only printed (dive disabled) |
| `demo` | `demo_sample_process()` Step_functions.cpp:585 | M, L, V1, V23, Vman, P, µP, F, BTN, LED | yes: waits for START between steps | yes (PID) | No `step_empty` |
| `cal_DNA` | `calibrate_DNA_pump()` maintenance.cpp:93 | M, µP, P, V23, Vman, V1, BTN | yes: until the RIGHT button at the end | cap | Button-driven. `rotateMotor(1)` first (F5). Pump at 11–25 % |
| `valves` | `test_valves()` Tests.cpp:1696 | V1, V23, Vman | 30 s (2 × 3 valves × 5 s) | no | Toggles each valve twice. V23 is physically removed: only its relay clicks (F51) |
| `motor_spool` | `test_motor_spool()` Tests.cpp:1678 | (S) | no | no | Skipped: `SPOOL_USE = false` |
| `motor_manifold` | none (prints only) | — | no | no | Placeholder |
| `encoder_manifold` | `test_encoder()` Tests.cpp:1803 | M | until any key | no | **Spins M continuously** (speed 30) while printing the encoder (F5) |
| `enc_rot` | `getRotationSPI()` Manifold.cpp:449 | M (encoder only) | no | no | Prints the AMT22 turn counter |
| `encoder_spool` | `test_encoder_spool()` Tests.cpp:1823 | spool encoder D31/33/35 | until any key | no | |
| `manifold` | `test_manifold()` Tests.cpp:773 | M | ~1–2 min: slot 0 ↔ 1..14 with 1 s + 3 s pauses | no | No abort (only a reset). F5 |
| `pressure_sensors` | `test_pressure_sensor()` Tests.cpp:751 | P1, P2 | until any key | reads both | Prints every ~1 s |
| `pressure` | `print_pressure_once()` Tests.cpp:686 | P1, P2 | no | reads both | As of FW0 neither sensor is installed: P1 prints an SPI error + 0.00, P2 a floating value (F50) |
| `calibrate_pressure2` | `calibrate_pressure2_zero()` Tests.cpp:711 | P2 | ~1 s after a key; `x` aborts | reads P2 | **Bug:** uses `OLD_OFFSET = 0.483` (Tests.cpp:744), but `Pressure_sensor.cpp:10` has `OffSet = 3.105`, so the printed offset is wrong. P2 is not installed anyway (retired in FW3) |
| `pump` | `test_pump()` Tests.cpp:807 | P, Vman | until any key | cap (polled every 20 ms) | Asks for a power 1–100 %. Opens Vman. No timeout |
| `micro_switch` | `test_micro_switch()` Tests.cpp:1762 | D27, D28, D29 | until any key | no | |
| `buttons_command` | `test_command_box()` Tests.cpp:1711 | BTN, LED | until any key | no | |
| `container` | none (prints only) | — | no | no | Placeholder |
| `spool` | `test_1_depth_20m()` Tests.cpp:361 | (S) | no | no | Skipped: `SPOOL_USE = false` |
| `40m` | `test_1_depth_40m()` Tests.cpp:436 | (S) | no | no | Skipped: `SPOOL_USE = false` |
| `stop` | `break` Tests.cpp:233 | — | — | — | **Leaves the menu** (setup() returns): the panel and the Pi protocol start. There is no way back without a reset. Not an emergency stop |
| `cal` | `calibrateEncoder()` Manifold.cpp:389 | M | until keys (`c`/other = direction, then any key stops) | no | Prints a new `purge_angle` to paste into Manifold.h. F5 |
| `zero` | `go_to_zero()` Tests.cpp:1841 | M | until the raw encoder reads 0 ± 5° | no | **No timeout and no encoder-failure check:** spins forever if the encoder fails. F5 |
| `slot0` | `rotateMotor(0)` Manifold.cpp:109 | M | ≤ 15 s (`MANIFOLD_ROTATE_TIMEOUT_MS`) | no | Purge slot. F5 |
| `slotN` (prefix `slot`, N = 1..14) | `rotateMotor(N)` Tests.cpp:261-273 | M | ≤ 15 s | no | Any other text starting with `slot` (e.g. `slot`, `slot15`, `slotx`) prints the usage line. F5 |
| `clockMM` (prefix `clock`, except `clocklist`) | `slot_for_clock_minutes()` Manifold.cpp:206 → `rotateMotor()`, Tests.cpp:277-290 | M | ≤ 15 s | no | Unknown minute → error message. **Bare `clock` or `clockxx` parses as minute 0 and rotates to slot 7** (clock 00, `MANIFOLD_RAW_INDEX_INCREASES_CW = true`). F5 |
| `clocklist` | inline, Tests.cpp:292 → `clock_minutes_for_slot()` Manifold.cpp:198 | — | no | no | Slot 0 = clock 30, slot 7 = clock 00 (see golden values in `test/test_manifold_geometry/`) |
| `micro_pump` | inline, Tests.cpp:300 | µP | 10 s (`delay`) | no | |
| `linear_actuator` | `test_linear_actuator()` Tests.cpp:992 | L | until `x` | no | Sub-commands: `f`/`b` = 1 turn forward/back (any key aborts), `e` enable, `s` disable, `p` position, `v<n>` speed, `x` exit. **Does not update the lock state** (`is_oring_locked()`) |
| `sample_cycle` | `test_load_sample()` Tests.cpp:1080 → `load_slot(500)` Step_functions.cpp:356 | L, V1, Vman, P, F | ≤ 3 min water + ≤ 60 s air, after a key (`x` aborts before start) | yes (PID) | At the current slot, no rotation. Lock → water → air → unlock |
| `sample_cycle_full` | `test_load_sample_full()` Tests.cpp:1150 → `load_slot_full(slot)` Step_functions.cpp:413 | M, L, V1, Vman, P, µP, F | whole sample + shield, after the slot number (`x` aborts) | yes (PID) | `step_sampling` + `step_DNA_shield` + unlock. Uses `STX_SAMPLE_MILLILITERS` (100 mL today) |
| `multi_sample` | `test_multi_sample_loading()` Tests.cpp:1105 → `load_slot(50)` per slot | M, L, V1, Vman, P, F | per slot, waits for a key between slots (`x` aborts) | yes (PID) | Every available slot in `FILL_ORDER`; unlocks before each rotation |
| `multi_sample_full` | `test_multi_sample_loading_full()` Tests.cpp:1183 → `load_slot_full()` per slot | M, L, V1, Vman, P, µP, F | per slot, key between slots (`x` aborts) | yes (PID) | |
| `calibrate_actuator` | `calibrate_linear_actuator()` Tests.cpp:1310 | L | interactive (`x` aborts) | no | Prints `LINEAR_ACT_LOCKED_STEPS` (today a PLACEHOLDER, 210). Does not update the lock state |
| `calibrate_flow` | `calibrate_flow_sensor()` Tests.cpp:1384 | P, F | until target volume / 5 min / any key | cap (at start only) | **Doesn't set any valve**, and the pump loop doesn't poll the cap (only `pump.start()` checks it). Prints `calibrationFactor_small` |
| `calibrate_dna_volume` | `calibrate_dna_pump_volume()` Tests.cpp:1484 | µP | T ms (`delay`) + prompts (`x` aborts) | no | Prints mL/s and the run time for a target volume |
| `calibrate_all` | `run_commissioning_calibration()` Tests.cpp:1638 | M, L, P, F, µP, BTN | interactive (y/n per stage) | cap (flow stage) | Chains `cal`, `calibrate_actuator`, `calibrate_flow`, `cal_DNA`, `calibrate_dna_volume` |
| `manifold_direction` | `identify_manifold_direction()` Tests.cpp:1566 | M | 0.5 s move + key + ≤ 3 s return | no | F5 |

Functions in Tests.cpp that no command reaches: `test_1_depth_40m_direct`, `test_2_remplissage_container_1m/40m`, `test_3_sterivex_1/2`, `test_serial_device`, `reset_encoder`.

## 2. Button panel: `button_control()` (main.cpp:357)

Active only in `loop()`, i.e. after `stop` in the menu. **START** (D24) cycles the mode (main.cpp:520) and prints it. The first mode after boot is **Pump**. Every press blocks in `waitPressedAndReleased()` until the button is released.

| Mode (index) | Function (file:line) | LEFT | RIGHT | Hardware touched | Blocking? | Needs P? | Notes |
|---|---|---|---|---|---|---|---|
| Pump (0) | main.cpp:367 | power +10 % (wraps above 100 back to 20); applied live if running | start (open Vman, pump at `pump_power`, 20 % at boot) / stop (close Vman) | P, Vman | no (until release) | cap at start only | **F2:** the cap is not polled while the pump runs in this mode. No timeout |
| Spool (1) | main.cpp:467 | prints "Spool disabled" | prints "Spool disabled" | — | no | no | `SPOOL_USE = false` |
| V23 (2) | main.cpp:403 | L way | I way | V23 | no | no | Valve physically removed, relay only (F51) |
| V1 (3) | main.cpp:417 | close | open | V1 | no | no | |
| Vman (4) | main.cpp:431 | close | open | Vman | no | no | |
| Man Slot (5) | main.cpp:445 | previous slot (0 → 14) + `rotateMotor` | next slot (mod 15) + `rotateMotor` | M | ≤ 15 s per move | no | **F5.** The slot counter starts at 0 at boot, whatever the real position. Hard-coded 15 / 14 (F16) |
| Micro Pump (6) | main.cpp:501 | stop | start | µP | no | no | Runs until LEFT is pressed; no timeout |

`SIC_control()` (main.cpp:531) is another button handler, commented out in `loop()`.

## 3. Legacy Pi protocol: `main_program()` (main.cpp:250)

Sent by `computer_interface/cowas_loop.py` over the same `Serial` port at 9600 baud. The Due answers every line, known or not, with ` - function accomplished - <line>` **after** the command has finished. The Pi takes the first line it receives as "done", but log lines come first (F24).

| Command | Function (file:line) | Hardware touched | Blocking? | Needs P? | Notes |
|---|---|---|---|---|---|
| `reloadManifoldFunction` | `manifold.reload()` Manifold.cpp:102 | — (RAM) | no | no | Marks every slot available again (F1) |
| `purgeSterivexFunction` | `step_purge(true)` Step_functions.cpp:220 | M, L, V1, Vman, P, F | rotation + lock + ≤ 3 min pumping | yes (PID) | `stop_pressure` is unused. **Leaves the O-ring locked** at the purge slot |
| `purgeContainerFunction` | `step_purge()` Step_functions.cpp:220 | same | same | yes (PID) | Same as above |
| `purgePipesFunction` | `purge_pipes_manifold()` maintenance.cpp:54 | BTN, M, V1, V23, Vman, P, F | **waits for the START button**, then fill (≤ 11 min) + 15 × (rotate + 27 s pump) + purge | cap + PID | Needs a person at the machine. `rotateMotor` without unlocking (F5): a prior purge leaves it locked |
| `fillContainerFunction` | `step_fill_container()` Step_functions.cpp:126 | V1, V23, P | until D27 reads 0 or 11 min (`FILL_CONTAINER_TIME`) | cap at start only | The container is gone, so it normally runs the full 11 min at 100 %. The fill loop doesn't poll the cap |
| `sample…` (prefix `sample`) | `sample_process(depth*100)` Step_functions.cpp:498 | M, L, V1, V23, Vman, P, µP, F | whole cycle: minutes (the pump phases' own caps add up to ~13.5 min) | yes (PID) | Expected `sampleFunction<d>` with d = 1–2 digits (m), parsed by fixed positions 14–15. **Any other line starting with `sample` prints "Error depth" and still runs a full sample** (depth 0). Depth is only printed (dive disabled). The Pi sends it without `\n` (F24) |
| `rollingFunctionUp` | main.cpp:299 | (S) | no | no | Prints "Spool disabled" |
| `rollingFunctionDown` | main.cpp:312 | (S) | no | no | Prints "Spool disabled" |
| `stopRolling` | main.cpp:325 | (S) | no | no | No-op while the spool is disabled |
| `startDNA` | `micro_pump.start()` Micro_pump.cpp:38 | µP | no | no | Runs until `stopDNA`; no timeout |
| `stopDNA` | `micro_pump.stop()` Micro_pump.cpp:70 | µP | no | no | |

## What production-relevant commands leave behind

| Command | O-ring at the end | Valves at the end |
|---|---|---|
| `sample_process` (`sample…`, `sample1m`) | unlocked | V1 closed, V23 off, Vman closed |
| `step_purge` (`purge…Function`) | **locked** at purge | V1 closed, Vman closed |
| `load_slot` / `load_slot_full` (`sample_cycle*`, `multi_sample*`) | unlocked | V1 closed |
| `abort` | unlocked | set by the last step |
