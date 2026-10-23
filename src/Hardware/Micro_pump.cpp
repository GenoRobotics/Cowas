#include <Arduino.h>
#include "Micro_pump.h"
#include "Led.h"
#include "C_output.h"
#include "Oring_lock.h"
#include "Run_keys.h"

extern C_output output;
extern Micro_Pump micro_pump;

/**
 * @brief Constructor for a micro pump, just ON/OFF
 * @param _control_pin Output connection on the board
 *
 */
void Micro_Pump::begin(byte _control_pin)
{
    control_pin = _control_pin;
    pinMode(control_pin, OUTPUT);
    stop();
    output.println("Micro Pump " + ID + " initiated");
}

/**
 * @brief Constructor for a micro pump, just ON/OFF
 * @param _control_pin Output connection on the board
 * @param _ID Name of hardware component used for outputs
 *
 */
void Micro_Pump::begin(byte _control_pin, String _ID)
{
    ID = _ID;
    begin(_control_pin);
}

/**
 * @brief Start the pump with current power setting until stop command.
 *
 */
void Micro_Pump::start()
{
    // target: the O-ring LOCKED, the shield goes into the slot under the manifold
    if (!oring_go_to(core::LOCK_STATE_LOCKED))
    {
        output.println("REFUSED | micro pump: O-ring not locked, shield pump not started");
        return;
    }
    digitalWrite(control_pin, HIGH);

    // display starting info only if previous state was OFF
    if (!running)
    {
        if(VERBOSE_PUMP){output.println("Micro Pump " + ID + " started ");}
        running = true;
    }
}

/**
 * @brief Start the pump for a certain time, then stop
 *
 * @param time_ms The time is millisecond to run the pump.
 */
void Micro_Pump::start(uint32_t _time_ms)
{
    if (run_abort_latched())
    {
        return; // x was typed earlier in this command
    }
    run_keys_flush();
    start();
    uint32_t current_time = millis();
    while (millis() - current_time < _time_ms)
    {
        // s skips this shield run, x aborts the command (S1-C addendum item 1)
        core::RunKey key = poll_run_key();
        if (key != core::RUN_KEY_NONE)
        {
            Serial.println(key == core::RUN_KEY_ABORT ? "ABORT | shield pump stopped by x" : "SKIP | shield pump skipped by s");
            break;
        }
        delay(5);
    }
    stop();
}

/**
 * @brief Stop the pump.
 *
 */
void Micro_Pump::stop()
{
    digitalWrite(control_pin, 0);
    running = false;
    if(VERBOSE_PUMP){output.println("Micro Pump " + ID + " stopped");}
}


