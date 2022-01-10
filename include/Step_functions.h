#ifndef STEP_FUNCTION_H
#define STEP_FUNCTION_H

#include <Arduino.h>

void step_dive(int _depth);
void step_purge();
void step_fill_container();
void step_sampling(uint8_t num_sterivex);
void step_rewind();
void step_dry(uint8_t num_sterivex);


#endif