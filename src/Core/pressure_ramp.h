#ifndef CORE_PRESSURE_RAMP_H
#define CORE_PRESSURE_RAMP_H

#include <stdint.h>

// S1-1b (2026-10-15): the pure parts of the `pressure_ramp` console test. The pump is driven by a
// PID towards a SETPOINT that rises slowly from 0 to a target, holds, then the pump stops and the
// decay is read. No Arduino.h; unit-tested with `pio test -e native` (test/test_pressure_ramp/).
// Used by cmd_pressure_ramp() in src/Tests_mech.cpp.

namespace core
{

enum RampPlanError
{
    RAMP_PLAN_OK = 0,
    RAMP_PLAN_TARGET_TOO_LOW,   // target <= 0
    RAMP_PLAN_TARGET_TOO_HIGH,  // target above (cap - margin): it would sit on the hard cap
    RAMP_PLAN_RATE_OUT_OF_RANGE // rate outside [min_rate, max_rate]
};

const char *ramp_plan_error_name(RampPlanError e);

// Is this ramp allowed? target and cap in bar, rate in bar/s. The target must stay at least
// `cap_margin` below the hard cap, so a normal run never touches it.
RampPlanError ramp_plan_check(float target_bar, float cap_bar, float cap_margin_bar,
                              float rate_bar_per_s, float min_rate, float max_rate);

// Setpoint (bar) `t_ms` after the start of the ramp: a straight line from `start_bar` to
// `target_bar` at `rate_bar_per_s`, clamped at the target. Also correct for a falling ramp.
float ramp_setpoint(float start_bar, float target_bar, float rate_bar_per_s, uint32_t t_ms);

// How long the ramp takes (ms), rounded up. 0 if the rate is not positive or start == target.
uint32_t ramp_duration_ms(float start_bar, float target_bar, float rate_bar_per_s);

// One output step limited to +-max_step (same unit as the values): the pump output may not jump.
float slew_limit(float previous, float wanted, float max_step);

// One step of a first-order low-pass (exponential average) on the pressure the PID reads.
// tau_s <= 0 returns x unchanged (filter= off).
float lowpass_step(float prev, float x, float dt_s, float tau_s);

} // namespace core

#endif
