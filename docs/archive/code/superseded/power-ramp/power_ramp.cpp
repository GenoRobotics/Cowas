#include "power_ramp.h"
#include <algorithm>

namespace core {

float power_ramp_value(
  float t_ms,
  float ramp_duration_ms,
  uint8_t start_power,
  uint8_t peak_power,
  uint8_t max_slew_pct_per_100ms
) {
  if (ramp_duration_ms <= 0) return (float)peak_power;
  if (t_ms <= 0) return (float)start_power;

  float t_s = t_ms / 1000.0f;
  float ramp_s = ramp_duration_ms / 1000.0f;

  // Linear ramp without slew limit
  float frac = t_s / ramp_s;
  frac = std::min(1.0f, frac);

  float target_power = start_power + frac * (peak_power - start_power);

  // Apply slew limit: max change per 100ms
  // Since we're called once per 100ms typically, check that change doesn't exceed limit
  float max_step_per_update = max_slew_pct_per_100ms;

  // On first call (no previous value), we can't enforce slew, so start at start_power
  // This function doesn't store state, so slew enforcement must be done by caller
  // We just return the target power, and caller enforces slew limit

  return std::max(0.0f, std::min(100.0f, target_power));
}

bool power_ramp_complete(float t_ms, float ramp_duration_ms) {
  if (ramp_duration_ms <= 0) return true;
  return t_ms >= ramp_duration_ms;
}

float rise_rate_limit(
  float p_bar,
  float p_prev_bar,
  uint32_t dt_ms,
  float max_rise_rate_per_sec,
  float power_cmd_requested
) {
  if (dt_ms == 0 || p_bar <= p_prev_bar) {
    return power_cmd_requested;  // Pressure not rising or no time has passed
  }

  float dp_bar = p_bar - p_prev_bar;
  float dt_s = dt_ms / 1000.0f;
  float rise_rate_per_sec = dp_bar / dt_s;

  // If rise rate exceeds limit, reduce power command
  if (rise_rate_per_sec > max_rise_rate_per_sec) {
    // Slow down: reduce requested power by 20% as a heuristic
    float reduced_power = power_cmd_requested * 0.8f;
    return std::max(0.0f, std::min(100.0f, reduced_power));
  }

  return power_cmd_requested;
}

bool is_clogged(
  float power_current_pct,
  float peak_power_pct,
  float pressure_bar,
  float clog_threshold_bar
) {
  // Clog detected if power has reached peak but pressure is still below threshold
  return (power_current_pct >= peak_power_pct - 1.0f) && (pressure_bar < clog_threshold_bar);
}

}  // namespace core
