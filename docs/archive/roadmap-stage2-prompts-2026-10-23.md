# CoWaS roadmap, stage 2: step prompts

The prompts of [stage2-pilots.md](../roadmap/stage2-pilots.md) §8. Moved here on 2026-10-21, text unchanged, so the roadmap itself stays short.

Copy one prompt into a **new** session, on a new branch, per step. Fill in `<...>`. Each prompt names the roadmap sections to read. Done steps keep their prompt for reference.

## 8. Prompts

> Copy each block into a **new** Claude Code session, on a new branch. Fill in `<...>`. After FW0, every prompt starts by reading `CLAUDE.md`.

### 8.1 Firmware track

#### FW0: Baseline, test harness, CI, pin map (P0)

```text
Project: CoWaS, an autonomous aquatic eDNA sampler (GenoRobotics), heading to a supervised pilot on
Lake Geneva in ~8 weeks. A diaphragm pump (24 V, driven from DAC1 + an enable relay on D42) pushes
lake water through Sterivex filters on a rotating manifold (slot 0 = purge, 1..14 = samples, FILL_ORDER
in Settings.h; AMT22 SPI encoder, CS D38; DC motor on a VNH5019 shield). An O-ring seals the manifold,
locked/unlocked open-loop by a NEMA17 + A4988 linear actuator (D9/D10/D11). A micro pump (D48) adds
DNA/RNA Shield. An Arduino Due (PlatformIO env `due`) controls everything. A legacy Raspberry Pi
script (computer_interface/, PyQt5 + ZMQ) sends text commands. The spool (depth) is broken
(SPOOL_USE=false).

Read stage2-pilots.md fully first (especially §3, §4 findings, §5 test strategy, §6 rules).
This step must NOT change firmware behavior.

1. CLAUDE.md at the repo root: a 10-line project summary, the hardware/pin list, build and test
   commands, the rules of ROADMAP §6 verbatim, the known hazards (placeholders such as
   LINEAR_ACT_LOCKED_STEPS; flowmeter calibrated (user 2026-10-23); safety reads the uninstalled A2 sensor, F7/F22;
   the setup() test-menu block, F23; spool disabled), and a pointer to stage2-pilots.md.
2. A PlatformIO `native` env with Unity and build_src_filter so src/Core/ builds natively without
   Arduino.h. Move ONLY pure manifold math into src/Core/ (clock_minutes_for_slot,
   slot_for_clock_minutes, scale_angle, the slot-angle table from Manifold::begin() as a pure
   function). Record the current outputs as golden values BEFORE moving anything and assert them
   bit for bit. Tests: the golden angle table (purge at index 0, the no-hole position never
   produced), the slot<->clock round trip, FILL_ORDER is a permutation of the sample slots, NB_SLOT
   matches the table, and the same builders with 16- and 24-position geometries.
3. .github/workflows/ci.yml: `pio run -e due` + `pio test -e native` (+ a placeholder pytest job
   that is skipped while host/ doesn't exist).
4. ../hardware/pinout.md from Settings.h and the code: every Due pin (D0-D53, A0-A11, DAC0/1, SPI, I2C,
   CAN), what uses it (file:line), direction, the expected voltage level, and EMPTY columns for me:
   "reachable on the PCB shield?", "measured level", "notes". Flag declared-but-unused and
   used-but-undeclared pins. Replace the "all pins are claimed" comments in Settings.h with a
   pointer to ../hardware/pinout.md (comment-only change).
5. ../software/commands.md: every console command of test_all_components() (including the prefix
   commands slotN/clockMM), every button_control() mode and every main_program() command:
   name | function (file:line) | hardware touched | blocking? | needs the pressure sensor?
6. the pending checks (empty template: test | blocked by | exact commands | pass criteria).
7. Report only (no fix): does `pio run -e uno` build; does include/interfaces exist (F12)?

Done when: `pio run -e due` builds, `pio test -e native` passes, and the firmware diff is only the
math extraction + comments. Give me a 5-minute hardware smoke test to run before I tag `baseline-v0`.
```

#### FW1: Protocol v1 specification (docs only) (P0)

```text
Read CLAUDE.md, stage2-pilots.md §3 (architecture, safety, link, modes, slots, sample record, eDNA
rules), findings F8, F23, F24, F33, and ../software/commands.md.
Write ../software/protocol.md and protocol/vectors.json. NO code. The host work starts from this document
next week, so keep it small, precise and extensible. This is v1 for the lake pilots.

- Transport (DECIDED, ROADMAP §3.4): HOST = Native USB (SerialUSB), CONSOLE = Programming port
  (Serial). One JSON object per line, UTF-8, max line length (propose one). On the host channel,
  every human log line is ALSO sent as {"t":"log","lvl":..,"msg":..}. Never open the Native port
  at 1200 baud.
- Message types: req {t:"req", id, cmd, args}; ack {t:"ack", id} sent at once for long commands;
  res {t:"res", id, ok, data | err:{code,msg}}; evt {t:"evt", type, ...} (mode change, fault,
  warning, procedure step start/end, phase change); tel {t:"tel", ...} at ~1 Hz while BUSY
  (P_in, flow, volume, phase, pump %, step); log.
- Commands (P0): HELLO (protocol version, firmware git hash, mode, recovery_required + cause,
  capabilities {p_in, p_out, q_in, q_out, estop_input, spool, water_temp, ...}), PING, STATUS,
  STOP (always accepted, any mode), CLEAR_FAULT, RECOVER {confirm_moving: bool}, GET_INFO (the
  manifold geometry: positions, sample slots, purge slot, FILL_ORDER, clock labels; the host must
  never assume 14), ACTUATOR_HOME, ROTATE {slot}, LOCK, UNLOCK, PRIME_SHIELD, and the main one:
  RUN_CYCLE {slot, purge_ml, volume_ml | null, max_duration_s, stages: {purge, sample, shield,
  empty}} = the production sequence purge -> sample -> shield -> empty -> unlock for one filter.
  Also PURGE, SAMPLE, DNA_SHIELD, EMPTY as separate commands for service use.
  Reserved (answer ERR not_available): SPOOL_*, NEG_CONTROL, POS_CONTROL, LEAK_TEST.
- RUN_CYCLE result (every field nullable when its sensor is absent): slot, volume_ml,
  duration_s, end_reason (volume_reached | clogged | timeout | pressure_cap | fault | stopped),
  p_in {max, mean, final}, flow_ml_min {mean, min, final}, purge_ml_measured, phases [{name,
  duration_s}], shield_ms, t_end_to_shield_s, faults[], warnings[], sensors[] {name, value, unit,
  stat}, and the reserved nullable fields p_out, q_out, dp, r_filter, clog_index (for P2).
- Rules: which commands each mode accepts (table); error codes (table); request ids are
  remembered (last 16) so a retried id never runs a cycle twice (it returns the stored result);
  link loss: the Due finishes or aborts the current procedure safely and never starts a new one;
  a host heartbeat (PING) period and what the Due does when it stops.
- Timing: expected durations per command and the host timeouts to use.
- A mapping table: every legacy main_program() command -> its v1 equivalent (or "dropped: why").
- protocol/vectors.json: 30+ request/response examples, including malformed JSON, unknown
  commands, wrong arg types, commands refused in the wrong mode, a retried id, and results with
  null fields. Both the C++ and the Python tests will load this file.
Ask me the open questions at the end instead of guessing.
```

#### FW2: Mechanical test suite, lock tri-state, test logging (P0)

```text
Read CLAUDE.md, stage2-pilots.md §5 (test strategy), findings F5, F16, F17, F18, ../software/commands.md.
Also read src/Tests.cpp (the existing tests: linear_actuator, sample_cycle, sample_cycle_full,
multi_sample, multi_sample_full, calibrate_actuator, slotN/clockMM), src/Step_functions.cpp
(load_slot, load_slot_full, goto_slot_locked, sample_process, step_*) and src/Hardware/Oring_lock.cpp.

Current hardware: pressure sensor <NOT trusted yet (FW3 pending) | working>. Flowmeter calibrated (factor 77, user 2026-10-23).
Goal: a reliable set of serial test commands to validate locking, sealing and rotation NOW, and
prepare the pumping tests. ADDITIVE: existing commands keep working, EXCEPT the lock tri-state.

0. Audit first: print a table "required test | exists / partial / missing | what it reuses". REUSE
   the existing procedure functions; don't duplicate logic.
1. Test output convention (ROADMAP §5): every test prints one machine-readable line per result:
   `TEST|<name>|<id>|<PASS/FAIL/SKIP>|<key=value;...>` plus a human summary table at the end.
   tools/hw_test_log.py (pyserial, cross-platform): opens a port, lets me type commands,
   mirrors everything to the screen, and appends the TEST lines to docs/logs/test-logs/<date>_<tag>.csv.
   pytest for its line parser.
2. Sensorless pump test mode (F17): PRESSURE_SENSOR_AVAILABLE in Settings.h. When false, every
   procedure that needs pressure REFUSES with a clear message. pump_open_loop_test(power,
   volume_ml, max_ms): a fixed power clamped to TEST_OPENLOOP_MAX_POWER (PLACEHOLDER, ask me);
   it stops on volume, max_ms, no flow pulses for 5 s, or any keypress, and returns the reason +
   volume. Detaching the pressure safety is allowed ONLY inside this helper, after I type NOSENSOR
   at every run, with a loud warning; it is re-attached on every exit path. Prove by grep that
   no production path can reach it.
3. O-ring lock state (F18): tri-state LOCKED / UNLOCKED / UNKNOWN, UNKNOWN at boot. `actuator_home`:
   only if the manifold is aligned to a slot (encoder within tolerance), run the overdrive lock
   (a stall against the physical stop = a known reference), then the normal unlock -> known
   UNLOCKED. Otherwise refuse. (From FW4 the rest state is LOCKED, §3.2: actuator_home then ends
   locked; FW2 keeps unlocked because its rotation tests don't auto-unlock yet.) rotateMotor() and every rotating test refuse while LOCKED or
   UNKNOWN (the automatic unlock comes in FW4). Tell me how production paths behave on the first
   rotation after a boot; ask me before changing sample_process().
4. Test commands. Each one: 'x' aborts at every pause; optional `auto` (no pauses);
   `from=N count=M`; FILL_ORDER by default or `order=index`; no hard-coded 14/15; a summary table.
   a. lock_cycle [n]: lock/unlock n times at the current slot (default 5). Also measure and print
      the time per lock and unlock.
   b. lock_endurance [n] (for HW7): n cycles (default 200), with the encoder angle checked every
      10 cycles, and the driver temperature asked every 50 (y/n "too hot to touch?").
   c. rotate_all [dry]: per slot: rotate -> encoder error -> lock -> hold N s -> unlock.
   d. encoder_repeat [slot] [n]: go away and come back n times; print the angle spread.
   e. valve_check: each valve on/off with a y/n confirmation of click/flow (V1 and Vman; V23 is
      physically removed, F51 - its code goes in FW4).
   f. leak_hold [mL|s]: lock -> pump (sensorless mode, or normal once FW3 is done) -> "leak? y/n +
      where" -> air push -> unlock.
   g. multi_sample (extend the existing one, default behaviour unchanged): sensorless variant when
      PRESSURE_SENSOR_AVAILABLE is false; volume per slot; the options above.
   h. full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=<mL>: the REAL production step
      functions in order; full_all runs it over the slots. They refuse while the sensor is not
      available, but must compile and be complete.
   i. shield_only [slot] [ms]: micro pump only (time-based); CAN run sensorless.
   Add all of them to the menu and to `help`.
5. Pure logic in src/Core/ + native tests: the slot iteration list builder (from/count/order, any
   slot count, FILL_ORDER validated as a permutation), the open-loop stop decision, the "may
   rotate?" rule, and the "may home?" rule. Every combination; 15 and 24 positions.
6. the pending checks: every test that needs the pressure sensor, with exact commands and pass
   criteria.
Don't touch the pump PID. Don't change sample_process() without asking.
Hardware checklist (in order, safe first), each with a numeric PASS: actuator_home -> lock_cycle 5
-> rotate_all dry (encoder error < <1>°) -> encoder_repeat (spread < <0.5>°) -> valve_check ->
leak_hold small volume -> multi_sample auto count=3 -> shield_only.
```

#### FW3: Pressure sensor, correct driver, safety on `P_in` (P0, critical path)

> **Deferred from stage 1 (2026-10-19):** delete the A2 code and rename `pressure1` to `p_in`; SPI diagnostics; re-check `ABP_FULL_SCALE_PSI = 60` against the real 150 psi range (F126: the firmware follows the manometer today, the calibration absorbs the factor) and the cap against the sensor range (F143); extend the calibration above 2.0 bar (F136).

```text
Read CLAUDE.md, stage2-pilots.md §3.3, findings F7/F22, F21, F27, F36, the pending checks,
include/Trustability_ABP_Gage.h, src/Hardware/Trustability_ABP_Gage.cpp,
src/Hardware/Pressure_sensor.cpp, main.cpp (read_pressure2, set_pressure_safety), Pump.cpp,
../logs/LOG.md (F44, F50).
Installed: P_in = Honeywell <ABPDANV150PGSA3 | ABPDANV060PGSA3 | NONE YET> on SPI, CS D8, 3.3 V,
between the pump and the manifold. Cable length to the sensor: <N> cm. P_out: <not installed |
ABPDANV060PGSA3 on CS D45>. (As of FW0 no sensor is installed at all, F50: the A2 analog sensor was
removed. If still NONE, do items 1-3 and 6 with native tests and leave the checklist in
the pending checks.) Before starting, ask me for the result of the pending "Is the spare 150 psi
ABP alive?" check in the pending checks (pin identification + wiring + `pressure_sensors`); if it
hasn't been done, walk me through it first.

0. Naming and dead code (F50, rule 2: the user approved the removal on 2026-09-24): `pressure1`
   (Trustability ABP, SPI) becomes `p_in` everywhere. The analog sensor on A2 is physically gone:
   delete BigPressure/SmallPressure (Pressure_sensor.*), `pressure2`, `read_pressure2()`,
   `pressure_2_pin`, `pressure_3_pin` and the `calibrate_pressure2` command (F44), after listing
   every call site. The `pressure` / `pressure_sensors` commands show P_in only.

1. Driver correctness (F27):
   - The range comes from the part number, as a constructor argument (060PG = 0-60 psi, 150PG =
     0-150 psi). Transfer function A: 10-90 % of 2^14 counts (1638..14745). psi -> bar
     (1 psi = 0.0689476 bar). Remove the hard-coded 7.63e-4 / 1.25 (they assume 10.0 bar).
   - Status bits: 00 normal; 10 STALE = no new conversion since the last read -> keep the last
     value, and only if nothing fresh arrives for STALE_TIMEOUT_MS is it a failure; 01 (command
     mode) and 11 (diagnostic fault) -> IMPLAUSIBLE. Mask the status bits before converting.
   - Diagnose the "invalid SPI response" message BEFORE blaming the sensor: log the raw status-bit
     histogram over 1000 reads at the current poll rate; try 100/400/800 kHz; check the CS
     timing and that the AMT22 transactions (encoder) never overlap. Give me a wiring checklist
     (3.3 V at the sensor pins, a common GND, MISO/SCK on the SPI header, cable length, a
     100 nF decoupling capacitor close to the sensor).
   - `calibrate_pressure`: pump off, lines vented, a 5 s average -> zero offset, printed as a
     Settings.h value (PRESSURE_ZERO_OFFSET_BAR).
2. Switch ALL safety and control to P_in (F7/F22): set_pressure_safety(...) and every
   CtrlPumpFlow/CtrlPumpNoWater::begin() call get a read_p_in() function. List every call site.
   Nothing may read A2 any more (item 0 deletes it).
3. Plausibility window for the installed range (below -0.3 bar or above range + 10 % =
   implausible -> the pump stops). NaN = implausible.
4. If P_out is installed: read-only display in the console `pressure` command (P_in, P_out, dP,
   raw counts, status). Not used for control or safety (P2).
5. PRESSURE_SENSOR_AVAILABLE = true. FW2's sensorless mode must refuse while a sensor is
   available, unless there is an explicit override.
6. Native tests: the count -> bar conversion at the datasheet points for BOTH ranges (1638 -> 0,
   14745 -> full scale, the midpoint), the status-bit handling including the stale timeout, the
   plausibility bounds (just below/at/above), and that the old constants are gone.
Then walk me through the pending checks in a safe order: first a cap-trip test with a
restrictor/clamp at the filter outlet, pump at low power, CAP temporarily at 1.0 bar (it must trip
at 1.0 ± 0.1 bar); then full_cycle on one slot. Update the file with the results I report.
```

#### FW4: Safety and sample-quality fixes (P0)

> **Deferred from stage 1 (2026-10-19):** (1) one set of PID gains and one pressure target: `PUMP_PID_*` and `PRESSURE_RAMP_PID_*` merge, the hard-coded setpoint 2 becomes `PRESSURE_TARGET_BAR` (F3); `pump_pid` goes if nothing uses it. (2) Dead code `Core/power_ramp` and `Core/gentle_ctrl` deleted 2026-10-19 (F138); the pump model `test_pid_plant` too (its method is in LOG, F146). (3) `Pump::set_power()` blocks 4 s on a step of 10 points or more (F132): remove it or route every caller through `set_power_live`. (4) The purge volume from hose length, hose diameter and dead volume as a tested pure function; the shield dose (with LA3). (5) The real sample record fields and the `end_reason` for the sampling phase (stage 1 only prints `end=volume|timeout|stalled`).

```text
Read CLAUDE.md, stage2-pilots.md §3.3, §3.7, findings F2, F3, F4, F5, F6, F15, F28, F30, F31.
SMALL, listed behavior changes. Nothing else.

1. One safety value (§3.3): PRESSURE_CAP = 2.5. pressure_target() = PRESSURE_CAP -
   PRESSURE_TARGET_MARGIN (0.5), in src/Core/. static_assert 0.5 <= cap <= 3.0. (2026-10-15: `STX_MAX_PRESSURE` is now 3.2, F143: revisit these numbers in FW3.) STX_MAX_PRESSURE
   becomes a deprecated alias. Replace every hard-coded setpoint `2` in the CtrlPumpFlow/
   CtrlPumpNoWater begin() calls with pressure_target(); keep the 0.4 emptying setpoint. List the
   call sites.
2. F2: in loop(), while pump.is_running(), call pump.enforce_pressure_safety() every iteration.
   List any other place where a pump can run without polling (they get fixed properly in FW5).
3. The cap decision (reading, cap, bounds -> OK / OVER_CAP / IMPLAUSIBLE) is a pure src/Core/
   function with boundary tests.
4. F5, automatic unlock before rotation: LOCKED and safe (pumps off; P_in < UNLOCK_MAX_PRESSURE)
   -> unlock, log, rotate. LOCKED and not safe -> wait up to N s, then refuse. UNKNOWN -> refuse,
   ask for actuator_home. "Already at target" must NOT unlock (step_DNA_shield relies on it).
   Check EVERY caller (goto_slot_locked, slotN, clockMM, slot0, panel Man Slot, calibrations, the
   FW2 tests); report before/after. The pure rule (ROTATE / UNLOCK_THEN_ROTATE / WAIT / REFUSE) is
   tested for every combination.
   Rest state = LOCKED (§3.2, decided in FW1): every rotation re-locks at its target (rotate =
   unlock -> rotate -> lock); remove the final unlock_oring() at the end of sample_process(),
   load_slot(), load_slot_full() and abort_sample(), so procedures end locked (the legacy
   step_purge() already does, F40 becomes the intended behaviour); actuator_home ends locked.
   List every caller that assumed "unlocked at rest" and every test that unlocks at the end.
5. Volume and timeout (F15, F6, F31):
   a. STX_SAMPLE_MILLILITERS = 2000 (or the target volume in ../lab/shield-protocol.md §8, C1); SAMPLE_VOLUME_CAP_ENABLED = true.
   b. The Flow_sensor calibration factor becomes a float (pulses per litre as float); fix the
      uint8_t/uint16_t truncation. Keep the behaviour identical for the current 77.0 factor
      (a test proves it).
   c. step_sampling()'s timeout = sample_max_duration_ms(volume_ml) in src/Core/:
      volume / SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN x 1.5, bounded by SAMPLE_MAX_DURATION_ABS_MS.
      SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN = <value measured in HW3, or PLACEHOLDER>.
   d. CtrlPump::run() returns WHY it stopped (condition met / timeout / pressure cap /
      implausible / aborted). step_sampling() warns loudly on a timeout; it must never look like
      a normal completion. Keep the reason available for FW8.
6. Purge volume (F30, §3.7): INTAKE_HOSE_ID_MM, INTAKE_HOSE_LENGTH_M, INTERNAL_DEAD_VOLUME_ML
   (PLACEHOLDER, measured in LA4), PURGE_DEAD_VOLUME_FACTOR = 3. purge_volume_ml() = max(
   PURGE_MIN_ML, factor x (pi x (ID/2)^2 x L + internal)). step_purge() uses it, and prints it.
   Check the purge timeout still fits the purge volume at the minimum expected flow.
7. DNA shield (F4): remove "/ 10." and use DNA_SHIELD_FILL_MS = <value from LA3 calibration, or
   FILL_STERIVEX_TIME until then>. Check nothing relied on the short time.
8. Air push (F28): do NOT change the logic yet, but log the pressure and duration of each
   emptying phase so LA3 can decide. Add EMPTY_STX_MIN_MS / EMPTY_STX_MAX_MS params (defaults =
   today's behaviour).
9. Stepper coils off while locked (F34; the lead screw self-locks, confirmed): lock_oring()
   disables the A4988 after the lock move; unlock_oring() re-enables it before moving. A param
   ORING_HOLD_WHILE_LOCKED = false (true restores today's behaviour). Tell me which callers
   assumed the driver stays enabled.
10. Don't touch DEBUG_MODE_PRINT (FW6).
11. V23 removal (F51, rule 2: the user approved it on 2026-09-24): the 3/2 valve on D40 is
    physically gone (only its relay remains). List every call site of `valve_23` (Step_functions,
    maintenance, Tests, the panel mode V23, test_valves), then delete them and VALVE_23_PIN; D40
    becomes free in ../hardware/pinout.md. Report which procedures used it to route water (the
    container era: step_fill_container, purge_pipes_manifold) and ask me whether to delete those
    too (P2 otherwise). Also read ../logs/LOG.md findings F40-F43 and F45: fix the ones this
    step's items already touch, and log the rest.
Native tests: the cap decision; the interlock rule; sample_max_duration_ms (2000 mL at the expected
flow finishes before the timeout; no volume cap -> the absolute maximum; bounds); purge_volume_ml
(the worked example of §3.7 ≈ 424 mL; zero length -> the minimum); the flow factor round trip.
Hardware checklist: calibrate_flow (if HW3 isn't done yet); lock then slot5 auto-unlocks and
rotates; with the pump running, slot5 waits then refuses; after a reboot, slot5 asks for
actuator_home; one full_cycle volume=500: the printed purge volume matches the formula, the
shield runs DNA_SHIELD_FILL_MS, and the cycle ends LOCKED; the next slotN unlocks before rotating.
```

#### FW4a: One brick per action, the run loop (P0)

> **Why (user, 2026-10-19):** the test functions are the bricks the run is built from. Tests have more steps, feedback, logs and visible parameters; the run needs the same work with fewer prints. Write each action once, so that a tested function is also the run's function. The user's name for the run is "the run loop"; this step calls it `run_samples`.
>
> **Status:** not started. Do after S1-C and S1-D pass (stage 1), at the start of stage 2 with FW4.

```text
Read CLAUDE.md, stage2-pilots.md §3.1, §6, FW4, and the stage 1 archive copy, [roadmap-stage1-2026-10-23.md](../archive/roadmap-stage1-2026-10-23.md) §3.3 (items 1 and 2: the skip/abort keys and the emptying rules must already be there, and this step must keep them).
BEHAVIOR-PRESERVING refactor, except the listed items. The test menu (test_all_components, src/Tests*.cpp) and the run must produce the same physical steps.

1. Name the bricks (one function each, in src/Step_functions.cpp or src/Hardware/, the physical action only):
   pump_through_filter(slot, volume_ml, opts)  (today: CtrlPumpFlow in step_sampling / load_slot_water, and fill_test's own loop)
   purge_line(opts)                           (step_purge)
   empty_line_through_purge(opts)             (step_empty / the emptying in step_sampling)
   empty_filter(slot, opts)                   (the air emptying of step_sampling)
   shield(slot, opts)                         (step_DNA_shield)
   Each brick returns a result struct: ok, end reason (volume, timeout, stalled, clogged, cap, sensor, skipped, aborted), numbers measured (mL, seconds, max pressure, pauses).
2. Options struct (one per run, passed by the caller): `log` (print every window, pressure and table line), `pause` (pause at the limit like fill_test; off in production until stage 2 decides it), `interactive` (wait for a key between steps), `verbose_test_line`. The test menu sets log=true, pause as the test needs, interactive=true. `run_samples` sets log=false, interactive=false.
3. `fill_test` becomes a caller of pump_through_filter with log=true and pause=true. Its TEST line, its table and its stop criteria stay as they are (the criteria are already in FilterWatch, src/Hardware/Filter_watch.cpp). Diff the bench log of `fill_test volume=200` before and after: same end reason and the same mL.
4. `full_cycle` becomes a caller of the same bricks (the sequence is already in run_full_cycle, Tests_mech.cpp: purge, sample, shield, empty, unlock). `run_samples(list, opts)` is the same sequence over a list of slots, with a skip/abort check between slots. `full_all` calls run_samples.
5. Nothing is copied. If a test needs something the run does not, it is an option of the brick, not a second function. List every place where a test had its own copy of a physical step, and say which brick replaced it.
6. Purge volume (stage 1, S1-C addendum item 3): the spool-installed rule, purge_volume_ml(spool_installed) in src/Core/ with a native test. Print purge_ml and spool= on the TEST line.
7. Keys: the skip and abort keys (S1-C addendum item 1) live in the bricks, so the run and the test get them the same way.
8. Do not change: the PID, the cap, the lock rules, the stop criteria, the placeholders' values. Do not start pauses in production (that is a separate decision, listed as open below).
9. Open decision for the user (write the answer in the log, do not guess): does the production run pause at the limit like fill_test (option pause=true), or keep the PID only?
Native tests: option rules (log and pause flags per caller), purge_volume_ml with and without the spool, the skip and abort mapping, the result struct's end reasons.
Hardware checklist: (a) `fill_test volume=200` on slot 7 before and after: same end reason, same mL within 5 %; (b) `full_cycle 1 volume=500` in the test menu and `run_samples` on slot 1 with the same volume: the same sequence of steps in the log (the test log has more lines; the physical steps match); (c) press `s` during a run: the step ends `skipped` in under 2 s; press `x`: the run ends `aborted` with the valves closed and the O-ring locked.
```

#### FW5: Supervisor, `wait_ms()`, watchdog (P0)

```text
Read CLAUDE.md and stage2-pilots.md §3.2 (safety layers), finding F14.
Goal: what must ALWAYS run keeps running during long procedures.

1. A Supervisor (logic in src/Core/, hardware glue outside) with service(), called at least every
   50 ms whenever anything blocks. It checks the pressure cap and P_in plausibility for any
   running pump, checks the per-procedure deadline, kicks the watchdog, and exposes
   should_abort(). Its time and sensor sources are injected so native tests can fake them.
2. bool wait_ms(ms) calls service() and returns false on abort. Replace delay() in the
   procedure-level code (Step_functions, maintenance, calibrations, tests); keep
   delayMicroseconds and the stepper pulse timing. Add service() calls in: CtrlPump::run, the
   rotateMotor wait loop, Linear_actuator::step_pulses (every N steps, with N chosen so a pulse
   train is not disturbed), Micro_Pump::start(ms), Pump::start(ms), Pump::stop (the 500 ms delay),
   Button::waitPressedAndReleased/waitReleasedAndPressed, and the keypress helpers. List every
   change.
3. Abort propagation: procedures return early when should_abort() is true. For now, stop the pumps
   and log; the full safe state comes in FW6 (leave one on_abort() hook).
4. The Due hardware watchdog: the Arduino SAM core disables it by default through a weak
   watchdogSetup(); override it, and enable once with a <4 s> timeout (it can be configured only
   once per reset). Measure the longest gap between service() calls first (instrument it, print
   the max) and report it. A hidden console command `wdt_test` deliberately hangs.
5. Report the reset cause at boot (the SAM3X RSTC status: power-on / watchdog / software / user).
6. Native tests: the cap trips at the exact threshold; NaN and implausible values trip; the deadline
   trips; the latch holds until cleared; the waits return false once should_abort() is true.
Before reading the VNH5019 EN/DIAG or CS pins (F47), check HW1 M22-M23 in ../hardware/power.md: they must
be <= 3.3 V at the Due pins.
Hardware checklist: wdt_test resets the board and the boot prints "watchdog"; a cap trip during
full_cycle (clamp at the outlet, CAP at 1.0 bar) stops the pump within <200 ms> and the procedure
returns; full_cycle timing unchanged within 5 %; the max service() gap printed < 50 ms.
```

#### FW6: STOP, safe state, fault latch, recovery, boot, lean modes (P0)

> **Deferred from stage 1 (2026-10-19):** a stop by key press (or STOP) inside every pump loop (`CtrlPump::run`, `fill_test`, `pressure_ramp` already read a key) and during a rotation or a lock move; stage 1 uses the 24 V switch. Recovery after a reset: stage 1 uses `actuator_home` and a paper list of used slots.

```text
Read CLAUDE.md, stage2-pilots.md §3.2 (DECIDED safe state and recovery), §3.5 (lean modes), F11, F23,
and ../logs/LOG.md F71 (RECOVER deadlock: lock unknown + manifold between slots): ask me how
to resolve it (manual procedure, or homing the actuator off-slot if mechanically safe).
Hardware: E-stop <switch in series with the 24 V actuator rail, second contact on pin <D37>, NC to
GND with INPUT_PULLUP | switch only, no input yet: ESTOP_INPUT_INSTALLED=false>.

1. enter_safe_state(), idempotent, callable from anywhere: pump + micro pump off, ALL valves
   closed, manifold motor stopped, spool stopped, O-ring STAYS LOCKED if locked (the lead screw
   self-locks; the A4988 stays disabled); if unlocked, don't drive it into a lock; record it.
   A snapshot at stop time: slot, lock state, procedure, step, volume so far.
2. FaultCode enum replacing the Critical_error.cpp stub: OVER_PRESSURE, IMPLAUSIBLE_PRESSURE,
   ESTOP, USER_STOP, TIMEOUT, ENCODER, ROTATION_REFUSED, WATCHDOG_RESET, SENSOR_DEAD, ... A latched
   current fault + a 16-entry ring buffer with millis timestamps. Warnings (non-safety) have their
   own severity and never latch.
3. Lean ModeMachine in src/Core/ (ROADMAP §3.5): BOOT, RECOVERY_REQUIRED, IDLE, BUSY, SERVICE,
   FAULT, ESTOP, with an explicit transition table; illegal transitions are rejected and logged.
   - SERVICE = today's console test menu, entered with `service` (and at boot, see 6), left with
     `exit`. The old `stop` (which left the menu) becomes the emergency STOP; `exit` replaces it.
   - IDLE runs the legacy text protocol (main_program) unchanged until FW7 replaces it.
   - The button panel (button_control) is only active in SERVICE.
4. The E-stop input (if installed): debounced; the ISR only sets a flag; service() acts on it.
   Pressed OR a broken wire = stop (fail-safe).
5. Software STOP: `stop` typed on the console is detected from ANY blocking wait (service() peeks
   at the serial buffer without consuming other input) -> enter_safe_state() + FAULT(USER_STOP).
   The same entry point will serve the protocol STOP (FW7).
6. Boot (fixes F23): setup() no longer ends in test_all_components(). Every boot:
   RECOVERY_REQUIRED; O-ring UNKNOWN; valves driven closed; non-moving self-checks (encoder
   reads, P_in plausible near 0, flow pulses = 0 with the pump off); E-stop pressed -> ESTOP.
   BOOT_DEFAULT = SERVICE for now (the console is shown, as today); FW7 switches it to IDLE.
   Moving recovery steps wait for confirmation (console) or the host's RECOVER {confirm_moving}.
   F11: DEBUG_MODE_PRINT becomes a real #define; before_start_program()/system_checkup() stay
   off at boot (SYSTEM_CHECKUP_AT_BOOT=false) and system_checkup() is exposed as `checkup`.
7. `clear` is refused while the cause persists. `recover` runs the §3.2 checklist step by step,
   printing PASS/FAIL, stops at the first FAIL (if the O-ring is UNKNOWN and the manifold is
   aligned, it runs actuator_home; if not aligned, it re-homes the manifold to the nearest slot
   first). While recovery is required, every procedure and moving command is refused with
   "RECOVERY REQUIRED (cause: ...)", with a console reminder every 30 s. `faults` prints the ring.
8. Native tests: every legal and illegal mode transition; the latch; clear refused while the
   cause persists; recovery ordering and stop-on-FAIL with fake hardware; procedures refused
   while recovery is required; every boot path (normal, E-stop pressed, watchdog reset); abort
   latency < 100 ms in simulation.
Hardware checklist: console STOP and E-stop during: pump, rotation, actuator move, DNA shield,
emptying -> valves closed, O-ring still locked, TEST lines logged; after release, commands are
refused; clear + recover -> normal run; power-cycle mid-lock -> the boot asks for recovery;
20 cold boots from the battery supply (F32) -> 20/20 reach the console.
```

#### FW7: Protocol v1 in firmware, Native USB, boot into host mode (P0)

```text
Read CLAUDE.md, ../software/protocol.md, protocol/vectors.json, stage2-pilots.md §3.4-3.5, F8, F10, F24, F33,
and ../logs/LOG.md F37 (strip CR) and F53 (SerialUSB writes may block), and the HO1 findings
F70 (id uniqueness across host restarts: fix the ../software/protocol.md §3.1 wording), F72 (vectors vs log
copies), F73 (STATUS.warnings format), F74 (which commands need P_in): answer each in ../software/protocol.md.
Test against the host simulator too: host/ (`cowas --sim`), protocol behaviour must match it.
1. ArduinoJson (pin the version). The parser/serializer + the dispatch table live in src/Core/
   (native-tested against vectors.json, plus malformed, oversized and binary-garbage lines).
2. Two channels (§3.4): HOST_PORT = SerialUSB (protocol + {"t":"log"} copies of every log line),
   CONSOLE_PORT = Serial (human text, service menu). Route all output through one small Log API
   (level + channel). Settings: HOST_PORT can be set to Serial for a single-cable bench setup
   (then JSON lines start with '{', and the host ignores other lines). Test both.
3. Handlers call the EXISTING procedures with explicit parameters. RUN_CYCLE takes the slot from
   the host (the internal slot search stays only for the legacy commands). Long commands: an
   immediate ack, events for each step, then the result with the same id. BUSY accepts only STOP,
   STATUS, PING, HELLO, GET_INFO.
   Deferred from FW1 (../software/protocol.md §14): PRIME_SHIELD's destination (purge slot?) and duration
   (PRIME_SHIELD_MS, PLACEHOLDER): ask me, using the LA3 results if they exist.
4. STOP from the protocol uses the FW6 path. Request-id dedupe (last 16 ids + their results).
   Link watchdog: no PING for LINK_TIMEOUT_S -> warning event; the current procedure finishes
   safely; nothing new starts until the host is back.
5. Telemetry at ~1 Hz while BUSY via service() (P_in, flow, volume, phase, pump %, step).
   F53: output to the host port must NEVER block (../software/protocol.md §2): check availableForWrite() on
   SerialUSB, drop + count lines (STATUS.tx_dropped, warning OUTPUT_DROPPED). Test it: a host that
   opens the port and stops reading mid-PURGE must not stall service() (max gap < 50 ms).
6. BOOT_DEFAULT = IDLE (the host mode). The console stays reachable: `service` on the
   Programming port (refused while BUSY). Legacy main_program() commands: keep them behind
   LEGACY_PROTOCOL_ENABLED (default false), and remove them in P2.
7. tools/protocol_smoke.py (pyserial, cross-platform, VID/PID auto-detect of the Native port):
   HELLO, STATUS, GET_INFO, ROTATE 3, LOCK/UNLOCK, STOP mid-PURGE, CLEAR_FAULT, RECOVER, a retried
   request id, NEG_CONTROL -> not_available. A pass/fail summary + TEST lines.
8. Native tests: every vector; fuzzing (random bytes, truncated lines, 10 kB lines) never crashes
   and never runs a command; the mode/command acceptance table; dedupe; link-timeout logic.
Hardware checklist: protocol_smoke.py over the Native USB port: all PASS; restarting the host
program during a PURGE does NOT reset the Due; with a laptop on the Programming port, the console
still works; boot -> RECOVERY_REQUIRED -> host RECOVER -> IDLE.
```

#### FW8: Sample record and minimal clog end condition (P0)

> **Deferred from stage 1 (2026-10-19):** the layered pressure controller (setpoint ramp, soft limit and rise-rate limit, gain scheduling) and the clog rule. Stage 1 only ends a sample on `volume`, `timeout` or `stalled` (less than 3 mL in 60 s, a PLACEHOLDER of `fill_test`). Bench facts to design from: nothing flows below about 10 % of pump power, the pressure is very steep above about 12 % (about 3 bar per %), the reading pulses ±0.25 bar every 2–3 s (F137, F141); a model of the bench with the real `MiniPID` is in `test/test_pid_plant` (F146).

```text
Read CLAUDE.md, ../software/protocol.md (the RUN_CYCLE result), stage2-pilots.md §3.6-3.7, F15, F28, F30,
../lab/README.md (the fields the lab needs), and HW3's measured flow: <mL/min clean filter>.
1. A SampleMetrics accumulator in src/Core/ fed by service(): volume, duration, p_in
   {max, mean, final}, flow {mean, min, final}, per-phase durations, measured purge volume,
   shield_ms, t_end_to_shield_s, faults, warnings. Phase-aware: a pumping phase variable
   (PRIME, PURGE, WATER, AIR_PUSH, SHIELD, EMPTY, IDLE) is set by every step function. Volume
   and flow statistics count ONLY in WATER (turbine meters spin on air). Stats ignore a short
   blanking window after each phase change. Fill the RUN_CYCLE result (FW7); nullable fields
   stay null.
2. Minimal clog end condition (WATER phase only; the volume stop and the timeout stay
   independent): after a settling time, if the flow < CLOG_FLOW_MIN_ML_MIN (a param,
   PLACEHOLDER = <30 %> of the flow measured in the first minute of this sample, with an absolute
   floor) for CLOG_T_S (30 s) while the pump is at >= 95 % or P_in >= target - 0.2 bar ->
   end_reason = clogged. If the flowmeter goes silent (below its rated minimum) while P_in is at
   target, that also counts as clogged, and the warning says so. The first condition met sets
   end_reason.
3. The console `last_sample` prints the last record (it survives until the next cycle only;
   the host is the store).
4. Native tests: the accumulator on synthetic series (water -> air push -> shield) where air adds
   no volume; stats per phase; each end_reason (volume, clogged, timeout, pressure cap, stopped);
   no false clog in the first minute; a slow, steady flow is not clogged.
Hardware checklist: RUN_CYCLE 500 mL clean filter -> end_reason=volume_reached, volume within 5 % of
the weighed outlet water; RUN_CYCLE with the outlet clamp slowly closed -> end_reason=clogged;
the air push adds < 5 mL to the volume.
```

#### FW9: Unattended hardening (P1)

```text
Read CLAUDE.md, stage2-pilots.md §3.2, §3.8, F35, ../hardware/power.md, HW7/HW8 results:
home switch on <D39>, leak sensor on <D41/A6>, DS18B20 water probe on <D43>.
1. E-stop input fully wired (if not done in FW6); capability flag in HELLO.
2. Enclosure leak sensor: debounced; FAULT LEAK_ENCLOSURE (safe state + event).
3. O-ring home switch at "unlocked" (Hall or micro-switch, INPUT_PULLUP): actuator_home becomes
   "move toward unlocked until the switch, then back off"; lock/unlock verify the switch state
   (unlock must reach it within N steps, else FAULT ACTUATOR). ORING_CONTACT_BUTTON_INSTALLED
   stays for a future contact sensor.
4. Water temperature (DS18B20, 1-Wire at 3.3 V): the per-sample mean in sensors[]; an absent or
   implausible sensor -> warning only; also logged every 30 min between samples.
5. Native tests: debounce, fail-safe on a broken wire, the home-switch logic (found / not found /
   stuck), the temperature conversion and plausibility (-5..40 °C), absent-sensor behaviour.
Hardware checklist: each sensor unplugged -> the expected warning or fault; 50 lock cycles with the
home switch; a water-temperature reading vs a reference thermometer (within 0.5 °C).
```

### 8.2 Host track

#### HO1: Host skeleton, protocol client, simulator, CLI (P0)

Aldready ran before

```text
Read CLAUDE.md, stage2-pilots.md §3.1, §3.4, §5, ../software/protocol.md, protocol/vectors.json, and
computer_interface/ (legacy; do not modify it, findings F9 and F24 explain why it is replaced).
host/: a Python 3.11+ package `cowas`, cross-platform (Windows laptop for Pilot A, Raspberry Pi
later). No PyQt, no ZMQ ipc://, no Pi-specific imports outside host/src/cowas/adapters/.
1. pyproject.toml, src layout, ruff, pytest, logging config. CI runs pytest (edit ci.yml).
2. cowas.protocol: pydantic models from ../software/protocol.md; tested against every vector.
3. cowas.transport: pyserial, auto-detects the Due's Native USB port by VID/PID (never 1200 baud),
   reconnects. '{' lines -> protocol; other lines and {"t":"log"} -> the device log (timestamped
   UTC, rotated files under data/logs/).
4. cowas.client: request/response with ids, acks, per-command timeouts (from ../software/protocol.md),
   retry-safe resend (the same id), an event/telemetry subscription, a heartbeat PING.
5. cowas.sim.FakeController: implements the protocol with virtual time and injectable faults:
   pressure trip, clog, slow procedure, link drop, a reboot mid-cycle (RECOVERY_REQUIRED after),
   a flowmeter dead, a watchdog reset. It emits human log lines too and plugs in as a transport.
6. CLI `cowas`: ports, hello, status, stop, recover, rotate, lock, unlock, run-cycle --slot
   --volume|--no-volume-cap, monitor (live telemetry + events + logs), logs. `--sim` everywhere.
7. Tests: vectors; client vs simulator (timeouts, reconnect, dedupe on retry, STOP during
   RUN_CYCLE, the log separation, a reboot mid-cycle is detected via HELLO).
Bench check (after FW7): `cowas hello` and `cowas monitor` against the real Due.
```

#### HO2: Slots, deployments, sample records, CSV export (P0)

> **Deferred from stage 1 (2026-10-19), also for HO3:** the "laptop campaign runner" of the old stage 1 (S1-3, [archive/roadmap-stage1-2026-10-15.md](../archive/roadmap-stage1-2026-10-15.md)): a plan of samples (slot, role, volume, time), a slot inventory file that survives a Due reset and refuses a used slot, one sample CSV row per `TEST|full_cycle|...` line with the stage 2 column names, and stop rules (SAFETY, REFUSED, boot banner, idle timeout). The `cowas` host replaces it; take the column list and the stop rules from there.

```text
Read CLAUDE.md, stage2-pilots.md §3.6, §3.7, finding F1, ../lab/README.md, host/.
1. cowas.domain: SlotState (EMPTY, READY, IN_PROGRESS, SAMPLED, FAILED, DISABLED, SUSPECT) with an
   explicit transition table; the inventory is sized from GET_INFO (never 14). Deployment (id,
   site name, lat/lon entered by hand, intake depth m, intake hose ID/length, operator, filter lot,
   shield lot, start/end). SamplingEvent (planned time, volume or none, replicates 1-3, role:
   sample | equipment_blank; LA1 proposes also field_blank and manual_sample, recorded by hand,
   see ../lab/README.md E1, H1). Filter (event, role, replicate index, slot, sample_id).
2. sample_id format: <device>-<deployment>-<slot>-<yyyymmddThhmm> (or what LA1 decided: ../lab/README.md
   §4, E1; blanks and manual samples may use EB1/FB/MANn in place of the slot); printed
   labels (tools/labels.py -> a PDF or CSV for a label printer) from the same ids.
3. cowas.store: SQLite, a repository layer, a schema version + migrations, one transaction per
   transition. At startup, IN_PROGRESS -> SUSPECT.
4. The reload flow: the operator confirms which slots got fresh filters (and the filter lot);
   the previous deployment is archived; those slots become READY; DISABLED keeps its reason.
   Command names used by ../lab/filter-sop.md (LA2): `cowas slots` (list), `cowas slots load
   <slots> --filter-lot <lot>`, `cowas slots unload` (enter the retrieval time per filter ->
   retrieval_time_utc, and mark unused filters), `cowas clean` (decontamination flush with no
   filters: PURGE, then SAMPLE through each sample slot, then EMPTY, pausing for the operator
   between the bleach / water / DNA-free water buckets).
5. A SampleRecord per filter = the RUN_CYCLE result + event + deployment + versions + notes.
   Export ONE CSV with all selected filters (a deployment, a date range, or all): one row per
   filter, the columns of ROADMAP §3.6 in the order of ../lab/README.md, a header comment
   with schema_version and the export time, times in UTC ISO 8601 + a local-time column.
   `cowas export --deployment X -o file.csv`. sensors[] -> <name>_<stat>_<unit> columns.
6. Tests: every legal and illegal transition; persistence across restarts; IN_PROGRESS ->
   SUSPECT; replicate allocation follows FILL_ORDER; 15- and 24-slot geometries; an export golden
   file; an unknown sensor adds a column with no code change; a blank row has role
   equipment_blank.
```

#### HO3: Mission runner, scheduler, cold-boot resume (P0)

```text
Read CLAUDE.md, stage2-pilots.md §3.1, §3.6, §3.7, host/ (HO1-HO2), ../software/protocol.md.
1. Plans: a YAML/CSV plan file (events: time, role, replicates, volume) + a recurring form
   (start, interval, count). Import the legacy formats (Samples_planned "dd/mm/yy HH:MM depth",
   CSV "date;time;depth") with a warning that depth is ignored (fixed intake). Validation:
   enough READY slots (replicates included), no overlap given the expected cycle duration
   (from the measured flow), times in the future, the clock is sane.
2. Runner: one procedure at a time. For each event, allocate filters to the next READY slots in
   FILL_ORDER (from GET_INFO), run RUN_CYCLE for each, store the record. Policies (config):
   - late event: run if late < X min, else skip + log;
   - sample fault (end_reason fault/timeout): slot FAILED, retry on the next slot up to N times;
   - system fault (STOP, ESTOP, OVER_PRESSURE, LEAK, RECOVERY_REQUIRED after a reboot): pause the
     mission, never auto-retry, wait for the operator (`cowas recover`, `cowas resume`);
   - warnings: continue, flag the record;
   - end_reason clogged: NOT a fault, the sample is kept;
   - equipment_blank events (../lab/filter-sop.md §6): pause BEFORE the event (the operator moves
     the intake hose into the DNA-free water bottle) and AFTER it (back to the lake, or retrieval
     after the end blank); resume only on the operator's confirmation, never on a timer.
3. Before the first event, and before each event, check the Due: HELLO (firmware hash as
   expected), STATUS IDLE, no recovery pending.
4. Cold-boot resume (the host restarts or loses power): reload the mission from SQLite;
   IN_PROGRESS -> SUSPECT; recompute the next event; apply the late policy.
5. `cowas mission load|validate|start|pause|resume|abort|status`, and `cowas mission dry-run`
   (runs the whole plan against FakeController in accelerated time and prints the timeline).
6. Tests (injectable clock + FakeController): 14 samples over 2 days; 4 events x 3 replicates +
   2 blanks; a power loss mid-cycle (SUSPECT, then continue); a link drop; a clog on sample 3 (kept,
   not a fault); STOP; a Due reboot mid-cycle; out of filters; overlapping events rejected.
Bench check: a 3-event plan every 30 min against the real Due on a bucket.
```

#### HO4: Deploy on the Pi (P1)

```text
Read CLAUDE.md, host/, stage2-pilots.md §3.8 (power), ../hardware/power.md.
Hardware: Raspberry Pi CM4 on a TOFU carrier, Raspberry Pi OS <version>; RTC battery <present?>.
1. Scripts + docs/DEPLOY_PI.md: a venv install, a systemd service (auto-restart, starts after the
   USB device is up), journald limits, a udev rule giving the Due's Native port a stable name
   (/dev/cowas-due), the SQLite on the SD card with a nightly backup copy, a read-mostly setup to
   survive power cuts (fsync'd writes; consider overlayfs for the OS), an update procedure.
2. Clock: refuse to start a mission unless the time is trustworthy (NTP synced, or the RTC valid
   and set within the last N days); log the source. Document how to set the time in the field
   with no internet (from the laptop over SSH).
3. A Wi-Fi hotspot fallback so a phone/laptop can SSH in on site.
4. A power-cut test script: start a mission against the simulator, cut power, reboot -> the
   service resumes, SUSPECT is correct. Measure the Pi's idle and busy current (for HW5).
5. docs: how to swap between the laptop and the Pi as the host (the same CLI, the same DB file).
```

#### HO5: Remote access and alerts lite (P1)

```text
Read CLAUDE.md, stage2-pilots.md §3.2 (layer 5), host/, docs/DEPLOY_PI.md.
1. Tailscale on the Pi + the phones and laptops of the people who use it (docs + scripts). LTE through the TOFU modem
   (ModemManager/NetworkManager, APN <...>), Wi-Fi first, LTE fallback.
2. cowas.alerts: rules -> severity (critical: STOP/E-stop/over-pressure/leak/fault pausing the
   mission/controller link lost > 10 min/recovery pending; warning: sample ended clogged or
   timeout, few READY slots left, low disk, clock not synced; info: sample done, mission complete).
   Critical alerts repeat every <15> min until acknowledged (`cowas alerts ack <id>`). Dedup and
   rate limiting. Every alert stored with its ack (who, when).
3. Channels behind one interface: ntfy (self-hosted or ntfy.sh with a private topic) or a
   Telegram bot (config), + a daily status message (slots used, last sample, battery voltage if
   known, disk, temperature).
4. Bandwidth: telemetry is not pushed over LTE; logs on demand (`cowas logs --since`).
5. If the TOFU modem has GNSS, record the position in the deployment automatically.
6. Tests: the rule table with the simulator, repeat-until-ack, dedup, rate limits, a delivery
   failure + retry, and the daily report content.
```

### 8.3 Lab / eDNA track

These prompts produce documents, tools and analysis scripts. The lab work itself is done by people.

#### LA1: Lab alignment (P0, week 1)

This was run before.

```text
Read stage2-pilots.md §3.6, §3.7. Draft ../lab/README.md as a meeting template with the lab
<lab name, contact>, with an empty "answer" column that we fill in during the meeting:
- extraction kit and whether it supports Sterivex + DNA/RNA Shield directly (e.g. a
  Sterivex-specific kit) or needs a protocol change;
- the target shield volume per Sterivex and the minimum acceptable; the max acceptable residual
  water (the shield/water ratio);
- the target filtered volume (2 L?) and the minimum usable volume;
- storage: how long filters may stay in the machine (temperature range), the transport
  conditions after retrieval, storage until extraction;
- labelling: the sample_id format, label type (freezer- and ethanol-proof), who prints them;
- blanks: equipment blanks per deployment (proposal: start + end), field blank, extraction
  blanks, PCR negatives; the DNA-free water source for equipment blanks;
- positive control policy (later: automated; now: none, or a lab mock community?HW1 );
- the metadata columns the lab needs and their names (start from ROADMAP §3.6), units, formats;
- chain of custody (who hands over what, a form);
- the markers/assays planned (e.g. 12S fish, COI, 16S) -> do they change the volume target?
- what a successful Pilot A means for the lab (e.g. "blanks clean, machine vs manual replicates
  give comparable detections").
Also add a one-page "Sterivex handling in CoWaS" summary for the lab.
```

#### LA2: SOPs: filter loading/unloading, cleaning, labels, chain of custody (P0)

This was run before.

**Status (2026-09-25): drafted, not done.** ../lab/filter-sop.md covers items 1–6 below, with a checkbox and a "Who" field on every step (branch `la2-sops`; LOG "LA2"). It was written **before** the lab meeting, so ../lab/README.md was not filled in.

Left to do:
1. (Done 2026-10-23, LA1 dropped: the lab markers are gone from the SOP; the values are in shield-protocol §8.)
2. On the hardware: check the Sterivex orientation text (inlet = female luer-lock) on the real filter model, and whether the rotor turns when a filter is screwed on with the O-ring locked.
3. Commands the SOP names but that don't exist yet: `cowas slots load` / `slots unload` / `cowas clean` (HO2 prompt, item 4) and the pause around equipment blanks (HO3 prompt). Once they exist, drop the "until HO2/HO3" notes in the SOP. The printed labels need HO2's `tools/labels.py`.
4. The SOP dry run in the pending checks (loading ≤ 30 min, unloading ≤ 20 min, 0 caps left on, 0 thread drips, 0 label mismatches). Loading and unloading alone can run before FW7.
5. Then tick LA2 in §7.

```text
Read stage2-pilots.md §3.7, ../lab/README.md (filled in), ../software/commands.md.
Write ../lab/filter-sop.md, a checklist people can print:
1. Before loading: clean the manifold contact faces and the wetted parts (bleach at the % agreed
   with the lab, a contact time, DNA-free water rinse, then dry), gloves, a clean surface, the
   filter lot recorded.
2. Loading: which slots, orientation, luer tightness check, cap removal order, the host command
   to mark the slots READY (`cowas slots load ...`), a photo of the loaded manifold.
3. Unloading: the order, capping both ends, labelling from the printed labels, into a cooler with
   ice packs, the time recorded, a photo.
4. Cleaning after a deployment + biosecurity (quagga mussels: clean, rinse, dry all wetted parts
   before any other water body).
5. The chain-of-custody form (a printable table).
6. The equipment-blank procedure: DNA-free water bottle, intake hose tip moved to the bottle, the
   purge volume of blank water first, the host event with role equipment_blank.
Each step has a checkbox and a "who" field.
```

#### LA3: Shield volume, residual water, shield retention (P0)

> **Deferred from stage 1 (2026-10-19):** the shield time is a test value (`FILL_STERIVEX_TIME / 10` = 25 s, F4) in stage 1; the dose and the residual water are measured here.

This was run before.

```text
Read CLAUDE.md, stage2-pilots.md §3.7, findings F4, F28, F29, ../lab/README.md (target shield
volume <x> mL, max residual water <y> mL, retention limit, temperatures: its §4, B1-B5, D1), and the FW4
emptying-phase logs.
Goal: the numbers we need to trust each filter. We have a 0.01 g scale.
1. A console procedure `shield_calibrate`: dispense the micro pump into a tared tube for T s, 3
   repeats at 2-3 durations -> mL/s (density ~1.0; ask me the product density if known) and the
   repeatability; prints DNA_SHIELD_FILL_MS for the target volume (TEST lines).
2. ../lab/shield-protocol.md: a weighing protocol per filter:
   dry filter mass -> after RUN_CYCLE without shield (water + air push) = the residual water ->
   after the full cycle = water + shield -> after 1, 3, 7 days in the manifold (O-ring LOCKED
   as in production, the rest state of §3.2, at room temperature and at <25-30> °C if possible) = retention.
   3 filters per condition. Also test the air-push variants: today's settings, and a longer
   EMPTY_STX_MAX_MS at a higher air pressure still below the membrane bubble point.
3. tools/analyze_la3.py: reads a CSV of masses, computes residual water, shield volume, the
   shield/water ratio, retention (% per day), mean ± SD, and prints PASS/FAIL against the lab's
   thresholds. pytest with a synthetic dataset.
4. Recommend: DNA_SHIELD_FILL_MS, the air-push settings, and whether the filter outlets need
   caps/check valves after the shield even with the O-ring locked at rest. Also answer the question
   deferred from FW1: how much shield PRIME_SHIELD must push to fill the line before the first
   sample, and where it should go (the purge slot?). Ask me before changing
   code; any change goes into a small follow-up PR.
```

#### LA4: Carry-over and slot cross-talk (P0)

This was run before.

```text
Read CLAUDE.md, stage2-pilots.md §3.7, findings F29, F30, and HW6 (intake hose ID/length).
Goal: prove that a sample doesn't contain the previous one, and that slots don't leak into each
other. Use a harmless tracer: food dye (absorbance) or salt (conductivity), whichever we have
instruments for: <spectrophotometer | conductivity meter | phone camera photometry>.
1. the LA4 protocol (not written yet):
   a. Dead volume: fill the intake + pump + sensors with dye, then pump clear water, sampling the
      outlet every 25 mL -> the washout curve -> the internal dead volume (the INTERNAL_DEAD_VOLUME_ML
      of FW4) and the purge factor needed for < 0.1 % carry-over.
   b. Carry-over into a filter: run a dyed "sample" on slot A, then a clear-water cycle on slot
      B with the FW4 purge -> measure the dye in slot B's outlet water and its filter.
   c. Cross-talk: fill slots A with dye-shield-like liquid, rotate through all other slots with
      clear-water cycles -> check the neighbours of A and the slots the rotor passed over.
   d. Optional with the lab (../lab/README.md G3): the same with a DNA spike (e.g. a species absent from the lake)
      and qPCR on the following filter.
2. tools/analyze_la4.py: the washout curve fit (exponential), the purge factor for a target
   carry-over, the cross-talk table; pytest on synthetic data.
3. Recommend the PURGE_DEAD_VOLUME_FACTOR and INTERNAL_DEAD_VOLUME_ML values for Settings.h, and
   whether a decontamination flush (bleach) is needed before Pilot B (a P2 item otherwise).
```

### 8.4 Hardware / field track (with Claude's help)

Part 1 and 2 already ran before

#### HW1: Electrical audit (P0, week 1) — prompt

```text
Read CLAUDE.md, ../hardware/pinout.md, stage2-pilots.md §3.8, §11, findings F20, F25, F26, F32, F36, and
include/Settings.h. I will measure things on the machine; you prepare and then record the audit.
1. Write ../hardware/power.md: a block diagram (ASCII) of how power flows today (supply -> 24 V rail ->
   pump driver, VNH5019 shield, A4988, valves, micro pump; -> the Due; -> the Pi), with empty
   fields for what I don't know yet, and the proposed pilot topology of ROADMAP §3.8.
2. Give me a measurement checklist, one line per item, with what to measure, how, and the
   acceptable value:
   - the pump driver: model, input range, what DAC1 0 % and 100 % give (F25), pump speed at
     0.55 V ("0 %") with the relay on; is the DAC buffered?
   - the Due's power input (Vin/jack/5V pin, which voltage), and the USB power path;
   - the flowmeter output high level on D13 (F26) and the flowmeter supply voltage;
   - the AMT22 MISO high level (F36), the ABP supply at its pins (3.3 V);
   - the VNH5019 EN/DIAG pull-up voltage seen by the Due pins;
   - flyback diodes on every valve and relay coil;
   - the 24 V rail current: idle, pump at 50/90 %, stepper moving, valves on;
   - the A4988 Vref vs the motor's rated current (42SHD034-20B).
3. After I send the results, update ../hardware/pinout.md and ../hardware/power.md, and list the fixes (divider, buffer,
   pin move, fuse) with the parts to add to ROADMAP §11.
```

**Status (2026-09-25):** items 1–2 done on branch `hw1-electrical-audit` (`../hardware/power.md`; the checklist is ../hardware/power.md §4, M1–M44, with commands and acceptance values; a row in the pending checks). **Part 2, prompt for the session that gets the results:**

```text
Read CLAUDE.md, ../logs/LOG.md (HW1 entry), ../hardware/power.md, ../hardware/pinout.md. Here are the HW1
results: <M-number: value, ...>.
1. Fill in the Result column of ../hardware/power.md §4, every ___ in ../hardware/power.md §1 and the load table, and
   ../hardware/pinout.md's "Reachable on the PCB shield?" (M44, F20) and "Measured level" columns.
2. Flag every result outside its "Acceptable" value. For each: the fix and its parts, added to
   ROADMAP §11.1 (or §11.2 for a pin move). Candidates: DAC buffer, with gain if the pump driver
   input is above 2.75 V full scale (F25); flowmeter at 3.3 V or a divider (F26 -> HW3); the
   ARDVIN=VOUT jumper removed (M9); the shield logic VDD / EN/DIAG level (M22-M23, needed before
   FW5 reads the VNH5019 diagnostics, F47); missing flyback diodes; the D11 pull-up (M25); fuse
   ratings = 1.5 x the measured branch currents (M32-M39) -> HW2; the E-stop DC rating and the
   boost rating from the peak (M34) -> HW2/HW5.
3. Decide the Due supply for the pilot topology (../hardware/power.md §3) and write it into HW5.
4. Log new findings in ../logs/LOG.md (<F-range if steps run in parallel>), finish the HW1 entry,
   move the the pending checks row to Done, tick HW1 in §7.
```

Notes for later steps: **don't change the A4988 Vref** without re-running `calibrate_actuator` (the stall point sets `LINEAR_ACT_LOCKED_STEPS`; HW7, FW4). Pumping during HW1 only with an open outlet (F50). ../hardware/power.md §5 (energy) is filled in by HW5.

#### HW2–HW8: tasks and checklists (people)

> **Deferred from stage 1 (2026-10-19):** **HW2** relief valve: the planned 2.5–3 bar of ../hardware/power.md would open below the cap of 3.2 bar (F143): choose it above the cap, or lower the cap; check the pressure rating of the weakest part (Sterivex housing, tubing, valves, O-ring). **HW3** flowmeter: a 3.3 V safe input and a calibration by weighing (F6, F26); stage 1 corrects the factor once by weighing. **HW7** the manifold seal drips near 3 bar (F144): lock force, lock steps, seal inspection.

| Task | Checklist (tick when done) |
|---|---|
| **HW2** Safety hardware | ☐ Relief valve set to 2.8 bar ±0.2 (check with the sensor and a clamp) and plumbed to waste · ☐ E-stop: latching mushroom switch in series with the 24 V actuator rail (DC rating ≥ 2 × max current), or through a relay if not · ☐ second contact to a Due pin (P1) · ☐ fuses per branch (pump, VNH5019, A4988, valves), rated ~1.5× the measured current (HW1 M32–M39) · ☐ ≥ 47 µF across the A4988 VMOT at the driver · ☐ the E-stop cuts the actuators only, not the Due (../hardware/power.md §3) · ☐ E-stop test: pump running → press → everything stops < 0.5 s |
| **HW3** Flowmeter | ☐ Start from HW1 M15–M17 (supply, D13 level, D12 wiring) · ☐ Output level safe for 3.3 V (sensor powered at 3.3 V if its spec allows, or a divider/open-collector + 3.3 V pull-up) · ☐ pin chosen (keep D13 if a pulse test passes, else D36) · ☐ `calibrate_flow` against a measuring cylinder **and** by weighing: 3 runs × 500 mL, spread < 3 % · ☐ **real sampling flow** measured with a clean Sterivex at 2 bar (mL/min) → FW4's `SAMPLE_MIN_EXPECTED_FLOW` and the outlet meter decision (P2) · ☐ flow with a half-clogged filter: does the meter still count (MW-FS-2.0 minimum 0.15 L/min)? |
| **HW4** Pressure sensor | ☐ Pin 1 identified before powering (6-pin DIP, not reverse-polarity protected; procedure in the pending checks) · ☐ powered from 3.3 V (confirmed at the connector in HW1 M20), SS on D8 · ☐ Mounted on a tee between the pump and the manifold, port facing down or sideways (no trapped air) · ☐ barbs clamped (the pop-out issue) · ☐ short cable (< 30 cm) or twisted pairs with GND · ☐ FW3 checklist passed |
| **HW5** Power | ☐ Battery (Pilot A: 12 V LiFePO4 ≥ 50 Ah with BMS) · ☐ regulated 24 V boost (≥ 1.5 × the peak current) for actuators · ☐ 5 V buck ≥ 3 A for the Pi · ☐ Due supply per HW1 (../hardware/power.md §3; the VNH5019 ARDVIN=VOUT jumper removed, HW1 M9) · ☐ fill in ../hardware/power.md §5 (energy) · ☐ main switch + main fuse · ☐ measured: idle W, one full cycle Wh → `../hardware/power.md` → battery sized for Pilot A and B · ☐ **20 cold boots** on the battery supply, 20/20 OK · ☐ a pump start doesn't brown out the Due (watch the reset cause in the boot log) |
| **HW6** Field fluidics (Pilot A) | ☐ Intake: strainer (~200–500 µm mesh) + weight + rope marked every metre, at a fixed depth `<1–5 m>` · ☐ hose ID and length recorded → FW4's dead volume · ☐ suction lift: the pump primes from the water level to the machine (test at the real height) · ☐ outlet discharges ≥ 5 m downstream of the intake · ☐ quick-disconnects for transport · ☐ splash-proof box (IP54+) with the electronics above the fluidics, shade cover (sun heats the box and the filters) · ☐ everything fits on one trolley/boat |
| **HW7** O-ring lock | ☐ A4988 Vref unchanged since the last `calibrate_actuator` (HW1 M42; any Vref change moves the stall point → recalibrate) · ☐ `lock_endurance 200`: no drift in the encoder angle, no leak at the end (`leak_hold`) · ☐ with the coils off while locked (FW4): pressurize to 2.5 bar with the outlet clamped for 10 min → no leak, no backlash (confirms the self-locking under pressure) · ☐ **rest locked** (FW1 decision, §3.2): O-ring locked for 7 days, then unlock/lock and `leak_hold` → no leak (checks compression set of an O-ring kept compressed between samples) · ☐ P1: "unlocked" home switch (sealed Hall + magnet) on D39 |
| **HW8** Pilot B hardware | ☐ Enclosure IP65+, cable glands, desiccant, internal temperature logger · ☐ enclosure leak sensor at the lowest point · ☐ DS18B20 water probe on the intake · ☐ mounting/mooring plan, anti-theft, a "scientific equipment + contact" label · ☐ access agreed with the pier/harbour owner · ☐ site access and a swap plan for the battery |

### 8.5 Validation

#### VA1: Bench acceptance campaign (P0, week 7) — prompt

```text
Read CLAUDE.md, stage2-pilots.md §3, §5, §9, ../software/commands.md, ../software/protocol.md, host/ (HO1-HO3),
../lab/shield-protocol.md and the LA4 protocol (not written yet) results.
Write docs/VALIDATION_VA1.md + tools/va1_run.py (drives the host CLI, collects logs, docs/logs/test-logs and
the export into docs/logs/test-logs/va1_<date>/). Each test: purpose, setup, steps, numeric pass criteria,
a results table. Tests:
1. A full 14-slot mission from the host on a bucket (2 L each or 500 mL for speed, decide with
   me), on the pilot battery, with the laptop as host: every record complete, volumes within 5 %
   of the weighed outlet water.
2. A 24 h unattended run (e.g. 6 events) with the machine alone in a room.
3. A power cut (main switch) in each phase: purge, water, air push, shield, empty, rotation, lock
   -> boot -> RECOVERY_REQUIRED -> recover -> the host marks SUSPECT, the mission resumes.
4. STOP from the CLI and the E-stop in each phase -> the safe state, then recovery.
5. A host restart (kill the process) mid-cycle -> the Due is NOT reset, the cycle ends, the host
   reconnects and gets the result by request id.
6. A clogged filter (outlet clamp) -> end_reason clogged; the cap is never exceeded (max P_in
   printed).
7. A pressure cap trip with the relief valve isolated -> trips at 2.5 ± 0.1 bar.
8. The USB cable pulled mid-cycle -> the cycle finishes safely, nothing new starts.
9. The export CSV opened by the lab (LA1 columns) -> accepted.
10. The battery: measured Wh per cycle and idle W -> the Pilot A margin ≥ 50 %.
Go/no-go rule: all tests 1-8 PASS; a FAIL on 9-10 needs a written workaround.
```

---
