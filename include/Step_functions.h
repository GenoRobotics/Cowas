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
void load_slot_water(float volume_ml); // load_slot()'s water phase (caller locked + checked the sensor)
void load_slot_air();                  // load_slot()'s air phase
bool load_slot(float volume_ml); // lock, pump water+air through the CURRENT slot, unlock - see Step_functions.cpp
bool load_slot_full(int slot_manifold); // FULL production sequence (step_sampling + step_DNA_shield) at the CURRENT slot, then unlock - see Step_functions.cpp
void step_rewind();                                               // rewind deployment module
bool step_empty();                                                // drains water from deployment module
bool step_DNA_shield(int slot_manifold);                          // insert DNA shield to sterivex
void sample_process(int depth, int manifold_slot = -1);

void abort_sample(); // call if something went wrong to get initial state

void DNA_shield_test(int slot_manifold);

void demo_sample_process();

// FW2: step_* return false if refused (no pressure sensor, lock state unknown) or if a move failed
bool goto_slot_locked(int slot);               // unlock -> rotate -> lock; false at the first refusal/failure
bool require_pressure_sensor(const char *who); // false + message while PRESSURE_SENSOR_AVAILABLE is false

#endif