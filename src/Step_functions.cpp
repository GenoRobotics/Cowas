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
bool goto_slot_locked(int slot)
{
    // FW2: each move can be refused (lock state UNKNOWN) or fail (encoder, timeout).
    // Stop at the first failure: never lock and pump at the wrong slot.
    if (!unlock_oring()) // no-op if already unlocked
    {
        return false;
    }
    if (!rotateMotor(slot))
    {
        output.println("ERROR | goto_slot_locked: rotation to slot " + String(slot) + " failed - O-ring left unlocked");
        return false;
    }
    return lock_oring();
}

/**
 * @brief FW2: gate for every procedure that needs the pressure reading (PID loops or the
 * pump cap). While PRESSURE_SENSOR_AVAILABLE is false (F50: no sensor installed), it
 * refuses with a clear message and the caller must not move anything.
 */
bool require_pressure_sensor(const char *who)
{
    if (PRESSURE_SENSOR_AVAILABLE)
    {
        return true;
    }
    output.println(String("REFUSED | ") + who + ": needs the pressure sensor (PRESSURE_SENSOR_AVAILABLE = false in Settings.h, F50). "
                   "Use the sensorless test commands (leak_hold, multi_sample, shield_only) - `help`");
    return false;
}

/**
 * @brief FW2: what a procedure does when a step fails: pumps off, valves closed; the
 * O-ring is left as it is (safe state, ROADMAP §3.2).
 */
static void stop_procedure(const char *who)
{
    pump.stop();
    micro_pump.stop();
    valve_1.set_close_way();
    valve_manifold.set_close_way();
    output.println(String("ERROR | ") + who + ": stopped - pumps off, valves closed, O-ring " + oring_lock_state_name());
}

/**
 * @brief Like goto_slot_locked(), but with no motion at all when the O-ring is already
 * locked at `slot` (encoder within ACTUATOR_HOME_SLOT_TOLERANCE_DEG). step_DNA_shield()
 * runs right after step_sampling() locked the same slot; the old bare rotateMotor() there
 * would now be refused while LOCKED.
 */
static bool ensure_locked_at_slot(int slot)
{
    float error;
    if (is_oring_locked() && manifold_slot_error(slot, &error) && fabsf(error) <= ACTUATOR_HOME_SLOT_TOLERANCE_DEG)
    {
        return true;
    }
    return goto_slot_locked(slot);
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
    if (!require_pressure_sensor("step_fill_container"))
    {
        return;
    }
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
bool step_purge(bool stop_pressure)
{
    if (!require_pressure_sensor("step_purge"))
    {
        return false;
    }
    if (VERBOSE_PURGE)
    {
        output.println("Step purge started");
    }

    // set the valves and manifold - flows through an open tube (no filter), but the
    // same rotary o-ring seal still needs to be locked for this slot before pumping
    if (!goto_slot_locked(PURGE_SLOT))
    {
        stop_procedure("step_purge");
        return false;
    }
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
    return true;
}

/**
 * @brief Empty water from container into choosen filter. Step_fill_container first.
 *
 * @param slot_manifold The filter in which the sampling is made
 */
bool step_sampling(int slot_manifold, bool stop_pressure, float volume_ml)
{
    if (!require_pressure_sensor("step_sampling"))
    {
        return false;
    }
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample through filter started");
    }

    if (VERBOSE_SAMPLE && !stop_pressure)
    {
        output.println("Press right button when container is empty -> will stop sampling sterivex");
    }

    // set the valves
    if (!goto_slot_locked(slot_manifold))
    {
        stop_procedure("step_sampling");
        return false;
    }
    valve_1.set_close_way();
    // * new setup, connection to deployment
    valve_manifold.set_open_way();

    delay(DELAY_ACTIONS);

    // sample certain amount
    // todo: detect when pressure is below certain threshhold == sterivex saturated
    CtrlPumpFlow ctrl_flow;
    ctrl_flow.begin(&pump, &pump_pid, read_pressure2, &flow_sensor_small, volume_ml, 2, true);
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
    if (!goto_slot_locked(PURGE_SLOT)) // unlocks before leaving the filter, re-locks at purge
    {
        stop_procedure("step_sampling");
        return false;
    }
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample: emptying through purge");
    }
    ctrl_empty.run();

    // emptying the sterivex filter
    if (!goto_slot_locked(slot_manifold)) // unlocks before leaving purge, re-locks back at the filter
    {
        stop_procedure("step_sampling");
        return false;
    }
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
    return true;
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
    if (!require_pressure_sensor("load_slot"))
    {
        return false;
    }
    if (!lock_oring())
    {
        return false;
    }

    load_slot_water(volume_ml);
    load_slot_air();
    unlock_oring();
    return true;
}

/**
 * @brief The water phase of load_slot() (split out in FW2 so leak_hold can pause between
 * the two phases, same code as before): pumps volume_ml through the CURRENT slot with the
 * PID loop. The caller has locked the O-ring and checked require_pressure_sensor().
 */
void load_slot_water(float volume_ml)
{
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
}

/**
 * @brief The air phase of load_slot(): pumps air through the CURRENT slot to empty the
 * filter, then closes V1. Same preconditions as load_slot_water().
 */
void load_slot_air()
{
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
bool load_slot_full(int slot_manifold)
{
    if (!require_pressure_sensor("load_slot_full"))
    {
        return false;
    }
    bool ok = step_sampling(slot_manifold) && step_DNA_shield(slot_manifold);
    unlock_oring();
    return ok;
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
bool step_empty()
{
    if (!require_pressure_sensor("step_empty"))
    {
        return false;
    }

    // pump from deployment
    valve_1.set_close_way();
    valve_manifold.set_open_way(); // * new setup
    delay(DELAY_ACTIONS);

    // do it through the purge slot, not the sterivex
    if (!goto_slot_locked(PURGE_SLOT))
    {
        stop_procedure("step_empty");
        return false;
    }

    CtrlPumpNoWater pump_ctrl;
    pump_ctrl.begin(&pump, &pump_pid, read_pressure2, 2, false);
    pump_ctrl.set_end_cond_time(5000);
    pump_ctrl.set_max_runtime(EMPTY_DEPLOYMENT_TIME); // in ms

    // ! should add a security time, as no water at pressure sensor doesn't mean no water in system

    pump_ctrl.run();

    valve_manifold.set_close_way();
    return true;
}

/**
 * @brief Function to call to perform a sample
 *
 * @param depth: depth in centimeters
 * @param manifold_slot: slot in the manifold to use. If -1, will search for available slot
 */
void sample_process(int depth, int manifold_slot)
{
    // FW2 (user-approved 2026-09-25): refuse before claiming a slot if anything the cycle
    // needs is missing, and stop at the first failed step instead of carrying on.
    if (!require_pressure_sensor("sample_process"))
    {
        return;
    }
    if (oring_lock_state() == core::LOCK_STATE_UNKNOWN)
    {
        output.println("REFUSED | sample_process: O-ring lock state is UNKNOWN - run actuator_home first");
        return;
    }

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
    if (!step_purge()) // maybe add param to tell how many miliL
    {
        output.println("ERROR | sample_process: purge failed - cycle stopped, slot " + String(manifold_slot) + " stays marked used");
        set_system_state(state_error);
        return;
    }

    delay(DELAY_ACTIONS);
    if (!step_sampling(manifold_slot)) // sample place is a human number, start at 1
    {
        output.println("ERROR | sample_process: sampling failed - cycle stopped, slot " + String(manifold_slot) + " is suspect");
        set_system_state(state_error);
        return;
    }

    delay(DELAY_ACTIONS);
    step_rewind();

    // adding DNA-shield to sterivex
    delay(DELAY_ACTIONS);
    if (!step_DNA_shield(manifold_slot))
    {
        output.println("ERROR | sample_process: DNA shield failed - cycle stopped, slot " + String(manifold_slot) + " is suspect");
        set_system_state(state_error);
        return;
    }

    // emptying system
    if (VERBOSE_SAMPLE)
    {
        output.println("Last step: emptying system from water");
    }
    delay(DELAY_ACTIONS);
    if (!step_empty())
    {
        output.println("ERROR | sample_process: emptying failed - cycle stopped (the filter itself is done)");
        set_system_state(state_error);
        return;
    }

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
    if (!require_pressure_sensor("demo_sample_process"))
    {
        return;
    }
    if (oring_lock_state() == core::LOCK_STATE_UNKNOWN)
    {
        output.println("REFUSED | demo_sample_process: O-ring lock state is UNKNOWN - run actuator_home first");
        return;
    }

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
    bool ok = step_purge();
    status_led.off(); // ?

    if (ok)
    {
        button_start.waitPressedAndReleased();
        status_led.on();                   // ?
        ok = step_sampling(manifold_slot); // sample place is a human number, start at 1
        status_led.off();                  // ?
    }

    if (ok)
    {
        button_start.waitPressedAndReleased();
        status_led.on(); // ?
        ok = step_DNA_shield(manifold_slot);
        status_led.off(); // ?
    }

    if (!ok)
    {
        output.println("ERROR | demo_sample_process: a step failed - demo stopped");
        return;
    }

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

bool step_DNA_shield(int slot_manifold)
{
    if (!require_pressure_sensor("step_DNA_shield")) // the air push uses the big pump (cap)
    {
        return false;
    }
    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield started");
    }

    // go to right slot: step_sampling() normally leaves the o-ring locked at this exact
    // slot, so this does not move anything. Called standalone, it unlocks, rotates, locks.
    if (!ensure_locked_at_slot(slot_manifold))
    {
        stop_procedure("step_DNA_shield");
        return false;
    }

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
    if (!goto_slot_locked(PURGE_SLOT)) // unlocks before leaving the filter, re-locks at purge
    {
        stop_procedure("step_DNA_shield");
        return false;
    }

    valve_1.set_close_way();

    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield finished");
    }
    return true;
}

void DNA_shield_test(int slot_manifold)
{
    // ! need to leave water in pipe

    if (VERBOSE_SHIELD)
    {
        output.println("Step DNA-shield started");
    }

    // go to right slot
    if (!rotateMotor(slot_manifold))
    {
        return;
    }

    micro_pump.start(FILL_STERIVEX_TIME);
}

void abort_sample()
{
    if (!require_pressure_sensor("abort_sample"))
    {
        stop_procedure("abort_sample"); // what can be done without pumping
        return;
    }
    // emptying container
    if (!step_purge())
    {
        output.println("ERROR | abort_sample: purge failed - abort stopped");
        return;
    }
    // rewinding hose
    step_rewind();
    // emptying the deplyoment module
    if (!step_empty())
    {
        output.println("ERROR | abort_sample: emptying failed - abort stopped");
        return;
    }

    // rest state: make sure an abort never leaves the o-ring compressed
    unlock_oring();
}