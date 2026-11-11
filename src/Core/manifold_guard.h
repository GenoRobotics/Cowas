#ifndef CORE_MANIFOLD_GUARD_H
#define CORE_MANIFOLD_GUARD_H

// Motion guard of the manifold rotation (2026-11-06, F256): pure logic, no Arduino.h, unit-tested with
// `pio test -e native` (test/test_manifold_guard/). Called every millisecond by rotateMotor() (src/Hardware/Manifold.cpp).
// Two rules stop the motor long before the 15 s timeout: the encoder does not move (stall: a tangle, a jam, a dead motor),
// or the manifold has turned further than any slot change needs (over-travel: a wrong direction or a lost encoder).
// Angles are the unwrapped encoder angle in degrees (current_angle, turns included).

#include <stdint.h>

namespace core
{

enum ManifoldGuardState
{
    MANIFOLD_GUARD_OK = 0,
    MANIFOLD_GUARD_STALLED,    // the angle moved less than min_progress_deg in stall_ms
    MANIFOLD_GUARD_OVERTRAVEL  // the angle is more than max_travel_deg from where the rotation started
};

struct ManifoldGuardParams
{
    uint32_t stall_ms;      // longest time without min_progress_deg of movement
    float min_progress_deg; // movement that counts as turning
    float max_travel_deg;   // furthest from the start angle (either way)
};

struct ManifoldGuard
{
    float start_deg;
    float ref_deg;   // angle at the last time progress was seen
    uint32_t ref_ms; // time of that
};

void manifold_guard_init(ManifoldGuard *g, float angle_deg, uint32_t now_ms);

// One step. Order: over-travel, stall. Updates the reference when the manifold has turned min_progress_deg.
ManifoldGuardState manifold_guard_step(ManifoldGuard *g, const ManifoldGuardParams &p, float angle_deg, uint32_t now_ms);

const char *manifold_guard_name(ManifoldGuardState s);

} // namespace core

#endif
