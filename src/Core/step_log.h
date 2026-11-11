#ifndef CORE_STEP_LOG_H
#define CORE_STEP_LOG_H

#include <stddef.h>
#include <stdint.h>

// The timed steps of one run (a purge, a sample, a spool move, a shield pump run ...), written as one JSON line
// {"t":"steps",...} after the run, so every step of a sample has its own start, end and duration (2026-11-06, user: the
// sample file must hold all the timings). Pure logic, no Arduino.h, unit-tested with `pio test -e native` (test/test_step_log/).
//   - a step has a name (plain words: letters, digits, _, -) and times in millis() of the device
//   - the log has a fixed size: a step that does not fit is counted in dropped(), never silently lost
//   - integers only, no floats (the Due's printf does not print them by default)

namespace core
{

struct StepTime
{
    char name[24];
    uint32_t start_ms;
    uint32_t end_ms;
};

class StepLog
{
public:
    enum
    {
        CAPACITY = 48
    };

    void reset();
    // Opens a step. Returns its index (to pass to end()), or -1 when the log is full.
    int begin(const char *name, uint32_t now_ms);
    // Closes the step opened with begin(). -1 (a step that did not fit) is ignored.
    void end(int index, uint32_t now_ms);
    // Stores a step that is already finished.
    void add(const char *name, uint32_t start_ms, uint32_t end_ms);

    int count() const { return n_; }
    const StepTime &at(int i) const { return items_[i]; }
    int dropped() const { return dropped_; }

private:
    StepTime items_[CAPACITY];
    int n_ = 0;
    int dropped_ = 0;
};

// Writes the log as one JSON line without a line end, into buf (size bytes, with the terminating 0):
//   {"t":"steps","id":"<id>","slot":<slot>,"steps":[{"name":"purge","start_ms":..,"end_ms":..,"dur_ms":..},...],"steps_dropped":0}
// The id is escaped (" and \). Returns the number of characters written (not counting the 0), or -1 if buf is too small.
int format_steps_record(const StepLog &log, const char *id, int slot, char *buf, size_t size);

} // namespace core

#endif
