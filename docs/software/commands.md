# Commands

What the firmware does when you type a command. Version: stage 1, 2026-10-23. History of each change is in [LOG.md](../logs/LOG.md).

## How to talk to the Due

- **Serial menu:** the machine starts in it. Type a command and press ENTER (LF only). Unknown commands are ignored.
- **Leave the menu with `stop`.** Then the panel and the Pi protocol start. There is no way back without a reset. `stop` is not an emergency stop.
- **Any reset** puts the machine back in the menu. Run `session` and `actuator_home` again.
- **During a pump run:** `s` + ENTER skips the current pump step. `x` + ENTER aborts the whole command. Other typed lines are ignored.
- **Safety:** the pressure cap (3.6 bar) and the 24 V switch are the only pressure protection. Someone stays at the switch.

## 1. Operator commands (the field)

| Command | What it does | Notes |
|---|---|---|
| `actuator_home` | Finds the O-ring position and unlocks it. | Needs the manifold within 3° of a slot. Ends UNLOCKED. Run after every reset. `x` aborts (the lock state is then UNKNOWN). |
| `oring_state` | Prints LOCKED, UNLOCKED or UNKNOWN. | Read only. |
| `session` | Asks site, date, experiment, operator, and whether the spool is on the machine (y/n). | Sets the purge volume and the purge timeout for the session. |
| `run_sample <slot>` | One full sample: purge, sample through the filter, DNA shield, unlock. | Asks sample name, condition, notes. Takes about 5–8 min without the spool, about 20 min with it. Prints `STEP TIMES` and the `TEST` line. Prints a `WARNING` first if no PC session log is running (its record is then not saved). |
| `log_on` | Says the PC session log is running (sent by `tools/session_log.py` at its start). | Prints `META\|log;attached=yes`. Clears the `run_sample` warning until the next boot. Not a test. |
| `full_cycle`, `full_all` | Same steps as `run_sample`, with options. | For tests. Not the operator path. |

## 2. Checks and small moves

| Command | What it does | Notes |
|---|---|---|
| `slot0` … `slot14`, `slotN` | Turns the manifold to a slot. | Needs UNLOCKED. Up to 15 s. `slot0` is the purge slot; slot 7 is the reference. |
| `clockMM`, `clocklist` | Turns to the slot at a clock minute; lists the slots. | `clock` alone prints the usage. |
| `pressure` | Prints the pressure reading. | Read only. |
| `abp_raw` | Prints the raw pressure sensor frames for 4 s. | Read only. Use it after any rewiring. |
| `pressure_zero` | Sets the pressure zero (pump off, open to air). | Prints the value to paste into `Settings.h`. |
| `enc_rot` | Prints the manifold encoder angle. | Read only. |
| `encoder_manifold` | Spins the manifold and prints the encoder until a key. | **Turns the rotor continuously.** |
| `manifold` | Walks slot 0 to 14. | About 1–2 min. No abort except a reset. |
| `valves`, `valve_check` | Toggles the valves. | V23 is removed: only its relay clicks. |
| `micro_pump` | Runs the DNA-shield pump for 10 s. | |
| `micro_switch`, `buttons_command` | Tests the spool switches and the panel buttons. | Until a key. |
| `linear_actuator` | Moves the O-ring actuator by hand (`f`, `b`, `e`, `s`, `p`, `v<n>`, `x`). | Does not update the lock state. |
| `oring_lock`, `oring_unlock` | Locks or unlocks the O-ring. | Refuse while UNKNOWN. |
| `lock_cycle`, `lock_endurance` | Repeats lock and unlock for timing or wear. | |
| `rotate_all`, `encoder_repeat` | Checks every slot, or one slot repeatedly, for alignment. | |
| `leak_hold`, `pressure_ramp`, `fill_test` | Pressure tests: a set level, a ramp, or a filter run with stop rules. | `fill_test` is the filter test. Results are on the `TEST` line. |
| `multi_sample`, `sample_cycle`, `sample_cycle_full`, `multi_sample_full` | Older sample loops: per slot, with or without the shield. | Use `run_sample` in the field. |
| `pump` | Runs the main pump at a power you type (1–100 %). | **No timeout.** Stopped by a key. |
| `shield_only`, `prime_shield` | Runs the shield pump alone; `prime_shield` fills the line until a key. | Time-limited. |

## 3. Calibrations (once, on the bench)

| Command | What it does | Notes |
|---|---|---|
| `cal` | Finds `purge_angle` (the slot 7 reference). | Paste the printed value into `Manifold.h`. Rotor on slot 7. |
| `zero` | Turns the rotor to the raw encoder zero. | **No timeout.** Spins forever if the encoder fails. |
| `calibrate_actuator` | Finds the O-ring lock position (`LINEAR_ACT_LOCKED_STEPS`). | Interactive, `x` aborts. |
| `calibrate_pressure` | Pressure gain and zero against a manometer. | The pump is never started. |
| `calibrate_flow` | Flowmeter factor: pumps a known volume, you enter the measured one. | Runs under the pressure cap. |
| `calibrate_dna_volume` | Shield pump rate for a target volume. | |
| `shield_calibrate` | Shield dose: three runs per duration, weighed. Prints `DNA_SHIELD_FILL_MS`. | Procedure: [lab/shield-protocol.md](../lab/shield-protocol.md). Production does not read the printed time yet. |
| `cal_DNA` | DNA pump calibration, button driven. | **Its pump run has no cap check** (F202). Do not use until fixed. |
| `calibrate_all` | Runs the calibrations in order, asking y/n for each. | |
| `manifold_direction` | Checks the turning direction. | |
| `motor_spool`, `spool`, `40m`, `encoder_spool` | Spool tests. | The spool is disabled (`SPOOL_USE = false`). They do nothing useful. |

## 4. Legacy Pi protocol (after `stop`)

Sent by `computer_interface/cowas_loop.py` at 9600 baud. Not used by the operator.

| Command | What it does |
|---|---|
| `purgeSterivexFunction`, `purgeContainerFunction` | Purge. Leaves the O-ring **locked**. |
| `fillContainerFunction` | Fills the container (the container is gone). Runs up to 11 min. |
| `sample<depth>` | Full sample. Depth is printed only. |
| `startDNA`, `stopDNA` | Micro pump on and off. No timeout. |
| `reloadManifoldFunction` | Marks all slots available again. |
| `rollingFunctionUp`, `rollingFunctionDown`, `stopRolling` | Spool. Disabled. |

## 5. Button panel (after `stop`)

START cycles the mode. LEFT and RIGHT act on the current mode.

| Mode | LEFT | RIGHT |
|---|---|---|
| Pump | power +10 % (wraps above 100) | start or stop the pump |
| Valves V1, Vman | close | open |
| Man Slot | previous slot | next slot |
| Micro Pump | stop | start |

The slot counter starts at 0 at boot, whatever the real position.

## 6. Refusals to know

- **Rotation** (`slotN`, `clock`, `cal`, `zero`, panel Man Slot): refused unless the O-ring is UNLOCKED. Message `REFUSED | ...`.
- **Pressure-using commands** refuse when the sensor is not available. The sensor is available now.
- **Lock state after a reset is UNKNOWN.** Commands that need it refuse until `actuator_home`.

## 7. Known gaps

| Gap | Where | Ref |
|---|---|---|
| `cal_DNA` pump run has no cap check | `cal_DNA` | F202 |
| Stop rules read the flowmeter, which reads almost nothing below ~25 mL/min | `fill_test`, `run_sample` | F150, F194 |
| The vent does not stop a sample; the O-ring unlocks after it anyway | `run_sample` | F192 |
| Keys typed during an O-ring move are ignored | O-ring | F190 |
| Shield dose is timed, not measured | `run_sample`, shield | F156 |
| `temp`, `zero`, `pump` have no timeout (`temp` never leaves the menu) | menu | |

## 8. Removed

Removed on 2026-10-23 (code in `docs/archive/code/`): `test_1_depth_40m_direct`, `test_3_sterivex_1/2`, `reset_encoder`, `test_serial_device`, `test_multi_sample_loading`, the panel's `SIC_control()`, and `purgePipesFunction`.
