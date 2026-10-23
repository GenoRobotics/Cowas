#include "abp_convert.h"

#include <math.h>

namespace core
{

AbpFrameStatus abp_classify_frame(uint8_t b0, uint8_t b1, uint8_t b2)
{
    if (b0 == 0xFF && b1 == 0xFF && b2 == 0xFF)
    {
        return ABP_FRAME_NO_ANSWER;
    }
    if (b0 == 0 && b1 == 0 && b2 == 0)
    {
        return ABP_FRAME_STUCK_LOW;
    }
    switch (b0 >> 6)
    {
    case 0:
        return ABP_FRAME_VALID;
    case 1:
        return ABP_FRAME_COMMAND_MODE;
    case 2:
        return ABP_FRAME_STALE;
    }
    return ABP_FRAME_DIAGNOSTIC;
}

const char *abp_frame_status_name(AbpFrameStatus s)
{
    switch (s)
    {
    case ABP_FRAME_VALID:
        return "valid";
    case ABP_FRAME_STALE:
        return "stale";
    case ABP_FRAME_COMMAND_MODE:
        return "command_mode";
    case ABP_FRAME_DIAGNOSTIC:
        return "diagnostic";
    case ABP_FRAME_NO_ANSWER:
        return "no_answer";
    case ABP_FRAME_STUCK_LOW:
        return "stuck_low";
    }
    return "?";
}

uint16_t abp_frame_counts(uint8_t b0, uint8_t b1)
{
    return (uint16_t)(((uint16_t)(b0 & 0x3F) << 8) | b1);
}

float abp_counts_to_bar(uint16_t counts, float full_scale_psi)
{
    if (isnan(full_scale_psi) || !(full_scale_psi > 0.0f))
    {
        return NAN;
    }
    const double span = ABP_COUNTS_MAX - ABP_COUNTS_MIN;
    return (float)(((double)counts - ABP_COUNTS_MIN) / span * full_scale_psi * PSI_TO_BAR);
}

float abp_apply_calibration(float nominal_bar, float gain, float zero_offset_bar)
{
    return gain * (nominal_bar - zero_offset_bar);
}

AbpTrust abp_trust_init()
{
    AbpTrust t = {false, false, 0};
    return t;
}

AbpTrustState abp_trust_update(AbpTrust &t, AbpFrameStatus status, uint32_t now_ms, uint32_t stale_timeout_ms)
{
    if (status == ABP_FRAME_VALID)
    {
        t.has_valid = true;
        t.stale_run = false;
        return ABP_TRUSTED;
    }
    if (status == ABP_FRAME_STALE)
    {
        if (!t.has_valid)
        {
            return ABP_UNTRUSTED_NEVER_VALID;
        }
        if (!t.stale_run)
        {
            t.stale_run = true;
            t.stale_since_ms = now_ms;
        }
        // unsigned subtraction: right across a millis() wrap
        return ((uint32_t)(now_ms - t.stale_since_ms) >= stale_timeout_ms) ? ABP_UNTRUSTED_STALE : ABP_TRUSTED;
    }
    // a fault: the stored value is from before it, don't fall back on it
    t.has_valid = false;
    t.stale_run = false;
    return ABP_UNTRUSTED_FAULT;
}

const char *abp_trust_state_name(AbpTrustState s)
{
    switch (s)
    {
    case ABP_TRUSTED:
        return "trusted";
    case ABP_UNTRUSTED_FAULT:
        return "fault";
    case ABP_UNTRUSTED_STALE:
        return "stale";
    case ABP_UNTRUSTED_NEVER_VALID:
        return "never_valid";
    }
    return "?";
}

} // namespace core
