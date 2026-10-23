#ifndef CORE_FILL_STOP_H
#define CORE_FILL_STOP_H

// Stop criteria of fill_test that depend on time and on the flow. The volume and the hard timeout stay in the caller.
// Pumping time is split into stall windows (stall_ms of pumping time each; pauses do not count, and do not restart a window,
// F149). At the end of each window:
//   - STALLED:  the window counted less than stall_min_ml (absolute: almost no flow).
//   - CLOGGED:  the mean of the last two windows is below clog_fraction of the FIRST window (the healthy reference),
//               from the third window on (2026-10-19, user log: 8, 28, 9, 3, 1 mL per minute, the flowmeter is noisy, so a
//               pair of windows is the smallest reliable sample). Also a warning, CLOG?, below clog_fraction for one window.
//   - CLOGGED (absolute, F194): a window below slow_ml stops at once, from the FIRST window on. The relative rule above
//               cannot see a filter that is slow from the start (2026-10-23: 24, 10, 4 mL per minute stopped only at 213 s).
//               slow_ml <= 0 turns it off.
// PRESSURE:  the pump is paused and the pressure does not fall back below the resume level for stuck_ms.
// Pure logic, no Arduino.h, unit-tested with `pio test -e native` (test/test_fill_stop/). Used by FilterWatch (include/Filter_watch.h),
// which every filter run uses: fill_test (src/Tests_mech.cpp) and the operation sampling (CtrlPumpFlow, src/Hardware/Pump.cpp).

namespace core
{

enum FillStop
{
    FILL_GOING_ON = 0,
    FILL_STOP_STALLED,
    FILL_STOP_PRESSURE,
    FILL_STOP_CLOGGED
};

// State of the criteria, kept by the caller between steps.
struct FillWatch
{
    float pumping_ms;    // pumping time in the current stall window
    float stuck_ms;      // paused time with the pressure still above the resume level
    float win_ml;        // flowmeter volume at the start of the current stall window
    float first_gain_ml; // mL counted in the first closed window (the reference), -1 until it closes
    float prev_gain_ml;  // mL counted in the window before the last one
    float last_gain_ml;  // mL counted in the last closed window
    int windows;         // closed windows so far
    bool window_closed;  // set when a window closes; the caller reads it and clears it
    float slow_ml;       // absolute floor per window (mL). A window below it stops the run as clogged. <= 0 = off
};

void fill_watch_start(FillWatch &w, float ml);

// Single window below fraction of the first window (a warning, not a stop).
bool fill_gain_low(float gain_ml, float first_gain_ml, float fraction);

// Mean of the last two windows below fraction of the first window (the stop rule).
bool fill_pair_clogged(float prev_gain_ml, float last_gain_ml, float first_gain_ml, float fraction);

// One step of dt_ms. paused = the pump is paused now. stall_ms <= 0 turns the window rules off.
// stuck_limit_ms <= 0 turns the pressure rule off. clog_fraction <= 0 turns the clog rule off.
FillStop fill_watch_step(FillWatch &w, float dt_ms, bool paused, float ml, float p, float resume_bar,
                         float stall_ms, float stall_min_ml, float stuck_limit_ms, float clog_fraction);

// The reason as the log prints it.
const char *fill_stop_name(FillStop reason);

} // namespace core

#endif
