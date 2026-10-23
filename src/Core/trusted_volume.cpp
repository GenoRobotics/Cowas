#include "trusted_volume.h"

namespace core
{

void TrustedVolume::reset(float total_ml, uint32_t now_ms)
{
    trusted_ml_ = 0.0f;
    win_ml_ = total_ml;
    win_ms_ = now_ms;
}

void TrustedVolume::close_window(float total_ml, uint32_t now_ms, float min_ml_per_min)
{
    const uint32_t dt_ms = now_ms - win_ms_;
    const float gained = total_ml - win_ml_;
    if (dt_ms > 0 && gained > 0.0f)
    {
        const float rate_ml_per_min = gained * 60000.0f / (float)dt_ms;
        if (rate_ml_per_min >= min_ml_per_min)
        {
            trusted_ml_ += gained;
        }
    }
    win_ml_ = total_ml;
    win_ms_ = now_ms;
}

void TrustedVolume::update(float total_ml, uint32_t now_ms, float min_ml_per_min, uint32_t window_ms)
{
    if (now_ms - win_ms_ >= window_ms)
    {
        close_window(total_ml, now_ms, min_ml_per_min);
    }
}

void TrustedVolume::close(float total_ml, uint32_t now_ms, float min_ml_per_min)
{
    close_window(total_ml, now_ms, min_ml_per_min);
}

} // namespace core
