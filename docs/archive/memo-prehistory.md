# PROJECT MEMO — CoWaS firmware/hardware changes (for later report)

> Archived on 2026-10-21, copied verbatim from the user's notes. The memo is undated: it was written before FW0 (2026-09-24). Its content is integrated into [LOG.md](../logs/LOG.md), section "Prehistory". The log entry gives the status of each item.

## INITIAL STATE
- Arduino Due, PlatformIO. main.cpp: setup() -> test_all_components() (loop1, serial test menu) -> on "stop" -> loop() (loop2: button_control() + main_program() RPi serial protocol)
- DEBUG_MODE_PRINT is `const bool` not `#define` -> #if(DEBUG_MODE_PRINT) blocks in main.cpp never compile (before_start_program(), system_checkup() dead code) — known pre-existing issue, not fixed
- Manifold: purge_angle calibrated via `cal` command (Manifold.cpp/calibrateEncoder) — user recalibrated purge_angle in Manifold.h, confirmed working (slot0 aligned)

## HARDWARE ADDED

### NEMA17 42SHD0034-20B + A4988 driver (bring-up, screw/o-ring linear actuator)
- Pins: STEP=9, DIR=10, ENABLE=11 (Due pins 6,7,8,12 already in use — avoided)
- MS1-3 tied GND (full step, 200 steps/rev)
- Root cause of major vibration/noise debugging session: DIR wire was disconnected on breadboard (not a code/resonance/current issue — coil wiring, Vref, MS pins all confirmed correct beforehand)
- **Why this component exists**: the manifold/plate assembly has a core trade-off between rotation and sealing — enough force to compress the O-ring and stop leaks makes the manifold too stiff to rotate; too little force and it leaks. The linear actuator resolves this by cycling: loosen to allow rotation → tighten to compress the O-ring → loosen again for the next cycle.
- Recommendation: move motor-power wiring (VMOT/GND/coil leads) off breadboard onto solder/direct connections; breadboard current-carrying contacts are a known reliability risk for stepper phase current

### A4988 wiring (as-built)
- VMOT/GND (motor power) — sourced from an existing Pololu Dual VNH5019 Motor Driver Shield's VIN/GND rail (24V), shared with the shield's other DC motors, not a dedicated supply. **Flag for follow-up**: total current budget on this shared rail (existing motors + stepper) not yet verified against source capacity.
- VDD (logic) <- Due 3.3V; GND (logic) <- Due GND
- STEP <- Due pin 9; DIR <- Due pin 10; ENABLE <- Due pin 11 (active LOW)
- RESET + SLEEP <- jumpered together (required, else driver stays disabled)
- MS1/MS2/MS3 <- GND (full step, 200 steps/rev)
- Recommended addition (not yet done): 10kΩ pull-up resistor from ENABLE to 3.3V (VDD rail), to prevent motor twitch on power-up/upload before firmware sets ENABLE state
- Motor coil pairs (4-wire bipolar, verified via multimeter continuity): Blue→1B, Red→1A, Green→2A, Black→2B (~6Ω within a pair, open/high-MΩ across pairs)
- VMOT/GND filter capacitor: spec called for ≥47–100µF. As-built: bank of 6×10µF/50V in parallel (measured ~43µF, continuity OL — healthy) plus a 7th unit added (measured ~55µF total, but continuity showed ~8MΩ instead of OL). **Open item**: isolate and re-test the 7th unit alone to confirm whether it (or a connection introduced when adding it) is the source of the leakage before relying on it long-term.
- Vref measured/set to 0.37V, against a calculated target of 0.4V (assuming 1.0A/phase from generic search-result specs, not the unit's own printed label). **Open item**: confirm actual rated current from the physical motor label; recalculate Vref if it differs.

### Mechanical / sealing issues (manifold–plate assembly)
- Alignment: the rotating manifold and the perpendicular port plate were not orthogonal — gap on one side, excessive compression on the other during rotation. Orthogonality fix identified as essential, to be addressed.
- Primary leak (water/liquid) was substantially reduced once the linear actuator began compressing the O-ring only when the manifold is properly aligned.
- Secondary leak identified between the aluminum plate and PTFE — planned fix: replace seals and bond PTFE to the aluminum plate.
- Pressure sensor pops out of its seal above a certain pressure — mechanical sealing issue, unresolved, to monitor.
- Sealing redesigned as a double O-ring arrangement (smaller inner seal + outer seal at same or slightly larger diameter than the previous single seal) to improve sealing performance.
- Water routing redesigned: previous design routed water through PLA (not watertight by design); new design uses two heat-set metal inserts embedded in the PLA with a seal between them, so water never contacts the PLA directly.
- Electrical schematic: some additions still need to be made manually / kept in mind for later — not yet formalized as of this memo.

### Force sensor: Yiwu GS02889 FSR (10kg thin-film, generic/no-brand)
- Pin A1 (A2 reserved/in-use for pressure2/BigPressure — do not reuse)
- Voltage divider: 3.3V - FSR - node(A1) - 10kΩ - GND
- RC low-pass filter added: 10µF cap (node to GND, parallel with 10kΩ) to fix severe noise from ~0.7m cable (EMI pickup on high-impedance unloaded FSR node, likely worsened by nearby A4988 switching) — confirmed fixed via test (clean 0 baseline, smooth ~600–670 peak on real press, previously spiking 0–100+ randomly)
- **Note for later**: cheap generic part, no repeatability spec from manufacturer — used ONLY as a binary contact/threshold detector, not a force reading. Recommend replacing with a calibrated force/load sensor if precise force control is ever needed.

### Motor sizing (planned, not yet done)
- Corrected earlier mix-up: NEMA number = frame size, not torque. NEMA14 is smaller/less torque than NEMA17. Recommended NEMA23 (or longer-body NEMA17) if more torque is needed. Verify actual target frame size before ordering.

## FIRMWARE ADDED

### Linear_actuator class (include/Linear_actuator.h, src/Hardware/Linear_actuator.cpp)
- `begin(step, dir, enable, steps_per_rev, microsteps, lead_mm_per_rev)`: pinMode setup, starts disabled, default speed 400 steps/s
- `set_direction(dir)`: writes DIR pin once, stores current_direction, 5µs settle delay
- `step_pulses(n)`: pulses STEP n times at step_delay_us half-period, updates position_steps using stored current_direction — does NOT touch DIR. **Fixes the vibrate/stall bug** from re-asserting DIR every chunk.
- `move_steps(n, dir)` = set_direction(dir) + step_pulses(n) — convenience wrapper
- `position_steps`: open-loop step count, no encoder feedback — can drift from true position if motor stalls
- `move_mm()`: unused, lead_mm_per_rev=0 placeholder

### Force_sensor class (include/Force_sensor.h, src/Hardware/Force_sensor.cpp)
- analogRead wrapper, `read_averaged()` (50 samples default) for smoothing

### Tests.cpp — jog helpers
- `jog_actuator_steps(total_steps, dir)`: moves in 10-step chunks, checks Serial each chunk, aborts on any real keypress (ignores stray \r/\n left over from the command that started the jog — fixes a false-abort bug), returns `aborted` bool
- `jog_until_contact(dir, threshold, max_steps, *aborted)`: same chunking, stops on FORCE_DEBOUNCE_CHUNKS (3) consecutive averaged readings ≥ threshold (debounce rejects single-chunk noise spikes), waits FORCE_SETTLE_DELAY_MS (100ms) after each chunk before reading (lets RC filter settle), else runs to max_steps (safety cap) or aborts. Returns true only if debounce-confirmed contact.

### test_sample_cycle() — full lock/pump/unlock sequence
1. Wait for start keypress ('x' aborts before anything moves)
2. **LOCK**: `jog_until_contact(backward, FORCE_CONTACT_THRESHOLD, LINEAR_ACT_LOCK_MAX_STEPS)` to find contact — dynamic reference point, self-corrects for O-ring wear → if found, `jog_actuator_steps(LINEAR_ACT_LOCK_EXTRA_STEPS, backward)` fixed dead-reckoning turn to fully compress → soft warning if post-lock reading < contact threshold (possible stall)
3. **PUMP**: `pump.start(POWER_PUMP)`, polls flow_sensor_small + pressure1 every loop; stops on whichever first: volume ≥ TEST_SAMPLE_VOLUME_ML, pressure > STX_MAX_PRESSURE, 3min timeout, or manual keypress. Hand-rolled, not CtrlPumpFlow, because CtrlPump's max_pressure_ field is unused/unenforced in existing pre-project code.
4. **UNLOCK**: fixed `jog_actuator_steps(LINEAR_ACT_PARTIAL_UNLOCK_STEPS, forward)` — dead reckoning from the contact point found in step 2, NOT sensor-based (sensor too close to noise floor for a second "partial" threshold, even post-filter) — always runs even if pump stage stopped early, so the O-ring isn't left fully clamped

### Calibration functions (guided, interactive; print suggested Settings.h constants — user manually edits + reflashes, same pattern as existing manifold `cal`)
- `calibrate_force_sensor()`: 3 rounds x 4 manual measurement stages (unloaded-before / loaded-locked / partial-unlock / unloaded-after), each stage: prompt → wait keypress → `force_sensor.read_averaged()` → store. After 3 rounds: per-stage mean + spread(max−min) printed, WARNING if spread > mean/2 (repeatability flag) or if unloaded-before vs unloaded-after differ significantly (drift/hysteresis flag). Suggests `FORCE_CONTACT_THRESHOLD = mean(unloaded) + 10% of (mean(loaded) − mean(unloaded))`. Partial-unlock stage measured/reported for reference only, not converted to a threshold (see Design Decisions).
- `calibrate_unlock_cap()`: single guided jog forward from a manually-set locked position, TWO checkpoints via keypress (doesn't stop actuator between them): 1st keypress = partial-unlock position reached (records position_steps as LINEAR_ACT_PARTIAL_UNLOCK_STEPS), 2nd keypress = fully disengaged (records position_steps as LINEAR_ACT_UNLOCK_MAX_STEPS suggestion = measured × 1.2 margin). Safety bound: 100,000 steps if 2nd keypress never comes.
- `test_force_sensor()`: raw diagnostic stream, `force_sensor.read_averaged()` printed every 200ms until keypress — used to visually confirm signal quality; this is what caught the pre-RC-filter noise problem and confirmed the post-filter fix.

### Tests.cpp — loop1 serial commands (summary)
- `linear_actuator` — manual jog test, max 1 rev/press, `v<n>` sets speed live
- `sample_cycle` — runs test_sample_cycle() above
- `force_sensor` — runs test_force_sensor()
- `calibrate_force` — runs calibrate_force_sensor()
- `calibrate_unlock` — runs calibrate_unlock_cap()

### Settings.h additions
- LINEAR_ACT_* (pins, steps/rev, slow speed ~6s/rev, lock/unlock step counts — mostly still PLACEHOLDERS, not yet calibrated)
- FORCE_* (pin, contact threshold placeholder=300, debounce=3 chunks, settle delay=100ms for RC filter)

## DESIGN DECISIONS / WHY
- Force sensor used ONLY for contact detection (lock stage) — large/clear signal. Partial-unlock threshold via sensor was tried and abandoned (too close to noise floor even after filtering, before final RC fix confirmed clean) — replaced with fixed calibrated step count (dead reckoning from contact point). Revisit sensor-based partial-unlock only if calibration data post-RC-fix shows a clean, separable "partial" signal.
- All auto sequences keep hard step-count safety caps independent of sensor (fallback if sensor fails/never triggers)
- CtrlPump's max_pressure_ field (existing pre-project code) is set but never enforced anywhere — sample_cycle's pump-stop logic was hand-rolled instead of reusing CtrlPumpFlow for this reason

## CALIBRATIONS — what was done/edited

Manifold encoder (pre-existing, extended):
- calibrateEncoder(): speed unified to new MANIFOLD_CAL_SPEED=15 (Manifold.h) - was
  inconsistent (20 in one call site, unused default 40 in another)
- Added direction choice: prompts 'c' for clockwise or any other key for anticlockwise
  before starting, so the operator can pick the safe direction from the manifold's
  current position (avoids re-approaching a known limit, per earlier vibration incident)
- Still prints purge_angle to manually copy into Manifold.h, same as before

Linear actuator + force sensor (new, consolidated):
- Old calibrate_force_sensor() (hand-turned) + calibrate_unlock_cap() merged into
  ONE motor-driven calibrate_linear_actuator() - closes a gap: LINEAR_ACT_LOCK_EXTRA_STEPS
  was previously never measured (just a "tune by feel" placeholder)
- New LINEAR_ACT_CAL_STEPS_PER_SEC=10 (~20s/rev), deliberately slower than the
  operational LINEAR_ACT_SLOW_STEPS_PER_SEC=33, for reaction time during calibration
- Flow: Part 1 (3x repeated, auto-retracts between rounds) measures contact
  threshold repeatability; Part 2 (single continuous pass) measures lock-extra,
  partial-unlock, and full-unlock step counts, all operator-marked via keypress
- Prints all 4 Settings.h values in one summary at the end
- New jog_until_keypress() helper: actuator moves in 10-step chunks, stops on any
  real keypress (ignores stray \r/\n) - operator decides "satisfied" moment

Flow sensor (new - none existed before):
- calibrate_flow_sensor(): pumps a target volume (operator-entered, default 2000mL),
  stops on sensor target/timeout/manual key, operator enters the ACTUAL measured
  volume, computes corrected calibrationFactor_small = old_factor * (sensor_reported/actual)
- Prints suggested value for Flow_sensor.h
- flow_sensor_big not covered (unused elsewhere in the codebase beyond init)

DNA-shield pump (pre-existing calibrate_DNA_pump() untouched; new optional add-on):
- calibrate_dna_pump_volume(): timed test run (operator-chosen duration) + operator-
  measured dispensed volume -> flow rate (mL/s) -> suggested run time for a target
  volume. No flow sensor on this line, so this informs the existing time-based
  constants (PUMP_SHIELD_TIME, FILL_STERIVEX_TIME in Settings.h) rather than
  computing a calibration factor directly - operator updates those manually if
  the suggested time differs meaningfully
- Explicitly optional, offered as a follow-up after calibrate_DNA_pump()

Pressure sensor: SKIPPED - no calibration hook exists (BigPressure's zero-offset
is a hardcoded constant inside convertToPressure()); would require reworking the
sensor class itself, out of scope for now.

Commissioning orchestrator (new):
- run_commissioning_calibration() (`calibrate_all`): walks through manifold encoder,
  linear actuator/force sensor, flow sensor, and (optional) DNA pump + its volume
  check, asking y/n before each so any can be skipped. Each sub-calibration still
  prints its own values; this just chains them for one-pass field commissioning.
- New ask_yes_no() helper for the y/n gating

Commands: calibrateEncoder via existing `cal`; calibrate_actuator; calibrate_flow;
calibrate_dna_volume; calibrate_all (runs everything above in sequence)

## MANIFOLD PHYSICAL LAYOUT & FILL ORDER — what was done/edited

CONTEXT
- 16 physical positions around the manifold's rotation circle, 22.5deg apart
- 1 position has no hole at all (must never be targeted - pumping into it pumps
  into solid material) - fixed relative to the calibration point, unaffected by
  where purge is assigned
- Purge relocated from top (calibration point) to bottom (180deg away)
- 14 remaining positions = samples

KEY INSIGHT (avoided a much larger, riskier change)
- scale_angle()/directionDetermination() (the existing anti-tangle shortest-path
  CW/CCW logic) are already expressed relative to purge_angle, not an absolute
  "top" - so none of that logic needed to change
- Calibration procedure/meaning of purge_angle is UNCHANGED (still calibrated at
  the physical top, via existing `cal` command)
- Moving purge is purely a re-indexing exercise: purge already sits at position
  index 8 in the existing angle formula (180deg = 8 steps of 22.5deg) - nothing
  needed recalculating

CHANGES MADE
- Manifold.h: new PURGE_RAW_OFFSET=8 constant (raw steps from calibration ref to
  the actual purge port). omitted_angle_nb stays 12 (unchanged - the no-hole
  position never moved, only which slot is "purge" moved)
- Manifold.cpp (Manifold::begin()): angle-building loop rewritten to walk outward
  from the purge position (offset by PURGE_RAW_OFFSET) instead of from the
  calibration point, so index 0 = purge and 1-14 = samples in ONE unified list
  (single index space, not two - simpler than an earlier two-space design that
  was proposed and discarded before implementation)
- Settings.h: PURGE_SLOT stays 0 (unchanged). NB_SLOT changed 14 -> 15 (see bug
  below). New FILL_ORDER[14] const array (see strategy below)
- Step_functions.cpp: both slot-search loops (sample_process, demo_sample_process)
  now walk FILL_ORDER instead of scanning raw index order
- main.cpp: removed 2 now-redundant manifold.change_state(PURGE_SLOT, unavailable)
  calls - purge is structurally unreachable via FILL_ORDER, no flag needed

PRE-EXISTING BUG FOUND AND FIXED AS A SIDE EFFECT
- slots[NB_SLOT] was sized 14 (indices 0-13) but the angle table always produced
  15 addressable positions (0-14) once one no-hole position is excluded - meaning
  ONE physical sample slot (index 14) was structurally unreachable by the
  slot-search loop even before any of this session's changes (search loop never
  reached index 14, and get_state(14) would have been out-of-bounds). Fixed by
  NB_SLOT: 14->15, matching the array size to the actual index range.

FILL ORDER STRATEGY (why this order)
- Rationale: gravity - if a filled slot leaks, drip only reaches slots physically
  below it. Filling low-to-high means any leak can only reach ALREADY-FILLED
  (closed off) slots, never an unused one
- Grouped by physical quadrant (not strict global height-ordering) to minimize
  manifold rotation - accepted as a reasonable trade-off since realistic leak
  spread is local (adjacent holes), not global across the whole circle
- One correction made during design: the single highest point (directly opposite
  purge) must be filled dead last (nothing is higher, so nothing after it is at
  risk) - an earlier draft had it filled before 3 other lower slots, which was
  caught and fixed before implementation
- Final order (human_idx 0=purge, 1-14=samples): {1,2,3,14,13,12,11,10,9,8,4,5,6,7}

DIRECTION AMBIGUITY - RESOLVED, THEN ADDED A DIAGNOSTIC
- The code's internal slot-numbering direction (which way "increasing slot" goes)
  turned out to run OPPOSITE to the user's own clockwise-labeling convention -
  caught via cross-checking the existing omitted_angle_nb=12 against the
  no-hole's stated physical position. Confirmed algebraically that this ambiguity
  does NOT affect correctness of any slot math (purge is 180deg = symmetric;
  height-tiers are symmetric by construction) - only affects which word ("CW"/
  "CCW") a human uses to describe it
- Added identify_manifold_direction() (Tests.cpp): commands a known motor
  direction for a short, small rotation, asks the operator to report what they
  observed, returns to start. Purely informational/diagnostic, wired as a
  standalone command (`manifold_direction`) and as an optional step in
  run_commissioning_calibration()

VERIFICATION STATUS
- Math verified algebraically (angle formulas, symmetry arguments) across several
  rounds of user correction - NOT yet verified against the physical machine
- User should run `cal` (unchanged procedure) then sanity-check the human_idx
  table against the physical device before trusting sample_process()/
  demo_sample_process() for a real run

## PENDING / NOT YET DONE (as of this memo)
- calibrate_force and calibrate_unlock not yet run with final hardware — Settings.h still has placeholder values (FORCE_CONTACT_THRESHOLD=300, LINEAR_ACT_LOCK_EXTRA_STEPS/PARTIAL_UNLOCK_STEPS/UNLOCK_MAX_STEPS all guesses)
- Hardware changes in progress: motor swap (torque), possible other rewiring — recalibrate everything (manifold purge_angle already done; force sensor + actuator steps still pending) after hardware settles, in one batched commissioning session (manifold + force sensor + unlock cap), single reflash
- Manifold/plate orthogonality fix — not yet resolved
- Aluminum/PTFE seal replacement and bonding — not yet done
- Pressure sensor pop-out at high pressure — mechanical fix not yet identified
- ENABLE pull-up resistor (10kΩ to 3.3V) — recommended, not yet installed
- 7th filter capacitor showing ~8MΩ leakage — needs isolation/re-test before trusting long-term
- Motor's actual rated current (for Vref) — confirm from physical label, currently based on generic spec

## RECOMMENDATIONS FOR FUTURE WORK
- Replace generic FSR with a calibrated force/load sensor if proportional force control (not just contact detection) is ever wanted
- Fix DEBUG_MODE_PRINT (`const bool` -> real macro) to restore system_checkup()/before_start_program()
- Get motor-power wiring off breadboard permanently
- Consider wiring MS1/MS2 to spare pins for microstepping if full-step cogging/resonance becomes an issue again at other speeds
- Verify current budget on the shared Pololu VNH5019 VIN rail once stepper is in regular use, or move stepper to its own dedicated supply

1. Hard pressure cap (never exceed STX_MAX_PRESSURE = 3 bar) Centralized in the Pump class itself rather than patched into every call site: Pump.cpp now has set_pressure_safety()/enforce_pressure_safety(), checked inside set_power(), start(), and the polling loop in start(uint32_t). CtrlPump::run()'s previously-dead max_pressure_ field is now a real cutoff. wait_for_keypress() (used by nearly every manual test) also polls it while blocked. Wired up once in main.cpp via pump.set_pressure_safety(&pressure1, STX_MAX_PRESSURE); — this covers every pump usage in the codebase, tests included, without touching each call site individually.
2. Force sensor removed, open-loop lock/unlock, button hook prepared Deleted Force_sensor.h/.cpp. Oring_lock.cpp is now pure dead-reckoning (LINEAR_ACT_LOCKED_STEPS from a known "unlocked" reference), with a confirm_lock_contact() seam gated by ORING_CONTACT_BUTTON_INSTALLED (false for now) ready for a future button — no pin claimed yet since none are free (documented in Settings.h).
3. Calibration simplified to unlocked/locked calibrate_linear_actuator() in Tests.cpp now just marks the unlocked reference (reset_position()) then jogs to locked, printing LINEAR_ACT_LOCKED_STEPS.
4. New reusable loading procedure + test Added load_slot(volume_ml) in Step_functions.cpp: lock → pump water (volume/pressure stop) → pump air → unlock, assuming the manifold is already in place. test_load_sample() (command sample_cycle) is a thin wrapper — no duplicated logic. Note: production step_sampling() still has its own more elaborate purge-then-filter double air-emptying; I left that untouched rather than risk destabilizing working code — let me know if you want it switched over to load_slot().
5. Clock-face slot labeling Added clock_minutes_for_slot()/slot_for_clock_minutes() to Manifold.cpp (top = 00 min) plus clockMM/clocklist serial commands. Direction is controlled by MANIFOLD_RAW_INDEX_INCREASES_CW in Settings.h — verify this against the physical rig with clocklist + slotN before trusting the labels; flip it if they run backwards.
6. Multi-sample dry-run test test_multi_sample_loading() (command multi_sample) walks FILL_ORDER with TEST_MULTI_SAMPLE_VOLUME_ML (50 mL) per slot, pausing for leak inspection between each.
You'll need to reflash and re-run calibrate_actuator (or run_commissioning_calibration) to get a real LINEAR_ACT_LOCKED_STEPS before anything lock-related is trustworthy — it's currently a placeholder (340).
