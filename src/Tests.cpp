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
#include "Serial_device.h"
#include "Tests.h"
#include "Step_functions.h"
#include "Manifold.h"
#include "maintenance.h"
#include "Pressure_sensor.h"
#include "Linear_actuator.h"
#include "Flow_sensor.h"
#include "Oring_lock.h"

extern int manifold_slot;
extern C_output output;
extern Serial_device serial;
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
extern struct Timer timer_control_pressure2;

extern BigPressure pressure2;

extern Linear_actuator linear_actuator;
extern Manifold manifold;

void test_serial_device()
{
    if (Serial.available())
    {
        output.println(Serial.read());
        output.println("");
    }
}

void test_all_components()
{
    // call all sub-functions

    // go_to_zero();

    String test_command;
    while (42)
    {
        if (Serial.available())
        {
            // test_command = Serial.readString();
            test_command = Serial.readStringUntil('\n');
            // test_command = Serial.readStringUntil('b');
            Serial.println(test_command);

            if (test_command == "abort")
            {
                Serial.println("Abort: empty container, rewind and empty deplyoment");
                abort_sample();
            }

            if (test_command == "temp")
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
            if (test_command == "pressure_sensors")
            {
                Serial.println("testing pressure_sensors");
                test_pressure_sensor();
            }
            if (test_command == "pressure")
            {
                print_pressure_once();
            }
            if (test_command == "calibrate_pressure2")
            {
                calibrate_pressure2_zero();
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
                int requested_minutes = test_command.substring(5).toInt();
                int slot = slot_for_clock_minutes(requested_minutes);
                if (slot < 0)
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

            if (test_command == "multi_sample")
            {
                test_multi_sample_loading();
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

void test_1_depth_40m_direct()
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - test skipped");
        return;
    }
    output.println("TEST 1 - ALTIMETRE 19m direct");
    uint32_t lastTime = 0;

    spool.start_origin();
    status_led.on();
    output.println("=========== go 19m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(19000);
    output.println(millis() - lastTime);

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis() - lastTime);

    green_led.on();
    output.println("END OF TEST 1 19 meters direct");
    button_start.waitPressedAndReleased();
    green_led.off();
}

void test_2_remplissage_container_1m()
{

    output.println("TEST 2 - container 1m");

    spool.start(100);
    valve_23.set_I_way();
    delay(200);
    pump.set_power(255);
    timerStart(timer_control_pressure2);
    pump.start();
    while (button_container.getState() == 1)
        ;
    pump.stop();
    timerStop(timer_control_pressure2);
    valve_23.set_L_way();

    step_purge();
    step_rewind();
}

void test_2_remplissage_container_40m()
{

    output.println("TEST 2 - container 40m");

    spool.start(40000);
    valve_23.set_I_way();
    delay(200);
    pump.set_power(100);
    timerStart(timer_control_pressure2);
    pump.start();
    while (button_container.getState() == 1)
        ;
    pump.stop();
    timerStop(timer_control_pressure2);
    valve_23.set_L_way();

    step_purge();
    step_rewind();
}

void test_3_sterivex_1()
{

    output.println("TEST 3 - sterivex 1");

    spool.start(1000);
    for (int i = 0; i < 3; i++)
    {
        output.println("Fill container number " + String(i + 1));
        step_fill_container();
        output.println("Purge number " + String(i + 1));
        step_purge();
    }

    output.println("Fill container number 4");
    step_fill_container();

    // sampling sterivex 1
    uint32_t lastTime = millis();
    step_sampling(1);
    output.println("Temps " + String(millis() - lastTime));

    output.println("Control manuel pompe");
    status_led.on();
    button_start.waitPressedAndReleased();
    status_led.off();

    while (!button_start.isPressed())
    {
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(100);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(100);
            pump.stop();
        }
        delay(10);
    }
    button_start.waitPressedAndReleased();
    output.println("purge");
    green_led.on();
    button_start.waitPressedAndReleased();
    green_led.off();

    step_purge();
    output.println("rewind");
    button_start.waitPressedAndReleased();
    step_rewind();
}

void test_3_sterivex_2()
{
    output.println("TEST 3 - sterivex 2");

    spool.start(1000);
    output.println("Fill container number 5");
    step_fill_container();

    // sampling sterivex 2
    uint32_t lastTime = millis();
    step_sampling(2);
    output.println("Temps " + String(millis() - lastTime));

    output.println("Control manuel pompe");
    status_led.on();
    button_start.waitPressedAndReleased();
    status_led.off();

    // mnaual contorl
    // int pot_last_value = potentiometer.get_value(0, 100);
    // int pot_value = 0;
    // int speedy = 60;
    while (!button_start.isPressed())
    {
        // pot_value = potentiometer.get_value(0, 100);
        // if (pot_value <= pot_last_value - 4 || pot_value >= pot_last_value + 4)
        // {
        //     // speedy = pot_value;
        //     // pot_last_value = pot_value;
        //     output.println("speed " + String(speedy));
        // }
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(100);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(100);
            pump.stop();
        }
        delay(10);
    }
    button_start.waitPressedAndReleased();
    output.println("purge");
    green_led.on();
    button_start.waitPressedAndReleased();
    green_led.off();

    step_purge();
    output.println("rewind");
    button_start.waitPressedAndReleased();
    step_rewind();
}

/**
 * @brief Reads and prints both pressure sensors once (not a loop - see
 * test_pressure_sensor() for the continuous stream version).
 */
void print_pressure_once()
{
    output.println("Pressure (Trustability, pressure1): " + String(pressure1.getPressure()) + " bar");
    output.println("Pressure (new sensor, pressure2): " + String(pressure2.readPressure()) + " bar");
}

char wait_for_keypress(); // defined below - forward declared for use here
bool wait_for_keypress_or_abort(); // defined below - forward declared for use here

/**
 * @brief Quick zero-point calibration for pressure2 ("new sensor", BigPressure,
 * analog, pressure_2_pin). There's no live-settable offset on this sensor - the
 * OffSet in BigPressure::convertToPressure() (Pressure_sensor.cpp) is a hardcoded
 * constant - so like calibrate_flow_sensor(), this just computes and prints a
 * corrected value to paste in by hand, rather than reworking that class.
 *
 * Averages many samples over ~1s instead of trusting a single reading, and prints
 * the spread (max-min) so you can tell a noisy/unreliable signal from a clean one
 * that just needs its offset corrected - a single bad sample previously risked
 * calibrating the offset to a noise spike instead of the true zero point. With NO
 * pressure applied (sensor open to air / disconnected), the true pressure is 0, so
 * whatever pressure2 currently reports is pure offset error. From BigPressure's
 * formula (pressure = (voltage - OffSet) * 2.5), the corrected
 * OffSet = old OffSet + mean_reading / 2.5.
 */
void calibrate_pressure2_zero()
{
    output.println("=== Pressure2 (new sensor) zero-point calibration ===");
    output.println("Make sure NO pressure is applied (open to air / disconnected), then send any key to read, or 'x' to abort.");

    if (wait_for_keypress_or_abort())
    {
        output.println("Aborted.");
        return;
    }

    const uint8_t SAMPLES = 50;
    float total = 0;
    float min_reading = 0;
    float max_reading = 0;
    for (uint8_t i = 0; i < SAMPLES; i++)
    {
        float reading = pressure2.readPressure();
        if (i == 0 || reading < min_reading) min_reading = reading;
        if (i == 0 || reading > max_reading) max_reading = reading;
        total += reading;
        delay(20);
    }
    float mean_reading = total / SAMPLES;
    float spread = max_reading - min_reading;

    output.println("Mean reading over " + String(SAMPLES) + " samples (should be ~0 bar at true ambient): " + String(mean_reading, 3) + " bar");
    output.println("Spread (max-min): " + String(spread, 3) + " bar");
    if (spread > 0.3)
    {
        output.println("WARNING: that spread is large for a sensor just sitting still - this looks like signal noise or a wiring issue (loose connector, floating pin, missing ground), not just an offset error. Fixing the offset alone won't help until the underlying signal is stable - worth checking the physical wiring before trusting this sensor for anything safety-related.");
    }

    const float OLD_OFFSET = 0.483; // must match the OffSet constant in BigPressure::convertToPressure() (Pressure_sensor.cpp) - update this too if that constant is ever changed by hand
    float new_offset = OLD_OFFSET + mean_reading / 2.5;

    output.println("Edit this in Pressure_sensor.cpp (BigPressure::convertToPressure()), then rebuild and reflash:");
    output.println("  const float OffSet = " + String(new_offset, 3) + "; // V");
}

void test_pressure_sensor()
{
    float pressure;
    float pressureBig;

    while (!Serial.available())
    {
        delay(500); // don't read pressure to fast
        pressure = pressure1.getPressure();
        pressureBig = pressure2.readPressure();

        Serial.print("Pressure Trustability value : ");
        Serial.println(pressure);
        Serial.print("Pressure New sensor value : ");
        Serial.println(pressureBig);
        // Serial.print("   Analog value: ");
        // Serial.println(analogRead(pressure_2_pin));

        delay(500);
    }
}

void test_manifold()
{
    uint16_t pos;
    float angle_deg;

    for (int j = 1; j < 15; j++)
    {
        rotateMotor(0);
        delay(1000);
        rotateMotor(j);

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
            output.println("  pressure (pressure2, backs the safety cap): " + String(pressure2.readPressure()) + " bar");
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
 * exact same function production code will use - no duplicated logic here.
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
 * @brief Multi-sample dry run: rotates through every available slot in FILL_ORDER
 * and runs load_slot() at each with a small volume (TEST_MULTI_SAMPLE_VOLUME_ML,
 * not a real sample) - goal is to exercise every joint for leaks and confirm the
 * fill order/manifold rotation itself, without spending a full sample's worth of
 * water per slot. Pauses for operator confirmation between slots so leaks can be
 * inspected before moving on; send any key to continue, 'x' to abort the whole run.
 */
void test_multi_sample_loading()
{
    output.println("=== Multi-sample loading test (dry run, " + String(TEST_MULTI_SAMPLE_VOLUME_ML) + " mL/slot) ===");
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

        bool ok = load_slot(TEST_MULTI_SAMPLE_VOLUME_ML);
        output.println(ok ? "  ok" : "  FAILED - lock_oring() did not confirm lock, check manually");

        output.println("Inspect joints for leaks. Send any key to continue to the next slot, or 'x' to abort.");
        if (wait_for_keypress_or_abort())
        {
            output.println("Aborted by operator.");
            return;
        }
    }

    output.println("Multi-sample loading test complete - all available slots visited.");
}

/**
 * @brief Manual single-slot test of load_slot_full() (Step_functions.cpp): the
 * FULL production sequence (step_sampling() + step_DNA_shield()) - real
 * STX_SAMPLE_MILLILITERS volume, the elaborate purge-then-filter double
 * air-emptying, and the DNA-shield preservative push, exactly as sample_process()
 * runs it per sample. Precondition: the manifold must already be rotated to the
 * target slot (use a `slotN`/`clockMM` command first) and be unlocked.
 */
void test_load_sample_full()
{
    output.println("=== Sample-load test (FULL production sequence) ===");
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
 * @brief Multi-sample test using the FULL production sequence (load_slot_full()) -
 * real STX_SAMPLE_MILLILITERS volume per slot, unlike test_multi_sample_loading()'s
 * reduced-volume dry run. Rotates through every available slot in FILL_ORDER,
 * running step_sampling()+step_DNA_shield() at each. Pauses for operator
 * confirmation between slots; send any key to continue, 'x' to abort the whole run.
 */
void test_multi_sample_loading_full()
{
    output.println("=== Multi-sample loading test (FULL production sequence, " + String(STX_SAMPLE_MILLILITERS) + " mL/slot) ===");
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
 * @param steps_out set to the number of steps travelled in the call that ended in
 * a confirmed stall.
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

    while (true)
    {
        linear_actuator.set_direction(dir);
        long steps = 0;
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
 * comment block in Settings.h/Oring_lock.cpp: production lock_oring() re-uses this
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

    // --- Stage 1: find and mark the UNLOCKED reference position ---
    output.println("Jog the actuator to the UNLOCKED (fully retracted / o-ring released) reference position:");
    output.println("f = jog forward, b = jog backward (send any key to stop the jog), c = mark this as UNLOCKED, x = abort");
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
            output.println("f: forward, b: backward, c: mark as UNLOCKED, x: abort");
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

    linear_actuator.disable();

    // --- Summary ---
    output.println("=== Calibration summary ===");
    output.println("Edit this in Settings.h, then rebuild and reflash:");
    output.println("  const long LINEAR_ACT_LOCKED_STEPS = " + String(locked_steps) + ";");
    output.println("Production locking (lock_oring()) will automatically over-drive " + String((LINEAR_ACT_LOCK_OVERDRIVE_FACTOR - 1.0) * 100, 0) + "% past this every cycle (LINEAR_ACT_LOCK_OVERDRIVE_FACTOR, Settings.h) - no need to add margin here yourself.");
}

/// @brief Prints `prompt`, waits for one keypress, and returns true for y/Y.
bool ask_yes_no(const char *prompt)
{
    output.println(String(prompt) + " (y/n)");
    char c = wait_for_keypress();
    return (c == 'y' || c == 'Y');
}

/**
 * @brief Guided flow-sensor (flow_sensor_small) calibration. Pumps a target volume
 * (you choose, or a 2000 mL default) - stopping on sensor-reported target, manual
 * key, or a safety timeout - then asks for the ACTUAL measured volume and computes
 * a corrected calibrationFactor_small from the ratio between them.
 */
void calibrate_flow_sensor()
{
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
    const uint16_t RETURN_TOLERANCE = 8;  // encoder counts (~0.7 degrees)
    const uint32_t RETURN_MAX_MS = 3000;  // safety timeout for the return move

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
 * chains them together. Pressure sensors are NOT included: there's no offset/
 * calibration hook built into PressureSensor/BigPressure to hang a simple field
 * calibration off of (the zero-offset is a hardcoded constant in convertToPressure()),
 * so that's skipped for now rather than reworking those classes. After running
 * this, review everything printed above, edit Manifold.h / Settings.h / Flow_sensor.h
 * accordingly, then rebuild and reflash once before using the machine.
 */
void run_commissioning_calibration()
{
    output.println("=== CoWaS commissioning calibration ===");
    output.println("Answer y/n for each - skip any that don't need redoing.");

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
    Serial.println("Going to zero manifold");
    // SETUP//
    bool end_rotation = false;
    float angle_to_reach = 0;

    // readEncoder(true);
    // readEncoder(false);
    float current_angle;

    manifold_motor.start(40, down);
    // ROTATE//
    while (end_rotation == false)
    {
        current_angle = getPositionSPI(ENCODER_MANIFOLD, RES12) * encoder_to_deg;

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
void reset_encoder()
{
    Serial.println("Resetting ");
}