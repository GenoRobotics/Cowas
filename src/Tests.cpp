/**
 * @file Tests.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief Try things, be free. No guarantie that those functions work.
 * @version 0.1
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>
#include "Button.h"
#include "Trustability_ABP_Gage.h"
#include "Valve_3_2.h"
#include "Valve_2_2.h"
#include "Pump.h"
#include "Micro_pump.h"
#include "Motor.h"
#include "Encoder.h"
#include "Led.h"
#include "C_output.h"
#include "Settings.h"
#include "Timer.h"
#include "Tests.h"
#include "Step_functions.h"
#include "Manifold.h"
#include "maintenance.h"
#include "Linear_actuator.h"
#include "Flow_sensor.h"
#include "Oring_lock.h"
#include "Core/shield_calibration.h"
#include "Core/abp_convert.h"
#include "Core/pressure_calibration.h"
#include "Test_report.h"
#include "Tests_mech.h"
#include "Run_keys.h"

extern int manifold_slot;
extern C_output output;
extern Led status_led;
extern Led green_led;
extern Trustability_ABP_Gage pressure1;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_manifold;
extern Pump pump;
extern Flow_sensor flow_sensor_small;

extern Micro_Pump micro_pump;

extern Motor spool;
extern Motor manifold_motor;
extern Encoder encoder;
extern Button button_start;
extern Button button_container;
extern Button button_spool_up;
extern Button button_spool_down;
extern Button button_left;
extern Button button_right;
// extern Potentiometer potentiometer;
extern struct Timer timer_control_pressure1;

extern Linear_actuator linear_actuator;
extern Manifold manifold;

void test_all_components()
{
    // call all sub-functions

    // go_to_zero();

    String test_command;
    while (Serial.available())
    {
        Serial.read(); // what was typed during the boot is incomplete (F49): start clean
    }
    Serial.println("=== Test menu READY: type a command + ENTER (`help` lists them) ===");
    while (42)
    {
        if (Serial.available())
        {
            // test_command = Serial.readString();
            test_command = Serial.readStringUntil('\n');
            // test_command = Serial.readStringUntil('b');
            // F37: a CR, a space or a stray byte around the command made it silently do nothing
            test_command.trim();
            while (test_command.length() > 0 && ((uint8_t)test_command[0] < 0x21 || (uint8_t)test_command[0] > 0x7E))
            {
                test_command.remove(0, 1);
            }
            if (test_command.length() == 0)
            {
                continue; // a lone ENTER
            }
            Serial.println("> " + test_command);
            run_keys_reset(); // a new command: an abort (x) from the one before does not carry over (S1-C)

            // FW2 test suite (Tests_mech.cpp): help, actuator_home, lock_cycle, rotate_all, ...
            // Handles its own commands (including the extended multi_sample) and skips the rest.
            if (mech_tests_dispatch(test_command))
            {
                continue;
            }

            if (test_command == "abort")
            {
                Serial.println("Abort: empty container, rewind and empty deplyoment");
                abort_sample();
            }

            if (test_command == "temp" && !core::may_rotate(oring_lock_state()))
            {
                output.println(String("REFUSED | temp: O-ring lock state is ") + oring_lock_state_name() + " - the manifold must not rotate");
            }
            else if (test_command == "temp")
            { // quick debugging, change this function
                while (true)
                {
                    if (Serial.available() > 0)
                    {
                        float d1;
                        d1 = Serial.parseFloat();
                        if (d1 == 0.0)
                        {
                            Serial.println("zerrooooooooooo");
                            continue;
                        }

                        angle_offset_pos = d1;
                        Serial.println(d1);

                        // up is CCW, down is CW
                        manifold_motor.start(30, up);

                        while (!Serial.available())
                        {

                            delay(200);
                        }
                        manifold_motor.stop();

                        // go back to slot0 to check
                        rotateMotor(0);
                    }
                    delay(2000);
                }
            }

            if (test_command == "sample1m")
            {
                Serial.println("test Sampling at 1 meter, press START");
                button_start.waitPressedAndReleased();
                sample_process(1 * 80);
            }

            if (test_command == "demo")
            {
                Serial.println("Demo sampling");
                demo_sample_process();
            }

            if (test_command == "cal_DNA")
            {
                Serial.println("Cal DNA shield");
                calibrate_DNA_pump();
            }

            // all valves, both motors, pump, pressure sensors, push buttons(spool),
            // leds, control buttons + potentiometer, container
            if (test_command == "valves")
            {
                // 1, 2, 3, 4 with a second delay each
                Serial.println("testing valves");
                test_valves();
            }
            if (test_command == "motor_spool")
            {
                Serial.println("testing motor");
                test_motor_spool();
            }
            if (test_command == "motor_manifold")
            {
                Serial.println("testing motor manifold");
                // test straight if manifold is working? or motor and encoder seperate and then together
            }
            // turns
            if (test_command == "encoder_manifold")
            {
                Serial.println("testing encoder Manifold");
                test_encoder();
            }
            if (test_command == "enc_rot")
            {
                // Serial.println("");
                getRotationSPI(ENCODER_MANIFOLD);
            }
            if (test_command == "encoder_spool")
            {
                Serial.println("testing encoder SPOOL");
                test_encoder_spool();
            }
            if (test_command == "manifold")
            {
                Serial.println("testing manifold");
                // goes to all slots and to 0 in between, takes a lot of time
                test_manifold(); // all manifold, test first motor and encoder
            }
            if (test_command == "pressure")
            {
                print_pressure_once();
            }
            // 2026-10-23: was declared, documented and printed to by the code, but never dispatched (the operator tutorial uses it)
            if (test_command == "abp_raw")
            {
                print_abp_raw();
            }
            if (test_command == "pressure_zero")
            {
                pressure_zero();
            }
            if (test_command == "calibrate_pressure")
            {
                calibrate_pressure();
            }
            if (test_command == "pump")
            {
                Serial.println("testing pump");
                test_pump();
            }
            if (test_command == "micro_switch")
            {
                Serial.println("testing micro switch");
                test_micro_switch();
            }
            if (test_command == "buttons_command")
            {
                Serial.println("testing buttons");
                test_command_box();
            }
            if (test_command == "container")
            {
                Serial.println("testing container");
            }
            if (test_command == "spool")
            {
                Serial.println("testing spool");
                test_1_depth_20m();
            }
            if (test_command == "40m")
            {
                Serial.println("testing spool - 40m");
                test_1_depth_40m();
            }

            if (test_command == "stop")
            {
                Serial.println("stop testing mode");
                break;
            }

            if (test_command == "cal")
            {
                Serial.println("Calibrating encoder Manifold");
                calibrateEncoder(MANIFOLD_CAL_SPEED);
            }

            if (test_command == "zero")
            {
                // Serial.println("Calibrating encoder Manifold");
                go_to_zero(); // ! maybe need change
            }

            if (test_command == "slot0")
            {
                // Serial.println("Calibrating encoder Manifold");
                rotateMotor(0); // ! maybe need change
            }

            // slotN (e.g. "slot5"): rotate the manifold directly to slot N (0 = purge,
            // 1-14 = samples) without leaving the serial test menu, so slot-dependent
            // tests below (linear_actuator, sample_cycle, calibrate_actuator) can be
            // run at a chosen slot in the same session.
            if (test_command.startsWith("slot") && test_command != "slot0")
            {
                int requested_slot = test_command.substring(4).toInt();
                if (requested_slot <= 0 || requested_slot > 14)
                {
                    output.println("Usage: slot<N>, N = 1 to 14 (use plain 'slot0' for purge), e.g. slot5");
                }
                else
                {
                    output.println("Rotating to slot " + String(requested_slot));
                    rotateMotor(requested_slot);
                }
            }

            // clockMM (e.g. "clock15"): rotate to whichever slot sits at that clock-face
            // minute label (top = 00). clocklist: print every slot's label to look one up.
            if (test_command.startsWith("clock") && test_command != "clocklist")
            {
                // F38 (FW2): a bare `clock` or `clockxx` used to parse as minute 0 and rotate to slot 7
                String digits = test_command.substring(5);
                digits.trim();
                bool numeric = digits.length() > 0 && digits.length() <= 2;
                for (unsigned int k = 0; k < digits.length(); k++)
                {
                    numeric = numeric && isDigit(digits.charAt(k));
                }
                int requested_minutes = numeric ? digits.toInt() : -1;
                int slot = numeric ? slot_for_clock_minutes(requested_minutes) : -1;
                if (!numeric)
                {
                    output.println("Usage: clockMM, MM = the clock minute label of a slot (e.g. clock15) - send 'clocklist' to see all labels");
                }
                else if (slot < 0)
                {
                    output.println("No slot at clock " + String(requested_minutes) + " min - send 'clocklist' to see all labels");
                }
                else
                {
                    output.println("Rotating to clock " + String(requested_minutes) + " min (slot " + String(slot) + ")");
                    rotateMotor(slot);
                }
            }

            if (test_command == "clocklist")
            {
                for (int s = 0; s < NB_SLOT; s++)
                {
                    output.println("Slot " + String(s) + (s == PURGE_SLOT ? " (purge)" : "") + " = clock " + String(clock_minutes_for_slot(s)) + " min");
                }
            }

            if (test_command == "micro_pump")
            {
                Serial.println("Testing micro pumps");
                micro_pump.start();
                delay(10000);
                micro_pump.stop();
            }

            if (test_command == "fill_DNA")
            {
                Serial.println("Filling DNA shield tube");
                fill_DNA_shield_tube();
            }

            if (test_command == "linear_actuator")
            {
                test_linear_actuator();
            }

            if (test_command == "sample_cycle")
            {
                test_load_sample();
            }

            if (test_command == "sample_cycle_full")
            {
                test_load_sample_full();
            }

            if (test_command == "multi_sample_full")
            {
                test_multi_sample_loading_full();
            }

            if (test_command == "calibrate_actuator")
            {
                calibrate_linear_actuator();
            }

            if (test_command == "calibrate_flow")
            {
                calibrate_flow_sensor();
            }

            if (test_command == "calibrate_dna_volume")
            {
                calibrate_dna_pump_volume();
            }

            if (test_command == "shield_calibrate")
            {
                calibrate_shield_pump();
            }

            if (test_command == "calibrate_all")
            {
                run_commissioning_calibration();
            }

            if (test_command == "manifold_direction")
            {
                identify_manifold_direction();
            }
        }
    }
}

void test_1_depth_20m()
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - test skipped");
        return;
    }
    output.println("TEST 1 - ALTIMETRE 20");
    uint32_t lastTime = 0;

    spool.start_origin();
    status_led.on();
    output.println("=========== go 30cm");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(30);
    output.println(millis() - lastTime);

    status_led.on();
    output.println("=========== go 1m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(100);
    output.println(millis() - lastTime);

    status_led.on();
    output.println("=========== go 2m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(200);
    output.println(millis() - lastTime);

    int i = 1;
    while (i < 5)
    {
        status_led.on();
        output.println("=========== go " + String(i * 5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i * 5 * 100);
        output.println(millis() - lastTime);
        i++;
    }

    i = 4;
    while (i > 0)
    {
        status_led.on();
        output.println("=========== go " + String(i * 5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i * 5 * 100);
        output.println(millis() - lastTime);
        i--;
    }

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis() - lastTime);

    green_led.on();
    output.println("END OF TEST 1 20 meters");
    button_start.waitPressedAndReleased();
    green_led.off();
}

void test_1_depth_40m()
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - test skipped");
        return;
    }
    output.println("TEST 1 - ALTIMETRE 40 indirect");
    uint32_t lastTime = 0;

    spool.start_origin();
    status_led.on();
    output.println("=========== go 20m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(2000);
    output.println(millis() - lastTime);

    int i = 5;
    while (i < 10)
    {
        status_led.on();
        output.println("=========== go " + String(i * 5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i * 5 * 100);
        output.println(millis() - lastTime);
        i++;
    }

    i = 9;
    while (i > 3)
    {
        status_led.on();
        output.println("=========== go " + String(i * 5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i * 5 * 100);
        output.println(millis() - lastTime);
        i--;
    }

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis() - lastTime);

    green_led.on();
    output.println("END OF TEST 1 40 meters");
    button_start.waitPressedAndReleased();
    green_led.off();
}

/**
 * @brief Reads and prints both pressure sensors once (not a loop - see
 * test_pressure_sensor() for the continuous stream version).
 */
void print_pressure_once()
{
    float p = pressure1.getPressure();
    if (pressure1.isTrusted())
    {
        output.println("Pressure (ABP on SPI, pressure1: backs the pump cap and the PID loops): " + String(p, 3) + " bar (datasheet conversion " +
                       String(pressure1.getNominalPressure(), 3) + " bar, gain " + String(PRESSURE_GAIN, 4) + ", zero offset " +
                       String(PRESSURE_ZERO_OFFSET_BAR, 4) + " bar)");
    }
    else
    {
        output.println("Pressure (ABP on SPI, pressure1): NOT TRUSTED - the pump can't run. Run abp_raw.");
    }
}

char wait_for_keypress();            // defined below - forward declared for use here
bool wait_for_keypress_or_abort();   // defined below - forward declared for use here
bool ask_yes_no(const char *prompt); // defined below - forward declared for use here

// Result of averaging the ABP's datasheet ("nominal") reading for a while.
struct NominalAverage
{
    bool ok;      // false: the sensor was not trusted at some sample
    float mean;   // bar
    float spread; // bar, max - min
    int n;
};

static NominalAverage average_nominal_pressure(uint32_t duration_ms)
{
    NominalAverage a = {true, 0, 0, 0};
    double total = 0;
    float lowest = 0, highest = 0;
    uint32_t start = millis();
    while (millis() - start < duration_ms)
    {
        float reading = pressure1.getNominalPressure();
        if (!pressure1.isTrusted())
        {
            a.ok = false;
            return a;
        }
        if (a.n == 0 || reading < lowest)
            lowest = reading;
        if (a.n == 0 || reading > highest)
            highest = reading;
        total += reading;
        a.n++;
        delay(20);
    }
    a.mean = (a.n > 0) ? (float)(total / a.n) : 0;
    a.spread = highest - lowest;
    a.ok = a.n > 0;
    return a;
}

/**
 * @brief S1-1: zero of the ABP pressure sensor (pressure1). With the pump off and the line
 * open to air, averages the datasheet reading for PRESSURE_ZERO_AVG_MS and prints the
 * PRESSURE_ZERO_OFFSET_BAR line to paste into Settings.h. A quick re-tare that needs no
 * manometer and leaves PRESSURE_GAIN alone. Read-only: nothing moves.
 */
void pressure_zero()
{
    output.println("=== pressure_zero (ABP, pressure1) ===");
    if (pump.is_running())
    {
        output.println("REFUSED | pressure_zero: the pump is running");
        return;
    }
    output.println("Pump off, line open to air (no pressure on the sensor). Send any key to read, or 'x' to abort.");
    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted.");
        return;
    }

    test_begin("pressure_zero");
    NominalAverage a = average_nominal_pressure(PRESSURE_ZERO_AVG_MS);
    if (!a.ok)
    {
        output.println("The sensor is not trusted (run abp_raw): no zero.");
        test_result("zero", core::TEST_FAIL, "reason=sensor_not_trusted");
        test_end();
        return;
    }
    bool ok = fabs(a.mean) <= PRESSURE_ZERO_MAX_ABS_BAR && a.spread <= CAL_PRESSURE_MAX_SPREAD_BAR;
    output.println("Mean over " + String(a.n) + " readings: " + String(a.mean, 4) + " bar, spread (max-min) " + String(a.spread, 4) + " bar");
    if (fabs(a.mean) > PRESSURE_ZERO_MAX_ABS_BAR)
    {
        output.println("That is too far from 0 to be an offset: pressure on the sensor, or the wrong ABP_FULL_SCALE_PSI. Nothing to paste.");
    }
    else if (a.spread > CAL_PRESSURE_MAX_SPREAD_BAR)
    {
        output.println("The reading was not steady: check the wiring and that nothing is moving in the line, then redo it.");
    }
    else
    {
        output.println("Edit this in Settings.h, then rebuild and reflash:");
        output.println("  const float PRESSURE_ZERO_OFFSET_BAR = " + String(a.mean, 4) + "f;");
    }
    test_result("zero", ok ? core::TEST_PASS : core::TEST_FAIL,
                "zero_offset_bar=" + String(a.mean, 4) + ";spread_bar=" + String(a.spread, 4) + ";n=" + String(a.n) +
                    ";was_bar=" + String(PRESSURE_ZERO_OFFSET_BAR, 4));
    test_end();
}

// Throws away what is waiting on the console. A key typed as "k" + ENTER leaves its line
// feed behind (at 9600 baud it arrives after the key was read), and the next prompt would
// take it for an answer: that made calibrate_pressure skip the hold of its first point.
static void drain_serial_input()
{
    uint32_t quiet_since = millis();
    while (millis() - quiet_since < 100) // until nothing has arrived for 100 ms
    {
        if (Serial.available())
        {
            Serial.read();
            quiet_since = millis();
        }
    }
}

// The one way calibrate_pressure waits for the operator. Everything typed BEFORE the call is
// thrown away, then it blocks until a whole new line arrives (ENTER, as CR, LF or both) and
// throws away what follows it. One ENTER = one go: no byte can be left over for the next
// question. Returns false if the line was "x" (abort).
static bool wait_for_enter()
{
    drain_serial_input();
    String line = "";
    while (true)
    {
        if (!Serial.available())
        {
            delay(5);
            continue;
        }
        char c = Serial.read();
        if (c == '\r' || c == '\n')
        {
            break;
        }
        line += c;
    }
    drain_serial_input();
    line.trim();
    return !(line == "x" || line == "X");
}

/**
 * @brief S1-1: calibration of the ABP pressure sensor (pressure1) against the manometer.
 * Sensor and manometer are taken off the pump line and joined to a syringe of air on a
 * tee: both see the same static pressure, and the pump is never started.
 * Flow asked by the user (2026-10-09), the same for every point (0 bar, then
 * CAL_PRESSURE_TARGETS_BAR): ASK whether the pressure is applied, WAIT for ENTER, only then
 * MEASURE for CAL_PRESSURE_AVG_MS. Nothing is measured before the ENTER of that point
 * (wait_for_enter()). The manometer value of a point is its target. Fits
 * reference = gain x nominal + offset (core::fit_pressure_cal) and prints the PRESSURE_GAIN /
 * PRESSURE_ZERO_OFFSET_BAR lines and TEST| lines. It works on the datasheet reading, so the
 * values already in Settings.h don't matter and it can be rerun.
 */
void calibrate_pressure()
{
    const int n_targets = sizeof(CAL_PRESSURE_TARGETS_BAR) / sizeof(CAL_PRESSURE_TARGETS_BAR[0]);
    const int MAX_POINTS = n_targets + 1;
    core::PressureCalPoint points[MAX_POINTS];
    float spreads[MAX_POINTS];
    int n = 0;

    output.println("=== calibrate_pressure (ABP against the manometer) ===");
    if (pump.is_running())
    {
        output.println("REFUSED | calibrate_pressure: the pump is running");
        return;
    }
    output.println("Sensor + manometer + syringe of air on a tee, off the pump line. The pump is not used.");
    output.println("Each step asks first, waits for ENTER, then measures. x + ENTER aborts.");

    for (int i = 0; i <= n_targets; i++)
    {
        const float target = (i == 0) ? 0.0f : CAL_PRESSURE_TARGETS_BAR[i - 1];
        output.println("");
        if (i == 0)
        {
            output.println("Step 1 of " + String(n_targets + 1) + ": 0 bar. Is everything open to air? Press ENTER when it is.");
        }
        else
        {
            output.println("Step " + String(i + 1) + " of " + String(n_targets + 1) + ": " + String(target, 1) +
                           " bar. Push until the manometer shows " + String(target, 1) + " and hold. Press ENTER when it is there.");
        }
        if (!wait_for_enter())
        {
            output.println("Aborted.");
            return;
        }
        output.println("  measuring " + String(target, 1) + " bar for " + String(CAL_PRESSURE_AVG_MS / 1000) + " s - hold...");
        NominalAverage a = average_nominal_pressure(CAL_PRESSURE_AVG_MS);
        if (!a.ok)
        {
            output.println("The sensor is not trusted (run abp_raw). Aborted.");
            return;
        }
        output.println("  done: sensor " + String(a.mean, 3) + " bar (moved by " + String(a.spread, 3) + " bar)." + (i == 0 ? "" : " You can release."));
        points[n].reference_bar = target;
        points[n].nominal_bar = a.mean;
        spreads[n] = a.spread;
        n++;
    }
    drain_serial_input();

    // --- fit and report ---
    core::PressureCalFit fit = core::fit_pressure_cal(points, n);
    core::PressureCalVerdict verdict = core::pressure_cal_verdict(fit, CAL_PRESSURE_MIN_POINTS, CAL_PRESSURE_GAIN_MIN,
                                                                  CAL_PRESSURE_GAIN_MAX, CAL_PRESSURE_MAX_RESIDUAL_BAR);
    test_begin("calibrate_pressure");
    output.println("--- calibrate_pressure results ---");
    for (int i = 0; i < n; i++)
    {
        float residual = core::pressure_cal_residual(fit, points[i]);
        float before = core::abp_apply_calibration(points[i].nominal_bar, PRESSURE_GAIN, PRESSURE_ZERO_OFFSET_BAR);
        bool ok = fit.valid && fabs(residual) <= CAL_PRESSURE_MAX_RESIDUAL_BAR;
        test_result(("p" + String(i)).c_str(), ok ? core::TEST_PASS : core::TEST_FAIL,
                    "manometer_bar=" + String(points[i].reference_bar, 3) + ";abp_datasheet_bar=" + String(points[i].nominal_bar, 4) +
                        ";abp_before_bar=" + String(before, 4) + ";abp_after_bar=" + String(points[i].reference_bar - residual, 4) +
                        ";residual_bar=" + String(residual, 4) + ";spread_bar=" + String(spreads[i], 4));
    }
    test_result("fit", verdict == core::PRESSURE_CAL_PASS ? core::TEST_PASS : core::TEST_FAIL,
                String("verdict=") + core::pressure_cal_verdict_name(verdict) + ";n=" + String(n) + ";gain=" + String(fit.gain, 4) +
                    ";zero_offset_bar=" + String(fit.zero_offset_bar, 4) + ";r2=" + String(fit.r2, 5) +
                    ";max_residual_bar=" + String(fit.max_residual_bar, 4) + ";limit_bar=" + String(CAL_PRESSURE_MAX_RESIDUAL_BAR, 3));
    test_end();

    if (verdict == core::PRESSURE_CAL_PASS)
    {
        output.println("Edit these in Settings.h, then rebuild and reflash:");
        output.println("  const float PRESSURE_GAIN = " + String(fit.gain, 4) + "f;");
        output.println("  const float PRESSURE_ZERO_OFFSET_BAR = " + String(fit.zero_offset_bar, 4) + "f;");
    }
    else if (verdict == core::PRESSURE_CAL_FAIL_GAIN)
    {
        output.println("Gain outside " + String(CAL_PRESSURE_GAIN_MIN, 1) + "-" + String(CAL_PRESSURE_GAIN_MAX, 1) +
                       ": this is not a calibration error. Check the part number against ABP_FULL_SCALE_PSI, the wiring, and that sensor and manometer share the same volume. Nothing to paste.");
    }
    else if (verdict == core::PRESSURE_CAL_FAIL_POINTS)
    {
        output.println("Too few points (" + String(n) + ", need " + String(CAL_PRESSURE_MIN_POINTS) + "). Nothing to paste.");
    }
    else
    {
        output.println("A point is too far from the line (see the residuals above): the needle was not on the mark during the 3 s, or a leak. Run calibrate_pressure again. Nothing to paste.");
    }
}

/**
 * @brief Diagnostics for the SPI pressure sensor (pressure1, CS D8): prints 20 raw frames,
 * 200 ms apart, unchecked. Tells the wiring faults apart: FF FF FF = nobody drives MISO
 * (no supply, wrong wire, dead part), 00 00 00 = MISO held low, status 11 = the sensor's
 * own diagnostic fault, status 10 = stale data (alive; polled faster than it updates, F27),
 * status 00 = a valid reading. Read-only: nothing moves.
 */
void print_abp_raw()
{
    const char *status_names[] = {"00 valid", "01 command mode", "10 stale", "11 diagnostic fault"};
    output.println("abp_raw: 20 raw frames from the SPI pressure sensor (CS D8), 200 ms apart");
    uint16_t lowest = 0, highest = 0;
    for (int i = 0; i < 20; i++)
    {
        byte f[3];
        pressure1.read_raw(f);
        char hex[12];
        snprintf(hex, sizeof hex, "%02X %02X %02X", f[0], f[1], f[2]);
        uint16_t counts = core::abp_frame_counts(f[0], f[1]);
        if (i == 0 || counts < lowest)
            lowest = counts;
        if (i == 0 || counts > highest)
            highest = counts;
        output.println(String("  ") + hex + "  status " + status_names[f[0] >> 6] + "  counts " + String(counts) +
                       "  (" + String(core::abp_counts_to_bar(counts, ABP_FULL_SCALE_PSI), 3) + " bar, datasheet conversion for " +
                       String(ABP_FULL_SCALE_PSI, 0) + " psi)  temp " + String((f[2] << 3) * 9.77e-2 - 50, 0) + " C");
        delay(200);
    }
    // F124: a frame can carry status 00 and still be garbage. A healthy sensor at a constant
    // pressure repeats nearly the same counts and the same room temperature on every line.
    output.println("counts min " + String(lowest) + ", max " + String(highest) + ", spread " + String(highest - lowest) + " (" +
                   String(core::abp_counts_to_bar(highest, ABP_FULL_SCALE_PSI) - core::abp_counts_to_bar(lowest, ABP_FULL_SCALE_PSI), 3) +
                   " bar). At a constant pressure, expect a few counts and the same temperature on every line.");
    output.println("FF FF FF = no answer (supply / MISO wire / dead); 00 00 00 = MISO stuck low; counts ~1638 = 0 bar on a 10-90 % part");
}

void test_manifold()
{
    uint16_t pos;
    float angle_deg;

    for (int j = 1; j < 15; j++)
    {
        if (!rotateMotor(0))
        {
            return; // FW2: refused (lock state) or failed - stop instead of printing 14 bad lines
        }
        delay(1000);
        if (!rotateMotor(j))
        {
            return;
        }

        pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        angle_deg = pos * encoder_to_deg;
        Serial.print("Manifold encoder value : ");
        Serial.print(pos);
        Serial.print(",    Angle in degrees : ");
        Serial.print(angle_deg);
        Serial.print(",    Slot number : ");
        Serial.println(j);

        delay(3000);
    }
}

/// @brief How often test_pump() (and any other manual pump test that wants it)
/// prints a live pressure reading while running - kept loose so the terminal
/// doesn't get flooded during a long manual run.
const uint32_t PRESSURE_PRINT_INTERVAL_MS = 3000;

/**
 * @brief Manual pump test: asks for a power percentage, then runs the pump at
 * that power until any key is pressed, printing the live pressure reading at most
 * once every PRESSURE_PRINT_INTERVAL_MS.
 */
void test_pump()
{
    if (!require_pressure_sensor("pump"))
    {
        return;
    }
    output.println("Pump power percentage (1-100)?");
    long power;
    while (true)
    {
        while (!Serial.available()) // wait for real input first, so parseInt() doesn't time out and re-print the usage line every second while idly waiting
        {
            delay(10);
        }
        power = Serial.parseInt();
        while (Serial.available())
        {
            Serial.read(); // flush the trailing newline
        }
        if (power >= 1 && power <= 100)
        {
            break;
        }
        output.println("Usage: enter a number from 1 to 100");
    }

    output.println("Running pump at " + String(power) + "% - send any key to stop");

    // open manifold valve to avoid building up pressure
    valve_manifold.set_open_way();

    pump.set_power(power);
    pump.start();

    uint32_t last_print = 0;
    while (!Serial.available())
    {
        pump.enforce_pressure_safety(); // pump is running for the whole loop here, so no need to gate this behind is_running() like wait_for_keypress() does
        if (millis() - last_print > PRESSURE_PRINT_INTERVAL_MS)
        {
            output.println("  pressure (ABP, backs the safety cap): " + String(pressure1.getPressure()) + " bar");
            last_print = millis();
        }
        delay(20);
    }
    while (Serial.available())
    {
        Serial.read();
    }

    pump.stop();
    valve_manifold.set_close_way();
    output.println("Pump stopped");
}

/**
 * @brief Moves the linear actuator in small chunks, checking serial between each
 * chunk so a keypress can abort mid-move. Returns true if aborted early. Ignores
 * a lone trailing '\r'/'\n' left over from the command that started the jog.
 */
/**
 * @brief Checks Serial for a real keypress, silently discarding a lone leftover
 * '\r'/'\n' (left over from the command that started whichever jog is calling
 * this) without treating it as one. Used by every jog helper below so the abort/
 * checkpoint check is written once.
 * @return true if a real key was found (buffer is then fully flushed).
 */
char real_keypress_waiting()
{
    if (!Serial.available())
    {
        return 0;
    }
    char c = Serial.peek();
    if (c == '\r' || c == '\n')
    {
        Serial.read();
        return 0;
    }
    Serial.read(); // consume the triggering character (caller can still see it via the return value)
    while (Serial.available())
    {
        Serial.read(); // flush anything queued behind it
    }
    return c;
}

/**
 * @brief Blocks until a key is pressed, flushes any remaining buffered bytes, and
 * returns the character read. Used throughout the calibration/test tools for both
 * simple gate prompts ("press any key to continue" - callers ignore the return
 * value) and single-character decisions (e.g. y/n - callers use it).
 */
char wait_for_keypress()
{
    while (!Serial.available())
    {
        if (pump.is_running())
        {
            pump.enforce_pressure_safety(); // hard cap even while a test is just sitting here waiting for a key - skipped (no sensor read at all) when the pump is off, e.g. during calibrate_linear_actuator()
        }
        delay(10);
    }
    char c = Serial.read();
    while (Serial.available())
    {
        Serial.read();
    }
    return c;
}

/**
 * @brief Like wait_for_keypress(), but treats 'x'/'X' as an abort request instead
 * of a normal continue. Use at any gate that's about to trigger real motion (pump,
 * motor) so there's always a way to back out instead of being forced to proceed.
 * @return true if the key was 'x'/'X' (abort).
 */
bool wait_for_keypress_or_abort()
{
    char c = wait_for_keypress();
    return (c == 'x' || c == 'X');
}

/**
 * @brief Waits for input, then checks (without consuming) whether it's an abort
 * request ('x'/'X'). If so, consumes and flushes it and returns true. Otherwise
 * leaves the buffer untouched for the caller's own Serial.parseInt()/parseFloat()
 * to read normally - use this right before a numeric-entry prompt so 'x' can
 * still abort even though the rest of that prompt expects a number.
 */
bool numeric_prompt_aborted()
{
    while (!Serial.available())
    {
        delay(10);
    }
    if (Serial.peek() == 'x' || Serial.peek() == 'X')
    {
        Serial.read();
        while (Serial.available())
        {
            Serial.read();
        }
        return true;
    }
    return false;
}

/**
 * @brief Moves the actuator in 10-step chunks until total_steps is reached or a
 * real keypress arrives (a lone leftover '\r'/'\n' is ignored).
 * @param abort_char if non-null and the move is aborted, set to the key that
 * triggered it - the character is otherwise consumed and lost, so callers that
 * want to react to it (e.g. treat it as the next command) need this.
 */
bool jog_actuator_steps(long total_steps, actuator_direction dir, char *abort_char = nullptr)
{
    const long CHUNK_STEPS = 10;
    long steps_done = 0;

    linear_actuator.set_direction(dir); // set once for the whole move, not re-asserted every chunk

    while (steps_done < total_steps)
    {
        char key = real_keypress_waiting();
        if (key)
        {
            if (abort_char)
            {
                *abort_char = key;
            }
            return true; // aborted
        }

        long steps_this_chunk = min(CHUNK_STEPS, total_steps - steps_done);
        linear_actuator.step_pulses(steps_this_chunk);
        steps_done += steps_this_chunk;
    }

    return false;
}

/**
 * @brief Manual jog test for the linear actuator (NEMA17 42SHD034-20B + A4988).
 * f = jog forward, b = jog backward - each moves at most one full revolution,
 * slowly, and can be aborted mid-move by sending any key. e = enable driver,
 * s = disable driver (cuts holding torque), p = print step position,
 * v<n> = set speed to n steps/sec (e.g. "v100"), x = exit.
 */
void test_linear_actuator()
{
    output.println("Testing linear actuator - f: forward, b: backward (max 1 turn), e: enable, s: disable, p: position, v<n>: set speed (steps/sec), x: exit");
    output.println("Starts DISABLED - send 'e' when ready.");

    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);

    while (true)
    {
        if (!Serial.available())
        {
            continue;
        }

        char command = Serial.read();

        if (command == 'v')
        {
            long new_speed = Serial.parseInt(); // reads the digits typed right after 'v', e.g. "v100"
            while (Serial.available())
            {
                Serial.read();
            }
            if (new_speed > 0)
            {
                linear_actuator.set_speed(new_speed);
                output.println("Speed set to " + String(new_speed) + " steps/sec");
            }
            else
            {
                output.println("Usage: v<steps_per_sec>, e.g. v100");
            }
            continue;
        }

        while (Serial.available())
        {
            Serial.read(); // flush rest of the line
        }

        if (command == 'x')
        {
            break;
        }
        else if (command == 'e')
        {
            linear_actuator.enable();
            output.println("Actuator enabled");
        }
        else if (command == 's')
        {
            linear_actuator.disable();
            output.println("Actuator disabled");
        }
        else if (command == 'p')
        {
            output.println("Position (steps): " + String(linear_actuator.get_position_steps()));
        }
        else if (command == 'f' || command == 'b')
        {
            actuator_direction dir = (command == 'f') ? forward : backward;
            output.println(command == 'f' ? "Jogging forward, 1 turn max, send any key to abort" : "Jogging backward, 1 turn max, send any key to abort");

            char abort_char = 0;
            oring_mark_unknown("linear_actuator jog"); // FW2: a manual move the lock code did not make
            bool aborted = jog_actuator_steps(LINEAR_ACT_STEPS_PER_REV, dir, &abort_char);
            if (aborted)
            {
                output.println("Jog aborted by '" + String(abort_char) + "' - that key was consumed as the abort, send it again to run it as a command.");
            }
            else
            {
                output.println("Jog complete (1 turn)");
            }
        }
    }

    linear_actuator.disable();
    output.println("Linear actuator test ended");
}

/**
 * @brief Manual single-slot test of load_slot() (Step_functions.cpp): lock, pump
 * water (stopping on TEST_SAMPLE_VOLUME_ML or the pump's hard pressure cap - see
 * Pump::set_pressure_safety()), pump air to empty the filter, unlock. This is the
 * exact same function operation code will use - no duplicated logic here.
 * Precondition: the manifold must already be rotated to the target slot (use a
 * `slotN`/`clockMM` command first) and be unlocked.
 */
void test_load_sample()
{
    output.println("=== Sample-load test (single slot) ===");
    output.println("Precondition: manifold must already be aligned to the target sample slot.");
    output.println("Target volume: " + String(TEST_SAMPLE_VOLUME_ML) + " mL. Send any key to start, or 'x' to abort now.");

    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted before start");
        return;
    }

    bool ok = load_slot(TEST_SAMPLE_VOLUME_ML);

    output.println(ok ? "Sample-load test complete." : "Sample-load test FAILED - lock_oring() did not confirm lock, check manually.");
}

/**
 * @brief Manual single-slot test of load_slot_full() (Step_functions.cpp): the
 * FULL operation sequence (step_sampling() + step_DNA_shield()) - real
 * STX_SAMPLE_MILLILITERS volume, the elaborate purge-then-filter double
 * air-emptying, and the DNA-shield preservative push, exactly as sample_process()
 * runs it per sample. Precondition: the manifold must already be rotated to the
 * target slot (use a `slotN`/`clockMM` command first) and be unlocked.
 */
void test_load_sample_full()
{
    output.println("=== Sample-load test (FULL operation sequence) ===");
    output.println("Precondition: manifold must already be aligned to the target sample slot.");
    output.println("Target volume: " + String(STX_SAMPLE_MILLILITERS) + " mL. Enter the slot number currently aligned (1-14), or 'x' to abort:");

    if (numeric_prompt_aborted())
    {
        output.println("Aborted.");
        return;
    }
    int slot = Serial.parseInt();
    while (Serial.available())
    {
        Serial.read();
    }
    if (slot < 1 || slot > 14)
    {
        output.println("Invalid slot - aborted");
        return;
    }

    load_slot_full(slot);
    output.println("Full sample-load test complete.");
}

/**
 * @brief Multi-sample test using the FULL operation sequence (load_slot_full()) -
 * real STX_SAMPLE_MILLILITERS volume per slot, unlike the old reduced-volume multi-sample test (removed 2026-10-23)
 * reduced-volume dry run. Rotates through every available slot in FILL_ORDER,
 * running step_sampling()+step_DNA_shield() at each. Pauses for operator
 * confirmation between slots; send any key to continue, 'x' to abort the whole run.
 */
void test_multi_sample_loading_full()
{
    output.println("=== Multi-sample loading test (FULL operation sequence, " + String(STX_SAMPLE_MILLILITERS) + " mL/slot) ===");
    output.println("Visits every available slot in FILL_ORDER. Send any key to start, or 'x' to abort now.");

    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted before start");
        return;
    }

    for (uint8_t i = 0; i < 14; i++)
    {
        int slot = FILL_ORDER[i];
        if (manifold.get_state(slot) != available)
        {
            continue;
        }

        output.println("--- Slot " + String(slot) + " (clock " + String(clock_minutes_for_slot(slot)) + " min), " + String(i + 1) + "/14 in FILL_ORDER ---");
        unlock_oring(); // no-op if already unlocked - guards the first iteration too, in case a prior run left it locked
        rotateMotor(slot);

        load_slot_full(slot);

        output.println("Inspect joints for leaks. Send any key to continue to the next slot, or 'x' to abort.");
        if (wait_for_keypress_or_abort())
        {
            output.println("Aborted by operator.");
            return;
        }
    }

    output.println("Multi-sample loading test (full) complete - all available slots visited.");
}

/**
 * @brief Jogs the actuator continuously in the given direction until a real
 * keypress (a lone leftover '\r'/'\n' is ignored, same as elsewhere) or max_steps
 * is hit. Used by calibrate_linear_actuator() to find the STALL point: the
 * operator watches/listens and presses a key the instant the motor appears to stop
 * turning (torque maxed against the o-ring), then confirms that impression before
 * it's accepted.
 *
 * Open-loop stepper safety note: there's no encoder, so software can't detect a
 * stall directly - only the operator watching the actuator can. So after every
 * keypress, this asks the operator to confirm the motor had ACTUALLY stopped
 * turning at that point; if they say no (it was still turning, they pressed too
 * early), the SAME stage is retried by continuing to jog from wherever it
 * physically is now, rather than accepting a premature measurement.
 *
 * @param steps_out set to the total number of steps travelled in this call,
 * "not stalled yet" continuations included.
 * @param abort_requested if non-null, set to true if the operator explicitly
 * aborted at the confirmation prompt (distinct from "not stalled yet, keep going") -
 * callers must check this and stop, not just retry.
 * @return true if a confirmed stall was received, false if max_steps was hit or
 * the operator aborted (steps_out set either way).
 */
bool jog_until_keypress(actuator_direction dir, long max_steps, long *steps_out, bool *abort_requested = nullptr)
{
    if (abort_requested)
    {
        *abort_requested = false;
    }

    long steps = 0; // total over the whole call: a "not stalled yet" answer continues the count (and max_steps bounds the total)

    while (true)
    {
        linear_actuator.set_direction(dir);
        bool got_keypress = false;

        while (steps < max_steps)
        {
            if (real_keypress_waiting())
            {
                got_keypress = true;
                break;
            }

            linear_actuator.step_pulses(10);
            steps += 10;
        }

        if (!got_keypress)
        {
            *steps_out = steps;
            return false; // safety cap hit
        }

        output.println("Has the motor now fully stalled (stopped turning, even though it's still being commanded)? y = yes, stalled - use this measurement. n = no, it was still turning - keep going. x = abort calibration.");
        char confirm = wait_for_keypress();

        if (confirm == 'y' || confirm == 'Y')
        {
            *steps_out = steps;
            return true;
        }

        if (confirm == 'x' || confirm == 'X')
        {
            if (abort_requested)
            {
                *abort_requested = true;
            }
            *steps_out = steps;
            return false;
        }

        output.println("Not stalled yet - continuing to jog from the current position.");
        // loop back and keep jogging - position isn't reset, so it continues from wherever it physically is now
    }
}

/**
 * @brief Two-position calibration for the o-ring lock mechanism: UNLOCKED (the
 * reference, becomes position 0) and LOCKED (steps backward from there, to the
 * point the motor stalls against the o-ring). The actuator does all the moving -
 * the operator only watches and presses a key at each stage.
 *
 * Stage 2 deliberately drives the motor into stall (torque maxed) rather than
 * stopping at a subjective "looks compressed enough" point - see the anti-drift
 * comment block in Settings.h/Oring_lock.cpp: operation lock_oring() re-uses this
 * same stall point every cycle (with a further overdrive margin), so the o-ring
 * always compresses against the same real physical stop no matter how many
 * lock/unlock cycles have run, instead of open-loop step drift accumulating.
 */
void calibrate_linear_actuator()
{
    output.println("=== Linear actuator calibration (unlocked/locked) ===");
    output.println("Make sure the manifold is aligned to a sample slot.");

    linear_actuator.enable();
    linear_actuator.set_speed(LINEAR_ACT_CAL_STEPS_PER_SEC); // deliberately slower than normal operation - see Settings.h
    oring_mark_unknown("calibrate_actuator");                // FW2: it ends at the stall (physically locked) or wherever it was aborted

    // --- Stage 1: find and mark the UNLOCKED reference position ---
    output.println("Jog the actuator to the UNLOCKED (fully retracted / o-ring released) reference position:");
    output.println("f = jog forward = RELEASE, b = jog backward = towards LOCK (send any key to stop the jog), c = mark this as UNLOCKED, x = abort");
    output.println("Press c while the O-ring is RELEASED, not at the stall: the stall is found in the next stage.");
    while (true)
    {
        char adjust_cmd = wait_for_keypress();
        if (adjust_cmd == 'x' || adjust_cmd == 'X')
        {
            output.println("Aborted before starting.");
            linear_actuator.disable();
            return;
        }
        if (adjust_cmd == 'c' || adjust_cmd == 'C')
        {
            break;
        }
        if (adjust_cmd == 'f' || adjust_cmd == 'b')
        {
            actuator_direction dir = (adjust_cmd == 'f') ? forward : backward;
            output.println(adjust_cmd == 'f' ? "Jogging forward - send any key to stop" : "Jogging backward - send any key to stop");
            jog_actuator_steps(LINEAR_ACT_CAL_SAFETY_CAP_STEPS, dir);
        }
        else
        {
            output.println("f: forward (release), b: backward (towards lock), c: mark as UNLOCKED, x: abort");
        }
    }
    linear_actuator.reset_position(); // this is now position 0, the "unlocked" reference
    output.println("UNLOCKED reference marked.");

    // --- Stage 2: jog backward to STALL (torque maxed against the o-ring) ---
    output.println("Jogging backward - the actuator will keep driving past the point the o-ring is fully compressed.");
    output.println("Watch/listen for the motor to STALL (stop turning despite still being commanded) and send a key the instant you believe it has.");
    long locked_steps;
    bool aborted = false;
    if (!jog_until_keypress(backward, LINEAR_ACT_CAL_SAFETY_CAP_STEPS, &locked_steps, &aborted))
    {
        output.println(aborted ? "Aborted by operator." : "Safety bound reached without a confirmed stall - calibration aborted, try again");
        linear_actuator.disable();
        return;
    }
    output.println("Stall point measured: " + String(locked_steps) + " steps (from unlocked).");
    output.println("The lock state stays UNKNOWN: the firmware still uses the old step count. After pasting the value and reflashing, run actuator_home.");

    linear_actuator.disable();

    // --- Summary ---
    output.println("=== Calibration summary ===");
    output.println("Edit this in Settings.h, then rebuild and reflash:");
    output.println("  const long LINEAR_ACT_LOCKED_STEPS = " + String(locked_steps) + ";");
    output.println("Operation locking (lock_oring()) will automatically over-drive " + String((LINEAR_ACT_LOCK_OVERDRIVE_FACTOR - 1.0) * 100, 0) + "% past this every cycle (LINEAR_ACT_LOCK_OVERDRIVE_FACTOR, Settings.h) - no need to add margin here yourself.");
}

/// @brief Prints `prompt`, waits for one keypress, and returns true for y/Y.
bool ask_yes_no(const char *prompt)
{
    output.println(String(prompt) + " (y/n)");
    char c = wait_for_keypress();
    return (c == 'y' || c == 'Y');
}

/**
 * @brief F130: asks whether the spool module is on the water path (default: yes), then prints the dead volume
 * and the purge that goes with that answer (PURGE_SPOOL_ML or PURGE_NO_SPOOL_ML, user 2026-10-23).
 * Any key except n/N counts as yes (Enter included). Prints only; it does not change the purge.
 */
void ask_spool_and_purge_check()
{
    output.println("Is the spool module connected to the water path? (y/n, Enter = y, default yes)");
    char c = wait_for_keypress();
    bool connected = !(c == 'n' || c == 'N');
    float dead_ml = INTAKE_PATH_ML + (connected ? SPOOL_PATH_ML : 0.0f);
    float purge_ml = core::purge_volume_ml(connected, PURGE_NO_SPOOL_ML, PURGE_SPOOL_ML);
    output.println(String("Spool module: ") + (connected ? "connected" : "NOT connected") +
                   "; dead volume about " + String(dead_ml, 0) + " mL; purge " + String(purge_ml, 0) + " mL");
}

/**
 * @brief Guided flow-sensor (flow_sensor_small) calibration. Pumps a target volume
 * (you choose, or a 2000 mL default) - stopping on sensor-reported target, manual
 * key, or a safety timeout - then asks for the ACTUAL measured volume and computes
 * a corrected calibrationFactor_small from the ratio between them.
 */
void calibrate_flow_sensor()
{
    if (!require_pressure_sensor("calibrate_flow"))
    {
        return;
    }
    output.println("=== Flow sensor calibration ===");
    output.println("Enter a target volume to pump in mL (e.g. 2000), 0 for the default 2000 mL, or 'x' to abort:");
    if (numeric_prompt_aborted())
    {
        output.println("Aborted.");
        return;
    }
    long target_ml = Serial.parseInt();
    while (Serial.available())
    {
        Serial.read();
    }
    if (target_ml <= 0)
    {
        target_ml = 2000;
    }
    output.println("Target: " + String(target_ml) + " mL");

    output.println("Set up the pump to fill a container you can measure precisely. Send any key when ready to start pumping, or 'x' to abort.");
    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted before starting.");
        return;
    }

    flow_sensor_small.reset_values();
    flow_sensor_small.activate();
    pump.set_power(POWER_PUMP);
    pump.start();

    const uint32_t CAL_PUMP_MAX_RUNTIME_MS = 5UL * 60 * 1000; // safety timeout
    uint32_t start_time = millis();
    float volume_ml = 0;

    output.println("Pumping - send any key to stop early if needed.");
    while (true)
    {
        flow_sensor_small.update();
        volume_ml = flow_sensor_small.get_totalFlowMilliL();

        if (volume_ml >= target_ml)
        {
            output.println("Target volume reached (sensor reading)");
            break;
        }
        if (pump.enforce_pressure_safety()) // hard cap, checked on every pass (2026-10-23 safety check)
        {
            break;
        }
        if (millis() - start_time > CAL_PUMP_MAX_RUNTIME_MS)
        {
            output.println("Safety timeout reached, stopping pump");
            break;
        }
        if (Serial.available())
        {
            while (Serial.available())
            {
                Serial.read();
            }
            output.println("Manual stop received");
            break;
        }
        delay(50);
    }

    pump.stop();
    output.println("Sensor-reported volume: " + String(volume_ml) + " mL");
    output.println("Measure the ACTUAL volume collected, then enter it in mL, or 'x' to abort:");

    if (numeric_prompt_aborted())
    {
        output.println("Aborted.");
        return;
    }
    float actual_ml = Serial.parseFloat();
    while (Serial.available())
    {
        Serial.read();
    }

    if (actual_ml <= 0)
    {
        output.println("Invalid volume entered - calibration aborted");
        return;
    }

    float new_cal_factor = calibrationFactor_small * (volume_ml / actual_ml);

    output.println("Sensor reported " + String(volume_ml) + " mL, actual was " + String(actual_ml) + " mL.");
    output.println("Edit this in Flow_sensor.h, then rebuild and reflash:");
    output.println("  const float calibrationFactor_small = " + String(new_cal_factor, 3) + ";");
}

/**
 * @brief Optional, small-scale volume check for the DNA-shield micro pump. There's
 * no flow sensor on this line, so instead of a calibration factor this measures a
 * flow rate (mL/s) from a timed test run + your measured dispensed volume, then
 * computes the run time needed for a target volume you specify - to sanity-check
 * or update the existing time-based constants (PUMP_SHIELD_TIME, FILL_STERIVEX_TIME
 * in Settings.h) rather than replace them automatically.
 */
void calibrate_dna_pump_volume()
{
    output.println("=== DNA-shield pump volume check (optional) ===");
    output.println("Enter a test run time in ms (e.g. 3000), 0 for the default 3000 ms, or 'x' to abort:");
    if (numeric_prompt_aborted())
    {
        output.println("Aborted.");
        return;
    }
    long test_time_ms = Serial.parseInt();
    while (Serial.available())
    {
        Serial.read();
    }
    if (test_time_ms <= 0)
    {
        test_time_ms = 3000;
    }

    output.println("Position a measuring container under the DNA-shield output, then send any key to run the pump for " + String(test_time_ms) + " ms, or 'x' to abort.");
    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted before starting.");
        return;
    }

    micro_pump.start();
    delay(test_time_ms);
    micro_pump.stop();

    output.println("Measure the dispensed volume (mL), then enter it, or 'x' to abort:");
    if (numeric_prompt_aborted())
    {
        output.println("Aborted.");
        return;
    }
    float measured_ml = Serial.parseFloat();
    while (Serial.available())
    {
        Serial.read();
    }

    if (measured_ml <= 0)
    {
        output.println("Invalid volume entered - check aborted");
        return;
    }

    float ml_per_ms = measured_ml / test_time_ms;
    output.println("Measured rate: " + String(ml_per_ms * 1000.0, 4) + " mL/s");

    output.println("Enter a target volume in mL to compute the run time for, or 0 to skip:");
    while (!Serial.available())
    {
        delay(10);
    }
    float target_ml = Serial.parseFloat();
    while (Serial.available())
    {
        Serial.read();
    }

    if (target_ml > 0)
    {
        uint32_t needed_ms = (uint32_t)(target_ml / ml_per_ms);
        output.println("For " + String(target_ml) + " mL, run the micro pump for approximately " + String(needed_ms) + " ms.");
        output.println("Compare against PUMP_SHIELD_TIME / FILL_STERIVEX_TIME in Settings.h and update if this differs meaningfully.");
    }
}

/**
 * @brief Runs the micro pump for up to max_ms. Any real keypress stops it at once.
 * @param stopped_by_key set to true if a key stopped it before max_ms.
 * @return the on-time actually run, in ms.
 */
static uint32_t run_micro_pump_for(uint32_t max_ms, bool *stopped_by_key)
{
    *stopped_by_key = false;
    micro_pump.start();
    uint32_t t0 = millis();
    while (millis() - t0 < max_ms)
    {
        if (real_keypress_waiting())
        {
            *stopped_by_key = true;
            break;
        }
        delay(1);
    }
    micro_pump.stop();
    return millis() - t0;
}

/**
 * @brief Numeric prompt: 'x' aborts, 0 (or nothing parsable) takes the default.
 * @return false if aborted.
 */
static bool prompt_float_or_default(const String &question, float default_value, float *value)
{
    output.println(question + " (0 = " + String(default_value, 3) + ", 'x' aborts):");
    if (numeric_prompt_aborted())
    {
        return false;
    }
    float v = Serial.parseFloat();
    while (Serial.available())
    {
        Serial.read();
    }
    *value = (v > 0) ? v : default_value;
    return true;
}

/**
 * @brief LA3 shield micro pump calibration (docs/lab/shield-protocol.md Part A). The shield
 * tube is taken off the manifold and dispenses into a tube on a 0.01 g scale.
 * Optionally times the filling of an empty line (for PRIME_SHIELD), primes the line,
 * then runs SHIELD_CAL_REPEATS times each of 2-3 durations, interleaved so a drift
 * doesn't look like a duration effect. Fits mL = rate x s + offset
 * (core::fit_shield_rate) and prints the repeatability per duration, the fill time for
 * the target volume, and TEST| lines. Micro pump only: no main pump, no valve, no
 * rotation. Every pump run has a time limit; any key stops it and aborts.
 */
void calibrate_shield_pump()
{
    const int MAX_DURATIONS = 3;
    const int MAX_RUNS = MAX_DURATIONS * SHIELD_CAL_REPEATS;
    const int n_defaults = sizeof(SHIELD_CAL_DEFAULT_MS) / sizeof(SHIELD_CAL_DEFAULT_MS[0]);
    bool key = false;

    output.println("=== shield_calibrate (LA3) ===");
    output.println("Setup: take the shield tube off the manifold and hold its end over a tube on the 0.01 g scale. 'x' aborts at every prompt; any key stops a running pump.");

    float density = 0, target_ml = 0;
    if (!prompt_float_or_default("Shield density in g/mL", SHIELD_DENSITY_G_PER_ML, &density) ||
        !prompt_float_or_default("Target shield volume per filter in mL", SHIELD_TARGET_ML, &target_ml))
    {
        output.println("Aborted.");
        return;
    }

    uint32_t durations[MAX_DURATIONS];
    int n_durations = 0;
    for (int i = 0; i < MAX_DURATIONS; i++)
    {
        uint32_t def = (i < n_defaults) ? SHIELD_CAL_DEFAULT_MS[i] : 0;
        output.println("Duration " + String(i + 1) + " in ms (0 = " + (def ? String(def) : String("none")) +
                       (i >= 2 ? ", -1 = only 2 durations" : "") + ", 'x' aborts):");
        if (numeric_prompt_aborted())
        {
            output.println("Aborted.");
            return;
        }
        long d = Serial.parseInt();
        while (Serial.available())
        {
            Serial.read();
        }
        if (d < 0 && i >= 2)
        {
            break;
        }
        if (d == 0)
        {
            d = def;
        }
        if (d < 1000 || (uint32_t)d > SHIELD_CAL_MAX_RUN_MS)
        {
            output.println("Duration must be 1000.." + String(SHIELD_CAL_MAX_RUN_MS) + " ms. Aborted.");
            return;
        }
        for (int k = 0; k < n_durations; k++)
        {
            if (durations[k] == (uint32_t)d)
            {
                output.println("Each duration must be different. Aborted.");
                return;
            }
        }
        durations[n_durations++] = (uint32_t)d;
    }

    // time to fill an empty line: the line volume, for PRIME_SHIELD (LA3 protocol A3)
    uint32_t t_fill_line_ms = 0;
    if (ask_yes_no("Is the shield line empty (bottle connected, no liquid in the tube)? Then the time to the first drop is measured."))
    {
        output.println("Any key starts the pump; press any key again at the FIRST drop at the tube end (stops by itself after " + String(SHIELD_CAL_PRIME_MAX_MS / 1000) + " s).");
        if (wait_for_keypress_or_abort())
        {
            output.println("Aborted.");
            return;
        }
        t_fill_line_ms = run_micro_pump_for(SHIELD_CAL_PRIME_MAX_MS, &key);
        if (!key)
        {
            output.println("TEST|shield_calibrate|line_fill|FAIL|reason=no_drop_before_" + String(SHIELD_CAL_PRIME_MAX_MS) + "_ms");
            output.println("No drop reached the end: check the bottle, the tube and the pump. Aborted.");
            return;
        }
    }

    output.println("Priming: any key starts the pump; press any key when the liquid drips steadily with no bubbles (stops by itself after " + String(SHIELD_CAL_PRIME_MAX_MS / 1000) + " s).");
    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted.");
        return;
    }
    run_micro_pump_for(SHIELD_CAL_PRIME_MAX_MS, &key);

    core::ShieldRun runs[MAX_RUNS];
    int n_runs = 0;
    const int total = n_durations * SHIELD_CAL_REPEATS;
    for (int r = 0; r < SHIELD_CAL_REPEATS; r++)
    {
        for (int i = 0; i < n_durations; i++)
        {
            output.println("Run " + String(n_runs + 1) + "/" + String(total) + " (" + String(durations[i]) + " ms): put an empty tube on the scale, tare, hold the tube end over it, then any key starts ('x' aborts).");
            if (wait_for_keypress_or_abort())
            {
                output.println("Aborted.");
                return;
            }
            run_micro_pump_for(durations[i], &key);
            if (key)
            {
                output.println("Stopped by a key: the run is incomplete. Aborted.");
                return;
            }

            float mass = 0;
            while (!(mass > 0))
            {
                output.println("Touch the tube end to the tube wall to collect the hanging drop, then enter the mass in g ('x' aborts):");
                if (numeric_prompt_aborted())
                {
                    output.println("Aborted.");
                    return;
                }
                mass = Serial.parseFloat();
                while (Serial.available())
                {
                    Serial.read();
                }
                if (!(mass > 0))
                {
                    output.println("The mass must be > 0 g.");
                }
            }
            runs[n_runs].duration_ms = durations[i];
            runs[n_runs].mass_g = mass;
            n_runs++;
            output.println("  run " + String(n_runs) + ": " + String(durations[i]) + " ms -> " + String(mass, 2) + " g = " + String(core::shield_mass_to_ml(mass, density), 3) + " mL");
        }
    }

    // results
    core::ShieldDurationStats stats[MAX_DURATIONS];
    int n_stats = core::shield_duration_stats(runs, n_runs, density, stats, MAX_DURATIONS);
    core::ShieldFit fit = core::fit_shield_rate(runs, n_runs, density);
    core::ShieldCalVerdict verdict = core::shield_cal_verdict(fit, stats, n_stats, SHIELD_CAL_REPEATS, SHIELD_CAL_MAX_CV_PCT, SHIELD_CAL_MIN_R2);
    uint32_t fill_ms = core::shield_fill_ms(fit, target_ml, SHIELD_CAL_MAX_RUN_MS);

    output.println("--- shield_calibrate results ---");
    for (int i = 0; i < n_stats; i++)
    {
        bool ok = stats[i].n >= SHIELD_CAL_REPEATS && stats[i].cv_pct <= SHIELD_CAL_MAX_CV_PCT;
        output.println("TEST|shield_calibrate|d" + String(stats[i].duration_ms) + "|" + (ok ? "PASS" : "FAIL") +
                       "|n=" + String(stats[i].n) + ";mean_ml=" + String(stats[i].mean_ml, 4) + ";sd_ml=" + String(stats[i].sd_ml, 4) +
                       ";cv_pct=" + String(stats[i].cv_pct, 2) + ";max_cv_pct=" + String(SHIELD_CAL_MAX_CV_PCT, 1));
    }
    output.println(String("TEST|shield_calibrate|fit|") + (verdict == core::SHIELD_CAL_PASS ? "PASS" : "FAIL") +
                   "|verdict=" + core::shield_cal_verdict_name(verdict) + ";rate_ml_s=" + String(fit.rate_ml_per_s, 5) +
                   ";offset_ml=" + String(fit.offset_ml, 4) + ";r2=" + String(fit.r2, 5) + ";density_g_ml=" + String(density, 3) +
                   ";runs=" + String(fit.n_runs) + ";durations=" + String(fit.n_durations));
    output.println(String("TEST|shield_calibrate|fill|") + (fill_ms > 0 && verdict == core::SHIELD_CAL_PASS ? "PASS" : "FAIL") +
                   "|target_ml=" + String(target_ml, 3) + ";fill_ms=" + String(fill_ms));
    if (t_fill_line_ms > 0 && fit.valid)
    {
        float line_ml = fit.rate_ml_per_s * t_fill_line_ms / 1000.0f;
        output.println("TEST|shield_calibrate|line_fill|PASS|t_first_drop_ms=" + String(t_fill_line_ms) + ";line_ml=" + String(line_ml, 3) +
                       ";prime_ms_suggested=" + String((uint32_t)(t_fill_line_ms * SHIELD_PRIME_MARGIN)));
    }

    if (verdict != core::SHIELD_CAL_PASS)
    {
        output.println("Calibration FAILED (" + String(core::shield_cal_verdict_name(verdict)) + "): don't use these numbers. See docs/lab/shield-protocol.md Part A for what to check.");
        return;
    }
    if (fill_ms == 0)
    {
        output.println("No fill time: the target is below the offset, or it needs more than " + String(SHIELD_CAL_MAX_RUN_MS) + " ms.");
        return;
    }
    float min_mean = stats[0].mean_ml, max_mean = stats[0].mean_ml;
    for (int i = 1; i < n_stats; i++)
    {
        min_mean = min(min_mean, stats[i].mean_ml);
        max_mean = max(max_mean, stats[i].mean_ml);
    }
    if (target_ml > max_mean * 1.5f || target_ml < min_mean * 0.5f)
    {
        output.println("WARNING: the target is outside the calibrated range; add a duration near " + String(fill_ms) + " ms and rerun.");
    }
    output.println("Record it in docs/lab/shield-protocol.md (Part A results); FW4 item 7 puts it in Settings.h:");
    output.println("  const uint32_t DNA_SHIELD_FILL_MS = " + String(fill_ms) + "; // LA3 shield_calibrate, " + String(target_ml, 2) + " mL");
}

/**
 * @brief Rotates the manifold a small, fixed amount in a known commanded direction,
 * asks the operator to report the observed rotation direction, then returns it to
 * its starting position - confirmed via the encoder, not a symmetric fixed-time
 * move (real friction isn't perfectly symmetric between directions, especially
 * this close to the motor's stall threshold, so timing alone can't be trusted to
 * land back exactly). Purely informational: the slot-layout math doesn't depend
 * on resolving this, but it lets the operator cross-check the physical layout
 * against the human_idx table, and against the clock-face labels (`clocklist`,
 * clock_minutes_for_slot() in Manifold.cpp) - if those labels run backwards from
 * what you observe on the rig, flip MANIFOLD_RAW_INDEX_INCREASES_CW in Settings.h.
 */
void identify_manifold_direction()
{
    OringScope oring(core::LOCK_STATE_UNLOCKED); // locked again on return
    if (!oring.ok())
    {
        output.println("REFUSED | manifold_direction: manifold not moved (O-ring not unlocked)");
        return;
    }
    output.println("=== Manifold direction identification ===");
    output.println("Send any key to rotate the manifold a small amount (it will return afterward), or 'x' to abort.");
    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted before starting.");
        return;
    }

    uint16_t start_pos = getPositionSPI(ENCODER_MANIFOLD, RES12);

    const uint16_t TEST_SPEED = 30; // comfortable margin above the ~20 stall threshold found during manifold calibration
    const uint32_t TEST_DURATION_MS = 500;
    const uint16_t RETURN_TOLERANCE = 8; // encoder counts (~0.7 degrees)
    const uint32_t RETURN_MAX_MS = 3000; // safety timeout for the return move

    manifold_motor.start(TEST_SPEED, down);
    delay(TEST_DURATION_MS);
    manifold_motor.stop();

    output.println("Rotation direction observed: send 'c' for clockwise, any other key for anticlockwise.");
    char observed = wait_for_keypress();

    output.println("Returning to starting position (encoder-confirmed)...");
    manifold_motor.start(TEST_SPEED, up);
    uint32_t return_start = millis();
    while (millis() - return_start < RETURN_MAX_MS)
    {
        uint16_t pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        int16_t diff = (int16_t)pos - (int16_t)start_pos;
        if (diff > 2048)
        {
            diff -= 4096; // shortest-path wraparound for the 12-bit (0-4095) circular position
        }
        if (diff < -2048)
        {
            diff += 4096;
        }
        if (abs(diff) <= RETURN_TOLERANCE)
        {
            break;
        }
        delay(2);
    }
    manifold_motor.stop();

    uint16_t final_pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
    output.println("Start position: " + String(start_pos) + ", final position: " + String(final_pos));
    if (abs((int16_t)final_pos - (int16_t)start_pos) > RETURN_TOLERANCE * 2)
    {
        output.println("WARNING: did not return within tolerance - check manually before continuing.");
    }

    output.println((observed == 'c' || observed == 'C')
                       ? "Recorded: commanding 'down' (Motor.h) produces clockwise rotation."
                       : "Recorded: commanding 'down' (Motor.h) produces anticlockwise rotation.");
}

/**
 * @brief Field/pre-deployment commissioning: walks through every calibration this
 * system has (manifold encoder, linear actuator unlocked/locked positions, flow
 * sensor, and optionally the DNA-shield pump + its dispensed volume), one at a time, asking
 * before each so you can skip ones that don't need redoing. Each sub-calibration
 * prints its own suggested values as it runs - this is just the checklist that
 * chains them together. The pressure sensor is not in this chain: calibrate_pressure
 * (the ABP against a manometer) runs first, as its own stage. After running
 * this, review everything printed above, edit Manifold.h / Settings.h / Flow_sensor.h
 * accordingly, then rebuild and reflash once before using the machine.
 */
void run_commissioning_calibration()
{
    output.println("=== CoWaS commissioning calibration ===");
    output.println("Answer y/n for each - skip any that don't need redoing.");
    ask_spool_and_purge_check(); // F130: the spool module changes the dead volume and the purge

    // first: the stages below that pump rely on the pressure cap
    if (ask_yes_no("Calibrate the pressure sensor against the manometer (syringe on a tee, no pump)?"))
    {
        calibrate_pressure();
    }

    if (ask_yes_no("Calibrate the manifold encoder (purge angle)?"))
    {
        bool aligned = false;
        while (!aligned)
        {
            calibrateEncoder();
            aligned = ask_yes_no("Is this alignment satisfying?");
        }
        output.println("-> copy the printed purge_angle value into Manifold.h");
    }

    if (ask_yes_no("Calibrate the linear actuator (unlocked/locked positions)?"))
    {
        calibrate_linear_actuator();
    }

    if (ask_yes_no("Calibrate the flow sensor (pumps a measured volume)?"))
    {
        calibrate_flow_sensor();
    }

    if (ask_yes_no("Calibrate the DNA-shield pump? (this one uses the physical buttons, not serial)"))
    {
        calibrate_DNA_pump();

        if (ask_yes_no("Also check the DNA-shield pump's dispensed volume? (optional)"))
        {
            calibrate_dna_pump_volume();
        }
    }

    output.println("=== Commissioning calibration complete ===");
    output.println("Review the suggested values printed above, edit Manifold.h / Settings.h accordingly, then rebuild and reflash before using the machine.");
}

void test_motor_spool()
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - test skipped");
        return;
    }
    spool.set_speed(20, down);
    spool.start();
    delay(1000);
    spool.stop();
    delay(500);
    spool.set_speed(20, up);
    spool.start();
    delay(1000);
    spool.stop();
}

void test_valves()
{
    // extern valve_1;valve_23; valve_manifold;
    for (uint8_t i = 0; i < 2; i++)
    {
        Serial.println("loop " + String(i));
        valve_1.switch_way();
        delay(5000);
        valve_23.switch_way();
        delay(5000);
        valve_manifold.switch_way();
        delay(5000);
    }
}

void test_command_box()
{
    bool start_last = button_start.isPressed();
    bool left_last = button_left.isPressed();
    bool right_last = button_right.isPressed();
    bool start;
    bool left;
    bool right;

    green_led.off();
    status_led.on();
    uint32_t last_led_switch = millis();
    // uint32_t last_pot_print = millis();

    while (!Serial.available())
    {
        if (last_led_switch + 1000 < millis())
        {
            green_led.switch_state();
            status_led.switch_state();
            last_led_switch = millis();
        }

        start = button_start.isPressed();
        left = button_left.isPressed();
        right = button_right.isPressed();

        if (start_last != start)
        {
            Serial.print("Button start ");
            Serial.println((start) ? "Pressed" : "Released");
        }

        if (left_last != left)
        {
            Serial.print("Button left ");
            Serial.println((left) ? "Pressed" : "Released");
        }

        if (right_last != right)
        {
            Serial.print("Button right ");
            Serial.println((right) ? "Pressed" : "Released");
        }

        start_last = start;
        left_last = left;
        right_last = right;
    }
}

void test_micro_switch()
{
    bool up_last = button_spool_up.getState();
    bool down_last = button_spool_down.getState();
    bool container_last = button_container.getState();
    bool up;
    bool down;
    bool container;

    while (!Serial.available())
    {

        // the switches are normally pressed
        up = button_spool_up.getState();
        down = button_spool_down.getState();
        container = button_container.getState();

        if (up_last != up)
        {
            Serial.print("Micro-switch spool up :");
            Serial.println((!up) ? "Pressed" : "Released");
        }

        if (down_last != down)
        {
            Serial.print("Micro-switch spool down ");
            Serial.println((!down) ? "Pressed" : "Released");
        }

        if (container_last != container)
        {
            Serial.print("Micro-switch container ");
            Serial.println((!container) ? "Pressed" : "Released");
        }

        up_last = up;
        down_last = down;
        container_last = container;
    }
}

void test_encoder()
{
    uint16_t pos;
    float angle_deg;

    OringScope oring(core::LOCK_STATE_UNLOCKED); // locked again on return
    if (!oring.ok())
    {
        output.println("REFUSED | encoder_manifold: manifold not moved (O-ring not unlocked)");
        return;
    }
    manifold_motor.start(30, down);

    while (!Serial.available())
    {
        pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        angle_deg = pos * encoder_to_deg;
        Serial.print("Manifold encoder value : ");
        Serial.print(pos);
        Serial.print(",    Angle in degrees : ");
        Serial.println(angle_deg);
        delay(500);
    }
    manifold_motor.stop();
}

void test_encoder_spool()
{
    uint32_t last_print = millis();
    while (!Serial.available())
    {
        encoder.step_counter();
        if (last_print + 3000 < millis())
        {
            Serial.print("Pulses encoder : ");
            Serial.println(encoder.get_pulses_A());
            Serial.print("Depth : ");
            Serial.println(encoder.get_distance());

            last_print = millis();
        }
    }
}

void go_to_zero()
{
    // FW2 (F43): refuses unless the O-ring is known unlocked, stops on the rotation
    // timeout and on repeated encoder failures (it used to spin forever).
    OringScope oring(core::LOCK_STATE_UNLOCKED); // locked again on return
    if (!oring.ok())
    {
        output.println("REFUSED | zero: manifold not moved (O-ring not unlocked)");
        return;
    }
    Serial.println("Going to zero manifold");
    // SETUP//
    bool end_rotation = false;
    float angle_to_reach = 0;
    uint32_t start = millis();
    uint8_t consecutive_failures = 0;

    float current_angle;

    manifold_motor.start(40, down);
    // ROTATE//
    while (end_rotation == false)
    {
        uint16_t pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        if (pos == 0xFFFF)
        {
            if (++consecutive_failures >= MANIFOLD_ENCODER_MAX_CONSECUTIVE_FAILURES)
            {
                manifold_motor.stop();
                output.println("ERROR | zero: aborted - lost the encoder (too many consecutive bad reads). Motor stopped.");
                return;
            }
            delay(1);
            continue;
        }
        consecutive_failures = 0;
        current_angle = pos * encoder_to_deg;

        if (millis() - start > MANIFOLD_ROTATE_TIMEOUT_MS)
        {
            manifold_motor.stop();
            output.println("ERROR | zero: aborted - timed out before reaching 0 deg. Motor stopped.");
            return;
        }

        if (VERBOSE_MANIFOLD)
        {
            output.print("   Angle to reach: ");
            output.print(angle_to_reach);
            output.println(2);
        }

        if ((current_angle >= angle_to_reach - 5) && (current_angle <= angle_to_reach + 5))
        {
            manifold_motor.stop();
            end_rotation = true;
        }

        delay(1); // smaller delay -> better precision
    }
}

// ! to implement, only needed if encoder manifold still changes absolut 0
