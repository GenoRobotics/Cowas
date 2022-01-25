#include <Arduino.h>
#include "Button.h"
#include "Trustability_ABP_Gage.h"
#include "Valve_3_2.h"
#include "Valve_2_2.h"
#include "Pump.h"
#include "Motor.h"
#include "Encoder.h"
#include "Potentiometer.h"
#include "Led.h"
#include <SPI.h>
#include "Serial_output.h"
#include "sample.h"
#include "Settings.h"
#include "Critical_error.h"
#include "Timer.h"
#include "Serial_device.h"
#include "Step_functions.h"

#define DELAY_ACTIONS 1000

extern Serial_output output;
extern Serial_device serial;
extern Led blue_led;
extern Led green_led;
extern Trustability_ABP_Gage pressure1;
extern Trustability_ABP_Gage pressure2;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_purge;
extern Valve_2_2 valve_stx_in[MAX_FILTER_NUMBER]; // see schematics
extern Valve_3_2 valve_stx_out[MAX_FILTER_NUMBER];
extern Pump pump;
extern Pump pump_vacuum;
extern Motor spool;
extern Encoder encoder;
extern Button button_start;
extern Button button_container;
extern Button button_spool_up;
extern Button button_spool_down;
extern Button button_left;
extern Button button_right;
extern Potentiometer potentiometer;
extern struct Timer timer_control_pressure1;

// ============= DESCRIPTION ==============
// Code for all steps of execution of COWAS with high level functions.
// Each step should be independant, that is be responible to have all valves
// and actuators being correctely set.
// See fluidic diagram for details.

/**
 * @brief Unroll the spool at the correct depth
 * 
 * @param _depth absolute depth at which the sampling is going to be done.
 */
void step_dive(int _depth)
{
    output.println("Step dive started");

    valve_23.set_L_way();
    // let air escape the system while diving
    valve_1.set_open_way();
    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();

    spool.set_speed(SPEED_DOWN, down);
    spool.start(_depth);

    output.println("Time to dive : " + String(millis() - time1) + " ms");

    valve_1.set_close_way();

    output.println("Step dive ended");
}

/**
 * @brief Fill with water the container. Step_dive first.
 * 
 */
void step_fill_container()
{
    output.println("Step fill container started");

    valve_1.set_close_way();
    valve_23.set_I_way();
    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();
    bool run = true;

    pump.set_power(90);
    pump.start();

    do
    {

        // conditions ouside loop to print what condition is responible for stopping
        if (button_container.getState() == 0)
        {
            run = false;
            output.println("Fill container stopped by button");
        }
        // TODO : better function. cannot be constant time because time depends of how deep we sample
        // --> water need to go through the spool tube before entering the system
        else if (millis() - time1 > FILL_CONTAINER_TIME)
        {
            run = false;
            output.println("Fill container stopped by security timer");
            // TODO : raise a system warning to user
        }
    } while (run);

    pump.stop();

    output.println("Time to fill container : " + String(millis() - time1) + " ms");

    delay(DELAY_ACTIONS);
    valve_23.set_L_way();

    output.println("Step fill container ended");
}

/**
 * @brief Empty the container through purge channel. Step_fill_container first.
 * 
 */
void step_purge()
{
    output.println("Step purge container started");

    // set the valves
    for (uint8_t i = 0; i < MAX_FILTER_NUMBER; i++)
        valve_stx_in[i].set_close_way();
    valve_23.set_L_way();
    valve_purge.set_open_way();
    delay(DELAY_ACTIONS);

    pump.set_power(100);

    // first action is to fill all tubes with water
    // otherwise the sensor will detect end of emptying
    pump.start(FILL_TUBES_WITH_WATER_TIME);

    // then start with sensor reading
    pump.start();
    uint32_t time1 = millis();
    uint8_t validation_tick = 0;
    float pressure;

    bool run = true;

    // loop stops after 2.5 seconds under threshold pressure or when time max is reach
    do
    {
        delay(10); // don't read pressure to fast

        pressure = pressure1.getPressure();

        // adapt pump power to pressure to not go over limit
        if (pressure > 3)
        {
            pump.set_power(pump.get_power() - 2);
        }
        else if (pressure < 2.6f)
        {
            pump.set_power(pump.get_power() + 2);
        }

        // if pressure low enough, engage stopping processure
        if (pressure1.getPressure() < EMPTY_WATER_PRESSURE_PURGE_THRESHOLD)
        {
            validation_tick++;
        }
        else
        {
            validation_tick = 0;
        }

        // conditions outside while loop to allow printing which condition is responsible for stop
        if (validation_tick >= 250)
        {
            run = false;
            output.println("Purge stopped by low pressure");
        }
        else if (millis() - time1 > EMPTY_CONTAINER_TIME_PURGE)
        {
            run = false;
            output.println("Purge stopped by security timer");
            // TODO : raise a system warning to user
        }

    } while (run);

    // pump a little bit more to flush all water
    pump.start(EMPTY_WATER_SECURITY_TIME);
    pump.stop();

    output.println("Time to purge container : " + String(millis() - time1) + " ms");

    delay(DELAY_ACTIONS);
    valve_purge.set_close_way();

    output.println("Step purge container ended");
}

/**
 * @brief Empty water from container into choosen sterivex. Step_fill_container first.
 * 
 * @param num_sterivex The sterivex in which the sampling is made
 */
void step_sampling(uint8_t num_sterivex)
{
    output.println("Step sample through filter started");

    // set the valves
    valve_23.set_L_way();
    valve_purge.set_close_way();
    for (uint8_t i = 0; i < MAX_FILTER_NUMBER; i++)
    {
        if (i == num_sterivex)
        {
            valve_stx_in[i].set_open_way();
            valve_stx_out[i].set_I_way();
        }
        else
        {
            valve_stx_in[i].set_close_way();
            valve_stx_out[i].set_L_way();
        }
    }
    delay(DELAY_ACTIONS);

    pump.set_power(POWER_STX);

    // start by filling all tubes with water
    pump.start(FILL_TUBES_WITH_WATER_TIME);

    pump.start();
    uint32_t time1 = millis();
    uint8_t validation_tick = 0;
    float pressure;
    bool run = true;

    // loop stops after 2.5 seconds under threshold pressure or when time max is reach
    do
    {
        delay(10); // don't read pressure to fast

        pressure = pressure1.getPressure();

        // adapt pump power to pressure
        if (pressure > 3)
        {
            pump.set_power(pump.get_power() - 2);
        }
        else if (pressure < 2.6f)
        {
            pump.set_power(pump.get_power() + 2);
        }

        // if pressure low enough, engage stopping processure
        if (pressure < EMPTY_WATER_PRESSURE_STX_THRESHOLD)
        {
            validation_tick++;
        }
        else
        {
            validation_tick = 0;
        }

        // conditions outside while loop to allow printing which condition is responsible for stop
        if (validation_tick >= 250)
        {
            run = false;
            output.println("Purge stopped by low pressure");
        }
        else if (millis() - time1 > EMPTY_CONTAINER_TIME_FILTER)
        {
            run = false;
            output.println("Purge stopped by security timer");
            // TODO : raise a system warning to user
        }

    } while (run);

    // pump a little bit more to flush all water
    pump.start(EMPTY_WATER_SECURITY_TIME);

    pump.stop();

    output.println("Time to sample water : " + String(millis() - time1) + " ms");

    delay(DELAY_ACTIONS);
    valve_stx_in[num_sterivex].set_close_way();
    valve_stx_out[num_sterivex].set_L_way();

    // end with a purge
    step_purge();

    // now that tubes are empty, ensure to have the tube before filter emmpty
    // set the valves
    valve_23.set_L_way();
    valve_purge.set_close_way();
    for (uint8_t i = 0; i < MAX_FILTER_NUMBER; i++)
    {
        if (i == num_sterivex)
        {
            valve_stx_in[i].set_open_way();
            valve_stx_out[i].set_I_way();
        }
        else
        {
            valve_stx_in[i].set_close_way();
            valve_stx_out[i].set_L_way();
        }
    }
    delay(DELAY_ACTIONS);
    pump.set_power(100);
    pump.start(EMPTY_WATER_SECURITY_TIME * 2);
    delay(DELAY_ACTIONS);
    valve_stx_in[num_sterivex].set_close_way();
    valve_stx_out[num_sterivex].set_L_way();

    output.println("Step sample through filter ended");
}

/**
 * @brief Roll the spool back. Step_dive first.
 * 
 */
void step_rewind()
{
    output.println("Step rewind started");

    // let air enter the system
    valve_1.set_open_way();
    valve_23.set_L_way();
    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();

    spool.set_speed(SPEED_UP, up);
    // go to origin
    spool.start(-1);

    output.println("Time to rewind : " + String(millis() - time1) + " ms");

    delay(DELAY_ACTIONS);
    valve_1.set_close_way();

    output.println("Step rewind ended");
}

/**
 * @brief Dry a sterivex. Step_sample first.
 * 
 * @param num_sterivex the sterivex to dry
 */
void step_dry(uint8_t num_sterivex)
{
    output.println("Step drying started");

    // 10 secondes toutes les 2 minutes 3x
    // temporaire

    // set the valves
    for (uint8_t i = 0; i < MAX_FILTER_NUMBER; i++)
    {
        valve_stx_in[i].set_close_way();

        if (i == num_sterivex)
            valve_stx_out[i].set_L_way();
        else
            valve_stx_out[i].set_I_way();
    }
    delay(DELAY_ACTIONS);

    // TODO : function interrupt for pressure2
    uint32_t time1 = millis();

    for (uint8_t i = 0; i < 3; i++)
    {
        pump_vacuum.set_power(100);
        pump_vacuum.start(10000);
        while (millis() - time1 < DRYING_TIME)
        {
            // if(pressure2.getPressure() > VACUUM_MINIMUM){
            //     pump_vacuum.start();
            //     while(pressure2.getPressure() > VACUUM_TO_ACHIEVE && millis() - time1 < DRYING_TIME);
            //     pump_vacuum.stop();
            // }
            delay(50);
        }
    }

    for (uint8_t i = 0; i < MAX_FILTER_NUMBER; i++)
    {
        valve_stx_out[i].set_L_way();
    }

    output.println("Step drying ended");
}