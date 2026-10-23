#include "Filter_watch.h"

// Thin driver around src/Core/fill_stop: time, flow and the printed window lines. The rules themselves are in Core.

void FilterWatch::begin(float stall_s, float clog_fraction, float stuck_s, bool print)
{
    stall_s_ = stall_s;
    stall_ms_ = stall_s * 1000.0f;
    clog_fraction_ = clog_fraction;
    stuck_ms_ = stuck_s * 1000.0f;
    print_ = print;
    clog_warnings_ = 0;
    slow_ml_ = FILL_CLOG_MIN_WINDOW_ML;
    core::fill_watch_start(w_, 0.0f);
    w_.slow_ml = slow_ml_;
    last_ = millis();
}

void FilterWatch::arm(uint32_t now, float ml)
{
    core::fill_watch_start(w_, ml);
    w_.slow_ml = slow_ml_;
    last_ = now;
    clog_warnings_ = 0;
}

core::FillStop FilterWatch::step(uint32_t now, bool paused, float ml, float p, float resume_bar)
{
    const core::FillStop reason = core::fill_watch_step(w_, (float)(now - last_), paused, ml, p, resume_bar,
                                                        stall_ms_, FILL_STALL_MIN_ML, stuck_ms_, clog_fraction_);
    last_ = now;
    if (w_.window_closed)
    {
        w_.window_closed = false;
        const bool low = core::fill_gain_low(w_.last_gain_ml, w_.first_gain_ml, clog_fraction_);
        if (low)
        {
            clog_warnings_++;
        }
        if (print_)
        {
            Serial.println("  window " + String(stall_s_, 0) + " s pumping: " + String(w_.last_gain_ml, 0) + " mL (first " +
                           String(w_.first_gain_ml, 0) + " mL)" + (low ? "  CLOG? below " + String(clog_fraction_ * 100.0f, 0) + " % of the first window" : String("")));
        }
    }
    return reason;
}
