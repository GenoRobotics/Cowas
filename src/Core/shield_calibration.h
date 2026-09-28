#ifndef CORE_SHIELD_CALIBRATION_H
#define CORE_SHIELD_CALIBRATION_H

// Pure math for the DNA/RNA Shield micro pump calibration (LA3, console `shield_calibrate`):
// no Arduino.h, no globals, no hardware. Unit-tested on the host with `pio test -e native`
// (test/test_shield_calibration/). The procedure is described in docs/LA3_PROTOCOL.md.
//
// The micro pump is time-based (no flowmeter on the shield line). Each calibration run
// dispenses into a tared tube for a fixed time and the operator weighs it. A straight line
// volume = rate x t + offset is fitted over every run: the offset absorbs the spin-up and
// the drop left hanging at the tube end, so the rate stays right at any duration.

#include <stdint.h>

namespace core
{

// One weighed calibration run.
struct ShieldRun
{
    uint32_t duration_ms; // micro pump on-time
    float mass_g;         // dispensed mass read on the scale
};

// mass (g) -> volume (mL). NaN if the density is not > 0 or an input is NaN.
float shield_mass_to_ml(float mass_g, float density_g_per_ml);

// Volume of a round tube: pi x (ID/2)^2 x length, in mL. NaN if an input is negative or NaN.
float tube_volume_ml(float inner_diameter_mm, float length_m);

// Least-squares line volume_ml = rate_ml_per_s x t_s + offset_ml over all runs.
struct ShieldFit
{
    bool valid;          // false: < 2 distinct durations, a bad input, or rate <= 0
    int n_runs;          // runs used
    int n_durations;     // distinct durations among them
    float rate_ml_per_s; // slope
    float offset_ml;     // intercept (volume at t = 0)
    float r2;            // coefficient of determination (1 = every run on the line)
};

ShieldFit fit_shield_rate(const ShieldRun *runs, int n, float density_g_per_ml);

// Repeatability at one duration.
struct ShieldDurationStats
{
    uint32_t duration_ms;
    int n;
    float mean_ml;
    float sd_ml;  // sample SD (n - 1); NaN when n < 2
    float cv_pct; // 100 x sd / mean; NaN when n < 2 or mean <= 0
};

// Groups the runs by duration, in the order each duration first appears. Writes at most
// `capacity` entries and returns how many it wrote.
int shield_duration_stats(const ShieldRun *runs, int n, float density_g_per_ml,
                          ShieldDurationStats *out, int capacity);

// Micro pump on-time for a target volume: (target - offset) / rate, rounded to the nearest
// ms. Returns 0 when it can't be computed: invalid fit, target not above the offset, or the
// result is above max_ms.
uint32_t shield_fill_ms(const ShieldFit &fit, float target_ml, uint32_t max_ms);

enum ShieldCalVerdict
{
    SHIELD_CAL_PASS = 0,
    SHIELD_CAL_FAIL_FIT,           // the fit is invalid (see ShieldFit::valid)
    SHIELD_CAL_FAIL_REPEATS,       // a duration has fewer than min_repeats runs
    SHIELD_CAL_FAIL_REPEATABILITY, // a duration's CV is above max_cv_pct
    SHIELD_CAL_FAIL_LINEARITY      // r2 below min_r2
};

// Pass/fail of a calibration, checked in the order of the enum.
ShieldCalVerdict shield_cal_verdict(const ShieldFit &fit, const ShieldDurationStats *stats,
                                    int n_stats, int min_repeats, float max_cv_pct, float min_r2);

// Short name of a verdict, for the TEST lines ("PASS", "FIT", "REPEATS", ...).
const char *shield_cal_verdict_name(ShieldCalVerdict v);

} // namespace core

#endif
