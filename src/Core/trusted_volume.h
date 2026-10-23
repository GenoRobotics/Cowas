#ifndef CORE_TRUSTED_VOLUME_H
#define CORE_TRUSTED_VOLUME_H

#include <stdint.h>

// The flowmeter volume that counts as confident (2026-10-23, user). The flowmeter is trusted only at fast flow (F150):
// 2026-10-23 it read +2 % at about 140 mL/min, -32 % at about 50 mL/min, and almost nothing below about 25 mL/min.
// The volume is counted in windows of window_ms. A window counts (its whole volume) when its mean flow is at least
// min_ml_per_min. The plain total is logged next to it, so both can be compared later. Pure logic, no Arduino.h;
// unit-tested with `pio test -e native` (test/test_trusted_volume/).

namespace core
{

class TrustedVolume
{
public:
    // starts counting at total_ml (the flowmeter total) at now_ms
    void reset(float total_ml, uint32_t now_ms);
    // called on every pass of a run: closes a window when window_ms has passed
    void update(float total_ml, uint32_t now_ms, float min_ml_per_min, uint32_t window_ms);
    // called at the end of a run: closes the last, partial window the same way
    void close(float total_ml, uint32_t now_ms, float min_ml_per_min);
    // the confident volume so far (mL)
    float trusted_ml() const { return trusted_ml_; }

private:
    void close_window(float total_ml, uint32_t now_ms, float min_ml_per_min);
    float trusted_ml_ = 0.0f;
    float win_ml_ = 0.0f;     // flowmeter total at the start of the current window
    uint32_t win_ms_ = 0;     // time at the start of the current window
};

} // namespace core

#endif
