#include "pressure_calibration.h"

#include <math.h>

namespace core
{

PressureCalFit fit_pressure_cal(const PressureCalPoint *points, int n)
{
    PressureCalFit fit = {false, 0, NAN, NAN, NAN, NAN, NAN};
    if (points == 0 || n < 2)
    {
        return fit;
    }

    double sx = 0, sy = 0;
    for (int i = 0; i < n; i++)
    {
        if (isnan(points[i].reference_bar) || isinf(points[i].reference_bar) ||
            isnan(points[i].nominal_bar) || isinf(points[i].nominal_bar))
        {
            return fit; // one bad point spoils the calibration: redo it, don't skip it
        }
        sx += points[i].nominal_bar;
        sy += points[i].reference_bar;
    }
    fit.n = n;

    const double mx = sx / n, my = sy / n;
    double sxx = 0, sxy = 0, syy = 0;
    for (int i = 0; i < n; i++)
    {
        const double dx = points[i].nominal_bar - mx;
        const double dy = points[i].reference_bar - my;
        sxx += dx * dx;
        sxy += dx * dy;
        syy += dy * dy;
    }
    if (!(sxx > 1e-9))
    {
        return fit; // every point at the same reading: no slope to fit
    }

    const double slope = sxy / sxx;
    const double intercept = my - slope * mx;
    fit.gain = (float)slope;
    fit.intercept_bar = (float)intercept;
    fit.r2 = (syy > 0) ? (float)((sxy * sxy) / (sxx * syy)) : 1.0f;
    if (!(slope > 0))
    {
        return fit;
    }
    fit.zero_offset_bar = (float)(-intercept / slope);

    double worst = 0;
    for (int i = 0; i < n; i++)
    {
        const double r = fabs(points[i].reference_bar - (slope * points[i].nominal_bar + intercept));
        if (r > worst)
        {
            worst = r;
        }
    }
    fit.max_residual_bar = (float)worst;
    fit.valid = true;
    return fit;
}

float pressure_cal_residual(const PressureCalFit &fit, const PressureCalPoint &p)
{
    if (!fit.valid)
    {
        return NAN;
    }
    return p.reference_bar - (fit.gain * p.nominal_bar + fit.intercept_bar);
}

PressureCalVerdict pressure_cal_verdict(const PressureCalFit &fit, int min_points, float gain_min, float gain_max,
                                        float max_residual_bar)
{
    if (!fit.valid)
    {
        return PRESSURE_CAL_FAIL_FIT;
    }
    if (fit.n < min_points)
    {
        return PRESSURE_CAL_FAIL_POINTS;
    }
    if (!(fit.gain >= gain_min && fit.gain <= gain_max))
    {
        return PRESSURE_CAL_FAIL_GAIN;
    }
    if (!(fit.max_residual_bar <= max_residual_bar))
    {
        return PRESSURE_CAL_FAIL_RESIDUAL;
    }
    return PRESSURE_CAL_PASS;
}

const char *pressure_cal_verdict_name(PressureCalVerdict v)
{
    switch (v)
    {
    case PRESSURE_CAL_PASS:
        return "PASS";
    case PRESSURE_CAL_FAIL_FIT:
        return "FIT";
    case PRESSURE_CAL_FAIL_POINTS:
        return "POINTS";
    case PRESSURE_CAL_FAIL_GAIN:
        return "GAIN";
    case PRESSURE_CAL_FAIL_RESIDUAL:
        return "RESIDUAL";
    }
    return "?";
}

} // namespace core
