#ifndef CORE_PUMP_RAMP_H
#define CORE_PUMP_RAMP_H

// Soft start of the pump output (2026-10-12, user): the output rises from its current value
// to the set power over a few seconds, so the pressure can be seen rising before it reaches the
// cap. Pure schedule, no Arduino.h, unit-tested with `pio test -e native` (test/test_pump_ramp/).
// Used by Pump::move_output_to() (src/Hardware/Pump.cpp).

namespace core
{

// Output (percent) after step `step` of `steps` of a ramp from `from` to `to` (both 0-100).
// step 0 gives `from`, step `steps` gives `to`. Rounds towards zero.
int pump_ramp_percent(int from, int to, int step, int steps);

// Does a change from `from` to `to` have to ramp? A start (always = true) always ramps. A live
// change ramps only when it is at least min_step percent (smaller changes are PID trims).
bool pump_ramp_needed(int from, int to, bool always, int min_step);

} // namespace core

#endif
