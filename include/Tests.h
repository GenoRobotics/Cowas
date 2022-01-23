#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>

void test_1_depth_20m();
void test_1_depth_40m();
void test_1_depth_40m_direct();

void test_2_remplissage_container_1m();
void test_2_remplissage_container_40m();

void test_3_sterivex_1();
void test_3_sterivex_2();

void tests();
void test_pressure();
void test_hardware_general();

void test_sampling(uint8_t num_sterivex);
void test_purge();



#endif