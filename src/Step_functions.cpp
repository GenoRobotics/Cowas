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
#include "Step_functions.h"
#include "Manifold.h"
#include "Micro_pump.h"
#include "Flow_sensor.h"
#include "Oring_lock.h"
#include "Run_keys.h"
#include "Core/sample_record.h"

// Delay when actuating valves
#define DELAY_ACTIONS 1000

extern C_output output;
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

extern float read_p_in(); // main.cpp - the pressure sensor backing pump control and the cap: the ABP on SPI (S1-1)

/**
 * @brief Moves the manifold to `slot` for a operation step that will pump through
 * it (water or air) - unlocks the o-ring first if currently locked (required
 * before any rotation), rotates, then re-locks once aligned. This is the "lock for
 * the whole slot dwell" model: the o-ring stays compressed for as long as the
 * manifold sits at a given slot (whether pumping water or just air-emptying), and
 * is only backed off right when actually leaving that slot. Use this instead of a
 * bare rotateMotor() for any slot change in the sampling/purge/DNA-shield sequence.
 */
bool goto_slot_locked(int slot)
{
    // rotateMotor() unlocks for the turn and leaves the O-ring locked again (rest state).
    // FW2: a refusal (lock state UNKNOWN) or a failure (encoder, timeout) stops here.
    if (!rotateMotor(slot))
    {
        output.println("ERROR | goto_slot_locked: rotation to slot " + String(slot) + " failed");
        return false;
    }
    return oring_go_to(core::LOCK_STATE_LOCKED);
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
 * @brief S1-C addendum item 1: x typed during a pump run latches an abort. Every step checks it at its boundaries, so the
 * command stops at the next one (the step's own pump run already stopped). Returns true if the procedure must stop now.
 */
static bool aborted_stop(const char *who)
{
    if (!run_abort_latched())
    {
        return false;
    }
    stop_procedure(who);
    output.println(String("ABORT | ") + who + ": the command was aborted with x");
    return true;
}

/**
 * @brief Waits ms, polling the console keys every 100 ms, so x or stop still ends a vent. False = abort latched.
 */
static bool vent_wait(uint32_t ms)
{
    const uint32_t start = millis();
    while (millis() - start < ms)
    {
        poll_run_key();
        if (run_abort_latched())
        {
            return false;
        }
        delay(100);
    }
    return true;
}

/**
 * @brief F192, vent redone 2026-10-23 (user): the line drops by itself with the pump off and the valves shut for
 * PRESSURE_DECAY_WAIT_MS; then the air vent V1 is open for PRESSURE_AIR_VENT_MS and shut again. Nothing here stops the
 * sample for the pressure: the O-ring unlock goes on whatever the readings (the readings are logged). Only a faulty
 * sensor read or an abort returns false, and the caller stops with the O-ring LOCKED.
 */
static bool vent_line(const char *who, uint32_t pressure_decay_ms = PRESSURE_DECAY_WAIT_MS)
{
    pump.stop();
    valve_1.set_close_way();
    valve_manifold.set_close_way();
    output.println(String("Vent: pump off, V1 and Vman closed for ") + String(pressure_decay_ms / 1000) +
                   " s, then air vent V1 open for " + String(PRESSURE_AIR_VENT_MS / 1000) + " s");

    const float p_before = read_p_in();
    if (p_before == PRESSURE_SENSOR_INVALID_BAR)
    {
        output.println(String("ERROR | ") + who + ": pressure sensor invalid before the vent - O-ring stays LOCKED");
        return false;
    }
    if (!vent_wait(PRESSURE_DECAY_WAIT_MS))
    {
        return false;
    }

    const float p_waited = read_p_in();
    if (p_waited == PRESSURE_SENSOR_INVALID_BAR)
    {
        output.println(String("ERROR | ") + who + ": pressure sensor invalid after the wait - O-ring stays LOCKED");
        return false;
    }

    valve_1.set_open_way();
    const bool vented = vent_wait(PRESSURE_AIR_VENT_MS);
    valve_1.set_close_way();
    if (!vented)
    {
        return false;
    }

    const float p_vented = read_p_in();
    if (p_vented == PRESSURE_SENSOR_INVALID_BAR)
    {
        output.println(String("ERROR | ") + who + ": pressure sensor invalid after the vent - O-ring stays LOCKED");
        return false;
    }
    output.println(String("Vent: ") + String(p_before, 2) + " bar before, " + String(p_waited, 2) + " bar after the wait, " +
                   String(p_vented, 2) + " bar after the air vent" +
                   (p_vented < PRESSURE_RELEASE_BAR ? " (released)" : " (NOT released below " + String(PRESSURE_RELEASE_BAR, 1) +
                                                                       " bar: going on, the sample is not stopped for this)"));
    return true;
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
        else if (pump.enforce_pressure_safety()) // hard cap, checked on every pass (2026-10-23 safety check)
        {
            run = false;
        }
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

// S1-C log (docs/software/code-guide.md §4, the sample record): one JSON line per purge or sample, printed when its pump run ends, so the stop
// reason and the volumes are written by the code that decided them. Update the version with each firmware step.
static const char *const SAMPLE_RECORD_FW = "S1-1b-dev";
static int sample_record_seq = 0; // id of the record in this boot: auto-1, auto-2, ... (the operator id is not in the firmware yet)

// The name of the sample being run (set by run_sample, 2026-10-23): it becomes the id of the sample record, so the JSON
// file carries the name. Empty = the id is the counter (auto-N). Purges always keep the counter.
static char sample_record_name[48] = "";

void set_sample_record_name(const char *name)
{
    snprintf(sample_record_name, sizeof(sample_record_name), "%s", name ? name : "");
}

// F199 (user, 2026-10-23): the session site and date and the sample condition, written in every record of the run_sample
// (purge and sample). Empty = not written. Set by run_sample before its run and cleared after it.
static char sample_meta_site[48] = "";
static char sample_meta_date[16] = "";
static char sample_meta_condition[48] = "";

void set_sample_record_meta(const char *site, const char *date, const char *condition)
{
    snprintf(sample_meta_site, sizeof(sample_meta_site), "%s", site ? site : "");
    snprintf(sample_meta_date, sizeof(sample_meta_date), "%s", date ? date : "");
    snprintf(sample_meta_condition, sizeof(sample_meta_condition), "%s", condition ? condition : "");
}

static void print_sample_record(const char *purpose, int slot, uint32_t start_ms, float target_ml, const CtrlPump &run,
                                const char *lock_before)
{
    // placeholders in use (CLAUDE.md, rule 5): listed on every record, so no record looks clean by accident (flow_uncal removed 2026-10-23, the flowmeter is calibrated)
    static const char *const flags[] = {"placeholder:ABP_FULL_SCALE_PSI", "placeholder:STX_MAX_PRESSURE"};
    const bool is_purge = strcmp(purpose, "purge") == 0;
    char id[64];
    if (!is_purge && sample_record_name[0] != '\0')
    {
        snprintf(id, sizeof(id), "%s", sample_record_name);
    }
    else
    {
        snprintf(id, sizeof(id), "auto-%d", ++sample_record_seq);
    }
    const float vol_ml = (float)flow_sensor_small.get_totalFlowMilliL();

    core::SampleRecord r;
    r.fw = SAMPLE_RECORD_FW;
    r.id = id;
    r.purpose = purpose;
    r.site = sample_meta_site[0] != ' ' ? sample_meta_site : NULL;
    r.date = sample_meta_date[0] != ' ' ? sample_meta_date : NULL;
    r.condition = sample_meta_condition[0] != ' ' ? sample_meta_condition : NULL;
    r.slot = slot;
    r.start_ms = start_ms;
    r.end_ms = millis();
    r.target_ml = target_ml;
    r.vol_ml = vol_ml;
    r.vol_trusted_ml = run.trusted_ml(); // the confident part of vol_ml (F150, 2026-10-23)
    r.purge_ml = is_purge ? vol_ml : NAN;
    r.end_reason = core::sample_end_name(run.end_reason());
    r.p_mean_bar = run.pressure_mean();
    r.p_max_bar = run.pressure_max();
    r.p_cap_bar = STX_MAX_PRESSURE;
    r.lock_before = lock_before;
    r.lock_after = oring_lock_state_name();
    r.shield_ml = NAN; // the shield pump is a separate step
    r.flow_factor = calibrationFactor_small;
    r.p_gain = PRESSURE_GAIN;
    r.p_offset_bar = PRESSURE_ZERO_OFFSET_BAR;
    r.abp_full_scale_psi = ABP_FULL_SCALE_PSI;
    r.linear_act_locked_steps = (int)LINEAR_ACT_LOCKED_STEPS;
    r.flags = flags;
    r.n_flags = (int)(sizeof(flags) / sizeof(flags[0]));

    char line[720];
    if (core::format_sample_record(r, line, sizeof(line)) < 0)
    {
        output.println("LOG ERROR: sample record did not fit, not written");
        return;
    }
    output.println(String(line));
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

    if (aborted_stop("step_purge"))
    {
        return false;
    }

    // S1-C addendum item 3: the purge volume follows the spool answer given in `session` (spool_installed_session).
    // User 2026-10-23: spool absent 500 mL, spool installed 7 L; 2026-10-26: spool installed 3 L.
    const float purge_ml = core::purge_volume_ml(spool_installed_session, PURGE_NO_SPOOL_ML, PURGE_SPOOL_ML);
    output.println("Step purge: " + String(purge_ml, 0) + " mL, spool " + (spool_installed_session ? "installed" : "absent"));

    const char *lock_before = oring_lock_state_name(); // for the sample record (S1-C log)

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
    ctrl_flow.begin(&pump, &pump_pid, read_p_in, &flow_sensor_small, purge_ml, OPERATION_TARGET_BAR, true);
    // the purge is the one water run at full power (PUMP_PURGE_MAX_POWER_PERCENT, user 2026-10-26); the 3 bar target and the cap stay
    ctrl_flow.set_full_power(true);
    // the purge timeout follows the spool answer (Settings.h): 13 min with the spool, 3 min without
    ctrl_flow.set_max_runtime(spool_installed_session ? PURGE_SPOOL_MAX_RUNTIME_MS : PURGE_MAX_RUNTIME_MS); // in ms

    uint32_t time1 = millis();
    // purge with certain flow
    if (VERBOSE_PURGE)
    {
        output.println("ctrl flow started");
    }
    ctrl_flow.run();
    last_purge_end = ctrl_flow.end_reason();
    print_sample_record("purge", PURGE_SLOT, time1, purge_ml, ctrl_flow, lock_before);

    valve_1.set_close_way();
    valve_manifold.set_close_way(); // default position of valves

    if (VERBOSE_PURGE || TIMER)
    {
        output.println("Time to purge : " + String(millis() - time1) + " ms (end=" + ctrl_flow.end_reason() + ")");
    }

    if (VERBOSE_PURGE)
    {
        output.println("Step purge ended");
    }
    return !aborted_stop("step_purge"); // a skip (s) is a normal end of this step; an abort (x) stops the command
}

/**
 * @brief Empty water from container into choosen filter. Step_fill_container first.
 *
 * @param slot_manifold The filter in which the sampling is made
 */
const char *last_sample_end = "none"; // why the last step_sampling() pumped stopped: volume, timeout, cap, sensor (S1-C)
const char *last_purge_end = "none";  // why the last step_purge() pumped stopped (S1-C)
float last_sample_trusted_ml = 0.0f;  // the confident flowmeter volume of the last step_sampling() pump run (F150, 2026-10-23)
const char *last_empty_end = "none";  // why the last step_empty() pumped stopped (S1-C)

bool step_sampling(int slot_manifold, bool stop_pressure, float volume_ml)
{
    if (!require_pressure_sensor("step_sampling"))
    {
        return false;
    }
    if (aborted_stop("step_sampling"))
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

    const char *lock_before = oring_lock_state_name(); // for the sample record (S1-C log)

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
    ctrl_flow.begin(&pump, &pump_pid, read_p_in, &flow_sensor_small, volume_ml, OPERATION_TARGET_BAR, true);
    ctrl_flow.set_max_runtime(SAMPLE_MAX_DURATION_MS); // in ms (S1-C: was 3 min, F15)
    FilterWatch filter_watch; // stall, clogged and pressure stops, the same as fill_test (S1-C)
    filter_watch.begin(FILL_DEFAULT_STALL_S, FILL_CLOG_FRACTION, (float)FILL_PRESSURE_STUCK_S, true);
    ctrl_flow.set_filter_watch(&filter_watch);
    // pause at the cap margin and resume after (S1-C, user 2026-10-19): the filter fills without overshooting the cap
    ctrl_flow.set_pause(FILL_PAUSE_BAR, FILL_RESUME_BAR);

    flow_sensor_small.reset_values();

    CtrlPumpNoWater ctrl_empty; // here we want to empty sterivex
    ctrl_empty.begin(&pump, &pump_pid, read_p_in, OPERATION_TARGET_BAR, true);
    ctrl_empty.set_end_cond_time(EMPTY_AIR_HOLD_MS);
    ctrl_empty.set_max_runtime(EMPTY_MAX_TIME_MS);

    uint32_t time1 = millis();

    // purge with certain flow
    if (VERBOSE_SAMPLE)
    {
        output.println("Step sample: pumping water through sterivex");
    }
    ctrl_flow.run();
    last_sample_trusted_ml = ctrl_flow.trusted_ml(); // for the TEST line (F150, 2026-10-23)
    last_sample_end = strcmp(ctrl_flow.end_reason(), "cond") == 0 ? "volume" : ctrl_flow.end_reason();
    output.println("Step sample ended: " + String(last_sample_end) + ", pauses " + String(ctrl_flow.pauses()));
    print_sample_record("sample", slot_manifold, time1, volume_ml, ctrl_flow, lock_before);
    if (aborted_stop("step_sampling"))
    {
        return false;
    }

    // F192 (user, 2026-10-23): wait, then a 5 s air vent. Not a gate for the sample: only a faulty sensor or an abort stops here
    if (!vent_line("step_sampling"))
    {
        stop_procedure("step_sampling");
        return false;
    }

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
    if (aborted_stop("step_sampling"))
    {
        return false;
    }

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
    ctrl_empty.set_end_cond_time(EMPTY_STX_AIR_HOLD_MS); // the sterivex air drying is longer than the purge one (user, 2026-10-23)
    ctrl_empty.set_max_runtime(EMPTY_MAX_TIME_MS + EMPTY_STX_EXTRA_MAX_MS);
    ctrl_empty.run(); // emptying sterivex
    valve_1.set_close_way();
    if (aborted_stop("step_sampling"))
    {
        return false;
    }

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
 * the old multi-sample test, removed 2026-10-23) - not currently called from
 * step_sampling(), which has its own more elaborate purge-then-filter double
 * air-emptying for the full operation sequence.
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
    if (aborted_stop("load_slot_water"))
    {
        return;
    }
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(DELAY_ACTIONS);

    CtrlPumpFlow ctrl_flow;
    ctrl_flow.begin(&pump, &pump_pid, read_p_in, &flow_sensor_small, volume_ml, OPERATION_TARGET_BAR, true);
    ctrl_flow.set_max_runtime(SAMPLE_MAX_DURATION_MS); // in ms (S1-C: was 3 min, F15)
    FilterWatch filter_watch; // stall, clogged and pressure stops, the same as fill_test (S1-C)
    filter_watch.begin(FILL_DEFAULT_STALL_S, FILL_CLOG_FRACTION, (float)FILL_PRESSURE_STUCK_S, true);
    ctrl_flow.set_filter_watch(&filter_watch);
    // pause at the cap margin and resume after (S1-C, user 2026-10-19): the filter fills without overshooting the cap
    ctrl_flow.set_pause(FILL_PAUSE_BAR, FILL_RESUME_BAR);
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
    if (aborted_stop("load_slot_air"))
    {
        return;
    }
    valve_1.set_open_way();         // take ambient air instead of water
    valve_manifold.set_close_way(); // close the deployment line
    delay(DELAY_ACTIONS);

    CtrlPumpNoWater ctrl_empty;
    ctrl_empty.begin(&pump, &pump_pid, read_p_in, OPERATION_TARGET_BAR, true);
    ctrl_empty.set_end_cond_time(EMPTY_AIR_HOLD_MS);
    ctrl_empty.set_max_runtime(EMPTY_MAX_TIME_MS);
    ctrl_empty.set_pressure_thresh(EMPTY_WATER_PRESSURE_STX_THRESHOLD); // stop when pressure drops below this (air detected)
    if (VERBOSE_SAMPLE)
    {
        output.println("load_slot: pumping air through to empty the filter");
    }
    ctrl_empty.run();

    valve_1.set_close_way();
}

/**
 * @brief Loads one sample at the manifold's CURRENT slot using the FULL operation
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
    if (aborted_stop("step_empty"))
    {
        return false;
    }
    const uint32_t t_empty = millis();

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
    pump_ctrl.begin(&pump, &pump_pid, read_p_in, OPERATION_TARGET_BAR, true); // printed: the 2026-10-19 trapped empty left no pressure record
    pump_ctrl.set_end_cond_time(EMPTY_AIR_HOLD_MS);
    pump_ctrl.set_max_runtime(EMPTY_MAX_TIME_MS); // in ms

    // ! should add a security time, as no water at pressure sensor doesn't mean no water in system

    pump_ctrl.run();
    last_empty_end = pump_ctrl.end_reason();
    valve_manifold.set_close_way();
    output.println(String("Step empty: end=") + pump_ctrl.end_reason() + ", " + String((millis() - t_empty) / 1000) + " s");
    return !aborted_stop("step_empty"); // a skip (s) ends this step normally; an abort (x) stops the command
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

    // the deployment-line emptying is no longer part of the cycle (user, 2026-10-19: 5 min is too long, and the cycle
    // ends back at the purge). step_empty() stays for abort_sample().

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

    // F156 (2026-10-19): the micro pump is time-based (no flow sensor), so the log shows the time it really ran. The volume is
    // not measured here: weigh the bottle, or use shield_calibrate (LA3). A skip (s) ends this pump run only; the air push below still runs.
    const uint32_t t_shield = millis();
    micro_pump.start(FILL_STERIVEX_TIME); // ! remove /100, only to be faster for testing
    output.println("Step DNA-shield: micro pump ran " + String(millis() - t_shield) + " ms (time-based, volume not measured)");
    if (aborted_stop("step_DNA_shield"))
    {
        return false;
    }

    delay(500);

    // only depending on architecture
    valve_manifold.set_close_way();
    valve_1.set_open_way(); // push shield with air

    pump.set_power(PUMP_SHIELD_POWER);

    delay(500);
    const uint32_t t_push = millis();
    pump.start(PUMP_SHIELD_TIME);
    // there is a 500ms delay to close the 24V valve of the pump before reaching the print
    output.println("Step DNA-shield: air push ran " + String(millis() - t_push - 500) + " ms at " + String(PUMP_SHIELD_POWER) + " %");
    delay(500);
    if (aborted_stop("step_DNA_shield"))
    {
        return false;
    }

    // also vent here, not only when sampling water. Pressure builds up withh the DNA shield. waiting lets it move into the filter
    if (!vent_line("step_DNA_shield", 40000))
    {
        stop_procedure("step_DNA_shield");
        return false;
    }

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

void abort_sample()
{
    if (!require_pressure_sensor("abort_sample"))
    {
        stop_procedure("abort_sample"); // what can be done without pumping
        return;
    }
    // emptying container. Not installed anymore
    // if (!step_purge())
    // {
    //     output.println("ERROR | abort_sample: purge failed - abort stopped");
    //     return;
    // }
    // -----------------

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