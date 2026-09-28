#ifndef TEST_REPORT_H
#define TEST_REPORT_H

#include <Arduino.h>
#include "../src/Core/test_line.h"

// FW2 on-device test output (ROADMAP §5): every result prints one line
//   TEST|<name>|<id>|<PASS/FAIL/SKIP>|<key=value;...>
// (captured by tools/hw_test_log.py), and test_end() prints a human summary table plus
// one TEST|<name>|summary|... line. Always printed on Serial, whatever the verbose flags.

void test_begin(const char *name);                                    // starts a new table
void test_result(const char *id, core::TestResult r, const String &kv); // one result line
int test_end(const String &extra_kv = "");                             // summary; returns the FAIL count

// Blocking read of one console line (trimmed). Used for typed confirmations (NOSENSOR)
// and free text ("where is the leak?").
String read_console_line();

#endif
