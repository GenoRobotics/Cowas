#ifndef STEP_FUNCTION_H
#define STEP_FUNCTION_H

#include <Arduino.h>

/**
 * @brief Step functions used to do the sampling process
 * @note step_process: the sample function to call, regroups all necessery steps
 */

void step_dive(int _depth);                 // unroll deplayment module to given depth
void step_purge(bool stop_pressure = true); // emptying container
void step_purge_flow(float milliliters);    // purge amount of water through slot 0
void step_fill_container();
void step_sampling(int slot_manifold, bool stop_pressure = true); // pump water through filter
void step_rewind();                                               // rewind deployment module
void step_empty();                                                // drains water from deployment module
void step_DNA_shield(int slot_manifold);                          // insert DNA shield to sterivex
void sample_process(int depth, int manifold_slot = -1);

void abort_sample(); // call if something went wrong to get initial state

void DNA_shield_test(int slot_manifold);

void demo_sample_process();

#endif