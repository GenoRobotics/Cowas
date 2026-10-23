#ifndef CORE_TEST_LINE_H
#define CORE_TEST_LINE_H

// The on-device test output convention (ROADMAP §5, FW2): one machine-readable line per
// result,
//   TEST|<name>|<id>|<PASS/FAIL/SKIP>|<key=value;...>
// read back by tools/hw_test_log.py. Pure formatting, no Arduino.h, unit-tested with
// `pio test -e native` (test/test_test_line/).

namespace core
{

enum TestResult
{
    TEST_PASS = 0,
    TEST_FAIL,
    TEST_SKIP
};

const char *test_result_name(TestResult r); // "PASS", "FAIL", "SKIP"

// Writes the line (no newline) into out. In name and id, '|', ';', '=', CR and LF become
// '_'; in kv only '|', CR and LF do (kv keeps its own ';' and '='). Returns the length,
// or -1 if the line did not fit (out then holds a truncated, terminated line).
int format_test_line(char *out, int cap, const char *name, const char *id, TestResult r, const char *kv);

// In place: makes free text (e.g. "where is the leak?") safe as a kv value:
// '|', ';', '=', CR and LF become '_'.
void sanitize_kv_value(char *s);

} // namespace core

#endif
