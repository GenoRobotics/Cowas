#ifndef TESTS_MECH_H
#define TESTS_MECH_H

#include <Arduino.h>

// FW2 mechanical test suite (src/Tests_mech.cpp), reached only from the serial test menu
// (test_all_components()). Returns true if `line` was one of its commands (handled,
// possibly with a usage message), false to let the menu try its own commands.
bool mech_tests_dispatch(const String &line);

#endif
