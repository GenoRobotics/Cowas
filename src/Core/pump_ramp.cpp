#include "pump_ramp.h"

namespace core
{

int pump_ramp_percent(int from, int to, int step, int steps)
{
    if (steps <= 0)
    {
        return to;
    }
    if (step <= 0)
    {
        return from;
    }
    if (step >= steps)
    {
        return to;
    }
    return from + (to - from) * step / steps;
}

bool pump_ramp_needed(int from, int to, bool always, int min_step)
{
    if (always)
    {
        return true;
    }
    int change = (to > from) ? (to - from) : (from - to);
    return change >= min_step;
}

} // namespace core
