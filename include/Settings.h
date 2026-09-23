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
const uint32_t FILL_STERIVEX_TIME = 250000; //
const int8_t PUMP_SHIELD_POWER = 13;        // power of big pump to push the DNA-shield into the sterivex
const uint32_t PUMP_SHIELD_TIME = 300;      // time for big pump to push DNA-shield

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
const float STX_MAX_PRESSURE = 3.0;
// Fail-safe bound for Pump::enforce_pressure_safety() (Pump.cpp): a gauge-pressure
// reading this far below 0 is not a real physical pressure (a properly working,
// calibrated sensor near ambient will read close to 0, not several bar negative) -
// it means the sensor/wiring can't currently be trusted, and the cap should assume
// the worst and stop the pump rather than pass the reading through unchecked. This
// closes a real gap found in the field: a garbage NEGATIVE reading never satisfies
// "> STX_MAX_PRESSURE" on its own, so an untrustworthy sensor could previously read
// nonsense while actual pressure rose unchecked (a tube popped this exact way).
const float PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR = -0.5;
const float EMPTY_WATER_PRESSURE_PURGE_THRESHOLD = 0.04f; // 0.06f;       // bar from atmosphere. Threshold of pressure in tube considered as empty when purging
const float EMPTY_WATER_PRESSURE_STX_THRESHOLD = 0.8f;    // 1.7f;          // bar from atmosphere. Threshold of pressure in tube considered as empty when filtering
const uint32_t EMPTY_WATER_SECURITY_TIME = 5 * 1000;      // ms. Time to ensure a correct flush of the container when purging
const uint32_t EMPTY_WATER_STX_SECURITY_TIME = 60 * 1000; // ms. Time to ensure a correct flush of the conainter when filtering
const uint32_t EMPTY_DEPLOYMENT_TIME = 5 * 60 * 1000;

// ! need to change this
const uint32_t PREPARATION_TIME = 60 * 30; // ms. system needs 30 minutes preparation before sampling

// ! adapt here the volume for purge and sample
const float PURGE_MILLILITERS = 0.1 * 1000;
const float STX_SAMPLE_MILLILITERS = 100; // amount of maximum volume to filter. Need to add a way to determine filter saturation and stop based on that

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
String format_date_friendly(time_t t);
time_t timeToEpoch(uint8_t _hour, uint8_t _minute, uint8_t _day, uint8_t _month, uint16_t _year);

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

System_state get_system_state();
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
// To update with pinout table sheet
const uint8_t STATUS_LED_PIN = 23;
const uint8_t GREEN_LED_PIN = 22;
const uint8_t PRESSURE1_PIN = 8;
const uint8_t pressure_2_pin = A2; // pressure 0-16Mpa (water and air)
const uint8_t pressure_3_pin = A1; // pressure 0-12 bar (cheaper one, only water)
const uint8_t VALVE_1_PIN = 44;
const uint8_t VALVE_23_PIN = 40;
const uint8_t VALVE_MANIFOLD = 46; // valve manifold
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
const uint8_t FLOW_SMALL_PIN = 13; // probably fried
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
const float TEST_MULTI_SAMPLE_VOLUME_ML = 50.0;  // small per-slot volume for the multi-sample joint-leak/order test (test_multi_sample_loading()) - just enough to see every joint wet, not a real sample

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
// since production code only ever used partial.
//
// Future option (not implemented yet): a physical button to confirm the o-ring has
// actually made contact when locking, instead of trusting dead reckoning alone.
// ORING_CONTACT_BUTTON_INSTALLED gates that check off for now - lock_oring() runs
// fully open-loop as long as this stays false. To wire it up later: pick a free Due
// pin (all pins in this file are currently claimed - see the PIN DEFINITIONS section
// below), add an ORING_CONTACT_BUTTON_PIN constant here, begin() a Button on it in
// main.cpp, flip this to true, and fill in the confirm check in lock_oring().
const bool ORING_CONTACT_BUTTON_INSTALLED = false;
// const uint8_t ORING_CONTACT_BUTTON_PIN = <TBD - assign a free pin>;

const long LINEAR_ACT_LOCKED_STEPS = 210;          // placeholder - measure with calibrate_linear_actuator() (steps backward from unlocked to the point the motor stalls)
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