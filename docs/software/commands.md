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
| `session` | Asks site, date, experiment, operator, and whether the spool is on the machine (y/n). With `y`, asks the session's deepest depth in m (ENTER = 40). | Sets the purge volume and the purge timeout for the session. The deepest depth is the dive point of every fill (2026-11-02). |
| `run_sample <slot> [depth=m]` | One full sample: purge, sample through the filter, DNA shield, unlock. `depth=` (spool answer `y` in `session`): dive first, roll up after the shield. | Asks sample name, condition, notes. With `depth=` it ends by asking `spool: next sample depth` (ENTER = roll up; a depth = the tube stays out and the next run goes straight there). Refused if `depth=` is above the range or the spool answer is `n`. The total tube (depth + spool height) never goes beyond 40 m: a deeper depth is cut there. Cut or deeper than the session's deepest point: a warning and a question (`y` = proceed to the cut depth, anything else = stop). After the roll up, the spool tubing is emptied unless the next sample is at the same timepoint (`spool_emptied=`). A failed sample is rolled up and emptied the same way (after an `x` the emptying is skipped: `spool_emptied=NO`); the next sample fills the tube again at the session depth. Before the purge the tube is filled and risen to the depth (`spool_fill_and_rise`: dive to the session depth if dry, flow check at the purge slot with the pump, then roll up with the deployment valve shut). Takes about 5–8 min without the spool, about 20 min with it plus the spool moves. Prints `STEP TIMES` and the `TEST` line. Prints a `WARNING` first if no PC session log is running (its record is then not saved). |
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
| `ethanol_purge [skip=3,7,12]` | Ethanol cleaning of the tubing (user 2026-11-02). **No filter installed**, inlet and outlet in 70 % ethanol. Prints a warning, asks which of slots 1-14 were unused (or reads them after `skip=`; numbers separated by commas or spaces, ranges like `3-5`, `none`), then y/n to start. Prime through the purge slot (3 L with the spool on in `session`, else 0.5 L), 0.5 L through each used slot in `FILL_ORDER`, all at **full power** (second exception to the 60 % water limit). Then pushes air 30 s through each used slot, and 1 min at the purge slot, all by the air valve V1 (no question, the tubes stay in the ethanol; the intake line before Vman is not emptied). | Needs the pressure sensor. The cap, the timeouts, `x` (abort) and `s` (skip a run) stay active. A cap, sensor, abort or rotation failure stops everything (pump off, valves closed, O-ring locked); a skip or an air run that never reached air level only warns. The flowmeter reads low at slow flow (F150), so the real volume is not below the indicated one. Settings: `CLEAN_*` in Settings.h. Ethanol needed: prime + 0.5 L per slot (about 10 L with the spool, 7.5 L without). |
| `oring_lock`, `oring_unlock` | Locks or unlocks the O-ring. | Refuse while UNKNOWN. |
| `lock_cycle`, `lock_endurance` | Repeats lock and unlock for timing or wear. | |
| `rotate_all`, `encoder_repeat` | Checks every slot, or one slot repeatedly, for alignment. | |
| `leak_hold`, `pressure_ramp`, `fill_test` | Pressure tests: a set level, a ramp, or a filter run with stop rules. | `fill_test` is the filter test. Results are on the `TEST` line. |
| `multi_sample`, `sample_cycle`, `sample_cycle_full`, `multi_sample_full` | Older sample loops: per slot, with or without the shield. | Use `run_sample` in the field. |
| `pump` | Runs the main pump at a power you type (1–100 %). | **No timeout.** Stopped by a key. |
| `shield_only`, `prime_shield` | Runs the shield pump alone; `prime_shield` fills the line until a key. | Time-limited. |
| `fill_DNA` | Fills the DNA shield tube. | To be done when DNA shield is loaded before an experiment. |

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
| `spool_status` | Position, height (also sent to the PC as `META|spool;height_cm=`), drum diameter, switches, encoder pulses. | |
| `spool_height [cm]` | Distance from the top switch to the water surface. | RAM only: again after a reset. |
| `spool_home [drain=n]` | Rolls up to the top switch. `drain=y` (default) goes to the purge slot and opens V1 and Vman so the tube empties, then unlocks. | Needs the O-ring state known unless `drain=n`. Stops on stall, wrong way, a time limit of 1 min + distance at 1 cm/s (`SPOOL_MIN_CM_PER_S`), `x`. Each move logs its speed. |
| `spool_goto <m>`, `spool_goto length=<m>` | Intake depth (adds the height), or tube length from the top. | Same limits. 40 m maximum (`SPOOL_MAX_LENGTH_CM`); only 0 to 15 m is calibrated. |
| `spool_calibrate` | Dry: tube out to 2, 5, 10, 15 m, you type the tape reading, prints the drum diameter. | `y` applies it until the next reset. |
| `motor_spool`, `spool`, `40m`, `encoder_spool` | Old spool tests (`spool` and `40m` now go through the safe move, limited to 40 m). | Bench only. |

## 4. Legacy Pi protocol (after `stop`)

Sent by `computer_interface/cowas_loop.py` at 9600 baud. Not used by the operator.

| Command | What it does |
|---|---|
| `purgeSterivexFunction`, `purgeContainerFunction` | Purge. Leaves the O-ring **locked**. |
| `fillContainerFunction` | Fills the container (the container is gone). Runs up to 11 min. |
| `sample<depth>` | Full sample. Depth is printed only. |
| `startDNA`, `stopDNA` | Micro pump on and off. No timeout. |
| `reloadManifoldFunction` | Marks all slots available again. |
| `rollingFunctionUp`, `rollingFunctionDown`, `stopRolling` | Spool, plain motor on/off (no encoder limit; the end switch stops the roll up). |

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
| The spool has never run with this code. Switch polarity, encoder direction and speed are unmeasured | spool | F211 |
| The spool position is RAM only: unknown after a reset, the next move rolls up first | spool | F211 |

## 8. Removed

Removed on 2026-10-23 (code in `docs/archive/code/`): `test_1_depth_40m_direct`, `test_3_sterivex_1/2`, `reset_encoder`, `test_serial_device`, `test_multi_sample_loading`, the panel's `SIC_control()`, and `purgePipesFunction`.
