#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "TimeLib.h"

// ===> PINS DECLARATION AT THE BOTTOM

// global const = static = each file has its own declaration

// =============== COWAS VARIABLES ===============
// spool variables
const bool SPOOL_USE = false;         // set true once spool hardware is repaired - currently damaged, all spool movement is skipped
const uint8_t HEIGHT_FROM_WATER = 0;  // cm. between water level and spool endstop
const uint8_t DISTANCE_FROM_STOP = 5; // cm. distance from spool endstop at which speed is decreased
const uint8_t SPEED_UP = 100;         // over 100. Speed when moving up - experimentaly tested
const uint8_t SPEED_DOWN = 100;       // over 100. Speed when moving down - experimentaly tested
const uint16_t TUBE_LENGTH = 4900;    // cm. length of tube (not sure of this number)

// water pump variables
const uint8_t POWER_PUMP = 90;                               // over 100. Power when pumping from water. Experimentaly tested to not go over 500mA
const uint8_t POWER_FLUSH = 80;                              // over 100. Power when pumping from container
const uint8_t POWER_STX = 60;                                // over 100. Experimental. Start power for sterivex but code adapts it.
const uint32_t EMPTY_CONTAINER_TIME_PURGE = 60 * 1000 * 8;   // ms. Experimental. Time after which container should be empty
const uint32_t EMPTY_TUBE_TIME = 60 * 1000 * 2.3;            // ms. Experimental. Time after which Tubes should be empty
const uint32_t EMPTY_CONTAINER_TIME_FILTER = 60 * 1000 * 18; // ms. Experimental.
const uint32_t FILL_TUBES_WITH_WATER_TIME = 5 * 1000;        // ms. Experimental. Time to fill tubes for purge and sampling before sensor take over
const uint32_t FILL_CONTAINER_TIME = 60 * 1000 * 11;         // ms. Experimental.

// micro pump DNA-shield variables
const uint32_t FILL_STERIVEX_TIME = 190000; // 26-10-26 reduced by a minute
const int8_t PUMP_SHIELD_POWER = 13;        // power of big pump to push the DNA-shield into the sterivex
const uint32_t PUMP_SHIELD_TIME = 5000; // was 300      // ms for the big pump's run AFTER its 4 s soft ramp. The air push measures 4.8 s in total (ramp + this + the 500 ms relay stop); user accepted 4.8 s, 2026-10-19. Time for big pump to push DNA-shield

// shield micro pump calibration (LA3, console `shield_calibrate`, docs/lab/shield-protocol.md Part A).
// The prompts offer these as defaults; nothing in operation reads them.
const float SHIELD_DENSITY_G_PER_ML = 1.00f;                    // PLACEHOLDER: measure it (LA3 protocol A1)
const float SHIELD_TARGET_ML = 1.5f;                            // PLACEHOLDER: the lab's target (decision with Felix, shield-protocol §7)
const uint32_t SHIELD_CAL_DEFAULT_MS[] = {10000, 20000, 40000}; // ms. Run durations (at most 3 are used)
const uint8_t SHIELD_CAL_REPEATS = 3;                           // runs per duration
const uint32_t SHIELD_CAL_MAX_RUN_MS = 300000;                  // ms. Longest accepted run, and the bound on the printed fill time
const uint32_t SHIELD_CAL_PRIME_MAX_MS = 120000;                // ms. The priming run stops by itself after this
const float SHIELD_CAL_MAX_CV_PCT = 5.0f;                       // % repeatability limit per duration
const float SHIELD_CAL_MIN_R2 = 0.995f;                         // linearity limit of the fit
const float SHIELD_PRIME_MARGIN = 1.25f;                        // the suggested prime time = time to fill the empty line x this

// ! not used anymore?
// vacuum pump variables
const float VACUUM_TO_ACHIEVE = 0.13;                   // bar from atmsophere. Vacuum to achieve
const float VACUUM_MINIMUM = 0.20;                      // bar from atmosphere. Vacuum before restarting vacuum pump
const uint32_t DRYING_TIME = 2 * 60 * 1000 - 10 * 1000; // ms. Time for pumping hysteris and heating

// manifold variables
const int NB_SLOT = 15;         // total addressable slots (0=purge, 1-14=samples) - must match sterivex_angle[] size
const int PURGE_SLOT = 0;       // purge slot in the manifold
const bool MANIFOLD_USE = true; // use the manifold in the system

// Safety net for rotateMotor() (Manifold.cpp): without these, a persistent encoder
// SPI read failure or a genuine mechanical stall used to spin the wait loop
// forever with zero feedback - looked like "the manifold just stopped working"
// with no error printed anywhere. Now: repeated encoder failures or an overall
// timeout abort the rotation with a loud (always-on, not VERBOSE_MANIFOLD-gated)
// error and stop the motor, instead of hanging silently.
const uint8_t MANIFOLD_ENCODER_MAX_CONSECUTIVE_FAILURES = 50; // ~50ms of bad reads (readEncoder() is polled every ~1ms in the rotation loop)
const uint32_t MANIFOLD_ROTATE_TIMEOUT_MS = 15000;            // generous - normal rotations are well under this

// Sample fill order (human slot numbers, PURGE_SLOT excluded): visits slots close
// to purge before slots far from it, so a leak from an already-filled slot can
// only reach slots that are also already filled, never an unused one. Grouped by
// physical quadrant (pairs of same-height slots) to keep manifold rotation short.
const uint8_t FILL_ORDER[14] = {1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7};

// system variables
const int UPDATE_TIME = 1000; // ms. Refresh frequency for main program
// Soft start (2026-10-12, user): the pump output rises from 0 to the set power over PUMP_RAMP_MS, so the
// pressure is seen rising before any overshoot reaches the cap. A live change of PUMP_RAMP_MIN_STEP_PERCENT
// or more also ramps; smaller changes (PID trims) are written at once. Pump::move_output_to().
const uint32_t PUMP_RAMP_MS = 4000;             // ms. Start-up time from 0 to the set power (asked: 3-5 s)
const uint32_t PUMP_RAMP_STEP_MS = 20;          // ms. Between two output steps; the cap is checked at each step
const uint8_t PUMP_RAMP_MIN_STEP_PERCENT = 10;  // percent. Live changes at least this big ramp
const float STX_MAX_PRESSURE = 3.6; // bar. 2026-10-19, user decision: 3.6 for bench tests (was 3.2 from 2026-10-15; 3.5 for a few hours, 3.0 from 2026-10-12, 2.5 before): droplets leak at the manifold seal around 3 bar (F144). PLACEHOLDER: the sensor is calibrated to 2.0 bar only (F136), the bench manometer reads to 2.5 bar, and the weakest part's pressure rating (Sterivex housing, tubing, valves, O-ring) is not checked yet. Every other place reads this constant (main.cpp, Pump.cpp, the pressure sensor driver)
// Fail-safe bound for Pump::enforce_pressure_safety() (Pump.cpp): a gauge-pressure
// reading this far below 0 is not a real physical pressure (a properly working,
// calibrated sensor near ambient will read close to 0, not several bar negative) -
// it means the sensor/wiring can't currently be trusted, and the cap should assume
// the worst and stop the pump rather than pass the reading through unchecked. This
// closes a real gap found in the field: a garbage NEGATIVE reading never satisfies
// "> STX_MAX_PRESSURE" on its own, so an untrustworthy sensor could previously read
// nonsense while actual pressure rose unchecked (a tube popped this exact way).
const float PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR = -0.5;

// ============ S1-1: ABP PRESSURE SENSOR (pressure1, SPI, CS D8) ============
// It backs the pump cap and every PID loop (read_p_in() in main.cpp). Math: src/Core/abp_convert.
// Full scale used by the conversion, psi. PLACEHOLDER, unresolved (F126): the part was ordered
// and supplied as ABPDANV150PGSA3 (150 psi; user, 2026-10-09). On the bench the same day it
// read 2.30 and 4.78 bar with the 150 psi constant while the manometer on the same tee showed
// about 1 and 2 bar; with 60 psi the same frames give 0.92 and 1.91 bar. 60 is therefore the
// value that agrees with the manometer, which is the reference of `calibrate_pressure` (it
// refuses a gain outside 0.9-1.1). User decision (2026-10-09): trust the manometer and move
// on. If the manometer is the one that is wrong, the real pressure is 2.4 times what is shown.
const float ABP_FULL_SCALE_PSI = 60.0f;
const uint32_t ABP_STALE_TIMEOUT_MS = 500;  // ms. Only stale frames (status 10) for this long: the reading is no longer trusted
const uint32_t ABP_SPI_HZ = 100000;         // Hz. The part accepts 50-800 kHz. Was 800 kHz, the datasheet maximum; lowered after the garbage frames of 2026-10-09 (F124) to leave margin on long wires
// What getPressure() returns when the sensor can't be trusted (fault frame, no answer, stale
// for too long). Below PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR on purpose: the existing
// plausibility checks (Pump::enforce_pressure_safety, CtrlPump::run) then stop the pump.
const float PRESSURE_SENSOR_INVALID_BAR = -99.0f;
// pressure (bar) = PRESSURE_GAIN x (datasheet conversion - PRESSURE_ZERO_OFFSET_BAR).
// Paste the lines printed by `calibrate_pressure` (both) or `pressure_zero` (the offset only).
const float PRESSURE_GAIN = 1.0413f;          // calibrate_pressure 2026-10-12 (5 points, R2 0.99897, max residual 0.032 bar)
const float PRESSURE_ZERO_OFFSET_BAR = -0.0311f; // calibrate_pressure 2026-10-12

// `pressure_zero` and `calibrate_pressure` (syringe + manometer on a tee, pump not involved)
const uint32_t PRESSURE_ZERO_AVG_MS = 5000;                        // ms. Averaging time of the zero
const float PRESSURE_ZERO_MAX_ABS_BAR = 0.3f;                      // bar. A larger zero is not an offset: pressure applied, or the wrong full scale
const float CAL_PRESSURE_TARGETS_BAR[] = {0.5f, 1.0f, 1.5f, 2.0f}; // bar. Proposed points after the zero (the manometer ends at 2.5)
const float CAL_PRESSURE_MAX_REF_BAR = 2.5f;                       // bar. End of the manometer's scale: a typed value above it is refused
const uint32_t CAL_PRESSURE_AVG_MS = 3000;                         // ms. Measuring time of each calibrate_pressure point, the 0 bar one included
const float CAL_PRESSURE_MAX_SPREAD_BAR = 0.08f;                   // bar. `pressure_zero` only: max - min above this at rest is not a zero (calibrate_pressure lets the operator judge)
const int CAL_PRESSURE_MIN_POINTS = 3;                             // points needed for a PASS, the zero included
const float CAL_PRESSURE_GAIN_MIN = 0.9f;                          // a fitted gain outside 0.9-1.1 is refused:
const float CAL_PRESSURE_GAIN_MAX = 1.1f;                          // wrong part number or a wiring fault, not a calibration
const float CAL_PRESSURE_MAX_RESIDUAL_BAR = 0.06f;                 // bar. The manometer: class 1.6 of 2.5 bar (0.04 bar, dial read 2026-10-12) + about 0.02 bar of needle reading on 0.1 bar divisions
// F155 (2026-10-19): the old 0.04 bar was never reached on an air-only line (it reads about 0.05 bar), so the emptying ran to its
// maximum. Water pushed through the open tube read about 0.75-0.83 bar (the first purge), air about 0.02-0.09 bar. The middle
// of that gap is the new rule: below it for EMPTY_WATER_SECURITY_TIME = air. PLACEHOLDER: check it on the bench (S1-C checklist).
const float EMPTY_WATER_PRESSURE_PURGE_THRESHOLD = 0.4f;  // bar from atmosphere. Threshold of pressure in tube considered as empty when purging (was 0.04, F155)
const float EMPTY_WATER_PRESSURE_STX_THRESHOLD = 0.8f;    // 1.7f;          // bar from atmosphere. Threshold of pressure in tube considered as empty when filtering
const uint32_t EMPTY_WATER_SECURITY_TIME = 5 * 1000;      // ms. Time to ensure a correct flush of the container when purging
const uint32_t EMPTY_WATER_STX_SECURITY_TIME = 60 * 1000; // ms. Time to ensure a correct flush of the conainter when filtering
const uint32_t EMPTY_DEPLOYMENT_TIME = 5 * 60 * 1000;

// ! need to change this
const uint32_t PREPARATION_TIME = 60 * 30; // ms. system needs 30 minutes preparation before sampling

// ! adapt here the volume for purge and sample
// Purge volume, chosen by the user (2026-10-23). The spool answer is given in `session` (spool_installed_session).
const float PURGE_NO_SPOOL_ML = 500.0f;  // mL - spool off (user 2026-10-23: 500 mL)
const float PURGE_SPOOL_ML = 3000.0f;    // mL - spool on (user 2026-10-26: 3 L, "enough"). Dead volume 2.57 L (INTAKE_PATH_ML + SPOOL_PATH_ML) + about 0.4 L. Was 7 L (2026-10-23); the F110 rule of 3 x dead volume (7.7 L) is not kept, see §3.7 rule 1
// Purge timeout (ms). Spool off: 3 min (the old value). Spool on: 13 min (user, confirmed 2026-10-23). It is only a safety now: 3 L takes
// about 5 min at 60 % (1001 mL took 97 s at 60 %, 2026-10-23), less at the full-power purge (PUMP_PURGE_MAX_POWER_PERCENT).
const uint32_t PURGE_MAX_RUNTIME_MS = 3 * 60 * 1000;
const uint32_t PURGE_SPOOL_MAX_RUNTIME_MS = 13 * 60 * 1000;
// F130 (2026-10-12): dead volume of the water path (documentation and the calibrate_all check). Not used to set the purge.
const float INTAKE_PATH_ML = 30.0f;   // mL. Measured: 4 mm ID x 2.40 m = the whole path before the exit, pump, sensors and tubing included (12.6 mL/m)
const float SPOOL_PATH_ML = 2540.0f;  // mL. Spool path, 9 mm ID x 40 m: pi * 4.5^2 * 40000 mm^3 = 2.54 L. Connected by default
// S1-C addendum item 3 (2026-10-19, user): the default for the spool question. `session` asks y/n each time and sets spool_installed_session.
const bool SPOOL_INSTALLED = false;
extern bool spool_installed_session; // is the spool module on the machine now (answered in `session`); starts as SPOOL_INSTALLED
const float STX_SAMPLE_MILLILITERS = 1000; // user decision 2026-10-23: 1000 mL, or less when the clogging criteria stop the run (was 100, placeholder)// amount of maximum volume to filter. Need to add a way to determine filter saturation and stop based on that
const uint32_t SAMPLE_MAX_DURATION_MS = 30UL * 60UL * 1000UL; // ms. Stage 1 S1-C (F15): step_sampling stops here at the latest and says so (end=timeout). Was 3 min, silently

const uint32_t SYNC_TIME = 32400;     // ms. Time before refetching wifi time. Not implemented
const uint8_t MAX_FILTER_NUMBER = 14; // max filters possible in the system
extern uint8_t FILTER_IN_SYSTEM;      // max filters currently inserted in the system
extern bool ENABLE_OUTPUT;            // Enable or disable output printing, even if trying

// ============= TIME MANAGEMENT ==============
struct Time
{
    uint8_t hour;
    uint8_t minute;
};

struct Date
{
    uint16_t year;
    uint8_t day;
    uint8_t month;
    time_t epoch;
    struct Time time;
};

String format_date_logging(time_t t);

// ============ SYSTEM STATES ================
// system states are here for the human interface. The goal is to have the system updating its
// state when necessary and communicating it to the wifi card. When connecting to the server,
// the wifi card will send the state and by this mean enable or disable control functions.
enum System_state
{
    state_starting,      // indicate not in normal running mode
    state_idle,          // do nothing, wait for next sterivex. Available for other tasks
    state_sampling,      // currently sampling
    state_refill,        // need to refill sterivex
    state_communicating, // when communicating with wifi card
    state_error,         // system not working for a reason
};

void set_system_state(System_state state);
void enable_output(bool enable);

// ============ VERBOSE DEFINITIONS ==================
// To print or not the infos of a subsystem in the terminal
const bool DEBUG_MODE_PRINT = true;

const bool ENABLE_TIME_LOG = false; // If true, print the time before each printed output
const bool VERBOSE_INIT = false;
const bool VERBOSE_VALVES = true;
const bool VERBOSE_MOTOR = false;
const bool VERBOSE_PURGE = true;
const bool VERBOSE_PURGE_PRESSURE = true;
const bool VERBOSE_SAMPLE = true;
const bool VERBOSE_SAMPLE_PRESSURE = true;
const bool VERBOSE_PUMP = true;
const bool VERBOSE_REWIND = false;
const bool VERBOSE_FILL_CONTAINER = false;
const bool VERBOSE_DIVE = false;
const bool TIMER = false;
const bool PRESSURE_SENSOR_ERROR = true;
const bool VERBOSE_MANIFOLD = false;
const bool VERBOSE_SHIELD = true;

// ============ PIN DEFINITIONS ==================
// Full pin map (every Due pin, who uses it, direction, level, free pins): docs/hardware/pinout.md
const uint8_t STATUS_LED_PIN = 23;
const uint8_t GREEN_LED_PIN = 22;
const uint8_t PRESSURE1_PIN = 8;
const uint8_t pressure_3_pin = A1; // pressure 0-12 bar (cheaper one, only water)
const uint8_t VALVE_1_PIN = 44;     // is connected to the air
const uint8_t VALVE_23_PIN = 40;
const uint8_t VALVE_MANIFOLD = 46; // was manifold, is now the valve conencted to the water intake
const uint8_t PUMP_PIN = DAC1;
const uint8_t PUMP_ENABLE = 42;
// Output controlled by transistor, digital pin is toggeling voltage on and off
const uint8_t ON_OFF_33V = 48;
// const uint8_t PUMP_VACUUM = 34;
const uint8_t ENCODER_A_PIN = 31;
const uint8_t ENCODER_B_PIN = 33;
const uint8_t ENCODER_Z_PIN = 35;
const uint8_t BUTTON_START_PIN = 24;
const uint8_t BUTTON_CONTAINER_PIN = 27;
const uint8_t BUTTON_SPOOL_UP = 28;
const uint8_t BUTTON_SPOOL_DOWN = 29;
const uint8_t BUTTON_LEFT_PIN = 25;
const uint8_t BUTTON_RIGHT_PIN = 26;
const uint8_t MOTOR_INA1_PIN = 32;
const uint8_t MOTOR_INB1_PIN = 34;
const uint8_t MOTOR_PWM1_PIN = 2;
const uint8_t MOTOR_EN1DIAG1_PIN = 30; // enable pin
const uint8_t MOTOR_CS1_PIN = A10;
const uint8_t MOTOR_INA2_PIN = 6;
const uint8_t MOTOR_INB2_PIN = 7;
const uint8_t MOTOR_PWM2_PIN = 3;
const uint8_t MOTOR_EN2DIAG2_PIN = 5; // enable pin
const uint8_t MOTOR_CS2_PIN = A11;

const uint8_t ENCODER_MANIFOLD = 38;
const uint8_t FLOW_SMALL_PIN = 13; // to be calibrated

const uint8_t FLOW_BIG_PIN = 12;

// Linear actuator: NEMA17 (42SHD034-20B) driven by an A4988 driver.
// A4988 VMOT/GND (motor power) is currently sourced from the existing Pololu
// VNH5019 shield's 24V rail, shared with the shield's other DC motors - not a
// dedicated supply. TODO: verify total current budget on that shared rail once
// the stepper is in regular use, or move it to its own supply.
// TODO: ENABLE has no external pull-up yet - add a 10kOhm pull-up to 3.3V (VDD)
// to stop the motor twitching on power-up/upload, before firmware sets ENABLE.
const uint8_t LINEAR_ACT_STEP_PIN = 9;
const uint8_t LINEAR_ACT_DIR_PIN = 10;
const uint8_t LINEAR_ACT_ENABLE_PIN = 11;          // active LOW on the A4988
const uint16_t LINEAR_ACT_STEPS_PER_REV = 200;     // 1.8deg/step motor, full step (MS1-3 tied to GND)
const uint16_t LINEAR_ACT_SLOW_STEPS_PER_SEC = 33; // ~6 sec/revolution, used to lock/unlock the o-ring in normal operation
const uint16_t LINEAR_ACT_CAL_STEPS_PER_SEC = 30;  // ~30 sec/revolution - deliberately slower than normal operation, so there's time to react and stop calibrate_linear_actuator() at exactly the right position

// sample-load test cycle
const float TEST_SAMPLE_VOLUME_ML = 500.0;       // 0.5L, target volume for the single-sample pump-through test
const float TEST_MULTI_SAMPLE_VOLUME_ML = 50.0;  // small per-slot volume for the multi-sample joint-leak/order test (the old multi_sample test, removed 2026-10-23) - just enough to see every joint wet, not a real sample

// ============ FW2: PRESSURE SENSOR GATE, TEST SUITE ============
// Is a trusted pressure sensor backing the pump cap and the PID loops? Since S1-1 the cap
// and the PID loops read the ABP on SPI D8 (read_p_in(), main.cpp), but the flag stays
// false: on 2026-10-09 the ABP returned garbage frames on the bench (F124). Set it true
// only when `abp_raw` is steady at rest and `calibrate_pressure` has passed (S1-1 checklist
// rows 1-4, docs/logs/LOG.md). While false, every procedure
// that needs pressure (the step_* functions, sample_process, load_slot*, pump, the panel
// pump mode, calibrate_flow, cal_DNA) REFUSES with a message; pumping is only
// possible through the sensorless test mode below. After setting it true, the S1-1 bench
// checks (dead-sensor stop, cap trip: stage 1 roadmap, the T1 calibration) must pass before any real run.
const bool PRESSURE_SENSOR_AVAILABLE = true;

// Sensorless pump test mode (F17; ROADMAP §6 rule 4 exception): pump_open_loop_test() in
// src/Test_openloop.cpp, reached only from the FW2 test commands, after typing NOSENSOR.
const uint8_t TEST_OPENLOOP_MAX_POWER = 30;         // % - PLACEHOLDER (user, 2026-09-25): every open-loop request is clamped to this
const uint32_t TEST_OPENLOOP_MAX_MS_LIMIT = 120000; // ms - hard limit of one open-loop run, whatever is asked
const uint32_t TEST_OPENLOOP_NO_FLOW_MS = 5000;     // ms - stop when the flowmeter gives no pulse for this long
const uint32_t TEST_OPENLOOP_WATER_MAX_MS = 60000;  // ms - default time limit of a water phase (volume normally stops it first)
const uint32_t TEST_OPENLOOP_AIR_PUSH_MS = 10000;   // ms - PLACEHOLDER: sensorless air push (the operation one is ~5 s in practice, F28)

// O-ring lock tri-state (F18) and the rotation tests
const float ACTUATOR_HOME_SLOT_TOLERANCE_DEG = 3.0; // deg - PLACEHOLDER (user, 2026-09-25): actuator_home (and any lock at the current slot) needs the encoder this close to a slot
const float TEST_ROTATE_MAX_ERROR_DEG = 1.0;        // deg - rotate_all / encoder_repeat PASS: |encoder error| below this
const float TEST_ENCODER_MAX_SPREAD_DEG = 0.5;      // deg - encoder_repeat PASS: max - min of the arrival angles below this
const uint32_t TEST_ROTATE_SETTLE_MS = 500;         // ms - wait after a rotation before reading the encoder (coast)
const uint16_t TEST_LOCK_HOLD_S = 3;                // s - rotate_all: how long each slot stays locked (hold=N)
const float TEST_LEAK_HOLD_DEFAULT_ML = 50.0;       // mL - leak_hold default water volume ("small volume")

// S1-1b `pressure_ramp` and `fill_test` (2026-10-15 to 2026-10-19). Hard cap (STX_MAX_PRESSURE) is the only pressure limit.
const float PRESSURE_RAMP_DEFAULT_TARGET_BAR = 2.5;   // bar - droplets leaked at the manifold seal around 3 bar (user, 2026-10-15, F144)
const float PRESSURE_RAMP_DEFAULT_RATE_BAR_S = 0.05;  // bar/s - 0 to 2.5 bar in about 50 s
const float PRESSURE_RAMP_MIN_RATE_BAR_S = 0.01;      // bar/s
const float PRESSURE_RAMP_MAX_RATE_BAR_S = 0.5;       // bar/s
const float PRESSURE_RAMP_CAP_MARGIN_BAR = 0.2;       // bar - the target may not come closer than this to STX_MAX_PRESSURE
const uint16_t PRESSURE_RAMP_DEFAULT_HOLD_S = 20;     // s - PID holds the target
const uint16_t PRESSURE_RAMP_DEFAULT_DECAY_S = 30;    // s - pump off, pressure read
const uint8_t PRESSURE_RAMP_START_POWER = 15;         // % - applied at once, then the PID takes over
const uint8_t PRESSURE_RAMP_MAX_POWER = 60;           // % - the PID output never goes above this (maxpower= can lower it)
const uint8_t PRESSURE_RAMP_MAX_STEP_PERCENT = 3;     // % per PID update (slew limit)
const uint32_t PRESSURE_RAMP_UPDATE_MS = 100;         // ms - PID update; the cap is checked on every pass (about every 2 ms)
const float PRESSURE_RAMP_MAX_START_BAR = 0.3;        // bar - pressure_ramp refuses to start above this (a comparable run starts from an empty line)
const float PRESSURE_RAMP_REVERSAL_PERCENT = 5.0;     // % - a power swing this big, then back, counts as one reversal (chatter metric)
// Gains of the pressure tests. User decision 2026-10-19: the original ones. Calmer gains (kp 3, ki 0.2, kd 20) held the pressure
// but at 8-12 % of power, which may be too little to move water. The ripple of about 1 bar is accepted (F146).
const float PRESSURE_RAMP_PID_KP = 200.0;             // % per bar
const float PRESSURE_RAMP_PID_KI = 100.0;             // per update (100 ms)
const float PRESSURE_RAMP_PID_KD = 100.0;
const float PRESSURE_RAMP_DEFAULT_FILTER_S = 0.0;     // s - low-pass on the pressure the PID reads (filter=); 0 = raw. The cap always reads the raw value
// The verdict of pressure_ramp: "reached" also needs a not-too-wild run (the pump pulses by itself, F141)
const float PRESSURE_RAMP_PASS_MAX_OVERSHOOT_BAR = 0.4; // bar - PLACEHOLDER: peak above the target
const float PRESSURE_RAMP_PASS_MAX_RIPPLE_BAR = 0.8;    // bar - PLACEHOLDER: max - min in the second half of the hold
// fill_test: pump a volume through the filter; at the pressure limit the pump pauses and waits for the pressure to fall
const float FILL_DEFAULT_ML = 1000.0;                 // mL
// Everything below is derived from STX_MAX_PRESSURE (top of this file): change the cap there, and the pause, the resume and the
// maximum target follow (2026-10-19, user: one place in the field). The offsets are the only other numbers to edit.
const float FILL_PAUSE_BELOW_CAP_BAR = 0.25;          // bar - offset: the pump pauses when the RAW pressure comes this close to the cap
const float FILL_RESUME_BELOW_PAUSE_BAR = 0.2;        // bar - offset: and resumes when the filtered pressure is this far below the pause pressure
const float FILL_PAUSE_BAR = STX_MAX_PRESSURE - FILL_PAUSE_BELOW_CAP_BAR;   // bar - 3.35 with the 3.6 cap
const float FILL_RESUME_BAR = FILL_PAUSE_BAR - FILL_RESUME_BELOW_PAUSE_BAR; // bar - 3.15 with the 3.6 cap
const float FILL_TARGET_BELOW_CAP_BAR = 0.6;          // bar - offset: the highest target is this far below the cap (user, 2026-10-19: 3.0 with the 3.6 cap)
const float FILL_MAX_TARGET_BAR = STX_MAX_PRESSURE - FILL_TARGET_BELOW_CAP_BAR; // bar - 3.0. The default target; a higher target= is refused (it must sit under FILL_RESUME_BAR)
const float FILL_CLOG_FRACTION = 0.3;                 // PLACEHOLDER: a window below this share of the first window = CLOG? warning; the mean of two windows below it = stop "clogged" (src/Core/fill_stop.h)
const float FILL_BACKOFF_FACTOR = 0.85;               // the power is cut by this on every pause
const uint16_t FILL_DEFAULT_TIMEOUT_S = 1800;         // s - every procedure has a timeout
const uint16_t FILL_DEFAULT_STALL_S = 60;             // s - less than FILL_STALL_MIN_ML of PUMPING time in this long = clogged, stop (pauses do not count)
const float FILL_STALL_MIN_ML = 3.0;                  // mL - PLACEHOLDER, the flowmeter (calibrated, user 2026-10-23) reads almost nothing at low flow (F150)
const float PUMP_START_RATE_PERCENT_PER_S = 5.0;      // %/s - PLACEHOLDER (F196): the operation pump rises from 0 % at this rate after a start or a restart (0 to 15 % in 3 s)
// F192 vent (user, 2026-10-23): pump off, wait PRESSURE_DECAY_WAIT_MS so the line drops by itself, then open the air vent V1 for
// PRESSURE_AIR_VENT_MS. Nothing here stops the sample: the O-ring unlock goes on whatever the reading (only a faulty sensor stops it).
const float PRESSURE_RELEASE_BAR = 0.3;              // bar - PLACEHOLDER (F192): the reading the vent reports as released (log only, it does not stop the run)
const uint32_t PRESSURE_DECAY_WAIT_MS = 20000;        // ms - PLACEHOLDER (F192): the line drops with the pump off and the vent shut
const uint32_t PRESSURE_AIR_VENT_MS = 5000;           // ms - PLACEHOLDER (F192): the air vent V1 is open this long, then shut
// Flowmeter confidence (2026-10-23, user; F150): the volume counted in a window of FLOW_TRUST_WINDOW_MS counts as confident only
// when the window's mean flow is at least FLOW_TRUST_MIN_ML_PER_MIN. Logged as vol_trusted_ml; the plain total stays as vol_ml.
const float FLOW_TRUST_MIN_ML_PER_MIN = 50.0;         // mL/min - set 2026-10-23 by the user (was 110 from interpolation). Note: the flowmeter read -32 % at about 50 mL/min (LOG, 2026-10-23), so the lower windows of this rule read low
const uint32_t FLOW_TRUST_WINDOW_MS = 5000;           // ms - the window of the confident-volume rule
const float FILL_CLOG_MIN_WINDOW_ML = 30.0;        // mL per 60 s pumping window - PLACEHOLDER (F194): below this = clogged, stop at once, from the first window. 50 until 2026-10-23; 30 (user: a new filter stopped at 382 mL, its 4th window was 36 mL). The relative rule (FILL_CLOG_FRACTION) still stops a slow decline. A new filter gave about 115 mL/min at 2.2 bar (S1-A); the clogged run gave 24 mL in its first minute and stopped only at 213 s
const uint16_t FILL_PRESSURE_STUCK_S = 20;            // s - PLACEHOLDER: paused and still above the resume pressure this long = the filter holds the pressure, stop
const uint8_t PUMP_MINIMUM_POWER_PERCENT = 10;        // % - below this, the pump stalls (deadband, F137); the pressure loops never command less while running
const uint8_t PUMP_MAX_POWER_PERCENT = 60;           // % - the operation pressure loops (CtrlPump) with WATER: never more, whatever the pressure (user 2026-10-19). PLACEHOLDER: check on the bench
const uint8_t PUMP_AIR_MAX_POWER_PERCENT = 100;      // % - the same loops once the emptying sees AIR (the pressure fell below the threshold): full power (user 2026-10-19)
// % - the EXCEPTION (user 2026-10-26): the purge (step_purge) only, at full power. Its pressure target (OPERATION_TARGET_BAR) and the cap stay.
// Every other water run keeps PUMP_MAX_POWER_PERCENT. Pressure spikes and leaks at 100 % were seen in 2026-10-19 (F161, F207).
const uint8_t PUMP_PURGE_MAX_POWER_PERCENT = 100;
const float OPERATION_TARGET_BAR = 3.0f;             // bar - the pressure target of every operation pump run; the same as the fill_test target (user 2026-10-19)
const uint32_t EMPTY_MAX_TIME_MS = 60 * 1000;        // ms - every emptying phase (air push through the line) ends after this at the latest; it ends earlier once the air hold is met. A timeout is printed with the last pressure (user 2026-10-21: 60 s)
const uint32_t EMPTY_STX_AIR_HOLD_MS = 40000;         // ms - the sterivex air drying (after the water phase of a sample): doubled from 20 s (user, 2026-10-23). The purge emptying keeps EMPTY_AIR_HOLD_MS
const uint32_t EMPTY_AIR_HOLD_MS = 20000;            // ms - the emptying ends early once the pressure stays below the air threshold this long. 20 s (user, 2026-10-23, F188): 1 s ended every air phase at once and did not empty the line
const uint32_t EMPTY_STX_EXTRA_MAX_MS = EMPTY_STX_AIR_HOLD_MS - EMPTY_AIR_HOLD_MS; // ms - the sterivex phase may run this much longer than EMPTY_MAX_TIME_MS, so the longer hold can finish
const float PUMP_PID_KP = 200.0;                      // % per bar - operation loops (CtrlPump, main.cpp): same as PRESSURE_RAMP_PID_* for now (F147)
const float PUMP_PID_KI = 100.0;                      // per (bar x 100 ms update), MiniPID has no time base
const float PUMP_PID_KD = 100.0;

// O-ring lock/unlock, linear actuator (NEMA17 + A4988). The force sensor (FSR) that
// used to detect contact was removed - readings were unreliable in practice - so
// this is now fully open-loop/dead-reckoning: lock_oring()/unlock_oring() (Oring_lock.cpp)
// just move a fixed, calibrated number of steps from a known "unlocked" reference
// (see calibrate_linear_actuator()) rather than searching for contact.
//
// Anti-drift trick: LOCKING deliberately over-drives past the point where the
// motor stalls against the o-ring (LINEAR_ACT_LOCK_OVERDRIVE_FACTOR extra, e.g.
// 10%) every single cycle. A stalled stepper just skips steps rather than moving
// further, so no matter how many lock/unlock cycles have run, or how many steps
// were lost to slip along the way, the o-ring always ends up compressed against
// the SAME real physical stop - a self-correcting mechanical "home" position, the
// same principle as sensorless-stall homing. UNLOCKING then moves the plain
// (non-overdriven) LINEAR_ACT_LOCKED_STEPS forward from that known-good stop back
// to the unlocked reference, so drift never accumulates cycle over cycle. Both
// LINEAR_ACT_LOCKED_STEPS and the overdrive are measured/applied together in
// calibrate_linear_actuator() - see its comments for how the stall point is found.
//
// Only two positions matter in practice: UNLOCKED (the reference, treated as
// position 0) and LOCKED (LINEAR_ACT_LOCKED_STEPS backward from there, then
// over-driven) - the old separate "partial" vs "full" unlock distinction is gone
// since operation code only ever used partial.
//
// Future option (not implemented yet): a physical button to confirm the o-ring has
// actually made contact when locking, instead of trusting dead reckoning alone.
// ORING_CONTACT_BUTTON_INSTALLED gates that check off for now - lock_oring() runs
// fully open-loop as long as this stays false. To wire it up later: pick a free Due
// pin (docs/hardware/pinout.md lists the free ones and whether the PCB shield brings them
// out), add an ORING_CONTACT_BUTTON_PIN constant here, begin() a Button on it in
// main.cpp, flip this to true, and fill in the confirm check in lock_oring().
const bool ORING_CONTACT_BUTTON_INSTALLED = false;
// const uint8_t ORING_CONTACT_BUTTON_PIN = <TBD - assign a free pin, see docs/hardware/pinout.md>;

const long LINEAR_ACT_LOCKED_STEPS = 210;          // measured with calibrate_actuator on 2026-10-12 after the potentiometer was changed (was 160 before) (steps backward from unlocked to the point the motor stalls). Redo after any change of the A4988 Vref or the mechanism
const float LINEAR_ACT_LOCK_OVERDRIVE_FACTOR = 1.10; // lock_oring() commands LINEAR_ACT_LOCKED_STEPS * this, so it always re-stalls at the same physical stop regardless of drift (see comment block above)
const long LINEAR_ACT_CAL_SAFETY_CAP_STEPS = 5L * LINEAR_ACT_STEPS_PER_REV; // generous single-direction travel limit, calibration jogging only

// Manifold clock-face labeling (see clock_minutes_for_slot()/slot_for_clock_minutes()
// in Manifold.cpp): top of the manifold = "00" minutes, minutes increase the way a
// clock's minute hand does. Whether the internal raw slot index increases clockwise
// or anticlockwise on the real hardware isn't derivable from the geometry alone (see
// identify_manifold_direction() in Tests.cpp). Default assumes clockwise - verify
// once with `clocklist` + `slotN` against the physical rig and flip this if the
// printed clock labels run backwards from what you observe, then rebuild/reflash.
const bool MANIFOLD_RAW_INDEX_INCREASES_CW = true;
#endif