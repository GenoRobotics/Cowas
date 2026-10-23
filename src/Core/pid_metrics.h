#ifndef CORE_PID_METRICS_H
#define CORE_PID_METRICS_H

#include <stdint.h>

// S1-1b (2026-10-15): numbers that say whether a pressure PID is better or worse, from one run of
// `pressure_ramp`. Streaming (no array): the caller feeds one sample per PID update. Pure, no
// Arduino.h, unit-tested with `pio test -e native` (test/test_pid_metrics/).
// The comparison between two runs is done by tools/compare_tune.py from the TEST lines.

namespace core
{

struct PidRunStats
{
    float target;      // bar
    float rise_frac;   // fraction of the target that counts as "risen" (0.9)
    float reversal_percent;
    float max_power;   // % allowed for the output (for the saturation share)

    uint32_t n_all;
    float p_max;
    float rise_s;      // first time P >= rise_frac * target, in s; -1 = never

    // the "late hold" window (the second half of the hold: after the transient)
    uint32_t n_late;
    float p_sum, p_min, p_max_late;
    float pw_sum, pw_min, pw_max;
    uint32_t n_saturated;
    uint32_t reversals;
    int dir;           // power direction: 0 unknown, +1 rising, -1 falling
    float pw_ext;      // the last extreme of the power
};

void pid_stats_begin(PidRunStats *s, float target, float rise_frac, float reversal_percent, float max_power);

// One sample. t_ms since the start of the control loop, p in bar, power in %. late = the sample is
// in the second half of the hold.
void pid_stats_add(PidRunStats *s, uint32_t t_ms, float p, float power, bool late);

// Results (NAN-free: 0 when there is no late-hold sample, check pid_stats_has_late first)
bool pid_stats_has_late(const PidRunStats *s);
float pid_stats_overshoot(const PidRunStats *s);   // max(0, p_max - target), whole run
float pid_stats_mean_error(const PidRunStats *s);  // mean(P late) - target
float pid_stats_ripple(const PidRunStats *s);      // max - min of P in the late hold
float pid_stats_power_mean(const PidRunStats *s);
float pid_stats_power_p2p(const PidRunStats *s);   // max - min of the power in the late hold
float pid_stats_saturated_pct(const PidRunStats *s); // share of late samples at max power

} // namespace core

#endif
