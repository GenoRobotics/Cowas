/**
 * @file Test_openloop.cpp
 * @brief FW2 sensorless pump test mode (F17). The ONLY code in the firmware that detaches
 * the pump's pressure cap (Pump::set_pressure_safety(nullptr, ...)); it puts back exactly
 * what setup() registered before returning. Test-only, low power, time-limited, and
 * confirmed by typing NOSENSOR at every test run (ROADMAP §6 rule 4 exception).
 * The stop decision is core::openloop_stop_decision() (unit-tested natively).
 */
#include <Arduino.h>
#include "Test_openloop.h"
#include "Test_report.h"
#include "Pump.h"
#include "Flow_sensor.h"
#include "Settings.h"
#include "Tests.h"

extern Pump pump;
extern Flow_sensor flow_sensor_small;

static bool openloop_armed = false;

OpenLoopSession::OpenLoopSession(const char *test_name) : armed_(false)
{
    if (PRESSURE_SENSOR_AVAILABLE)
    {
        return; // a sensor backs the cap: the tests use the normal procedures, never this mode
    }
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("!!! SENSORLESS PUMP TEST MODE - THE PRESSURE CAP WILL BE DETACHED     !!!");
    Serial.println("!!! No pressure sensor protects the line (F50). Stay at the machine,   !!!");
    Serial.println("!!! outlet OPEN, hands near the E-stop. Any key stops the pump.        !!!");
    Serial.println(String("!!! Power is clamped to ") + TEST_OPENLOOP_MAX_POWER + " %, each run to " +
                   (TEST_OPENLOOP_MAX_MS_LIMIT / 1000) + " s.");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println(String("Type NOSENSOR then ENTER to allow it for this run of ") + test_name + ", anything else cancels:");
    String typed = read_console_line();
    while (typed.length() == 0)
    {
        typed = read_console_line(); // a stray empty line
    }
    if (typed != "NOSENSOR")
    {
        Serial.println("Not confirmed - sensorless mode stays off.");
        return;
    }
    armed_ = true;
    openloop_armed = true;
    Serial.println("Sensorless mode ARMED for this test run.");
}

OpenLoopSession::~OpenLoopSession()
{
    if (armed_)
    {
        openloop_armed = false;
        Serial.println("Sensorless mode disarmed.");
    }
}

OpenLoopResult pump_open_loop_test(long power, float volume_ml, uint32_t max_ms, bool stop_on_no_flow)
{
    OpenLoopResult res;
    res.ran = false;
    res.reason = core::OPENLOOP_RUN;
    res.volume_ml = 0;
    res.elapsed_ms = 0;
    res.power = core::openloop_clamp_power(power, TEST_OPENLOOP_MAX_POWER);

    if (!openloop_armed || PRESSURE_SENSOR_AVAILABLE)
    {
        Serial.println("REFUSED | pump_open_loop_test: sensorless mode not armed (type NOSENSOR when the test asks)");
        return res;
    }
    max_ms = core::openloop_clamp_max_ms(max_ms, TEST_OPENLOOP_MAX_MS_LIMIT);

    Serial.println(String("!!! SENSORLESS PUMP RUN: cap DETACHED, power ") + res.power + " %" +
                   (volume_ml > 0 ? ", target " + String(volume_ml, 0) + " mL" : String(", no volume target")) +
                   ", max " + (max_ms / 1000.0) + " s" + (stop_on_no_flow ? ", stops after 5 s without flow" : "") +
                   " - ANY KEY STOPS");

    flow_sensor_small.reset_values();
    flow_sensor_small.activate();

    // --- detach: from here to the re-attach below there is no return, break or goto ---
    PressureReadFn saved_fn = pump.get_pressure_safety_fn();
    float saved_cap = pump.get_pressure_safety_cap();
    pump.set_pressure_safety(nullptr, 0);

    pump.set_power(res.power);
    pump.start();

    uint32_t start = millis();
    uint32_t last_pulse = start;
    uint32_t last_print = start;
    core::OpenLoopInputs in;
    in.target_ml = volume_ml;
    in.max_ms = max_ms;
    in.no_flow_ms = TEST_OPENLOOP_NO_FLOW_MS;
    in.stop_on_no_flow = stop_on_no_flow;

    core::OpenLoopStop decision = core::OPENLOOP_RUN;
    while (decision == core::OPENLOOP_RUN)
    {
        uint32_t now = millis();
        if (pulseCount_small > 0) // read before update() zeroes it
        {
            last_pulse = now;
        }
        flow_sensor_small.update();

        in.elapsed_ms = now - start;
        in.volume_ml = flow_sensor_small.get_totalFlowMilliL();
        in.ms_since_pulse = now - last_pulse;
        in.keypress = real_keypress_waiting() != 0;
        decision = core::openloop_stop_decision(in);

        if (now - last_print >= 2000)
        {
            Serial.println("  open-loop: " + String(in.elapsed_ms / 1000.0, 1) + " s, " + String(in.volume_ml, 0) + " mL");
            last_print = now;
        }
        delay(20);
    }

    pump.stop();
    pump.set_pressure_safety(saved_fn, saved_cap); // --- re-attach ---

    res.ran = true;
    res.reason = decision;
    res.elapsed_ms = millis() - start;
    res.volume_ml = flow_sensor_small.get_totalFlowMilliL();
    Serial.println(String("Open-loop run ended: ") + core::openloop_stop_name(decision) + ", " +
                   String(res.volume_ml, 0) + " mL, " + String(res.elapsed_ms / 1000.0, 1) + " s. Pressure cap re-attached.");
    return res;
}
