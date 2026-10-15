#include "pid_metrics.h"

namespace core
{

void pid_stats_begin(PidRunStats *s, float target, float rise_frac, float reversal_percent, float max_power)
{
    s->target = target;
    s->rise_frac = rise_frac;
    s->reversal_percent = reversal_percent;
    s->max_power = max_power;
    s->n_all = 0;
    s->p_max = -1000.0f;
    s->rise_s = -1.0f;
    s->n_late = 0;
    s->p_sum = 0.0f;
    s->p_min = 1000.0f;
    s->p_max_late = -1000.0f;
    s->pw_sum = 0.0f;
    s->pw_min = 1000.0f;
    s->pw_max = -1000.0f;
    s->n_saturated = 0;
    s->reversals = 0;
    s->dir = 0;
    s->pw_ext = 0.0f;
}

static void count_reversal(PidRunStats *s, float power, bool first)
{
    if (first)
    {
        s->pw_ext = power;
        s->dir = 0;
        return;
    }
    const float thr = s->reversal_percent;
    if (s->dir == 0)
    {
        if (power - s->pw_ext >= thr)
        {
            s->dir = 1;
            s->pw_ext = power;
        }
        else if (s->pw_ext - power >= thr)
        {
            s->dir = -1;
            s->pw_ext = power;
        }
    }
    else if (s->dir > 0)
    {
        if (power > s->pw_ext)
        {
            s->pw_ext = power;
        }
        else if (s->pw_ext - power >= thr)
        {
            s->reversals++;
            s->dir = -1;
            s->pw_ext = power;
        }
    }
    else
    {
        if (power < s->pw_ext)
        {
            s->pw_ext = power;
        }
        else if (power - s->pw_ext >= thr)
        {
            s->reversals++;
            s->dir = 1;
            s->pw_ext = power;
        }
    }
}

void pid_stats_add(PidRunStats *s, uint32_t t_ms, float p, float power, bool late)
{
    s->n_all++;
    if (p > s->p_max)
    {
        s->p_max = p;
    }
    if (s->rise_s < 0.0f && p >= s->rise_frac * s->target)
    {
        s->rise_s = (float)t_ms / 1000.0f;
    }
    if (!late)
    {
        return;
    }
    count_reversal(s, power, s->n_late == 0);
    s->n_late++;
    s->p_sum += p;
    if (p < s->p_min)
    {
        s->p_min = p;
    }
    if (p > s->p_max_late)
    {
        s->p_max_late = p;
    }
    s->pw_sum += power;
    if (power < s->pw_min)
    {
        s->pw_min = power;
    }
    if (power > s->pw_max)
    {
        s->pw_max = power;
    }
    if (power >= s->max_power - 0.5f)
    {
        s->n_saturated++;
    }
}

bool pid_stats_has_late(const PidRunStats *s)
{
    return s->n_late > 0;
}

float pid_stats_overshoot(const PidRunStats *s)
{
    return (s->n_all > 0 && s->p_max > s->target) ? s->p_max - s->target : 0.0f;
}

float pid_stats_mean_error(const PidRunStats *s)
{
    return s->n_late ? s->p_sum / (float)s->n_late - s->target : 0.0f;
}

float pid_stats_ripple(const PidRunStats *s)
{
    return s->n_late ? s->p_max_late - s->p_min : 0.0f;
}

float pid_stats_power_mean(const PidRunStats *s)
{
    return s->n_late ? s->pw_sum / (float)s->n_late : 0.0f;
}

float pid_stats_power_p2p(const PidRunStats *s)
{
    return s->n_late ? s->pw_max - s->pw_min : 0.0f;
}

float pid_stats_saturated_pct(const PidRunStats *s)
{
    return s->n_late ? 100.0f * (float)s->n_saturated / (float)s->n_late : 0.0f;
}

} // namespace core
