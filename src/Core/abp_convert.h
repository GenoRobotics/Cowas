#ifndef CORE_ABP_CONVERT_H
#define CORE_ABP_CONVERT_H

// Honeywell ABP pressure sensor on SPI (S1-1, F27): frame classification, counts -> bar for
// any full scale, the gain / zero correction, and the rule that says when a reading can no
// longer be trusted. Pure logic, no Arduino.h, unit-tested with `pio test -e native`
// (test/test_abp_convert/). The driver (src/Hardware/Trustability_ABP_Gage.cpp) is a thin
// wrapper around these.
//
// Frame: byte 0 = 2 status bits + the 6 high bits of the 14-bit count, byte 1 = the 8 low
// bits, byte 2 = temperature. The "A" transfer function puts 0 at 10 % of 2^14 counts and
// the full scale at 90 %.

#include <stdint.h>

namespace core
{

const int ABP_COUNTS_MIN = 1638;  // 10 % of 2^14: 0 (gauge)
const int ABP_COUNTS_MAX = 14745; // 90 % of 2^14: the full scale
const float PSI_TO_BAR = 0.0689476f;

enum AbpFrameStatus
{
    ABP_FRAME_VALID = 0,    // status 00: a fresh reading
    ABP_FRAME_STALE,        // status 10: already read since the last conversion (the sensor is alive)
    ABP_FRAME_COMMAND_MODE, // status 01: never expected in normal use
    ABP_FRAME_DIAGNOSTIC,   // status 11: the sensor reports its own fault
    ABP_FRAME_NO_ANSWER,    // FF FF FF: nobody drives MISO (supply, wire, dead part)
    ABP_FRAME_STUCK_LOW     // 00 00 00: MISO held low (a real reading is never 0 counts)
};

AbpFrameStatus abp_classify_frame(uint8_t b0, uint8_t b1, uint8_t b2);
const char *abp_frame_status_name(AbpFrameStatus s); // "valid", "stale", "command_mode", ...

// The 14-bit count of a frame (the status bits are masked out).
uint16_t abp_frame_counts(uint8_t b0, uint8_t b1);

// Datasheet conversion ("nominal"): ABP_COUNTS_MIN -> 0 bar, ABP_COUNTS_MAX -> the full scale.
// NaN if the full scale is not > 0.
float abp_counts_to_bar(uint16_t counts, float full_scale_psi);

// Calibrated pressure = gain x (nominal - zero_offset). zero_offset is what the nominal
// conversion reads at 0 bar, so a re-tare changes it and leaves the gain alone.
float abp_apply_calibration(float nominal_bar, float gain, float zero_offset_bar);

// Can the last stored reading still be trusted?
struct AbpTrust
{
    bool has_valid;          // a valid frame was seen since boot or since the last fault
    bool stale_run;          // the last frame(s) were stale
    uint32_t stale_since_ms; // when that run of stale frames began
};

enum AbpTrustState
{
    ABP_TRUSTED = 0,
    ABP_UNTRUSTED_FAULT,        // command mode, diagnostic, no answer or stuck low
    ABP_UNTRUSTED_STALE,        // only stale frames for stale_timeout_ms or longer
    ABP_UNTRUSTED_NEVER_VALID   // stale frames, but no valid one to fall back on
};

AbpTrust abp_trust_init();

// Call once per frame read. A valid frame is trusted. A stale frame keeps the last value
// while the run of stale frames is shorter than stale_timeout_ms. Anything else is a fault
// at once, and it takes a new valid frame to be trusted again. millis() wrap-safe.
AbpTrustState abp_trust_update(AbpTrust &t, AbpFrameStatus status, uint32_t now_ms, uint32_t stale_timeout_ms);
const char *abp_trust_state_name(AbpTrustState s); // "trusted", "fault", "stale", "never_valid"

} // namespace core

#endif
