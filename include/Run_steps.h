#ifndef RUN_STEPS_H
#define RUN_STEPS_H

#include "Core/step_log.h"

// The timed steps of the current run (2026-11-06, user: every step of a sample needs its start, end and duration in the
// sample file). run_steps_begin() empties the log at the start of a run (run_full_cycle); the step functions and the spool
// moves add their steps while they run; run_full_cycle writes the log as one {"t":"steps",...} line when the run ends.
// Defined in src/Step_functions.cpp.

core::StepLog &run_steps();

// Empties the step log and the id of the last sample record (so a steps line without its own record has no id).
void run_steps_begin();

// The id of the last sample or purge record printed in this run ("" if none).
const char *last_sample_record_id();

// Times one step with its scope: the step is closed on every return path, so an early return cannot leave it open.
class StepScope
{
public:
    explicit StepScope(const char *name);
    ~StepScope();

private:
    int index_;
};

#endif
