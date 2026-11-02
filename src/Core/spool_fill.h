#ifndef CORE_SPOOL_FILL_H
#define CORE_SPOOL_FILL_H

// Pure logic of the water fill of the spool tube (2026-11-02, user): the tube is unrolled to the session's deepest point,
// the purge slot is primed until the flowmeter shows flow, then the tube is rolled up to the sample's depth with the
// deployment valve shut. No Arduino.h, no globals, no hardware. Unit-tested on the host with `pio test -e native`
// (test/test_spool_fill/). The sequence that calls it is spool_fill_and_rise() in src/Step_functions.cpp.

namespace core
{

// The depth the operator types at `session` for the deepest point of the session, in metres. Empty text = 0 (the caller uses the
// default). Returns 1 and sets *m when the text is a number in [lo_m, hi_m] (a comma works as a decimal point), 0 when it is
// empty, -1 when it is malformed or out of range.
int spool_session_depth_parse(const char *text, float lo_m, float hi_m, float *m);

} // namespace core

#endif
