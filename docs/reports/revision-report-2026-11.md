# Revision report: the auto sampler, August – November 2026

## Summary

- **Starting point:** the prototype as it was before August 2026 (Arduino Due, rotating manifold with Sterivex filters, diaphragm pump, DNA Shield micro pump, flowmeter, pressure sensor). Its hardware drivers and general sequence of steps were kept.
- **Aim:** an auto sampler that runs unattended for days, produces a documented sample each time, and can keep being improved without risk: a new seal, pressure-based pump control, defined end rules and a record for every sample, a schedule, a depth spool routine, and a test and documentation base.
- **Result:** two field works (26 Oct – 6 Nov) gave 66 filters from the auto sampler, including two 24 h unattended series on Lake Geneva, with no seal leak, no pressure above the limit, and no failure of the sampling program.

## 1. Scope at a glance

| | Before August 2026 | Revised version (Nov 2026) |
|---|---|---|
| Firmware | 48 files, about 5,600 lines | 107 files, about 16,600 lines (58 new files, 26 of the original 48 changed) |
| Unit tests | None | 221 firmware tests in 22 suites, plus 364 tests for the host program and 235 for the tools |
| Computer side | Console text | Host program (35 files) with a message protocol and simulator; console tool with a session log |
| Per sample | Console messages | A record: volumes, end reason, pressure, step timings, flags |
| Operation | Commands one at a time | Timed schedule, sample book, one command per sample |
| Documentation | – | Operator tutorial, field and manual protocols, lab protocols, pin map, 51 documented commands, findings and decisions registers |

## 2. How the code is organised, and why

The aim was a code base that can be improve safely with future improvements of the auto sampler.

- **Logic separated from hardware and unit-tested.** Calculations and rules (slot geometry, pressure conversion and calibration, stop rules, spool moves, DNA Shield timing, step log) are in their own files without hardware code. 221 unit tests check them on a computer in seconds, so a change is tested before it is flashed. When maths was moved, its previous outputs were recorded and checked bit for bit.
- **One place per safety rule.** Every pump run goes through one controller that holds the pressure limit; every rotation goes through one function that refuses to turn while the seal is locked or its state unknown. A new feature uses these and cannot bypass them.
- **Fixed safety rules:** the pump never runs above the limit, an implausible sensor stops it, every procedure has a time limit, the stop key is always accepted, and a stop leaves pumps off, valves closed and the seal locked.
- **Traceability:** settings in one file with unmeasured values marked as placeholders; every change built and tested automatically; every change logged with its evidence (187 findings, 109 decisions) and the bench and field transcripts kept.

## 3. What changed, area by area

**Seal and manifold mechanics**
- *Before:* a single O-ring seal at the manifold and no motorised lock.
- *Now:* a linear actuator (stepper and lead screw) locks the O-ring to seal and unlocks it to rotate. The lock has three states (locked, unlocked, unknown); the state is unknown after every restart until it is checked. New printed and machined parts: O-rings sealing the PTFE rotor, a concentric double O-ring to reduce leaks, metal inserts to avoid contact with leakable 3D printed parts, a sensor adapter, a reprinted outlet. The purge port is at the bottom of the manifold and the slots fill from the bottom up, so a leak can only reach slots already filled. A force sensor was tried for the lock and removed; the lock runs open-loop from a known reference.
- *Why:* the manifold must turn freely and seal tightly. A motorised lock makes this repeatable, and lets the firmware refuse any rotation while the seal is locked.

**Pressure sensor**
- *Before:* the reading detected the end of a purge or a sampling (low-pressure threshold). The sensor was read at 800 kHz.
- *Now:* the bus runs at 100 kHz after bench tests showed incorrect frames at 800 kHz; every frame is checked; gain and offset are calibrated against a manometer. A faulty frame, no answer or 500 ms of stale readings counts as an implausible reading: the pump stops and does not start.
- *Why:* the pump limit and the control loops rely on this reading, so it has to be calibrated, validated and monitored.

**Pump control and pressure limit** (detail in section 4, trials in appendix E)
- *Before:* fixed power levels (90 % pumping water, 80 % flushing), runs ended by timers, a threshold or a button.
- *Now:* the pump is regulated on the line pressure, with margins that pause it before the limit (3.35 bar) and a limit that stops it (3.6 bar).
- *Why:* a Sterivex filter changes resistance as it clogs. Regulating on pressure keeps the flow as high as the filter allows while staying inside the limits. The limit was raised step by step (2.5, 3.0, 3.2, 3.5, then 3.6 bar) as bench tests confirmed what the filter and seal withstand: a Sterivex filter takes at least 3 bar, and a lower pressure gives little flow and clogs the filter too fast.

**End of a sample** (section 4)
- *Before:* the water was collected in a container for a fixed time, then pushed through the filter until a low-pressure threshold or a time limit.
- *Now:* the volume is counted at the filter by the flowmeter: 1000 mL, or an earlier stop on defined clogging, stalled-flow and pressure criteria; the end reason is recorded; the volume is recorded twice (flowmeter total and confident volume).
- *Why:* the lab needs a defined and recorded volume per sample. Lake water clogs filters at very different rates, so a stop on volume or on clogging gives a valid sample either way, and the recorded end reason lets the lab interpret the volumes.

**Records and operation**
- *Before:* commands typed one at a time, with console text.
- *Now:* a record per sample; a console tool that saves every session with the computer's time; a timed schedule that reads back the start list, retries a failure before the filter and stops after a failed step; a sample book of used and free slots; a run-report tool.
- *Why:* an unattended run needs a plan the machine follows, and a record that shows afterwards what happened.

**Depth spool**
- *Before:* simple dive and roll-up steps within the sample sequence.
- *Now:* each sample unrolls the tube to the deepest point so that it fills with lake water, primes the pump until water flows, rises to the sample depth, and after the sample the tube is rolled up and emptied. End-stops, stall and wrong-way detection, a time limit on every move, a 40 m limit on the total tube and a log line per move. Much of the robustness was added during the field work (section 6).
- *Why:* the intake must be at a known depth, with lake water and not old water in the tube.

**Calibration, tests, cleaning and handover**
- Calibration commands for the pressure sensor, flowmeter, seal position, manifold reference, DNA Shield pump and dose, and spool drum, and one command that runs them in order. A mechanical test suite (lock cycles and endurance, all-slot rotation, encoder repeatability, valve check, leak hold, full-cycle runs, pressure ramp, fill test with weights), each test writing a result line to the log. *Why:* every setting had to be measured on this machine, and every repair is checked the same way.
- An ethanol cleaning procedure (tubing, each used slot, then air) under the same safety rules, an operator tutorial, protocols, a pin map and a command list. *Why:* other people must be able to run, clean and maintain the machine.

## 4. The sampling logic

**Pump control (every water run)**
- The pump is regulated on the line pressure: a PID loop aims at 3.0 bar.
- Power stays between 10 % (below it the pump does not turn) and 60 % with water. Air runs (emptying) and open-line runs (purge, spool prime, ethanol cleaning) may use 100 %; the limit still applies.
- The pump starts from 0 % and rises by 5 % per second, at the start and after every pause, so the pressure is seen rising before it nears the limit.
- **Pressure margins:** at 3.35 bar the pump pauses; it restarts when the pressure is back under 3.15 bar. At 3.6 bar the run stops.

**End of a sample:** the first rule met ends the filtration, and its name is written in the record.

| Rule | Criterion | Record |
|---|---|---|
| Volume reached | 1000 mL counted by the flowmeter | `volume` |
| Slow filter | Less than 30 mL in a 60 s window of pumping time (pauses not counted), from the first window | `clogged` |
| Filter clogging | From the third window, the mean of the last two windows is under 30 % of the first window (one low window only warns) | `clogged` |
| No flow | Less than 3 mL in a 60 s window | `stalled` |
| Pressure held | The pump is paused and the pressure stays above 3.15 bar for 20 s | `pressure` |
| Time limit | 30 min of filtration | `timeout` |
| Safety | Limit reached, or implausible sensor | `cap`, `sensor` |
| Operator | `x` aborts the sample, `s` ends the current pump step | aborted, skipped |

The volume is recorded twice: the plain flowmeter total, and a confident volume that counts only 5 s windows at 50 mL/min or more (the flowmeter reads low below that).

**The other steps**

| Step | How it ends |
|---|---|
| Purge (purge slot, no filter) | 0.5 L counted, or 3 L with the spool; time limit 3 min, or 13 min with the spool |
| Spool prime | 100 mL counted at the purge slot; up to 3 tries at the same depth; 4 min limit per try |
| Vent (before the seal opens) | Pump off and valves shut for 20 s, then the air valve open for 5 s; the result is logged |
| DNA Shield | Micro pump for 190 s (timed), then a short air push from the main pump |
| Emptying | Air pushed until the pressure stays under the air threshold (0.4 bar line, 0.8 bar filter) for 20 s, or 40 s to dry the filter; at most 60 s per phase |
| Any rotation | Only with the seal unlocked; the seal locks again after every rotation |

## 5. Sampling time

A sample takes about 20 min without the spool and about 29 min with it (field records: purge 7 min, dive 4 min, filtration 14 min, DNA Shield 4 min). Changes made to shorten it:

| Change | Effect |
|---|---|
| Purge with the spool cut from 7 L to 3 L (the dead volume is 2.6 L) | About 7 min instead of about 16 min (at the purge rate measured in the field) |
| Purge at full pump power (limit kept) | Faster purge |
| DNA Shield pump run cut from 250 s to 190 s | 1 min less per sample |
| The 5 min deployment-line emptying removed from the cycle (the other steps already empty it) | 5 min less per sample |
| Every step timed and written in the sample record | Shows where the next minutes are |

- **Built, not yet used in the field:** the spool prime at full power.
- **Time traded for a known depth:** a plan to dive once and take a group's samples on the way up was replaced by a dive and prime for every sample (about 4 min more), so a lost depth is recovered at every sample.

## 6. What the field work showed

- **Validated:** no seal leak in two weeks; the highest pressure in 155 records was 3.44 bar, under the 3.6 bar limit; clogged filters ended cleanly with a valid sample; the vent released the pressure before the seal opened in all but one of about 240 cases; an overnight river series and two 24 h lake series ran on the schedule, with failures before the filter retried on their own; every run has its record and log.
- **Revised during the campaign (the spool, in its first field use):** keeping the tube out between samples was replaced by a full dive, prime and rise for every sample, so each depth is known; switch presses are checked against the encoder and the origin is only accepted near zero; the prime retries up to 3 times; the roll-up runs twice; the schedule stops after a failed roll-up (appendix D).
- **Built during the campaign, to confirm on the bench:** the two-pass roll-up, the full-power prime, the step log in the sample record.
- **Gaps found:** nothing stops the motors if the controller itself fails (a power incident on the platform showed it; a safe state at any power loss and a stop independent of the controller are planned); the flowmeter reads low at low flow and the DNA Shield dose is timed, not measured; the frame can misalign and the manifold encoder coupling can slip.

## 7. Next

The next steps are in the [stage 2 roadmap](../roadmap/stage2-pilots.md): power-dropout robustness first, then the hardware fixes from the field, then the lake pilots.

## Appendix A. Timeline

| When | What |
|---|---|
| Aug – Sep | Linear actuator for the seal; O-rings for the PTFE rotor, double O-ring, metal inserts, sensor adapter; force sensor tried and removed; purge port and fill order moved; pressure limit centralised in the pump; first calibration tools |
| 24 – 28 Sep | Test harness and automatic checks; pin map and command list; host protocol; lab protocols; electrical audit; host program with simulator; mechanical test suite and the three-state lock |
| 9 – 12 Oct | Pressure sensor bus speed and calibration against a manometer; dead-sensor stop and limit trip checked on the bench |
| 15 – 19 Oct | Pump control on clogging filters; first water through filters; skip and abort keys; emptying rule; clog rule |
| 20 – 23 Oct | Manifold realigned, outlet reprinted; sample records and the console tool; vent before unlock; confident volume; first full sample from one command |
| 26 Oct | 3 L purge at full power; timed schedule |
| 26 – 29 Oct | Bois Chamblard field work |
| 2 Nov | Spool enabled: lake-water fill and prime, 40 m limit, emptying after every sample; ethanol cleaning procedure |
| 3 – 6 Nov | LéXPLORE field work; spool refinements on site |

## Appendix B. Main settings, original and now

| Setting | Before August 2026 | Now | Why |
|---|---|---|---|
| Pressure limit | Not applied to the pump | 3.6 bar; pause at 3.35, resume at 3.15 | Pass water at a useful flow, inside what the filter and seal withstand |
| Pressure sensor bus | 800 kHz (the sensor's maximum) | 100 kHz (within its 50–800 kHz range) | At 800 kHz the bench showed incorrect frames reported as valid; at 100 kHz the reading was steady at rest (5 counts of spread) |
| Pump power | Fixed levels (90 % water, 80 % flush) | Regulated: 10–60 % with water, up to 100 % with air and for the purge | Full power with water gave pressure spikes and seal leaks at the manifold |
| Sample volume | Container filled for a fixed time, then pushed through the filter | 1000 mL counted at the filter, or an earlier stop on the end rules | A defined, recorded volume per sample |
| Purge | Three container purges, each ended by a low-pressure threshold or a time limit | 0.5 L, or 3 L with the spool, counted by the flowmeter | Flush the dead volume |
| Clogged filter | Not an end rule | Stop below 30 mL per 60 s | A slow filter ends with a valid, documented sample |
| Emptying | Low-pressure threshold or a time limit | Air until the line is empty, 60 s at most per phase | Leave no water in the line |
| DNA Shield pump run | 250 s | 190 s | Shorter sample |
| Confident volume | – | Counted only at 50 mL/min or more | The flowmeter reads almost nothing at low flow |

## Appendix C. Unit test suites

| Suite | What it checks | Tests |
|---|---|---|
| abp_convert | Pressure frames, conversion, dead-sensor rule | 18 |
| pressure_calibration, pressure_ramp | Gain and offset fit; the slow pressure ramp | 18 |
| pump_power, pump_ramp, pump_start, pid_metrics | Power limits, start, control quality | 22 |
| fill_stop, trusted_volume | End rules (volume, clog, stall); confident volume | 20 |
| manifold_geometry, slot_iteration, lock_rules | Slot angles (checked against the previous code), fill order, seal-lock rules | 41 |
| manifold_guard | Stall and over-travel stop of the manifold (built after the field, not flashed) | 8 |
| spool_motion, spool_fill | Spool moves, origin rules, depth limits, fill and prime | 17 |
| shield_calibration | DNA Shield rate fit and fill time | 14 |
| sample_record, step_log | The sample record line and the step timings | 20 |
| clean_plan | Ethanol cleaning plan | 10 |
| run_keys, test_args, openloop | Skip and abort keys, command arguments, the sensorless test mode | 33 |
| **Total** | | **221** |

## Appendix D. Spool refinements made in the field

| Observation | Change | Decision |
|---|---|---|
| Position reset with no move | The origin switch acts only during a move | D65 |
| The schedule went on after a failed roll-up | The schedule stops on a failed roll-up or a refused sample | D71 |
| Roll-up timeout from a wrong position | Fixed time limit for a roll-up (20 min, 30 min from an unknown position) | D72, D79 |
| False top-switch presses | A press is accepted only near zero or when homing | D78 |
| Failures before the filter stopped the list | Retried automatically on the same filter, up to 2 times | D81 |
| The drum pushing against the top | Stop within 300 ms of no movement at the switch | D83 |
| Depth in doubt after a false press | Home before the next sample | D87 |
| A lost depth carried over to the next sample | Every sample dives, primes and rises on its own | D90 |
| Prime found no flow | Up to 3 tries at the same depth | D93 |
| Roll-up stopped short at the switch holder | Two passes for every roll-up | D95 |

## Appendix E. Pump control: approaches tested (9 – 23 Oct)

The diaphragm pump pulses by about ±0.25 bar every 2 to 3 s at a fixed power, and does not turn below 10 % power. Control on a clogging filter was refined over several bench sessions.

| Approach | Observation | Outcome |
|---|---|---|
| Start at 50 % through a 4 s soft ramp; limit read every 100 ms | During the ramp the loop could not react; on a clogging filter the limit was reached | Start at once; limit read on every pass |
| Setpoint-tracking PID | Oscillated on a high-resistance filter: 3 → 17 % power, 0.1 → 2.4 bar | Replaced |
| Power-ramp controller | Unstable ramp, false clog alarms, negative pressures | Abandoned and deleted |
| Gentle controller (slow integral) | Limit cycle of 0.4 ↔ 3.3 bar every 15 s that the test criteria did not catch | Deleted; the log now prints peaks and means |
| PID 200 / 100 / 100 | The pulsation becomes a ±50 % power swing: about 1 bar of ripple whatever the gains | Kept: the ripple stays under the limit |
| PID 3 / 0.2 / 20 | Passed one bench run (overshoot 0.23 bar, ripple 0.38 bar) | Not adopted; back to 200 / 100 / 100 |
| Full power with water | Pressure spikes and leaks at the manifold | 60 % limit with water |
| 15 % at once on a closed line | 0 to 3.35 bar in 3 s; the pause did not stop the rise (3.69 bar) | Start from 0 % at 5 %/s; the pause cuts the pump relay |
| No-flow window restarted by each pause | A pump pausing every few seconds never stopped | Windows count pumping time only |
| Pressure limit 2.5, 3.0, 3.2, 3.5 bar | Raised after each bench result, as the filter and seal were confirmed to withstand it (droplets seen at the seal near 3 bar on 15 Oct) | 3.6 bar on 19 Oct, with the pause and resume margins derived from it |

Open: on clogged filters the pump pauses at the margin many times (up to 113 in one sample). It is safe but slow; a better controller or a pressure damper is for the next machine.
