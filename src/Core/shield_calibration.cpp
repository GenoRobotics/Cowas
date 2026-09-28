#include "shield_calibration.h"

#include <math.h>

namespace core
{

static const double PI_D = 3.14159265358979323846;

float shield_mass_to_ml(float mass_g, float density_g_per_ml)
{
    if (isnan(mass_g) || isnan(density_g_per_ml) || !(density_g_per_ml > 0.0f))
    {
        return NAN;
    }
    return mass_g / density_g_per_ml;
}

float tube_volume_ml(float inner_diameter_mm, float length_m)
{
    if (isnan(inner_diameter_mm) || isnan(length_m) || inner_diameter_mm < 0.0f || length_m < 0.0f)
    {
        return NAN;
    }
    // mm^2 x mm = mm^3 = uL, / 1000 = mL
    const double r_mm = inner_diameter_mm / 2.0;
    return (float)(PI_D * r_mm * r_mm * (length_m * 1000.0) / 1000.0);
}

ShieldFit fit_shield_rate(const ShieldRun *runs, int n, float density_g_per_ml)
{
    ShieldFit fit = {false, 0, 0, NAN, NAN, NAN};
    if (runs == 0 || n <= 0)
    {
        return fit;
    }

    double sx = 0, sy = 0;
    for (int i = 0; i < n; i++)
    {
        const float ml = shield_mass_to_ml(runs[i].mass_g, density_g_per_ml);
        if (isnan(ml) || isinf(ml) || runs[i].duration_ms == 0)
        {
            return fit; // one bad run spoils the calibration: redo it, don't skip it
        }
        sx += runs[i].duration_ms / 1000.0;
        sy += ml;
    }

    int distinct = 0;
    for (int i = 0; i < n; i++)
    {
        bool seen = false;
        for (int j = 0; j < i; j++)
        {
            if (runs[j].duration_ms == runs[i].duration_ms)
            {
                seen = true;
                break;
            }
        }
        if (!seen)
        {
            distinct++;
        }
    }
    fit.n_runs = n;
    fit.n_durations = distinct;
    if (distinct < 2)
    {
        return fit;
    }

    const double mx = sx / n, my = sy / n;
    double sxx = 0, sxy = 0, syy = 0;
    for (int i = 0; i < n; i++)
    {
        const double dx = runs[i].duration_ms / 1000.0 - mx;
        const double dy = shield_mass_to_ml(runs[i].mass_g, density_g_per_ml) - my;
        sxx += dx * dx;
        sxy += dx * dy;
        syy += dy * dy;
    }
    const double slope = sxy / sxx; // sxx > 0: at least two distinct durations
    fit.rate_ml_per_s = (float)slope;
    fit.offset_ml = (float)(my - slope * mx);
    fit.r2 = (syy > 0) ? (float)((sxy * sxy) / (sxx * syy)) : 1.0f;
    fit.valid = slope > 0;
    return fit;
}

int shield_duration_stats(const ShieldRun *runs, int n, float density_g_per_ml,
                          ShieldDurationStats *out, int capacity)
{
    int count = 0;
    if (runs == 0 || out == 0)
    {
        return 0;
    }
    for (int i = 0; i < n && count < capacity; i++)
    {
        bool seen = false;
        for (int k = 0; k < count; k++)
        {
            if (out[k].duration_ms == runs[i].duration_ms)
            {
                seen = true;
                break;
            }
        }
        if (seen)
        {
            continue;
        }

        double sum = 0;
        int m = 0;
        for (int j = i; j < n; j++)
        {
            if (runs[j].duration_ms == runs[i].duration_ms)
            {
                sum += shield_mass_to_ml(runs[j].mass_g, density_g_per_ml);
                m++;
            }
        }
        const double mean = sum / m;
        double ss = 0;
        for (int j = i; j < n; j++)
        {
            if (runs[j].duration_ms == runs[i].duration_ms)
            {
                const double d = shield_mass_to_ml(runs[j].mass_g, density_g_per_ml) - mean;
                ss += d * d;
            }
        }

        ShieldDurationStats s;
        s.duration_ms = runs[i].duration_ms;
        s.n = m;
        s.mean_ml = (float)mean;
        s.sd_ml = (m >= 2) ? (float)sqrt(ss / (m - 1)) : NAN;
        s.cv_pct = (m >= 2 && mean > 0) ? (float)(100.0 * sqrt(ss / (m - 1)) / mean) : NAN;
        out[count++] = s;
    }
    return count;
}

uint32_t shield_fill_ms(const ShieldFit &fit, float target_ml, uint32_t max_ms)
{
    if (!fit.valid || isnan(target_ml) || !(target_ml > fit.offset_ml))
    {
        return 0;
    }
    const double ms = 1000.0 * (target_ml - fit.offset_ml) / fit.rate_ml_per_s;
    if (!(ms <= (double)max_ms))
    {
        return 0;
    }
    return (uint32_t)(ms + 0.5);
}

ShieldCalVerdict shield_cal_verdict(const ShieldFit &fit, const ShieldDurationStats *stats,
                                    int n_stats, int min_repeats, float max_cv_pct, float min_r2)
{
    if (!fit.valid)
    {
        return SHIELD_CAL_FAIL_FIT;
    }
    for (int i = 0; i < n_stats; i++)
    {
        if (stats[i].n < min_repeats)
        {
            return SHIELD_CAL_FAIL_REPEATS;
        }
    }
    for (int i = 0; i < n_stats; i++)
    {
        // NaN (n < 2) only gets here when min_repeats < 2: then it can't fail repeatability
        if (stats[i].cv_pct > max_cv_pct)
        {
            return SHIELD_CAL_FAIL_REPEATABILITY;
        }
    }
    if (!(fit.r2 >= min_r2))
    {
        return SHIELD_CAL_FAIL_LINEARITY;
    }
    return SHIELD_CAL_PASS;
}

const char *shield_cal_verdict_name(ShieldCalVerdict v)
{
    switch (v)
    {
    case SHIELD_CAL_PASS:
        return "PASS";
    case SHIELD_CAL_FAIL_FIT:
        return "FIT";
    case SHIELD_CAL_FAIL_REPEATS:
        return "REPEATS";
    case SHIELD_CAL_FAIL_REPEATABILITY:
        return "REPEATABILITY";
    case SHIELD_CAL_FAIL_LINEARITY:
        return "LINEARITY";
    }
    return "?";
}

} // namespace core
