#ifndef CORE_PUMP_START_H
#define CORE_PUMP_START_H

#include <stdint.h>

// F196 (2026-10-23): after a start or a restart the operation pump rises from 0 %, at a set rate, instead of jumping to
// the set power. On a dead-headed line (no flow, a clogged filter) a jump to 15 % took the pressure from 0 to the pause
// level in about 3 s and overshot the cap (P1, 3.69 bar). Pure logic, no Arduino.h; unit-tested with `pio test -e native`.

namespace core
{

// The highest pump power (%) allowed elapsed_ms after a start or a restart.
float pump_start_limit(uint32_t elapsed_ms, float rate_percent_per_s);

} // namespace core

#endif
