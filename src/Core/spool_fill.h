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

// A prime (the flow check at the purge slot) is tried again only when it found no flow: end_reason "timeout". A pump cap, a sensor
// fault, a skip or an abort is never retried (safety: the pump does not run again on a cap or a fault). `attempt` counts from 1;
// the retry is allowed while attempt < max_attempts (2026-11-05: a prime at 40 m timed out twice and passed on the third try).
bool spool_prime_may_retry(const char *end_reason, int attempt, int max_attempts);

} // namespace core

#endif
