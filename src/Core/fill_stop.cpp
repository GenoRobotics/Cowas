#include "fill_stop.h"

namespace core
{

void fill_watch_start(FillWatch &w, float ml)
{
    w.pumping_ms = 0.0f;
    w.stuck_ms = 0.0f;
    w.win_ml = ml;
    w.first_gain_ml = -1.0f;
    w.prev_gain_ml = 0.0f;
    w.last_gain_ml = 0.0f;
    w.windows = 0;
    w.window_closed = false;
    w.slow_ml = 0.0f; // the caller sets the floor after start (it is configuration, not run state)
}

bool fill_gain_low(float gain_ml, float first_gain_ml, float fraction)
{
    return first_gain_ml > 0.0f && gain_ml < fraction * first_gain_ml;
}

bool fill_pair_clogged(float prev_gain_ml, float last_gain_ml, float first_gain_ml, float fraction)
{
    return first_gain_ml > 0.0f && 0.5f * (prev_gain_ml + last_gain_ml) < fraction * first_gain_ml;
}

FillStop fill_watch_step(FillWatch &w, float dt_ms, bool paused, float ml, float p, float resume_bar,
                         float stall_ms, float stall_min_ml, float stuck_limit_ms, float clog_fraction)
{
    if (paused)
    {
        // a pause is not a stall, but a pressure that does not come back down is a blocked filter
        w.stuck_ms = (p > resume_bar) ? w.stuck_ms + dt_ms : 0.0f;
        if (stuck_limit_ms > 0.0f && w.stuck_ms >= stuck_limit_ms)
        {
            return FILL_STOP_PRESSURE;
        }
        return FILL_GOING_ON;
    }
    w.stuck_ms = 0.0f;
    if (stall_ms <= 0.0f)
    {
        return FILL_GOING_ON;
    }
    w.pumping_ms += dt_ms;
    if (w.pumping_ms < stall_ms)
    {
        return FILL_GOING_ON;
    }

    // the window closes
    const float gain = ml - w.win_ml;
    w.pumping_ms = 0.0f;
    w.win_ml = ml;
    w.prev_gain_ml = w.last_gain_ml;
    w.last_gain_ml = gain;
    w.window_closed = true;
    w.windows++;
    if (w.first_gain_ml < 0.0f)
    {
        w.first_gain_ml = gain;
    }
    if (gain < stall_min_ml)
    {
        return FILL_STOP_STALLED;
    }
    if (w.slow_ml > 0.0f && gain < w.slow_ml)
    {
        return FILL_STOP_CLOGGED; // F194: slow from the start, no need to wait for a second window
    }
    if (clog_fraction > 0.0f && w.windows >= 3 &&
        fill_pair_clogged(w.prev_gain_ml, w.last_gain_ml, w.first_gain_ml, clog_fraction))
    {
        return FILL_STOP_CLOGGED;
    }
    return FILL_GOING_ON;
}

const char *fill_stop_name(FillStop reason)
{
    switch (reason)
    {
    case FILL_STOP_STALLED:
        return "stalled";
    case FILL_STOP_PRESSURE:
        return "pressure";
    case FILL_STOP_CLOGGED:
        return "clogged";
    default:
        return "running";
    }
}

} // namespace core
