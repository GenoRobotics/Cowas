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
#include "Oring_lock.h"
#include "Core/clean_plan.h"
#include "Core/lock_rules.h"

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

// ---------------------------------------------------------------------------------------------------------------------
// Ethanol cleaning of the tubing (`ethanol_purge`, user 2026-11-02). 70 % ethanol at the inlet and outlet, NO filter in
// the manifold. 1) ethanol through the purge slot (prime: 3 L with the spool module, else 0.1 L), 2) 0.1 L through each
// slot that was used, in FILL_ORDER, 3) air through each of those slots to dry them (30 s), 4) a last air push at the purge
// slot (1 min). All air goes in by the air valve V1, like the drying of the filters (load_slot_air); the inlet is not used
// for air, so the tubes can stay in the ethanol. The water runs use full power: the second exception to the water limit.
// ---------------------------------------------------------------------------------------------------------------------

extern float read_p_in(); // main.cpp - the pressure sensor backing the pump control and the cap

static const uint32_t CLEAN_SETTLE_MS = 1000; // pause after a valve or a rotation, before the pump starts

enum CleanRun
{
    CLEAN_RUN_OK,   // the run ended as planned
    CLEAN_RUN_WARN, // not as planned, but harmless: the procedure goes on and says so at the end
    CLEAN_RUN_FAIL  // cap, sensor, abort, refusal...: the procedure stops
};

// One typed line (ENTER ends it). False = x typed (cancel) or nobody answered within 10 min.
static bool read_clean_answer(String &answer)
{
    const uint32_t start = millis();
    while (millis() - start < 600000UL)
    {
        if (Serial.available() <= 0)
        {
            delay(20);
            continue;
        }
        answer = Serial.readStringUntil('\n');
        answer.trim();
        return !answer.equalsIgnoreCase("x");
    }
    return false;
}

static void flush_console_input()
{
    while (Serial.available())
    {
        Serial.read();
    }
}

enum CleanAnswer
{
    CLEAN_ANSWER_YES,
    CLEAN_ANSWER_NO,
    CLEAN_ANSWER_CANCEL // x typed, or no answer in time
};

static CleanAnswer clean_ask_yes_no(const char *question)
{
    flush_console_input();
    Serial.println(String("ASK | ") + question + " (y/n, x = cancel)");
    String a;
    if (!read_clean_answer(a))
    {
        return CLEAN_ANSWER_CANCEL;
    }
    return (a.equalsIgnoreCase("y") || a.equalsIgnoreCase("yes")) ? CLEAN_ANSWER_YES : CLEAN_ANSWER_NO;
}

static void stop_clean(const char *why)
{
    stop_purge_manifold();
    Serial.println(String("ERROR | ethanol cleaning stopped: ") + why + " - pump off, valves closed, O-ring " + oring_lock_state_name());
}

static CleanRun judge_flush(const CtrlPump &run, const char *what, int slot)
{
    const char *r = run.end_reason();
    if (strcmp(r, "cond") == 0)
    {
        return CLEAN_RUN_OK;
    }
    if (strcmp(r, "skipped") == 0)
    {
        Serial.println(String("WARNING | ") + what + " slot " + String(slot) + ": skipped with s, less than the planned ethanol went through");
        return CLEAN_RUN_WARN;
    }
    Serial.println(String("ERROR | ") + what + " slot " + String(slot) + ": the pump run ended with " + r);
    return CLEAN_RUN_FAIL;
}

// Ethanol through `slot` until the flowmeter indicates `ml` mL, at full power. Deployment line open, air inlet shut.
static CleanRun clean_flush_slot(int slot, float ml, uint32_t max_ms, const char *what)
{
    if (!goto_slot_locked(slot))
    {
        return CLEAN_RUN_FAIL;
    }
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(CLEAN_SETTLE_MS);

    Serial.println(String("Ethanol ") + what + ": slot " + String(slot) + ", " + String(ml, 0) + " mL, full power");
    const uint32_t t0 = millis();
    pump_pid.reset(); // no integral left from the run before
    CtrlPumpFlow ctrl;
    ctrl.begin(&pump, &pump_pid, read_p_in, &flow_sensor_small, ml, OPERATION_TARGET_BAR, true);
    ctrl.set_full_power(true); // the exception, shown to the operator before the run
    ctrl.set_max_runtime(max_ms);
    ctrl.run();
    valve_manifold.set_close_way();
    Serial.println(String("Ethanol ") + what + ": slot " + String(slot) + " end=" + ctrl.end_reason() + ", " +
                   String((millis() - t0) / 1000) + " s, peak " + String(ctrl.pressure_max(), 2) + " bar");
    return judge_flush(ctrl, what, slot);
}

// Air through `slot` for at least `hold_ms` (the pressure must stay at air level that long): V1 open, Vman closed, as in
// load_slot_air() (the filter drying). Whatever is in the pump, V1 line and the slot is pushed out through the slot.
static CleanRun clean_air_slot(int slot, uint32_t hold_ms, const char *what)
{
    if (!goto_slot_locked(slot))
    {
        return CLEAN_RUN_FAIL;
    }
    valve_manifold.set_close_way(); // the deployment line stays shut: the ethanol bottle is not touched
    valve_1.set_open_way();         // take ambient air
    delay(CLEAN_SETTLE_MS);

    Serial.println(String("Air ") + what + ": slot " + String(slot) + ", " + String(hold_ms / 1000) + " s");
    const uint32_t t0 = millis();
    pump_pid.reset();
    CtrlPumpNoWater ctrl;
    ctrl.begin(&pump, &pump_pid, read_p_in, OPERATION_TARGET_BAR, true);
    ctrl.set_end_cond_time(hold_ms);
    ctrl.set_max_runtime(hold_ms + CLEAN_AIR_EXTRA_MAX_MS);
    ctrl.run();
    valve_1.set_close_way();
    valve_manifold.set_close_way();
    Serial.println(String("Air ") + what + ": slot " + String(slot) + " end=" + ctrl.end_reason() + ", " +
                   String((millis() - t0) / 1000) + " s");

    const char *r = ctrl.end_reason();
    if (strcmp(r, "cond") == 0)
    {
        return CLEAN_RUN_OK;
    }
    if (strcmp(r, "timeout") == 0 || strcmp(r, "skipped") == 0)
    {
        Serial.println(String("WARNING | air ") + what + " slot " + String(slot) + ": ended with " + r +
                       " (the pressure did not stay at air level for " + String(hold_ms / 1000) + " s): check the slot is dry");
        return CLEAN_RUN_WARN;
    }
    Serial.println(String("ERROR | air ") + what + " slot " + String(slot) + ": the pump run ended with " + r);
    return CLEAN_RUN_FAIL;
}

// The unused slots: from the command line (`ethanol_purge skip=3,7,12`) or asked. False = cancelled.
static bool ask_unused_slots(const char *args, bool *unused)
{
    String text = args ? String(args) : String("");
    text.trim();
    if (text.length() == 0)
    {
        // 2026-11-04 (user): ask whether ALL slots need cleaning; only on a no, ask which ones do NOT need it
        const CleanAnswer a = clean_ask_yes_no("Do all slots need cleaning?");
        if (a == CLEAN_ANSWER_CANCEL)
        {
            return false;
        }
        if (a == CLEAN_ANSWER_YES)
        {
            for (int i = 0; i < NB_SLOT; i++)
            {
                unused[i] = false;
            }
            return true;
        }
        Serial.println("ASK | Type the slot numbers that do NOT need cleaning, separated by commas or spaces (ranges like 3-5 work), then ENTER. x = cancel");
        if (!read_clean_answer(text))
        {
            return false;
        }
    }
    // An invalid list is refused and asked again here, not by the yes/no question above
    for (;;)
    {
        int bad = -1;
        const int n = core::parse_unused_slots(text.c_str(), NB_SLOT, PURGE_SLOT, unused, &bad);
        if (n >= 0)
        {
            return true;
        }
        if (n == core::UNUSED_OUT_OF_RANGE)
        {
            Serial.println(String("REFUSED | slot ") + String(bad) + " does not exist: use 1 to " + String(NB_SLOT - 1) + " (slot " +
                           String(PURGE_SLOT) + " is the purge)");
        }
        else
        {
            Serial.println("REFUSED | could not read the list. Examples: 3,7,12   or   3 7 12   or   2-5,9   or   none");
        }
        Serial.println("ASK | Type the slot numbers that do NOT need cleaning (x = cancel)");
        if (!read_clean_answer(text))
        {
            return false;
        }
    }
}

void purge_pipes_manifold(const char *args)
{
    if (!require_pressure_sensor("ethanol_purge"))
    {
        return;
    }

    // 2026-11-04 (user): a LOCKED O-ring is no reason to refuse. Each slot change unlocks it (rotateMotor / goto_slot_locked)
    // and locks it again at the slot, so only UNKNOWN stops here (the unlock would be a blind move).
    if (!core::may_unlock(oring_lock_state()))
    {
        Serial.println(String("REFUSED | ethanol_purge: O-ring lock state is ") + oring_lock_state_name() +
                       " - run actuator_home first (the manifold must rotate between slots)");
        return;
    }

    Serial.println("====================== ETHANOL CLEANING OF THE TUBING ======================");
    Serial.println("WARNING | NO FILTER may be installed: take every Sterivex out of the manifold first.");
    Serial.println("WARNING | The pump runs at FULL POWER (100 %), the exception to the 60 % water limit, because nothing is in the way.");
    Serial.println("WARNING | Inlet AND outlet tubes go in 70 % ethanol (and stay there, the air comes in by the air valve); the outlets must drain into a waste container.");
    Serial.println("WARNING | The pressure cap, the timeouts and x (abort) / s (skip this run) stay active.");

    bool unused[NB_SLOT];
    if (!ask_unused_slots(args, unused))
    {
        Serial.println("CANCELLED | ethanol cleaning: nothing was started");
        return;
    }

    int slots[NB_SLOT];
    const int n_slots = core::build_clean_list(FILL_ORDER, sizeof FILL_ORDER / sizeof FILL_ORDER[0], NB_SLOT, PURGE_SLOT, unused, slots, NB_SLOT);
    if (n_slots < 0)
    {
        Serial.println("ERROR | ethanol cleaning: slot list too long (internal), nothing was started");
        return;
    }

    const float prime_ml = spool_installed_session ? CLEAN_PRIME_SPOOL_ML : CLEAN_SLOT_ML;
    const uint32_t prime_timeout = spool_installed_session ? PURGE_SPOOL_MAX_RUNTIME_MS : CLEAN_FLUSH_MAX_RUNTIME_MS;
    const float total_ml = prime_ml + n_slots * CLEAN_SLOT_ML;
    Serial.print("Slots to clean (" + String(n_slots) + "), in this order:");
    for (int i = 0; i < n_slots; i++)
    {
        Serial.print(" " + String(slots[i]));
    }
    Serial.println();
    Serial.println(String("Spool module: ") + (spool_installed_session ? "ON" : "off") + "; ethanol needed: about " + String(total_ml / 1000.0f, 1) +
                   " L (prime " + String(prime_ml, 0) + " mL + " + String(n_slots) + " x " + String(CLEAN_SLOT_ML, 0) + " mL)");
    Serial.println(String("Then air: ") + String(CLEAN_AIR_SLOT_MS / 1000) + " s through each of those slots (air valve), then " + String(CLEAN_AIR_FINAL_MS / 1000) +
                   " s at the purge slot.");

    if (clean_ask_yes_no("Filters removed, inlet and outlet in 70 % ethanol, outlets draining to waste? Start the cleaning?") != CLEAN_ANSWER_YES)
    {
        Serial.println("CANCELLED | ethanol cleaning: nothing was started");
        return;
    }

    int warnings = 0;
    CleanRun r;

    // 1) prime through the purge slot
    r = clean_flush_slot(PURGE_SLOT, prime_ml, prime_timeout, "prime");
    if (r == CLEAN_RUN_FAIL)
    {
        stop_clean("prime through the purge slot");
        return;
    }
    warnings += (r == CLEAN_RUN_WARN);

    // 2) ethanol through each slot
    for (int i = 0; i < n_slots; i++)
    {
        r = clean_flush_slot(slots[i], CLEAN_SLOT_ML, CLEAN_FLUSH_MAX_RUNTIME_MS, "flush");
        if (r == CLEAN_RUN_FAIL)
        {
            stop_clean("ethanol flush of a slot");
            return;
        }
        warnings += (r == CLEAN_RUN_WARN);
    }

    // 3) air through each slot (V1, the tubes can stay in the ethanol)
    for (int i = 0; i < n_slots; i++)
    {
        r = clean_air_slot(slots[i], CLEAN_AIR_SLOT_MS, "dry");
        if (r == CLEAN_RUN_FAIL)
        {
            stop_clean("air drying of a slot");
            return;
        }
        warnings += (r == CLEAN_RUN_WARN);
    }

    // 4) last push at the purge slot
    r = clean_air_slot(PURGE_SLOT, CLEAN_AIR_FINAL_MS, "final");
    if (r == CLEAN_RUN_FAIL)
    {
        stop_clean("final air push at the purge slot");
        return;
    }
    warnings += (r == CLEAN_RUN_WARN);

    stop_purge_manifold();
    Serial.println(String("Ethanol cleaning complete: ") + String(n_slots) + " slots cleaned and dried, " + String(warnings) +
                   " warning(s). Pump off, valves closed, O-ring " + oring_lock_state_name() + ".");
    if (warnings > 0)
    {
        Serial.println("WARNING | read the lines above that start with WARNING before sampling.");
    }
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