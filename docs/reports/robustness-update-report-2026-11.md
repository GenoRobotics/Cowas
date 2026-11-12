# Report: autosampler robustness upgrade and pilots experiments

Quentin Perrin · November 2026

## 1. Summary

- **Starting point:** the autosampler prototype developed by GenoRobotics, as it was in August 2026. It was neither field-deployable nor autonomous. The pressure limit was not applied in every pump path. Water leaked at the manifold, the aluminium plates, the outlet seal and some tubing junctions. The spool driver was damaged. The hardware drivers and the general sampling sequence were kept.
- **Aim:** an autosampler that is autonomous and robust. It runs unattended for days, produces a documented sample each time, and rests on a firmware base that can be extended without risk.
- **Main changes:**
  - a motorised O-ring lock that makes the manifold watertight;
  - the pressure sensor repaired and a pressure limit enforced in every operation;
  - pump control on pressure, balancing the limit against the filtered volume;
  - defined end rules and a record of every sample;
  - a timed sampling schedule;
  - a depth spool routine with error detection and recovery;
  - unit tests and a documentation base.
- **Result:** two field pilots (26 Oct – 6 Nov 2026) produced **66 filters**, including an overnight river series and two 24 h unattended series on Lake Geneva. No seal leak, no pressure above the limit, no sample lost to the sampling program.
- **Next:** stage 2 upgrades the current prototype to run unattended for 1–2 weeks (a third pilot experiment). Stage 3 is the design of a second version and a step towards the future auto sequencer.

## 2. The current autosampler

The machine as it runs after the revision. Section 3 lists what was changed.

### 2.1 Components

| Component | Function |
|---|---|
| Diaphragm pump, 24 V | Pushes water through the filter; regulated on pressure |
| Rotating manifold | Holds 14 Sterivex filters and a purge port; positioned by a DC motor and an absolute encoder |
| O-ring lock | Stepper-driven linear actuator: locked to seal, unlocked to rotate |
| Pressure sensor | Upstream of the filter; drives the pressure limit and the pump control |
| Flowmeter | Measures the filtered volume |
| Micro pump | Adds DNA/RNA Shield after filtration (timed) |
| Valves | Air inlet (emptying) and deployment line |
| Depth spool | Lowers the intake to 40 m and rolls it back up |
| Controller | Arduino Due; a laptop runs the session tool, the schedule and the records |

### 2.2 Sampling cycle

1. **Dive and prime** (spool only): the tube unrolls to the deepest point and fills with lake water; the pump primes until 100 mL flow (up to 3 tries); the tube rises to the sample depth.
2. **Purge:** lake water flushes the line through the purge port, without a filter: 0.5 L, or 3 L with the spool.
3. **Filtration:** the manifold moves to the filter (unlock, rotate, lock); water is pumped through it under pressure control until an end rule is met.
4. **Vent and emptying:** the pressure is released before the seal opens; air empties the line and the filter.
5. **DNA Shield:** the micro pump runs 190 s, followed by a short air push.
6. **Roll-up** (spool only): the tube is rolled up and emptied.

**End rules.** The first rule met ends the filtration; its name is written in the record.

| Rule | Criterion | Record |
|---|---|---|
| Volume reached | 1000 mL counted | `volume` |
| Slow filter | < 30 mL in a 60 s window of pumping | `clogged` |
| Clogging | Mean of the last two windows < 30 % of the first | `clogged` |
| No flow | < 3 mL in a 60 s window | `stalled` |
| Pressure held | Paused and above 3.15 bar for 20 s | `pressure` |
| Time limit | 30 min of filtration | `timeout` |
| Safety | Limit reached, or implausible sensor reading | `cap`, `sensor` |

**Pump control.** PID on pressure, target 3.0 bar. Power 10–60 % with water, up to 100 % for the purge and air. Start from 0 % at +5 %/s. Pause at 3.35 bar, resume below 3.15 bar, stop at 3.6 bar.

**Volume.** Recorded twice: the flowmeter total, and a "confident" volume counted only above 50 mL/min, where the flowmeter is reliable.

### 2.3 Cycle time

- About **20 min without the spool**, **30 min with it** (field medians: dive 4.4 min, purge 7.2 min, filtration 13.6 min, DNA Shield 4.0 min).
- Shortened by: spool purge cut from 7 L to 3 L (dead volume 2.6 L), purge at full power, shield run cut from 250 s to 190 s, a redundant 5 min emptying removed.
- Every step is timed in the record.

## 3. Stage 1: repairs and improvements

Each area: the state in August 2026 (*Before*), the changes (*Changes*, including what was tried and dropped) and their purpose (*Why*).

### 3.1 Seal and manifold

- *Before:* a major leak at the manifold and minor leaks elsewhere (aluminium plate). A single O-ring pressed by a passive spring, no motorised lock. A stronger spring sealed better but made the manifold hard to rotate and align; a softer spring rotated easily but leaked.
- *Changes:*
  - Linear actuator (stepper and lead screw) that locks the O-ring to seal and unlocks it to rotate. Sealing and rotation are now independent.
  - Three lock states: locked, unlocked, unknown. The state is unknown after every restart until a homing procedure checks it; no rotation is allowed meanwhile.
  - Lock travel calibrated on the machine (210 steps); homing abortable with a key; actuator wiring soldered (a loose breadboard wire had caused vibrations).
  - New 3D-printed parts and O-rings sealing against the PTFE rotor; a concentric double O-ring.
  - Two metal inserts in series: the water channel in the manifold is fully metallic and never touches 3D-printed parts.
  - Outlet reprinted, thread sealed with PTFE tape.
  - **Fill order against cross-contamination:** the purge port is at the bottom and the slots fill from the bottom up, so a leak or a droplet can only reach slots already filled.
  - Positioning recalibrated: stop offsets measured (the rotor coasts about 1.2° after the motor stops), slot table checked against the encoder, shortest rotation between slots, a slot-count error fixed (the last slot was unreachable).
  - Encoder re-tightened after it slipped on its shaft (23 Oct); a mechanical lock is planned (stage 2).
  - Frame and outlet realigned by hand; 1.15° out of square was enough to leak (droplets at the seal near 3 bar on the bench, 15 Oct). Reference recalibrated after each realignment.
  - Stall and over-travel guard for the manifold motor (built after the pilots, not yet installed).
  - *Tried and dropped:* a force sensor to detect the seal contact (temperature-sensitive, inaccurate). The lock runs open-loop from a known reference; the stepper's torque limit absorbs small drifts.
- *Why:* the manifold must turn freely and seal tightly. The firmware now refuses any rotation while the seal is locked.

### 3.2 Pressure sensor

- *Before:* a digital sensor (Honeywell ABP, SPI) and an analog sensor; the reading detected the end of a purge or a sample by a low-pressure threshold.
- *Changes:*
  - The digital sensor is the single pressure reading, upstream of the filter, for the limit and the pump control. The analog sensor, no longer on the machine, was removed from the code.
  - Bus clock lowered from 800 to 100 kHz: at 800 kHz the sensor returned wrong frames flagged as valid.
  - Every frame checked. A fault frame, no answer or 500 ms of stale readings makes the reading implausible: the pump stops and cannot start.
  - Calibrated against a manometer (5 points; gain 1.0413, offset −0.031 bar, max residual 0.03 bar), valid to 2.0 bar, the manometer's range. Readings between 2.0 and 3.6 bar are extrapolated.
  - Open: the sensor reads 2.4 times higher than the manometer with its nominal range; the range that matches the manometer is used.
  - Raw-frame diagnostic command. Dead-sensor stop and limit trip (at a temporary 1.0 bar) verified on the bench.
- *Why:* the pressure limit and the pump control depend on this reading.

### 3.3 Pump control and pressure limit

- *Before:* fixed power (90 % for water, 80 % for flushing); runs ended by a timer, a threshold or a button. The pressure limit was not applied in every pump path.
- *Changes:*
  - PID on pressure, target 3.0 bar. Pause at 3.35 bar, stop at 3.6 bar.
  - Limit applied in every pump path (including the panel pump mode and the calibration routines) and checked on every loop pass instead of every 100 ms.
  - Power 10–60 % with water (no flow below 10 %); up to 100 % for the purge, the spool prime and air.
  - Pump output from whole percent (8-bit) to 12-bit: 1 % moved the pressure by about 0.2 bar.
  - Start from 0 % at +5 %/s, also after each pause; a pause cuts the pump relay. The blocking 4 s soft start was removed from the control loops.
  - Limit raised in steps (2.5, 3.0, 3.2, 3.5, 3.6 bar) as bench tests confirmed what the filter and the seal withstand.
  - Bench commands: a slow pressure ramp, and a fill test with weighing.
  - *Tried and dropped:* a 50 % start, a setpoint-tracking PID, a power-ramp controller, a slow-integral controller, gains 3 / 0.2 / 20, full power with water (Appendix C).
- *Why:* a Sterivex filter's resistance rises as it clogs. Pressure control keeps the flow as high as the filter allows, within the limit. Below about 3 bar the flow is low and the filter clogs early.

### 3.4 Purge, end of sample, venting and emptying

- *Before:* water was collected in a container for a fixed time, then pushed through the filter until a low-pressure threshold or a time limit.
- *Changes:*
  - Volume counted at the filter by the flowmeter: 1000 mL, or an earlier stop on an end rule (§2.2). End reason and volume recorded.
  - Purge counted by the flowmeter at full power: 0.5 L, or 3 L with the spool (initially 7 L).
  - Flow windows count pumping time only, so a pump that pauses often still ends on a stall.
  - Flowmeter factor confirmed (77), input wired at 3.3 V; "confident" volume recorded beside the total.
  - Vent before the seal opens: pump off and valves shut for 20 s, then the air valve open for 5 s. It released the pressure in all but one of about 240 field cycles.
  - Emptying by air until the pressure stays below 0.4 bar (line) or 0.8 bar (filter) for 20 s; at most 60 s per phase.
  - Operator keys: `s` ends the current pump step, `x` aborts the sample.
  - Bench results: 1000 mL counted for 1006 g weighed (clean line); with dirty water, a new filter clogged at 673 mL counted (865 g) and the clog rule stopped it.
  - *Tried and dropped:* an air threshold of 0.04 bar (an open line reads 0.05 bar, so the emptying never ended); a 1 s air hold (the line was not empty); a weighing checkpoint in the fill test (the restart overshot the limit).
- *Why:* the lab needs a defined, recorded volume per filter. With a stop on volume or on clogging, every filter is a valid sample.

### 3.5 DNA Shield

- *Before:* the micro pump ran for 250 s.
- *Changes:* run time logged in the record; a priming command; a calibration command (doses weighed at several durations, linear fit); run shortened to 190 s for the field (25 s on the bench for tests).
- *Why:* the preservative reaches the filter within minutes of filtration. The dose is still timed, not measured (stage 2).

### 3.6 Operation and records

- *Before:* commands typed one at a time in the console; a link to a Raspberry Pi.
- *Changes:*
  - One command per sample (`run_sample`): purge, filtration, shield, vent, unlock. Without a slot number, the slot is read from the encoder.
  - Timed schedule: starts only when confirmed; reads back the start list; can force a row onto a slot; retries a failure that occurred before the filter was used (same filter, up to 2 times); stops after a failed step or an abort; restarts at any row.
  - Automatic record per sample: name, site, date, condition, slot, volumes, end reason, pressures, step times, flags.
  - Console tool that saves every session with the computer's time; a sample book of used and free slots (loaded filters confirmed at each start); a run-report tool (pauses, purge flow, retries, warnings).
  - Host program with a new message protocol, tested against a simulator of the controller, for future remote operation.
  - The Pi link no longer reports "done" for commands it did not run.
- *Why:* an unattended run needs a plan the machine follows and a record of what happened.

### 3.7 Firmware structure and safety rules

- *Before:* 48 files, about 5,600 lines, organised by module (hardware drivers, sampling steps, console test menu), settings in one file.
- *Changes:* 107 files, about 16,600 lines (58 new, 26 of the 48 originals modified). The module organisation was kept.
  - Calculations and rules (slot geometry, pressure conversion, end rules, spool moves, shield timing) moved out of the driver files into hardware-free code that can be tested on a computer.
  - One place per safety rule: every pump run goes through one controller that enforces the pressure limit; every rotation goes through one function that refuses to turn while the seal is locked or its state is unknown.
  - Fixed safety rules: the pump never runs above the limit; an implausible sensor stops it; every procedure has a time limit; the stop key is always accepted; a stop leaves the pumps off, the valves closed and the seal locked.
  - Settings not yet measured on this machine are marked as placeholders and printed on every test line.
  - Unused code archived with the reason for its removal, not deleted.
- *Why:* the safety rules hold whatever is added later.

### 3.8 Tests and traceability

- *Before:* no automated tests.
- *Changes:*
  - 221 firmware unit tests in 22 categories, run on a computer in under a minute; 364 tests for the host program, 235 for the tools.
  - Manifold calculations checked value for value against the outputs of the original code.
  - Every change built and tested automatically (continuous integration).
  - A sensorless pump test mode for bench use only (explicit, low power, time-limited, unreachable from a sampling run).
  - Pin map, 51 documented console commands, and a change log with about 190 findings and 100 decisions.
- *Why:* a change is tested before it reaches the machine, and every design choice can be traced.

### 3.9 Depth spool

- *Before:* dive and roll-up steps in the sampling sequence, and spool test procedures in the console menu. The spool driver was damaged.
- *Changes:*
  - Each sample: the tube unrolls to the deepest point and fills with lake water; the pump primes until water flows; the tube rises to the sample depth; after the sample it is rolled up and emptied.
  - New move loop: end-stops, stall and wrong-way detection, a time limit on every move, the abort key; homing on the end switch, depth from a known zero.
  - 40 m limit on the total tube (depth + height above the water), checked before a schedule starts.
  - Deployment valve closed while the manifold turns, so the tube does not drain.
  - Spool height above the water measured on site; every move logged.
  - Ten refinements made on site during the first field use (Appendix D).
  - *Tried and dropped:* keeping the tube down between the samples of one time point, and a single dive per group of samples. Replaced by a full dive, prime and rise for each sample, so a lost depth is recovered every time.
- *Why:* the intake must be at a known depth and hold fresh lake water, not water from the previous sample.

### 3.10 Calibration, cleaning and documentation

- *Before:* a console test menu with component tests and two calibrations (manifold encoder, DNA Shield pump); no written documentation besides the code.
- *Changes:*
  - Calibration commands for the pressure sensor, flowmeter, seal position, manifold reference, DNA Shield pump and spool drum, plus one command that runs them in sequence.
  - Mechanical test suite: lock endurance, all-slot rotation, encoder repeatability, valves, leak hold, full cycles, fill tests with weighing; each test writes a result line to the log.
  - Diagnostic commands for the encoder and pressure frames.
  - Ethanol cleaning procedure (tubing, each used slot, then air), under the same safety rules.
  - Operator tutorial; field and manual sampling protocols; lab protocols (filter handling, shield dose, DNA extraction); electrical audit checklist (first part done).
  - Analysis tools for the shield-dose and carry-over tests (ready, not yet used); a list of all field samples.
- *Why:* every setting is measured on this machine, every repair is checked the same way, and others can run, clean and maintain it.

## 4. Field pilots

### 4.1 Pilots

Two pilots tested the revised machine on real water, within the sampling designs of partner projects: first without the depth spool, then with it.

- **Pilot 1, Bois Chamblard (26–29 Oct 2026), river and lake, no spool.** First test on real water, alongside an air and water eDNA project. Covered: samples started by hand, an overnight river series run by the schedule, clogging lake water, one blank per water body.
- **Pilot 2, LéXPLORE (3–6 Nov 2026), Lake Geneva, with spool.** First test of the spool and of 24 h unattended series, from the LéXPLORE research platform, 570 m offshore. Design from a study of diel changes in lake bacterial communities: two 24 h series (E1, E2), one sample every 4 h at 1, 5 and 20 m. Manual samples (Niskin bottle, filtered by hand) were taken at the same depths and times for comparison.

| | Pilot 1: Bois Chamblard | Pilot 2: LéXPLORE |
|---|---|---|
| Autosampler filters | 14 of 14 (12 samples, 2 blanks) | 48 (46 samples, 2 blanks); 2 planned samples not taken |
| Run by the schedule | Overnight river series, 6 of 6 | Both 24 h series |
| Manual samples | – (air samples by the partner project) | 18 samples, 1 blank |
| Cycle time | About 20 min | About 29 min (dive and larger purge) |

The 66 filters of the summary include 4 more taken for the Bois Chamblard project at LéXPLORE on 3 Nov (Appendix E).

### 4.2 Results

- **Unattended operation:** the overnight river series and both 24 h lake series ran on the schedule, with checks between samples. Failures before a filter was used were retried automatically on the same filter. Every run has a record and a full log.
- **End rules:** clogged lake filters stopped on the clog rule with a recorded volume and are valid samples. River water reached the 1 L target; Bois Chamblard lake water clogged earlier (854–912 mL). The river blank stopped at 414 mL (cause not recorded).
- **Seal and pressure:** no manifold leak in two weeks. Highest pressure in 155 records: 3.44 bar (limit 3.6 bar). The vent released the pressure before the seal opened in all but one of about 240 cycles.
- **Comparison with manual sampling:** the 18 manual samples were taken within −4 to +15 min of the autosampler's filtration at the same depth (mean 4.9 min). Filters extracted on 10–12 Nov 2026; DNA results pending.
- **Samples not taken:** two, neither due to the sampling program. E1_T5_01M_R1: wrong slot list typed at start; the run went to an empty slot. E2_T7_01M_R1: LéXPLORE platform power incident of 6 Nov (§4.3).

### 4.3 Problems encountered

| Category | Problem | Effect |
|---|---|---|
| **Spool software** (first field use) | Position lost several times on the first day: a false zero, a roll-up timeout, a schedule that continued | 5 failed tries on 3 Nov; corrected on site (§4.4) |
| **Prototype hardware** | Pressure sensor came off its tubing during the night run; its short disturbed the manifold encoder. The flat reading went undetected for about 3.5 h | 2 failed tries; no valid pressure in the samples of that period; sensor dried and refitted |
| | Pump relay can close when the controller resets or is reflashed (no defined level at start-up) | Pump supply cut before any reset |
| | USB link dropped once (3 Nov); console froze once after the sensor fault | Restarted; cause not found |
| | Wooden spool parts swelled when wet; then a loose screw, a slack belt and a cracked printed guide | Drum stalls; repaired on site |
| | Frame out of square; manifold shifted by three slots twice on 3 Nov (encoder coupling) | Recalibrated on site |
| **Set-up and handling** | Wrong slot list typed at a start | One sample not taken; the list is now read back |
| | Plastic spacer fitted instead of the 20 µm prefilter | 5 failed tries before the filter; corrected |
| | Last available filter past its expiry date; a bent shield tube leaked at a joint | Sample flagged; joint replaced |
| | Intake tip left the milliQ water near the end of the 6 Nov blank | Blank valid; tip must stay submerged |
| **Measurement limits** | Flowmeter reads 20–31 % low at field flow (two bucket checks) | Actual volumes higher than recorded |
| | DNA Shield dose timed, not measured; the amount in the filter varies with clogging | Variable shield volume per filter |
| | Frequent pump pauses at the pressure margin on clogged filters (up to 113 per sample) | Safe but slow |
| | The current pulls the spool tube sideways | Sample depth is an estimate |
| **Power** | 6 Nov, after the last 5 m sample: during a rotation the controller stopped responding and the motor kept running until the manifold was tangled in its tubing. The platform generator started at the same time. Main hypothesis: a power drop stopped the controller while the motor driver stayed on | One sample not taken; controller to be replaced |

### 4.4 On-site repairs and changes

All problems were diagnosed and handled on site (river bank, offshore platform, day and night); no pilot series was abandoned.

- **Firmware changes written, tested and flashed in the field:** ten changes to the spool (Appendix D): switch presses checked against the encoder, prime retries, two-pass roll-up, schedule stop after a failed roll-up and restart at any row. From the evening of 3 Nov, no failed try used a filter.
- **Hardware repairs:** pressure sensor dried and refitted during the night run; swollen wooden switch holder trimmed; spool screw, belt and level-wind guide repaired; leaking shield joint replaced.
- **Recalibration** of the manifold after the plate was squared and after each shift.
- **Procedures added:** start list read back before a run, blank procedure without spool movement, prefilter check, ethanol cleaning between filter sets.

### 4.5 Lessons

- The sampling logic and the schedule worked as designed. The failures came from conditions where a bench does not reproduce: the power supply, water on wood making it swell, connectors affected during transport, a sensor getting loose, and operator error.
- A machine on a supply it does not control must stop safely on any power loss, through a stop that does not depend on the controller.
- Volumes and the shield dose must be measured more precisely, not inferred.
- Whatever the machine can check (leaks with future flowsensors) should not rely on the operator alone.

These lessons are integrated in stage 2 (§5) and stage 3 (§6).

## 5. Stage 2: upgrades of the current prototype

**Goal:** run the current machine **unattended for 1–2 weeks** (third pilot) with remote alerts, by upgrading it without a new design. Stage 2 addresses the lessons of the pilots: survive any power loss, detect its own sensor faults, replace field repairs with permanent parts, measure what it pumps, report remotely.

**Organisation:**
- **Two parallel lanes.** Software (firmware, session tool, host program) is written and tested on a computer, without the machine. Hardware upgrades need the machine, parts and tools; each is a self-contained work package with its firmware follow-up, so it can be handed to a new team member.
- **The prototype stays usable at every step.** Existing commands and the schedule keep working; a part not yet fitted is a setting, and the firmware runs without it.
- **Three checkpoints:** machine back in service; safe for a supervised field run; ready for the third pilot after bench validation.

### 5.1 Safety on power loss

Goal: a power loss of any length, at any moment, ends with nothing moving and a checked restart.
- **Safe default levels** on every actuator line (pull-down resistors): a reset or an unpowered controller means everything off.
- **Stop independent of the controller:** motor and pump power held on by a heartbeat from the controller through an external watchdog; if the controller freezes, the power drops within 0.5 s. An emergency stop in the same line.
- **Protection:** fuses per motor channel, input protection, isolated USB link, separate supplies for electronics and motors, a battery or UPS between the site supply and the machine.
- **Firmware:** safe levels at the first instant of start-up, hardware watchdog, reset cause recorded, a non-volatile record of the motion in progress.
- **Checked restart:** after any reset the machine verifies that everything is off, the pressure is at rest, the encoder is stable, and the seal state and spool position are known. The interrupted sample is flagged; nothing resumes automatically.
- **Verification:** power cuts of 0.1 to 5 s on each supply, in every phase (idle, pumping, rotating, locking, spool moving, shield), 20 times in a row; nothing may move until the checks pass.

### 5.2 Fault detection and supervision

- **Field faults become automatic stops:** a pressure reading that stays flat while the pump runs (detached sensor); encoder angles accepted only when two readings agree; a purge stopped by the pressure limit counted as a failure.
- **Pressure handling:** the pump resumes after a pause on the filtered pressure; the raw reading is checked for artefacts, and a sensor that keeps producing them stops the pump.
- **Supervisor** called at least every 50 ms from every wait: pressure limit, time limits, stop keys, motor-driver diagnostics, watchdog and heartbeat.
- **Defined modes:** one emergency stop, a fault that stays latched until cleared, recovery required after every stop and every start.
- **One implementation per physical action** (filtration, purge, emptying, shield, rotation, spool move), shared by tests and sampling; the remaining code paths that bypass the safety rules are removed.

### 5.3 Hardware upgrades

| Upgrade | Addresses | Acceptance |
|---|---|---|
| Encoder coupling locked mechanically (option: independent index sensor at the purge slot) | Manifold shifted by three slots | 100 rotations without reference shift |
| Pressure sensor bonded to its fitting, strain relief, cleaner signal bus; range verified and calibrated to 3.6 bar | Sensor came off its tubing; readings above 2 bar extrapolated | Within 0.1 bar of a reference gauge at 2.0, 3.0, 3.5 bar |
| Flowmeter correction from weighed runs at five flow rates | Volumes 20–31 % low at field flow | Three 1 L runs within 5 % of the weighed water |
| Permanent spool parts: no wood, belt tensioner, stronger level-wind guide with metal fixings, a switch holder the drum cannot stop against | Swollen wood, slack belt, cracked guide, false zero | 5 dives and 5 roll-ups over 40 m without stall |
| Measured DNA Shield dose (load cell under the bottle or low-flow sensor), dose adapted to clogging | Timed, variable dose | Ten doses within 10 % of the target |
| Site power: battery or UPS, measured energy per sample, sized for a platform or an off-grid river station | Unreliable platform supply | A 1 min supply cut does not reach the machine |
| Field enclosure with leak sensor, water temperature probe, intake strainer, flood-safe mounting | Rain and splashes; water temperature is required metadata | Hose test, no water inside |
| O-ring endurance test and a switch confirming each unlock | Lock state assumed, not sensed | 50 lock cycles confirmed by the switch |

### 5.4 Operation, data and remote monitoring

- **Session tool:** resume an interrupted schedule; edit the time or depth of a waiting row; confirm the loaded filters slot by slot; prefilter check at start; detect a controller restart and flag the running sample; keep the previous record when a slot is reused.
- **Sample record:** reset cause, purge result, depth, corrected volume, measured shield dose.
- **Communication:** the message protocol on the controller's second USB port, beside the console, so that a computer restart never resets the controller. The host program then takes over the schedule, the sample book and the records.
- **Remote monitoring:** host on site (laptop or small computer) with a mobile link and a UTC-synchronised clock; critical events (stop, sensor fault, leak, lost link) send a phone alert repeated until acknowledged; daily status message.
- **Optional:** swappable filter cartridges (cleaning without losing sampling time), pump-control tuning, settings changeable without reflashing, an operator web page.

### 5.5 Lab work and validation

- **Pilot 2 results** (extraction 10–12 Nov 2026): the blanks and the machine vs manual comparison set the success criteria for the third pilot.
- **Shield dose and residual water** measured by weighing, to set the measured dose.
- **Carry-over between slots** measured with a tracer, to set the purge volume; reported against the most sensitive assay in view (qPCR of a synthetic DNA tracer), not only metabarcoding.
- **Supervised field run** with the first safety fixes, then a **72 h unattended bench test** with alerts, power cuts and a forced fault.
- **Third pilot:** 1–2 weeks unattended, on a lake (with spool) or at a river station (off-grid); one sample at a fixed interval (e.g. every 6 h); blanks at the start, middle and end of each filter set; visits every few days to reload. Success: ≥ 90 % of planned samples complete, clean blanks, machine and manual samples comparable, no motion after any power event, every critical alert received within 15 min.

**Outlook:** projects in preparation plan to use the machine at river stations, sampling every few hours for weeks, and as arrays of several units along one stream. Stage 2 tests the first of these needs; more slots, several identical units and harsher sites are for the second version.

## 6. Stage 3: second version

**Goal:** a new machine designed from the prototype's lessons: weeks to months of operation, several identical units, easy maintenance on site. The design starts from a requirements document (deployment length, samples per deployment, volumes and flows, depth, power, maintenance, cost), traced to the pilot lessons and the intended uses. The prototype stays in service meanwhile.

### 6.1 Filter handling and sealing

- **Moving filters, fixed inlet and outlet** (concept under study): the filters move to a fixed inlet and outlet instead of rotating under a seal. This removes the rotating seal and its alignment with the frame, the two main leak sources of the prototype. A bench mock-up must first demonstrate sealing at 1.2 × the pressure limit, repeatable positioning and no cross-contamination.
- **Self-sealing valves** on the filters and on every connection that is unplugged (filters, reservoirs, intake): no leak when a part is removed.
- Filter cartridges built into the design, more slots if required, a frame that holds its alignment, a sensed seal state.

### 6.2 Fluidics and safety sensors

- **Safety sensors at the inlet and outlet of the filter path:** two pressure sensors and two flowmeters accurate at low flow (about 10–1000 mL/min).
  - **Flow difference:** automatic leak detection; the pump stops.
  - **Pressure difference:** filter resistance through the run; clogging detected as it builds and reported for every sample.
  - Cross-checks: a dead flowmeter or a torn filter is detected instead of trusted.
- **Pump** with a linear low-flow range and a **pulsation damper**, selected after a measurement campaign on lake water, for a steady pressure without pauses.
- Air/water detector on the line; suction sensor at the pump inlet (blocked or dry intake); **measured shield dose**; **pluggable reservoirs** (bleach, ethanol, clean water, waste) that need no tubing to be reconnected.
- Later: negative and positive control reservoirs, and a decontamination flush between samples.

### 6.3 Depth, electronics and enclosure

- **Depth spool:** no absorbent materials, a robust level-wind, a top stop that cannot be mistaken for the origin, a **depth sensor at the intake** so the depth is measured even when the current pulls the tube.
- **Electronics:** a dedicated board with the stage 2 safety built in (safe defaults, heartbeat-held power, protections, isolated link, supply monitoring); a controller chosen for the requirements; on-board record storage; power management (duty-cycled computer, solar for deployments of months).
- **Enclosure and site sensors:** watertight, condensation-proof electronics (rivers, platforms, possibly caves); mooring and recovery plan; site data recorded with each sample: water temperature, conductivity, turbidity, and water level at river stations (to convert eDNA concentrations into fluxes).
- **Multiple units:** identical, calibrated machines on a common time base (satellite or network time), for arrays along a river.

### 6.4 Validation

Bench acceptance as in stage 2, then a supervised field run, then an unattended deployment at least as long as the third pilot, with manual comparison samples and blanks.

---

## Appendix A. Milestones

| Date | Milestone |
|---|---|
| Aug – Sep 2026 | Seal redesigned: motorised O-ring lock, double O-ring, metal water channel |
| 11 Sep | Manifold seal resolved with the linear actuator upgrade. Starting to resolve other problems of the autonomous ampler prototype |
| 28 Sep | Firmware foundations: unit tests, automatic checks, three-state lock, host message protocol |
| 12 Oct | Pressure sensor calibrated; pressure limit enforced and verified on the bench |
| 19 Oct | First water through filters under pressure control, with defined end rules |
| 23 Oct | First complete sample from one command, with its record |
| 26 Oct | Timed sampling schedule |
| 26 – 29 Oct | Pilot 1, Bois Chamblard: first unattended overnight series |
| 2 Nov | Depth spool in operation |
| 3 – 6 Nov | Pilot 2, LéXPLORE: two 24 h series at three depths, compared with manual sampling |
| 10 – 12 Nov | DNA extraction of the pilot filters |

## Appendix B. Main settings, before and after

| Setting | August 2026 | Now | Reason |
|---|---|---|---|
| Pressure limit | Not applied in every pump path | 3.6 bar; pause at 3.35, resume at 3.15 | Useful flow within what the filter and seal withstand |
| Pressure sensor bus | 800 kHz | 100 kHz | Wrong frames flagged as valid at 800 kHz |
| Pump power | Fixed (90 % water, 80 % flush) | 10–60 % with water; up to 100 % for air and purge | Full power with water caused pressure spikes and seal leaks |
| Sample volume | Container filled for a set time | 1000 mL counted, or an earlier end rule | Defined, recorded volume |
| Purge | Three container purges | 0.5 L, or 3 L with the spool | Flushes the dead volume |
| Clogged filter | No end rule | Stop below 30 mL per 60 s | A slow filter still gives a valid sample |
| Emptying | Threshold or time limit | Air until the line is empty, max 60 s per phase | No water left in the line |
| DNA Shield run | 250 s | 190 s | Shorter cycle |

## Appendix C. Pump control: approaches tested (9–23 Oct)

At constant power the diaphragm pump pulses by about ±0.25 bar every 2–3 s; it does not turn below 10 % power.

| Approach | Observation | Outcome |
|---|---|---|
| 50 % start with a 4 s soft ramp | The loop could not react during the ramp; a clogging filter reached the limit | Immediate start; limit read on every pass |
| Setpoint-tracking PID, power-ramp and slow-integral controllers | Oscillations, false clog alarms, a hidden 0.4 ↔ 3.3 bar cycle | Dropped |
| PID 200 / 100 / 100 | About 1 bar of ripple regardless of gains | Kept: ripple stays below the limit |
| Full power with water | Pressure spikes and seal leaks | 60 % limit with water |
| 15 % at once on a closed line | 0 → 3.35 bar in 3 s, overshoot to 3.69 bar | Start from 0 % at 5 %/s; a pause cuts the pump relay |

## Appendix D. Spool refinements made on site

| Observation | Change |
|---|---|
| Position reset without a move | Origin switch acts only during a move |
| Schedule continued after a failed roll-up | Schedule stops; restart at any row |
| Roll-up timeout from a wrong position | Fixed time limits for a roll-up |
| False top-switch presses | Press accepted only near zero or during homing |
| Failures before the filter stopped the list | Automatic retry on the same filter (up to 2) |
| Drum pushing against the top | Stop within 300 ms of no movement at the switch |
| Depth in doubt after a false press | Homing before the next sample |
| Lost depth carried over to the next sample | Each sample dives, primes and rises on its own |
| Prime found no flow | Up to 3 tries at the same depth |
| Roll-up stopped short at the switch holder | Two passes per roll-up |

## Appendix E. Samples collected

| Series | Planned | Collected | Missing |
|---|---|---|---|
| LéXPLORE E1 (3–4 Nov) | 24 | 23 | E1_T5_01M_R1 (human error: wrong slot list) |
| LéXPLORE E2 (5–6 Nov) | 24 | 23 | E2_T7_01M_R1 (LéXPLORE platform error: power reset) |
| LéXPLORE blanks, autosampler | 2 | 2 | – |
| LéXPLORE manual samples and manual blank | 19 | 19 | – |
| Bois Chamblard, river and lake (2 blanks) | 14 | 14 | – |
| Bois Chamblard project at LéXPLORE (1 blank) | 4 | 4 | – |

Full list: [sample list](lexplore-sample-list-2026-11.xlsx).
