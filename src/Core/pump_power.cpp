#include "pump_power.h"

namespace core
{

float pump_power_limit_percent(bool air, bool full_power, float water_max, float air_max, float full_max)
{
    if (full_power)
        return full_max;
    return air ? air_max : water_max;
}

} // namespace core
