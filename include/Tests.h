#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>


void test_all_components();

// Serial keypress helpers (Tests.cpp), shared with the FW2 suite (Tests_mech.cpp)
char real_keypress_waiting();       // non-blocking: the key, or 0 (a lone CR/LF is ignored)
char wait_for_keypress();           // blocking; keeps the pump cap alive while waiting
bool wait_for_keypress_or_abort();  // blocking; true for 'x'/'X'

void test_valves();
void test_motor_spool();
void test_pump();
void test_command_box();
void test_micro_switch();
void test_encoder();
void test_encoder_spool();

void test_1_depth_20m();
void test_1_depth_40m();



void tests();

void print_pressure_once();
void print_abp_raw();
void pressure_zero();      // S1-1: ABP zero (prints PRESSURE_ZERO_OFFSET_BAR)
void calibrate_pressure(); // S1-1: ABP gain + zero against the manometer (syringe, no pump)

void test_manifold();
void go_to_zero();

void test_linear_actuator();
void test_load_sample();
void test_load_sample_full();
void test_multi_sample_loading_full();

void calibrate_linear_actuator();
void calibrate_flow_sensor();
void calibrate_dna_pump_volume();
void calibrate_shield_pump();
void identify_manifold_direction();
void run_commissioning_calibration();

#endif