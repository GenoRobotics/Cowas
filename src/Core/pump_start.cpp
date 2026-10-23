#include "pump_start.h"

namespace core
{

float pump_start_limit(uint32_t elapsed_ms, float rate_percent_per_s)
{
    return rate_percent_per_s * ((float)elapsed_ms / 1000.0f);
}

} // namespace core
