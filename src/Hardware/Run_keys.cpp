#include <Arduino.h>
#include "Run_keys.h"

// Thin driver: reads whole lines from the console serial port. The rules are in src/Core/run_keys.cpp.

static bool abort_latched = false;
static String line_buffer;

static void drop_typed_bytes()
{
    while (Serial.available() > 0)
    {
        Serial.read();
    }
    line_buffer = "";
}

void run_keys_reset()
{
    abort_latched = false;
    drop_typed_bytes();
}

void run_keys_flush()
{
    drop_typed_bytes();
}

bool run_abort_latched()
{
    return abort_latched;
}

core::RunKey poll_run_key()
{
    while (Serial.available() > 0)
    {
        char c = (char)Serial.read();
        if (c == '\r')
        {
            continue;
        }
        if (c != '\n')
        {
            if (line_buffer.length() < 32)
            {
                line_buffer += c; // a longer line is cut, and the cut part is then ignored
            }
            continue;
        }
        String line = line_buffer;
        line_buffer = "";
        line.trim();
        if (line.length() == 0)
        {
            continue;
        }
        core::RunKey key = core::skip_or_abort(line.c_str());
        if (key == core::RUN_KEY_ABORT)
        {
            abort_latched = true;
        }
        if (key == core::RUN_KEY_NONE)
        {
            Serial.println("ignored during a pump run: s + ENTER = skip this pump run, x + ENTER = abort the command");
            continue;
        }
        return key;
    }
    return core::RUN_KEY_NONE;
}
