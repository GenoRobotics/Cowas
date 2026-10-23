#ifndef POWER_RAMP_H
#define POWER_RAMP_H

#include <stdint.h>
#include <cmath>

namespace core {

// Power ramp generator: linearly interpolate with slew limit
// Returns power (0-100%) at time t_ms, ramping from start_power to peak_power
// over ramp_duration_ms, respecting max_slew_pct_per_100ms
float power_ramp_value(
  float t_ms,
  float ramp_duration_ms,
  uint8_t start_power,
  uint8_t peak_power,
  uint8_t max_slew_pct_per_100ms
);

// Check if ramp has reached peak power or timeout
bool power_ramp_complete(float t_ms, float ramp_duration_ms);

// Rise-rate limiter: reduce power ramp slope if pressure rises too fast
// If dP/dt (bar/s) exceeds max_rise_rate_per_sec, return reduced power command
// Returns modified power_cmd (may be lower to slow the ramp)
float rise_rate_limit(
  float p_bar,
  float p_prev_bar,
  uint32_t dt_ms,
  float max_rise_rate_per_sec,
  float power_cmd_requested
);

// Detect clog: if power is at peak but pressure won't rise
// Returns true if clogged (high resistance)
bool is_clogged(
  float power_current_pct,
  float peak_power_pct,
  float pressure_bar,
  float clog_threshold_bar
);

}  // namespace core

#endif
