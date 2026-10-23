#ifndef STEP_FUNCTION_H
#define STEP_FUNCTION_H

#include <Arduino.h>
#include "Settings.h"

/**
 * @brief Step functions used to do the sampling process
 * @note step_process: the sample function to call, regroups all necessery steps
 */

void step_dive(int _depth);                 // unroll deplayment module to given depth
bool step_purge(bool stop_pressure = true); // emptying container
void step_purge_flow(float milliliters);    // purge amount of water through slot 0
void step_fill_container();
bool step_sampling(int slot_manifold, bool stop_pressure = true, float volume_ml = STX_SAMPLE_MILLILITERS); // pump water through filter
extern const char *last_sample_end; // why the last step_sampling() pump run stopped (S1-C)
extern float last_sample_trusted_ml; // the confident flowmeter volume of that run (F150, 2026-10-23)
extern const char *last_purge_end;  // why the last step_purge() pump run stopped: cond, skipped, timeout, ... (S1-C)
extern const char *last_empty_end;  // why the last step_empty() pump run stopped (S1-C)
void load_slot_water(float volume_ml); // load_slot()'s water phase (caller locked + checked the sensor)
void load_slot_air();                  // load_slot()'s air phase
bool load_slot(float volume_ml); // lock, pump water+air through the CURRENT slot, unlock - see Step_functions.cpp
bool load_slot_full(int slot_manifold); // FULL operation sequence (step_sampling + step_DNA_shield) at the CURRENT slot, then unlock - see Step_functions.cpp
void step_rewind();                                               // rewind deployment module
bool step_empty();                                                // drains water from deployment module
bool step_DNA_shield(int slot_manifold);                          // insert DNA shield to sterivex
void sample_process(int depth, int manifold_slot = -1);
// 2026-10-23: the name of the sample being run becomes the id of its sample record (run_sample). "" = counter ids
void set_sample_record_name(const char *name);
// F199 (2026-10-23): the session site and date and the condition, written in the sample records of a run_sample. "" = not written
void set_sample_record_meta(const char *site, const char *date, const char *condition);

void abort_sample(); // call if something went wrong to get initial state


void demo_sample_process();

// FW2: step_* return false if refused (no pressure sensor, lock state unknown) or if a move failed
bool goto_slot_locked(int slot);               // unlock -> rotate -> lock; false at the first refusal/failure
bool require_pressure_sensor(const char *who); // false + message while PRESSURE_SENSOR_AVAILABLE is false

#endif