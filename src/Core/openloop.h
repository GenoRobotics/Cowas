#ifndef CORE_OPENLOOP_H
#define CORE_OPENLOOP_H

// Sensorless pump test mode (FW2, F17): the stop decision of pump_open_loop_test()
// (src/Test_openloop.cpp) and its input clamps. Pure logic, no Arduino.h, unit-tested
// with `pio test -e native` (test/test_openloop/).

#include <stdint.h>

namespace core
{

enum OpenLoopStop
{
    OPENLOOP_RUN = 0,  // keep pumping
    OPENLOOP_KEYPRESS, // the operator pressed a key
    OPENLOOP_VOLUME,   // the flowmeter reached the target volume
    OPENLOOP_MAX_TIME, // max_ms elapsed
    OPENLOOP_NO_FLOW   // no flowmeter pulse for no_flow_ms
};

const char *openloop_stop_name(OpenLoopStop s); // "run", "keypress", "volume", "max_time", "no_flow"

struct OpenLoopInputs
{
    uint32_t elapsed_ms;        // since the pump started
    float volume_ml;            // flowmeter total since the start (NaN = unknown: no volume stop)
    float target_ml;            // <= 0 or NaN: no volume stop (time-only run, e.g. an air push)
    uint32_t max_ms;            // stop at elapsed_ms >= max_ms (0 = stop at once)
    uint32_t ms_since_pulse;    // since the last flowmeter pulse (or the start)
    uint32_t no_flow_ms;        // stop at ms_since_pulse >= no_flow_ms
    bool stop_on_no_flow;       // false for an air push: air gives no pulses
    bool keypress;
};

// Checked in this order: keypress, volume, max time, no flow. The first match wins.
OpenLoopStop openloop_stop_decision(const OpenLoopInputs &in);

// Requested power (percent) clamped to [0, max_power]; max_power itself is capped at 100.
uint8_t openloop_clamp_power(long requested, uint8_t max_power);

// Requested run time clamped to [0, limit_ms].
uint32_t openloop_clamp_max_ms(uint32_t requested_ms, uint32_t limit_ms);

} // namespace core

#endif
