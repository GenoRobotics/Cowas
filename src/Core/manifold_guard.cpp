#include "manifold_guard.h"

namespace core
{

static float absf(float x) { return x < 0.0f ? -x : x; }

void manifold_guard_init(ManifoldGuard *g, float angle_deg, uint32_t now_ms)
{
    g->start_deg = angle_deg;
    g->ref_deg = angle_deg;
    g->ref_ms = now_ms;
}

ManifoldGuardState manifold_guard_step(ManifoldGuard *g, const ManifoldGuardParams &p, float angle_deg, uint32_t now_ms)
{
    if (absf(angle_deg - g->start_deg) > p.max_travel_deg)
        return MANIFOLD_GUARD_OVERTRAVEL;

    if (absf(angle_deg - g->ref_deg) >= p.min_progress_deg)
    {
        g->ref_deg = angle_deg;
        g->ref_ms = now_ms;
        return MANIFOLD_GUARD_OK;
    }
    if ((uint32_t)(now_ms - g->ref_ms) > p.stall_ms)
        return MANIFOLD_GUARD_STALLED;
    return MANIFOLD_GUARD_OK;
}

const char *manifold_guard_name(ManifoldGuardState s)
{
    switch (s)
    {
    case MANIFOLD_GUARD_OK:
        return "ok";
    case MANIFOLD_GUARD_STALLED:
        return "stalled";
    case MANIFOLD_GUARD_OVERTRAVEL:
        return "over-travel";
    }
    return "?";
}

} // namespace core
