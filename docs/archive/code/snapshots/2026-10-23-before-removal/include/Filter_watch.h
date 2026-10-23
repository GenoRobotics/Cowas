#ifndef FILTER_WATCH_H
#define FILTER_WATCH_H

#include <Arduino.h>
#include "Settings.h"
#include "../src/Core/fill_stop.h"

// The stop criteria of one filter run, shared by fill_test and every run that pumps through a filter
// (step_sampling, load_slot_water via CtrlPumpFlow). The rules are pure (src/Core/fill_stop.h, unit-tested); this class
// keeps their state, passes the time and the flow in, and prints the stall windows. Stage 1 S1-C (2026-10-19): one set
// of rules for every filter, so a clogged operation sample stops the same way the test does.
class FilterWatch
{
public:
    // stall_s <= 0 turns the window rules off. stuck_s <= 0 turns the pressure rule off (it needs a pause: fill_test only).
    void begin(float stall_s, float clog_fraction, float stuck_s, bool print);
    // starts the stall windows at `now` with the flowmeter at `ml` (called at the start of every run)
    void arm(uint32_t now, float ml);
    // after a pause that is not pumping time (a checkpoint wait): the windows keep their state, the time since `now` is not counted
    void resume_at(uint32_t now) { last_ = now; }
    // one check. paused = the pump is paused now; p and resume_bar on the same scale as the caller's pressure
    core::FillStop step(uint32_t now, bool paused, float ml, float p, float resume_bar);
    int clog_warnings() const { return clog_warnings_; }

private:
    core::FillWatch w_;
    float stall_s_;
    float stall_ms_;
    float clog_fraction_;
    float slow_ml_;      // F194: absolute floor per window (FILL_CLOG_MIN_WINDOW_ML)
    float stuck_ms_;
    bool print_;
    uint32_t last_;
    int clog_warnings_;
};

#endif // FILTER_WATCH_H
