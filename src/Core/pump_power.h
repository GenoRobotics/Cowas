#ifndef CORE_PUMP_POWER_H
#define CORE_PUMP_POWER_H

// The power limit (%) of the operation pump (CtrlPump::run). Pure logic, no Arduino.h; unit-tested with
// `pio test -e native` (test/test_pump_power/).
//   water (default)   = water_max, 60 % (PUMP_MAX_POWER_PERCENT, user 2026-10-19)
//   air               = air_max, 100 % (PUMP_AIR_MAX_POWER_PERCENT, the emptying sees air)
//   full power        = full_max, 100 % (PUMP_PURGE_MAX_POWER_PERCENT): the one exception, set by step_purge() only
//                       (user 2026-10-26). The pressure target and the pressure cap still apply.

namespace core
{

float pump_power_limit_percent(bool air, bool full_power, float water_max, float air_max, float full_max);

} // namespace core

#endif
