#ifndef TEST_OPENLOOP_H
#define TEST_OPENLOOP_H

// FW2 sensorless pump test mode (F17; the single exception to ROADMAP §6 rule 4).
// TEST CODE ONLY: include this header from the console test suite (Tests_mech.cpp) and
// nowhere else. No operation path (main_program, button_control, Step_functions,
// maintenance) may reach it - see the grep proof in docs/logs/LOG.md (FW2).

#include <Arduino.h>
#include "../src/Core/openloop.h"

struct OpenLoopResult
{
    bool ran;                  // false: refused (not armed, or a pressure sensor is available)
    core::OpenLoopStop reason; // why it stopped
    float volume_ml;           // flowmeter total (calibrated factor 77; reads low at slow flow, F150)
    uint32_t elapsed_ms;
    uint8_t power;             // the power actually used, after the clamp
};

// Arms the mode for one test command: a loud warning, then the operator must type
// NOSENSOR (exactly, then ENTER). Never arms (and asks nothing) while PRESSURE_SENSOR_AVAILABLE is true.
// Disarmed when the session object goes out of scope, on every return path.
class OpenLoopSession
{
public:
    explicit OpenLoopSession(const char *test_name);
    ~OpenLoopSession();
    bool armed() const { return armed_; }

private:
    bool armed_;
};

// Runs the big pump at a FIXED power, clamped to TEST_OPENLOOP_MAX_POWER, with the
// pressure cap detached (it reads a floating pin today, F50). Stops on: any keypress,
// volume_ml reached (<= 0: no volume stop), max_ms (clamped to
// TEST_OPENLOOP_MAX_MS_LIMIT), or no flowmeter pulse for TEST_OPENLOOP_NO_FLOW_MS (unless
// stop_on_no_flow is false, e.g. an air push). The cap is re-attached before it returns.
// Valves and the O-ring are the caller's job. Refuses unless a session is armed.
OpenLoopResult pump_open_loop_test(long power, float volume_ml, uint32_t max_ms, bool stop_on_no_flow = true);

#endif
