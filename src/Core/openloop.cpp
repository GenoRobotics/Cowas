#include "openloop.h"

namespace core
{

const char *openloop_stop_name(OpenLoopStop s)
{
    switch (s)
    {
    case OPENLOOP_KEYPRESS:
        return "keypress";
    case OPENLOOP_VOLUME:
        return "volume";
    case OPENLOOP_MAX_TIME:
        return "max_time";
    case OPENLOOP_NO_FLOW:
        return "no_flow";
    default:
        return "run";
    }
}

OpenLoopStop openloop_stop_decision(const OpenLoopInputs &in)
{
    if (in.keypress)
    {
        return OPENLOOP_KEYPRESS;
    }
    // NaN compares false, so a NaN target or volume never ends the run on volume.
    if (in.target_ml > 0 && in.volume_ml >= in.target_ml)
    {
        return OPENLOOP_VOLUME;
    }
    if (in.elapsed_ms >= in.max_ms)
    {
        return OPENLOOP_MAX_TIME;
    }
    if (in.stop_on_no_flow && in.ms_since_pulse >= in.no_flow_ms)
    {
        return OPENLOOP_NO_FLOW;
    }
    return OPENLOOP_RUN;
}

uint8_t openloop_clamp_power(long requested, uint8_t max_power)
{
    long cap = max_power > 100 ? 100 : max_power;
    if (requested < 0)
    {
        return 0;
    }
    return (uint8_t)(requested > cap ? cap : requested);
}

uint32_t openloop_clamp_max_ms(uint32_t requested_ms, uint32_t limit_ms)
{
    return requested_ms > limit_ms ? limit_ms : requested_ms;
}

} // namespace core
