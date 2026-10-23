#ifndef CORE_RUN_KEYS_H
#define CORE_RUN_KEYS_H

// Keys typed while a pump runs (S1-C addendum item 1, 2026-10-19). Pure logic, no Arduino.h, unit-tested with
// `pio test -e native` (test/test_run_keys/). The serial reading lives in src/Hardware/Run_keys.cpp.
//   s / skip            = skip the CURRENT pump run only; the step goes on with its next phase (a skipped purge goes to
//                         the sample, a skipped sample goes on to its emptying, a skipped shield pump still gets its air push).
//   x / abort / stop    = abort the run: pumps off, valves closed, O-ring stays locked, and nothing else in the command runs.
//                         "stop" is a STOP, which the rules say is always accepted.
// Anything else is ignored (the caller says so on the console).

namespace core
{

enum RunKey
{
    RUN_KEY_NONE = 0,
    RUN_KEY_SKIP,
    RUN_KEY_ABORT
};

// One typed line (no line ending needed; spaces and CR/LF around it are ignored, the case is not checked).
RunKey skip_or_abort(const char *line);

// Purge volume in mL, chosen by the spool answer (user 2026-10-23): spool_ml when the spool is installed, else no_spool_ml.
float purge_volume_ml(bool spool_installed, float no_spool_ml, float spool_ml);

} // namespace core

#endif
