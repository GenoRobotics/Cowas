#include "spool_motion.h"

#include <math.h>

namespace core
{

bool spool_depth_to_length_cm(float depth_m, float height_cm, float max_length_cm, float *length_cm)
{
    if (!(depth_m >= 0.0f) || !(height_cm >= 0.0f) || !(max_length_cm > 0.0f)) // also false for NaN
    {
        return false;
    }
    const float length = depth_m * 100.0f + height_cm;
    if (!(length <= max_length_cm))
    {
        return false;
    }
    *length_cm = length;
    return true;
}

SpoolMove spool_move_state(const SpoolMoveParams &p, float start_cm, float now_cm, uint32_t elapsed_ms, uint32_t still_ms)
{
    if (!p.to_switch)
    {
        if ((p.dir > 0 && now_cm >= p.goal_cm) || (p.dir < 0 && now_cm <= p.goal_cm))
        {
            return SPOOL_MOVE_ARRIVED;
        }
        const float moved = (now_cm - start_cm) * (p.dir > 0 ? 1.0f : -1.0f);
        if (moved < -p.wrong_way_cm)
        {
            return SPOOL_MOVE_WRONG_WAY;
        }
    }
    if (still_ms >= p.stall_ms)
    {
        return SPOOL_MOVE_STALLED;
    }
    if (elapsed_ms >= p.max_ms)
    {
        return SPOOL_MOVE_TIMEOUT;
    }
    return SPOOL_MOVE_GO;
}

const char *spool_move_name(SpoolMove m)
{
    switch (m)
    {
    case SPOOL_MOVE_GO:
        return "going";
    case SPOOL_MOVE_ARRIVED:
        return "arrived";
    case SPOOL_MOVE_STALLED:
        return "stalled (encoder not counting)";
    case SPOOL_MOVE_WRONG_WAY:
        return "wrong way (encoder counts the other way)";
    case SPOOL_MOVE_TIMEOUT:
        return "timeout";
    }
    return "?";
}

uint8_t spool_approach_speed(float remaining_cm, float slow_zone_cm, uint8_t fast_speed, uint8_t slow_speed)
{
    return remaining_cm < slow_zone_cm ? slow_speed : fast_speed;
}

SpoolFit spool_fit_scale(const SpoolStop *stops, int n)
{
    SpoolFit fit = {false, 0, NAN, NAN};
    if (n < 1)
    {
        return fit;
    }
    double sum_rm = 0.0, sum_rr = 0.0;
    for (int i = 0; i < n; i++)
    {
        const float r = stops[i].reading_cm, m = stops[i].measured_cm;
        if (!(r > 0.0f) || !(m > 0.0f))
        {
            return fit;
        }
        const float ratio = m / r;
        if (ratio < 0.5f || ratio > 2.0f)
        {
            return fit;
        }
        sum_rm += (double)r * m;
        sum_rr += (double)r * r;
    }
    fit.scale = (float)(sum_rm / sum_rr);
    fit.n = n;
    float worst = 0.0f;
    for (int i = 0; i < n; i++)
    {
        const float dev = fabsf(stops[i].measured_cm - fit.scale * stops[i].reading_cm) / stops[i].measured_cm * 100.0f;
        if (dev > worst)
        {
            worst = dev;
        }
    }
    fit.worst_dev_pct = worst;
    fit.valid = true;
    return fit;
}

float spool_new_diameter_cm(float old_diameter_cm, float scale)
{
    return old_diameter_cm * scale;
}

} // namespace core
