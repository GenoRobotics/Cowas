/**
 * @file maintenance.cpp
 * @author Felix Schmeding
 * @brief Functions for maintenance of the CoWaS, read instructions for each function carefully
 * @version 0.1
 * @date 2024-05-20
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
#include "Flow_sensor.h"
#include "Timer.h"
#include "maintenance.h"
#include "Step_functions.h"
#include "Manifold.h"

extern int manifold_slot;
extern C_output output;
extern Led status_led;
extern Led green_led;
extern Trustability_ABP_Gage pressure1;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_manifold;
extern Pump pump;

extern Micro_Pump micro_pump;

extern Motor spool;
extern Motor manifold_motor;
extern Encoder encoder;
extern Flow_sensor flow_sensor_small;
extern Button button_start;
extern Button button_container;
extern Button button_spool_up;
extern Button button_spool_down;
extern Button button_left;
extern Button button_right;

enum DNA_state
{
    fill,
    start_micro,
    stop_micro,
    start_big,
    stop_big,
    results,
    start_end
};

bool run_pump(uint32_t max_time);
bool run_pump_until_volume(float target_ml, uint32_t max_time);

static void stop_purge_manifold()
{
    pump.stop();
    valve_1.set_close_way();
    valve_manifold.set_close_way();
}

void calibrate_DNA_pump()
{
    if (!require_pressure_sensor("cal_DNA"))
    { // runs the big pump (cap)
        return;
    }
    if (!rotateMotor(12))   // is chosen to match a clogged filter
    { // FW2: refused (lock state) or failed -> nothing else runs
        return;
    }
    DNA_state state = fill;
    uint32_t time_micro_pump = 0;
    uint32_t time_big_pump = 0;
    uint32_t start_time_stamp;

    bool DNA_running = false;
    bool pump_running = false;
    uint8_t speed_big = 11;

    bool stop = false;

    output.println("Press right button to start, left button to stop pump. When the dyed fluid is at valve press Start to continue");
    while (!stop)
    {
        switch (state)
        {
        case fill:
            time_micro_pump = 0;
            time_big_pump = 0;
            if (button_right.isPressed())
            {
                micro_pump.start();
                button_right.waitPressedAndReleased();
            }
            if (button_left.isPressed())
            {
                micro_pump.stop();
                button_left.waitPressedAndReleased();
            }
            if (button_start.isPressed())
            {
                micro_pump.stop();
                button_start.waitPressedAndReleased();
                state = start_micro;
                output.println("Press right to start micro pump, Start move on to calibrate big pump");
            }
            break;
        case start_micro:
            if (button_right.isPressed())
            {
                micro_pump.start();
                start_time_stamp = millis();
                DNA_running = true;
                state = stop_micro;
                output.println("Press left when enough dyed fluid is in sterivex");
            }
            else if (button_start.isPressed())
            {                      // next step
                micro_pump.stop(); // should not run, but for safety
                button_start.waitPressedAndReleased();
                state = start_big;
                output.println("Press left to change speed, right to start pump and \"Start\" to finish calibrating");
            }
            break;
        case stop_micro:
            if (button_left.isPressed())
            {
                micro_pump.stop();
                time_micro_pump += millis() - start_time_stamp;
                state = start_micro;
                button_left.waitPressedAndReleased();
                output.println("Press right to start micro pump, Start move on to calibrate big pump");
            }
            break;
        case start_big:
            if (button_right.isPressed())
            {
                valve_1.set_open_way();
                pump.set_power(speed_big);
                pump.start();
                start_time_stamp = millis();
                button_right.waitPressedAndReleased();
                state = stop_big;
            }
            if (button_left.isPressed())
            {
                speed_big += 1;
                if (speed_big > 25)
                    speed_big = 11;
                output.print("Pump speed is : ");
                output.println(speed_big);
                button_left.waitPressedAndReleased();
            }
            if (button_start.isPressed())
            {
                micro_pump.stop();
                button_start.waitPressedAndReleased();
                state = results;
            }
            break;
        case stop_big:
            if (button_left.isPressed())
            {
                pump.stop();
                time_big_pump += millis() - start_time_stamp;
                state = start_big;
                button_left.waitPressedAndReleased();
                output.println("Press left to change speed, right to start pump and \"Start\" to finish calibrating");
            }
            break;
        case results:
            output.print("Time DNA pump running (ms): ");
            output.println(time_micro_pump);
            output.print("Time big pump running (ms): ");
            output.println(time_big_pump);
            output.print("With power: ");
            output.println(speed_big);

            state = start_end;

            output.println("Press Start to go again, right to stop calibrating");
            break;
        case start_end:
            if (button_start.isPressed())
            {
                state = start_micro;
                button_start.waitPressedAndReleased();
                output.println("Press right to start micro pump, Start to calibrate big pump");
            }
            if (button_right.isPressed())
            {
                state = fill;
                stop = true;
                button_right.waitPressedAndReleased();
            }

        default:
            break;
        }

        delay(20);
    }

    // turning "off" valves, no voltage applied (even those that should not have been opened, just in case)
    valve_23.set_L_way();
    valve_manifold.set_close_way();
    valve_1.set_close_way();
    output.println("Calibration of DNA pump done");
}

bool run_pump_until_volume(float target_ml, uint32_t max_time, const char *stage)
{
    flow_sensor_small.reset_values();
    const uint32_t start_time = millis();
    pump.start();
    if (!pump.is_running())
    {
        Serial.print("ERROR | Ethanol ");
        Serial.print(stage);
        Serial.println(": pump did not start");
        return false;
    }

    while (true)
    {
        flow_sensor_small.update();
        const unsigned int volume_ml = flow_sensor_small.get_totalFlowMilliL();
        if (volume_ml >= target_ml)
        {
            pump.stop();
            Serial.print("Ethanol ");
            Serial.print(stage);
            Serial.print(" reached ");
            Serial.print(volume_ml);
            Serial.println(" mL indicated");
            return true;
        }

        if (button_start.isPressed())
        {
            pump.stop();
            button_start.waitPressedAndReleased();
            Serial.print("ABORT | Ethanol ");
            Serial.print(stage);
            Serial.println(" stopped by operator");
            return false;
        }

        if (pump.is_running() && pump.enforce_pressure_safety())
        {
            return false;
        }

        if (millis() - start_time >= max_time)
        {
            pump.stop();
            Serial.print("ERROR | Ethanol ");
            Serial.print(stage);
            Serial.print(" timed out before reaching ");
            Serial.print(target_ml, 0);
            Serial.println(" mL indicated");
            return false;
        }
        delay(20);
    }
}

void purge_pipes_manifold()
{
    if (!require_pressure_sensor("purge_pipes_manifold"))
    {
        return;
    }

    Serial.println("Ready to start purge of manifold pipes, please press start button");

    button_start.waitPressedAndReleased();

    // step_fill_container();
    delay(1000);
    pump.set_power(25);

    valve_1.set_close_way();
    valve_23.set_L_way();
    valve_manifold.set_close_way();

    if (!rotateMotor(PURGE_SLOT))
    {
        Serial.println("ERROR | Ethanol purge prime: could not rotate to purge slot");
        stop_purge_manifold();
        return;
    }

    delay(500);
    valve_manifold.set_open_way();
    Serial.print("Priming ethanol through purge slot until ");
    Serial.print(ETHANOL_MANIFOLD_FLUSH_ML, 0);
    Serial.println(" mL is indicated");
    if (!run_pump_until_volume(ETHANOL_MANIFOLD_FLUSH_ML, PURGE_MAX_RUNTIME_MS, "purge prime"))
    {
        stop_purge_manifold();
        return;
    }
    valve_manifold.set_close_way();
    delay(500);

    for (int slot = 0; slot < NB_SLOT; slot++)
    {
        if (slot == PURGE_SLOT)
        {
            continue;
        }

        Serial.print("Flushing ethanol through slot ");
        Serial.println(slot);
        if (!rotateMotor(slot))
        {
            Serial.print("ERROR | Ethanol slot flush: could not rotate to slot ");
            Serial.println(slot);
            stop_purge_manifold();
            return;
        }
        delay(500);
        valve_23.set_L_way();
        valve_manifold.set_open_way();
        if (!run_pump_until_volume(ETHANOL_MANIFOLD_FLUSH_ML, PURGE_MAX_RUNTIME_MS, "slot flush"))
        {
            stop_purge_manifold();
            return;
        }
        valve_manifold.set_close_way();
        delay(500);
    }

    if (!rotateMotor(PURGE_SLOT))
    {
        Serial.println("ERROR | Ethanol final purge: could not rotate to purge slot");
        stop_purge_manifold();
        return;
    }
    delay(500);
    valve_manifold.set_open_way();
    Serial.println("Sending the remaining ethanol through the purge slot (50 mL indicated)");
    if (!run_pump_until_volume(ETHANOL_MANIFOLD_FLUSH_ML, PURGE_MAX_RUNTIME_MS, "final purge"))
    {
        stop_purge_manifold();
        return;
    }

    stop_purge_manifold();
    Serial.println("Ethanol manifold flush complete");
}

void fill_DNA_shield_tube()
{
    output.println("THe DNA-shield need to be pumped until the first valve");
    output.println("Press RIGHT button to start, LEFT button to stop pump. When DNA-shield is at valve press STOP and then START to continue");
    while (button_start.isReleased())
    {
        if (button_right.isPressed())
        {
            micro_pump.start();
            button_right.waitPressedAndReleased();
        }
        if (button_left.isPressed())
        {
            micro_pump.stop();
            button_left.waitPressedAndReleased();
        }
    }
    button_start.waitPressedAndReleased();
}

#ifdef SYSTEM_CHECKUP
void system_checkup()
{
    // check if sensor are operationnal
    bool error = false;

    // check spool switch 1
    spool.start(20, down);
    delay(200);
    spool.stop();
    if (button_spool_up.getState() == 1)
    {
        output.println("CHECK | Button spool working");
        spool.start_origin();
    }
    else
    {
        output.println("ERROR | Button spool not working");
        error = true;
    }

    // check container switch
    if (button_container.getState() == 1)
        output.println("CHECK | Button container working");
    else
    {
        output.println("ERROR | Button container not working");
        error = true;
    }

    // check spool down switch
    if (button_spool_down.getState() == 1)
        output.println("CHECK | Button spool down working");
    else
    {
        output.println("ERROR | Button spool down not working");
        error = true;
    }

    // check temperature
    //  flush first time reading otherwise error (no idea why)
    pressure1.getTemperature();
    delay(10);
    if (pressure1.getTemperature() > 0)
    {
        output.println("CHECK | Temperature okay (" + String(pressure1.getTemperature()) + ")");
    }
    else
    {
        output.println("ERROR | Temperature too low (" + String(pressure1.getTemperature()) + ")");
        error = true;
    }

    if (error)
    {
        output.println("FATAL ERROR AT STARTUP ");
        set_system_state(state_error);
        while (true)
            delay(500);
    }
}

#endif