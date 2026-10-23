#ifndef RUN_KEYS_H
#define RUN_KEYS_H

#include <Arduino.h>
#include "../src/Core/run_keys.h"

// The keys typed during a pump run (S1-C addendum item 1): `s` + ENTER skips the current pump run, `x` + ENTER (or `stop`)
// aborts the whole command. Rules and words: src/Core/run_keys.h. Every pump loop polls poll_run_key() on each pass.

// A new command from the console or the Pi link: clears the abort latch and the bytes typed before it.
void run_keys_reset();

// The start of every pump run: drops what was typed before it (a key for the previous step must not stop this one).
void run_keys_flush();

// Non-blocking. Returns SKIP or ABORT for a complete typed line, NONE otherwise. Other lines are discarded with a message.
// An ABORT also latches: run_abort_latched() stays true until run_keys_reset().
core::RunKey poll_run_key();

// True once x / stop was typed during this command. Every pump run then refuses, and each step stops at its next boundary.
bool run_abort_latched();

#endif // RUN_KEYS_H
