#ifndef CORE_SAMPLE_RECORD_H
#define CORE_SAMPLE_RECORD_H

#include <stddef.h>
#include <stdint.h>

// One JSON line per sample (docs/software/code-guide.md §4, the sample record), printed by the firmware when a sample or a purge ends, so the
// stop criteria and the volumes are written by the code that made the decision. Pure logic, no Arduino.h, unit-tested with
// `pio test -e native` (test/test_sample_record/). The number formatting is done by hand (no %f): the Due's newlib-nano
// printf does not print floats by default.
//   - a NaN value is written as null, never as 0 (a missing value is not a zero)
//   - the strings are written as given, with " and \ escaped; the caller passes plain names

namespace core
{

// Running mean and peak of the pressure over one pump run (every plausible reading of the control loop).
class PressureStats
{
public:
    void reset();
    void add(float bar);
    float mean() const; // NaN with no reading
    float peak() const; // NaN with no reading
    int count() const { return n_; }

private:
    double sum_ = 0;
    float max_ = 0;
    int n_ = 0;
};

// Name written in a record for the end reason of a CtrlPump run (CtrlPump::end_reason()). "cond" is the normal end
// (the volume is reached): "volume". "cap" is "pressure". "aborted" is an x or a STOP: "stop_key".
const char *sample_end_name(const char *ctrl_end_reason);

struct SampleRecord
{
    const char *fw;          // firmware version
    const char *id;          // sample id, e.g. "auto-3"
    const char *purpose;     // "purge", "sample" or "blank"
    const char *site;        // the session's site (F199, 2026-10-23); NULL = not written
    const char *date;        // the session's date; NULL = not written
    const char *condition;   // the operator's condition of the sample (run_sample); NULL = not written
    int slot;                // manifold slot
    uint32_t start_ms;       // millis() at the start of the run
    uint32_t end_ms;         // millis() at the end of the run
    float target_ml;         // target volume of the run
    float vol_ml;            // flowmeter volume at the end: the plain total, not trusted at slow flow (F150)
    float vol_trusted_ml;    // the confident part: flowmeter volume in windows whose mean flow was at least FLOW_TRUST_MIN_ML_PER_MIN (F150)
    float purge_ml;          // purge: the volume pumped; sample: NaN
    const char *end_reason;  // from sample_end_name()
    float p_mean_bar;        // NaN if no reading
    float p_max_bar;         // NaN if no reading
    float p_cap_bar;         // STX_MAX_PRESSURE in force
    const char *lock_before; // LOCKED, UNLOCKED or UNKNOWN
    const char *lock_after;
    float shield_ml;         // NaN: the shield pump is a separate step
    float flow_factor;       // calibrationFactor_small
    float p_gain;            // PRESSURE_GAIN
    float p_offset_bar;      // PRESSURE_ZERO_OFFSET_BAR
    float abp_full_scale_psi;
    int linear_act_locked_steps;
    const char *const *flags; // e.g. "placeholder:ABP_FULL_SCALE_PSI"
    int n_flags;
};

// Writes the record as one JSON line without a line end, into buf (size bytes, with the terminating 0).
// Returns the number of characters written (not counting the 0), or -1 if buf is too small.
int format_sample_record(const SampleRecord &r, char *buf, size_t size);

} // namespace core

#endif
