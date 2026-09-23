/**
 * @file Step_functions.cpp
 * @author Timothée Hirt & Christophe Deloose & Paco Mermoud
 * @brief All detailed steps of a sampling process. Each function shall be independant
 *        from the other and make sure the valves are correctely set
 *        See fluidic diagram for all details and understand the steps
 * @version 0.1
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <Arduino.h>
#include "Button.h"
#include "Valve_3_2.h"
#include "Valve_2_2.h"
#include "Pump.h"
#include "Motor.h"
#include "Encoder.h"
#include "Potentiometer.h"
#include "Led.h"
#include <SPI.h>
#include "C_output.h"
#include "Settings.h"
#include "Critical_error.h"
#include "Timer.h"
#include "Serial_device.h"
#include "Step_functions.h"
#include "Manifold.h"
#include "Micro_pump.h"
#include "Flow_sensor.h"
#include "Oring_lock.h"

// Delay when actuating valves
#define DELAY_ACTIONS 1000

extern C_output output;
extern Serial_device serial;
extern Led status_led;
extern Led green_led;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_manifold;
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
extern Manifold manifold;
extern Micro_Pump micro_pump;

extern Flow_sensor flow_sensor_small;
extern Flow_sensor flow_sensor_big;

extern float read_pressure2(); // main.cpp - the pressure sensor currently backing pump control (see its comment there for why)

/**
 * @brief Moves the manifold to `slot` for a production step that will pump through
 * it (water or air) - unlocks the o-ring first if currently locked (required
 * before any rotation), rotates, then re-locks once aligned. This is the "lock for
 * the whole slot dwell" model: the o-ring stays compressed for as long as the
 * manifold sits at a given slot (whether pumping water or just air-emptying), and
 * is only backed off right when actually leaving that slot. Use this instead of a
 * bare rotateMotor() for any slot change in the sampling/purge/DNA-shield sequence.
 */
void goto_slot_locked(int slot)
{
    unlock_oring(); // no-op if already unlocked
    rotateMotor(slot);
    lock_oring();
}

/**
 * @brief Unroll the spool at the correct depth
 *
 * @param _depth Absolute depth in centimeters at which to go.
 */
void step_dive(int _depth)
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - skipping dive");
        return;
    }

    if (VERBOSE_DIVE)
    {
        output.println("Step dive started");
    }

    valve_23.set_L_way();
    valve_1.set_open_way(); // let air escape the system while diving
    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();

    spool.set_speed(SPEED_DOWN, down);
    spool.start(_depth);

    if (VERBOSE_DIVE)
    {
        output.println("Time to dive : " + String(millis() - time1) + " ms");
    }

    valve_1.set_close_way();

    if (VERBOSE_DIVE)
    {
        output.println("Step dive ended");
    }
}

// ! Useless function now
/**
 * @brief Fill with water the container. Step_dive first.
 *
 */
void step_fill_container()
{
    if (VERBOSE_FILL_CONTAINER)
    {
        output.println("Step fill container started");
    }

    valve_1.set_close_way();
    valve_23.set_I_way();
    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();
    bool run = true;

    // * useless for now, only when pumping to the manifold
    // flow_sensor_small.reset_values();
    // flow_sensor_small.activate();

    // flow_sensor_big.reset_values();
    // flow_sensor_big.activate();

    // uint32_t last_flow_read = millis();

    // pump.set_power(POWER_PUMP);
    pump.set_power(100);
    pump.start();

    // two possibilities to stop filling : switch or time
    // read flow every second
    do
    {
        if (button_container.getState() == 0)
        {
            run = false;
            if (VERBOSE_FILL_CONTAINER)
            {
                output.println("Fill container stopped by button");
            }
        }
        // TODO : better function. cannot be constant time because time depends of how deep we sample
        // --> water need to go through the spool tube before entering the system
        else if (millis() - time1 > FILL_CONTAINER_TIME)
        {
            run = false;
            if (VERBOSE_FILL_CONTAINER)
            {
                output.println("Fill container stopped by security timer");
            }
            // TODO : raise a system warning to user
        }
        // * useless so far because it is not on the pipe to container
        // // reading flowmeters
        // if (millis() - last_flow_read > 1000){
        //     flow_sensor_small.update();
        //     flow_sensor_big.update();

        //     if (true){  // add condition if want to print flow rate
        //         Serial.print("Flowrate small : ");
        //         Serial.println(flow_sensor_small.get_flowRate());
        //         Serial.print("Flowrate big : ");
        //         Serial.println(flow_sensor_big.get_flowRate());
        //     }

        //     last_flow_read = millis();
        // }
    } while (run); // conditions are ouside loop to print what condition is responible for stopping

    pump.stop();

    if (VERBOSE_FILL_CONTAINER || TIMER)
    {
        output.println("Time to fill container : " + String(millis() - time1) + " ms");
    }

    delay(DELAY_ACTIONS);
    valve_23.set_L_way();

    if (VERBOSE_FILL_CONTAINER)
    {
        output.println("Step fill container ended");
    }

    // printing volume pumped
    Serial.print("Total milliliters small : ");
    Serial.println(flow_sensor_small.get_totalFlowMilliL());
    Serial.print("Total milliliters big : ");
    Serial.println(flow_sensor_big.get_totalFlowMilliL());
}

/**
 * @brief Empty the container through purge channel. Step_fill_container first.
 * Can be stopped manually with buttons
 * @param stop_pressure if true, will stop purge when pressure is low enough
 */
void step_purge(bool stop_pressure)
{
    if (VERBOSE_PURGE)
    {
        output.println("Step purge started");
    }

    // set the valves and manifold - flows through an open tube (no filter), but the
    // same rotary o-ring seal still needs to be locked for this slot before pumping
    goto_slot_locked(PURGE_SLOT);
    valve_1.set_close_way();
    valve_manifold.set_open_way(); // enable connection to deployment

    delay(DELAY_ACTIONS);

    CtrlPumpFlow ctrl_flow;
    ctrl_flow.begin(&pump, &pump_pid, read_pressure2, &flow_sensor_small, PURGE_MILLILITERS, 2, true);
    ctrl_flow.set_max_runtime(3 * 60 * 1000); // in ms

    uint32_t time1 = millis();
    // purge with certain flow
    if (VERBOSE_PURGE)
    {
        output.println("ctrl flow started");
    }
    ctrl_flow.run();

    valve_1.set_close_way();
    valve_manifold.set_close_way(); // default position of valves

    if (VERBOSE_PURGE || TIMER)
    {
        output.println("Time to purge : " + String(millis() - time1) + " ms");
    }

    if (VERBOSE_PURGE)
    {
        output.println("Step purge ended");
    }
}

/**
 * @brief Empty water from container into choosen filter. Step_fill_container first.
 *
 * @param slot_manifold The filter in which the sampling is made
 */
void step_sampling(int slot_manifold, bool stop_pressure)
{
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample through filter started");
    }

    if (VERBOSE_SAMPLE && !stop_pressure)
    {
        output.println("Press right button when container is empty -> will stop sampling sterivex");
    }

    // set the valves
    goto_slot_locked(slot_manifold);
    valve_1.set_close_way();
    // * new setup, connection to deployment
    valve_manifold.set_open_way();

    delay(DELAY_ACTIONS);

    // sample certain amount
    // todo: detect when pressure is below certain threshhold == sterivex saturated
    CtrlPumpFlow ctrl_flow;
    ctrl_flow.begin(&pump, &pump_pid, read_pressure2, &flow_sensor_small, STX_SAMPLE_MILLILITERS, 2, true);
    ctrl_flow.set_max_runtime(3 * 60 * 1000); // in ms

    flow_sensor_small.reset_values();

    CtrlPumpNoWater ctrl_empty; // here we want to empty sterivex
    ctrl_empty.begin(&pump, &pump_pid, read_pressure2, 0.4, true);
    ctrl_empty.set_end_cond_time(5000);
    ctrl_empty.set_max_runtime(60 * 1000); // in ms

    uint32_t time1 = millis();

    // purge with certain flow
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample: pumping water through sterivex");
    }
    ctrl_flow.run();

    // emptying the tubes
    valve_1.set_open_way(); // here we want to empty the system, to take air instead of water
    // ! need to change name
    valve_manifold.set_close_way(); // * closing connection to deployment
    goto_slot_locked(PURGE_SLOT);   // unlocks (with pressure-settle wait) before leaving the filter, re-locks at purge
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample: emptying through purge");
    }
    ctrl_empty.run();

    // emptying the sterivex filter
    goto_slot_locked(slot_manifold); // unlocks (with pressure-settle wait) before leaving purge, re-locks back at the filter
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample: emptying sterivex");
    }

    ctrl_empty.set_pressure_thresh(EMPTY_WATER_PRESSURE_STX_THRESHOLD);
    ctrl_empty.run(); // emptying sterivex
    valve_1.set_close_way();

    if (VERBOSE_SAMPLE || TIMER)
    {
        output.println("Time to sample water : " + String(millis() - time1) + " ms");
    }

    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample through filter ended");
    }
}

/**
 * @brief Loads one sample at the manifold's CURRENT slot: locks the o-ring, pumps
 * water through the filter (stopping on target volume, or the pump's hard pressure
 * cap - see Pump::set_pressure_safety(), wired up in main.cpp), pumps air through
 * to empty the filter, then unlocks. Precondition: the manifold is already
 * positioned at the target slot and the o-ring is unlocked - this does NOT rotate
 * the manifold itself (use rotateMotor()/goto_slot_locked() before calling).
 * Standalone building block for manual testing (see test_load_sample() and
 * test_multi_sample_loading() in Tests.cpp) - not currently called from
 * step_sampling(), which has its own more elaborate purge-then-filter double
 * air-emptying for the full production sequence.
 * @param volume_ml target water volume to pump through, in mL.
 * @return true if the o-ring locked successfully, false if lock_oring() failed (in
 * which case nothing else runs).
 */
bool load_slot(float volume_ml)
{
    if (!lock_oring())
    {
        return false;
    }

    // --- pump water through the filter ---
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(DELAY_ACTIONS);

    CtrlPumpFlow ctrl_flow;
    ctrl_flow.begin(&pump, &pump_pid, read_pressure2, &flow_sensor_small, volume_ml, 2, true);
    ctrl_flow.set_max_runtime(3 * 60 * 1000); // in ms
    if (VERBOSE_SAMPLE)
    {
        output.println("load_slot: pumping water through the filter");
    }
    ctrl_flow.run();

    // --- pump air through to empty the filter ---
    valve_1.set_open_way();         // take ambient air instead of water
    valve_manifold.set_close_way(); // close the deployment line
    delay(DELAY_ACTIONS);

    CtrlPumpNoWater ctrl_empty;
    ctrl_empty.begin(&pump, &pump_pid, read_pressure2, 0.4, true); // low PID setpoint while emptying
    ctrl_empty.set_end_cond_time(5000);
    ctrl_empty.set_max_runtime(60 * 1000); // in ms
    ctrl_empty.set_pressure_thresh(EMPTY_WATER_PRESSURE_STX_THRESHOLD); // stop when pressure drops below this (air detected)
    if (VERBOSE_SAMPLE)
    {
        output.println("load_slot: pumping air through to empty the filter");
    }
    ctrl_empty.run();

    valve_1.set_close_way();
    unlock_oring();
    return true;
}

/**
 * @brief Loads one sample at the manifold's CURRENT slot using the FULL production
 * sequence: step_sampling() (locks, pumps water to STX_SAMPLE_MILLILITERS or the
 * pump's hard pressure cap, then the elaborate purge-then-filter double
 * air-emptying) followed by step_DNA_shield() (pushes DNA-shield preservative into
 * the filter). This is exactly what sample_process() runs per sample - it's the
 * building block for testing the real end-to-end procedure, as opposed to
 * load_slot()'s simplified single-pass version for quick manual/leak checks.
 *
 * Precondition: manifold already positioned at slot_manifold, o-ring unlocked.
 * step_sampling()/step_DNA_shield() leave the o-ring locked (at PURGE_SLOT, by the
 * time step_DNA_shield() finishes) - this unlocks it at the end so the machine is
 * left idle/rotatable, matching load_slot()'s postcondition.
 * @param slot_manifold the slot to load (must match wherever the manifold already is).
 */
void load_slot_full(int slot_manifold)
{
    step_sampling(slot_manifold);
    step_DNA_shield(slot_manifold);
    unlock_oring();
}

/**
 * @brief Roll the spool back. Step_dive first.
 *
 */
void step_rewind()
{
    if (!SPOOL_USE)
    {
        output.println("Spool disabled (SPOOL_USE=false in Settings.h) - skipping rewind");
        return;
    }

    if (VERBOSE_REWIND)
    {
        output.println("Step rewind started");
    }

    // connection from air inlet to deployment, it will empty partly the tubes
    valve_1.set_open_way(); // let air enter the system
    valve_manifold.set_open_way();

    // the remaining water will pe pumped with the empty() step

    delay(DELAY_ACTIONS);

    uint32_t time1 = millis();

    // ! 11.08: is ok to do with broken encoder, as it only uses the UP button
    spool.set_speed(SPEED_UP, up);
    // go to origin
    spool.start(-1);

    if (VERBOSE_REWIND)
    {
        output.println("Time to rewind : " + String(millis() - time1) + " ms");
    }

    delay(DELAY_ACTIONS);
    valve_1.set_close_way();

    if (VERBOSE_REWIND)
    {
        output.println("Step rewind ended");
    }
}

/**
 * @brief empties the deployment module and system, call at the end of sample
 */
void step_empty()
{

    // pump from deployment
    valve_1.set_close_way();
    valve_manifold.set_open_way(); // * new setup
    delay(DELAY_ACTIONS);

    // do it through the purge slot, not the sterivex
    goto_slot_locked(PURGE_SLOT);

    CtrlPumpNoWater pump_ctrl;
    pump_ctrl.begin(&pump, &pump_pid, read_pressure2, 2, false);
    pump_ctrl.set_end_cond_time(5000);
    pump_ctrl.set_max_runtime(EMPTY_DEPLOYMENT_TIME); // in ms

    // ! should add a security time, as no water at pressure sensor doesn't mean no water in system

    pump_ctrl.run();

    valve_manifold.set_close_way();
}

/**
 * @brief Function to call to perform a sample
 *
 * @param depth: depth in centimeters
 * @param manifold_slot: slot in the manifold to use. If -1, will search for available slot
 */
void sample_process(int depth, int manifold_slot)
{
    // Verify if available filter
    bool filter_available = false;
    if (manifold_slot == -1)
    { // if no slot is given, search FILL_ORDER for the next available slot
        for (uint8_t i = 0; i < 14; i++)
        {
            int candidate = FILL_ORDER[i];
            if (manifold.get_state(candidate) == available)
            {
                manifold.change_state(candidate, unaivailable);
                filter_available = true;
                manifold_slot = candidate;
                break;
            }
        }
    }
    else
    {
        if (manifold.get_state(manifold_slot) == available)
        {
            manifold.change_state(manifold_slot, unaivailable);
            filter_available = true;
        }
    }

    if (filter_available == false)
    {
        output.println("No filter available");
        return;
    }

    uint32_t time_sampling = millis();

    step_rewind();
    set_system_state(state_sampling);
    if (VERBOSE_SAMPLE)
    {
        output.println("It's sampling time !");
    }
    if (VERBOSE_SAMPLE)
    {
        output.println("Sample started at depth " + String(depth) + "cm in filter ");
    }
    // Sampling steps
    delay(DELAY_ACTIONS);

    // ! ------ temporarly disabled
    // step_dive(depth);
    Serial.println("Should dive to depth " + String(depth) + "cm, but is temporarly disabled");

    delay(DELAY_ACTIONS);
    step_purge(); // maybe add param to tell how many miliL

    delay(DELAY_ACTIONS);
    step_sampling(manifold_slot); // sample place is a human number, start at 1

    delay(DELAY_ACTIONS);
    step_rewind();

    // adding DNA-shield to sterivex
    delay(DELAY_ACTIONS);
    step_DNA_shield(manifold_slot);

    // emptying system
    if (VERBOSE_SAMPLE)
    {
        output.println("Last step: emptying system from water");
    }
    delay(DELAY_ACTIONS);
    step_empty();

    // rest state: don't leave the o-ring compressed once the whole cycle is done
    unlock_oring();

    // closing all valves
    valve_1.set_close_way();
    valve_23.set_off();
    valve_manifold.set_close_way();

    if (VERBOSE_SAMPLE || TIMER)
    {
        output.println("Time for complete sample : " + String(millis() - time_sampling) + " ms");
    }
}

void demo_sample_process()
{
    // Verify if available filter
    bool filter_available = false;
    int manifold_slot = 0;
    for (uint8_t i = 0; i < 14; i++)
    {
        int candidate = FILL_ORDER[i];
        if (manifold.get_state(candidate) == available)
        {
            manifold.change_state(candidate, unaivailable);
            filter_available = true;
            manifold_slot = candidate;
            break;
        }
    }

    if (filter_available == false)
    {
        output.println("No filter available");
        return;
    }

    uint32_t time_sampling = millis();

    set_system_state(state_sampling);
    if (VERBOSE_SAMPLE)
    {
        output.println("It's sampling time !");
    }
    if (VERBOSE_SAMPLE)
    {
        output.println("Sample started");
    }

    // ?
    green_led.on();

    // Sampling steps
    button_start.waitPressedAndReleased();

    status_led.on(); // ?
    step_purge();
    status_led.off(); // ?

    button_start.waitPressedAndReleased();

    status_led.on();              // ?
    step_sampling(manifold_slot); // sample place is a human number, start at 1
    status_led.off();             // ?

    button_start.waitPressedAndReleased();
    status_led.on(); // ?
    step_DNA_shield(manifold_slot);
    status_led.off(); // ?

    // empty deployment module
    // button_start.waitPressedAndReleased();
    // step_empty();

    // rest state: don't leave the o-ring compressed once the whole cycle is done
    unlock_oring();

    if (VERBOSE_SAMPLE || TIMER)
    {
        output.println("Time for complete sample : " + String(millis() - time_sampling) + " ms");
    }

    // closing all valves
    valve_1.set_close_way();
    valve_23.set_off();
    valve_manifold.set_close_way();
}

void step_DNA_shield(int slot_manifold)
{
    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield started");
    }

    // go to right slot - NOT wrapped with goto_slot_locked(): step_sampling() already
    // leaves the o-ring locked at this exact slot, so this rotateMotor() is normally a
    // no-op (encoder is already at the target angle). If this function is ever called
    // standalone/out of that sequence, lock_oring() first.
    rotateMotor(slot_manifold);

    micro_pump.start(FILL_STERIVEX_TIME / 10.); // ! remove /100, only to be faster for testing

    delay(500);

    // only depending on architecture
    valve_manifold.set_close_way();
    valve_1.set_open_way(); // push shield with air

    pump.set_power(PUMP_SHIELD_POWER);

    delay(500);
    pump.start(PUMP_SHIELD_TIME);
    delay(500);

    // avoid suction of shield when opening valves
    goto_slot_locked(PURGE_SLOT); // unlocks (with pressure-settle wait) before leaving the filter, re-locks at purge

    valve_1.set_close_way();

    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield finished");
    }
}

void DNA_shield_test(int slot_manifold)
{
    // ! need to leave water in pipe

    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield started");
    }

    // go to right slot
    rotateMotor(slot_manifold);

    micro_pump.start(FILL_STERIVEX_TIME);
}

void abort_sample()
{
    // emptying container
    step_purge();
    // rewinding hose
    step_rewind();
    // emptying the deplyoment module
    step_empty();

    // rest state: make sure an abort never leaves the o-ring compressed
    unlock_oring();
}