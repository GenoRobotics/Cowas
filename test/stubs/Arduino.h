// NATIVE TESTS ONLY - never used by the firmware build.
//
// Just enough of Arduino.h for a native test to #include the real include/Settings.h
// and read its plain constants (NB_SLOT, PURGE_SLOT, FILL_ORDER, ...). Nothing here is
// callable; src/Core/ itself never includes Arduino.h.
#pragma once

#include <stdint.h>
#include <stddef.h>

class String; // Settings.h only declares functions returning String

// Arduino Due pin numbers (framework-arduino-sam variants/arduino_due_x/variant.h)
static const uint8_t A0 = 54;
static const uint8_t A1 = 55;
static const uint8_t A2 = 56;
static const uint8_t A3 = 57;
static const uint8_t A4 = 58;
static const uint8_t A5 = 59;
static const uint8_t A6 = 60;
static const uint8_t A7 = 61;
static const uint8_t A8 = 62;
static const uint8_t A9 = 63;
static const uint8_t A10 = 64;
static const uint8_t A11 = 65;
static const uint8_t DAC0 = 66;
static const uint8_t DAC1 = 67;
