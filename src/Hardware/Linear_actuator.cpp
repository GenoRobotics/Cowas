/**
 * @file Linear_actuator.cpp
 * @brief STEP/DIR/ENABLE control for a stepper (NEMA17 42SHD034-20B) through an
 * A4988 driver, used to compress/release an o-ring via a lead screw. Open-loop:
 * position_steps is a commanded step count, not a measured position (no encoder
 * on this motor) - it can drift from the true position if the motor stalls.
 *
 * Why this exists: the manifold/plate assembly has a core trade-off between
 * rotation and sealing - enough o-ring compression to stop leaks makes the
 * manifold too stiff to rotate; too little and it leaks. This actuator resolves
 * it by cycling: loosen (rotate the manifold freely) -> tighten (compress the
 * o-ring, lock) -> loosen again for the next cycle. See load_slot()
 * (Step_functions.cpp) for the lock/pump/unlock sequence built on top of this class.
 */
#include <Arduino.h>
#include "Linear_actuator.h"

/**
 * @brief Configures pins, leaves the driver disabled (no holding torque) until
 * enable() is called, and sets a conservative default speed.
 */
void Linear_actuator::begin(byte _step_pin, byte _dir_pin, byte _enable_pin,
                             uint16_t _steps_per_rev, uint8_t _microsteps,
                             float _lead_mm_per_rev)
{
    step_pin = _step_pin;
    dir_pin = _dir_pin;
    enable_pin = _enable_pin;
    steps_per_rev = _steps_per_rev;
    microsteps = _microsteps;
    lead_mm_per_rev = _lead_mm_per_rev;
    position_steps = 0;
    current_direction = forward;

    pinMode(step_pin, OUTPUT);
    pinMode(dir_pin, OUTPUT);
    pinMode(enable_pin, OUTPUT);

    digitalWrite(step_pin, LOW);
    digitalWrite(dir_pin, LOW);
    disable(); // start disabled (no holding torque) until the caller explicitly enables it

    set_speed(400); // conservative default for bring-up
}

/// @brief Energizes the coils (holding torque on).
void Linear_actuator::enable()
{
    digitalWrite(enable_pin, LOW); // A4988 enable pin is active LOW
}

/// @brief De-energizes the coils (no holding torque, STEP pulses ignored by the driver).
void Linear_actuator::disable()
{
    digitalWrite(enable_pin, HIGH);
}

/// @brief Sets step rate; recompute the STEP pulse half-period used by step_pulses().
void Linear_actuator::set_speed(uint16_t steps_per_second)
{
    if (steps_per_second == 0)
    {
        steps_per_second = 1;
    }
    step_delay_us = 1000000UL / steps_per_second / 2; // half period
}

/**
 * @brief Sets the DIR pin and remembers it for step_pulses()'s position bookkeeping.
 * Intentionally separate from step_pulses(): call this ONCE per move, not per chunk -
 * re-asserting DIR mid-move (previous implementation) caused visible vibration/stall
 * on this driver, traced back to a loose DIR wiring connection, but keeping DIR
 * writes to a single call per move is the safer pattern regardless.
 */
void Linear_actuator::set_direction(actuator_direction dir)
{
    current_direction = dir;
    digitalWrite(dir_pin, dir == forward ? LOW : HIGH); // flipped vs. lead screw/motor wiring so "forward" matches the documented convention (toward the o-ring = backward)
    delayMicroseconds(5); // DIR must be stable before the first STEP pulse (A4988 setup time)
}

/**
 * @brief Pulses STEP `steps` times at the configured speed, using the direction
 * last set by set_direction() (does NOT touch the DIR pin - see set_direction()).
 * Updates position_steps accordingly. Blocking.
 */
void Linear_actuator::step_pulses(long steps)
{
    for (long i = 0; i < steps; i++)
    {
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(step_delay_us);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(step_delay_us);
    }

    position_steps += (current_direction == forward ? steps : -steps);
}

/// @brief Convenience wrapper: set_direction(dir) then step_pulses(steps) in one call.
void Linear_actuator::move_steps(long steps, actuator_direction dir)
{
    set_direction(dir);
    step_pulses(steps);
}

/// @brief Converts a linear distance to steps via lead_mm_per_rev and moves. No-op if
/// lead_mm_per_rev wasn't configured in begin() (currently unused/unconfigured project-wide).
void Linear_actuator::move_mm(float mm, actuator_direction dir)
{
    if (lead_mm_per_rev <= 0)
    {
        return; // lead screw pitch not configured, can't convert mm to steps
    }
    long steps = lround(mm / lead_mm_per_rev * steps_per_rev * microsteps);
    move_steps(steps, dir);
}

/// @brief Commanded step position (open-loop - see file header note on drift risk).
long Linear_actuator::get_position_steps()
{
    return position_steps;
}

/// @brief Zeroes the commanded step position, e.g. after manually placing a reference point.
void Linear_actuator::reset_position()
{
    position_steps = 0;
}
