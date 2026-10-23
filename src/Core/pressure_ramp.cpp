#include "pressure_ramp.h"

namespace core
{

const char *ramp_plan_error_name(RampPlanError e)
{
    switch (e)
    {
    case RAMP_PLAN_OK:
        return "ok";
    case RAMP_PLAN_TARGET_TOO_LOW:
        return "target_too_low";
    case RAMP_PLAN_TARGET_TOO_HIGH:
        return "target_too_high";
    case RAMP_PLAN_RATE_OUT_OF_RANGE:
        return "rate_out_of_range";
    }
    return "?";
}

RampPlanError ramp_plan_check(float target_bar, float cap_bar, float cap_margin_bar,
                              float rate_bar_per_s, float min_rate, float max_rate)
{
    if (!(target_bar > 0.0f))
    {
        return RAMP_PLAN_TARGET_TOO_LOW;
    }
    if (target_bar > cap_bar - cap_margin_bar)
    {
        return RAMP_PLAN_TARGET_TOO_HIGH;
    }
    if (!(rate_bar_per_s >= min_rate && rate_bar_per_s <= max_rate))
    {
        return RAMP_PLAN_RATE_OUT_OF_RANGE;
    }
    return RAMP_PLAN_OK;
}

float ramp_setpoint(float start_bar, float target_bar, float rate_bar_per_s, uint32_t t_ms)
{
    if (!(rate_bar_per_s > 0.0f))
    {
        return start_bar;
    }
    float moved = rate_bar_per_s * (float)t_ms / 1000.0f;
    float span = target_bar - start_bar;
    if (span >= 0.0f)
    {
        return (moved >= span) ? target_bar : start_bar + moved;
    }
    return (moved >= -span) ? target_bar : start_bar - moved;
}

uint32_t ramp_duration_ms(float start_bar, float target_bar, float rate_bar_per_s)
{
    float span = target_bar - start_bar;
    if (span < 0.0f)
    {
        span = -span;
    }
    if (!(rate_bar_per_s > 0.0f) || span <= 0.0f)
    {
        return 0;
    }
    float ms = span / rate_bar_per_s * 1000.0f;
    uint32_t whole = (uint32_t)ms;
    return ((float)whole < ms) ? whole + 1 : whole;
}

float slew_limit(float previous, float wanted, float max_step)
{
    if (wanted > previous + max_step)
    {
        return previous + max_step;
    }
    if (wanted < previous - max_step)
    {
        return previous - max_step;
    }
    return wanted;
}

float lowpass_step(float prev, float x, float dt_s, float tau_s)
{
    if (!(tau_s > 0.0f) || !(dt_s > 0.0f))
    {
        return x;
    }
    return prev + (dt_s / (tau_s + dt_s)) * (x - prev);
}

} // namespace core
