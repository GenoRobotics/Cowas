// Native unit tests for src/Core/manifold_guard (run: pio test -e native).
// The stall and over-travel rules of the manifold rotation (2026-11-06, F256).
#include <unity.h>

#include "manifold_guard.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static ManifoldGuardParams params()
{
    ManifoldGuardParams p;
    p.stall_ms = MANIFOLD_STALL_MS;
    p.min_progress_deg = MANIFOLD_STALL_MIN_DEG;
    p.max_travel_deg = MANIFOLD_MAX_TRAVEL_DEG;
    return p;
}

void test_turning_normally_is_ok(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 100.0f, 0);
    // 45 deg/s for 8 s, polled every ms
    for (uint32_t t = 1; t <= 8000; t++)
        TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 100.0f + 0.045f * t, t));
}

void test_turning_the_other_way_is_ok(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 300.0f, 0);
    for (uint32_t t = 1; t <= 5000; t++)
        TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 300.0f - 0.045f * t, t));
}

void test_no_movement_stalls_after_stall_ms(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 100.0f, 1000);
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 100.0f, 1000 + MANIFOLD_STALL_MS));
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_STALLED, manifold_guard_step(&g, params(), 100.0f, 1001 + MANIFOLD_STALL_MS));
}

void test_a_stop_after_turning_stalls(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 0.0f, 0);
    for (uint32_t t = 1; t <= 1000; t++)
        manifold_guard_step(&g, params(), 0.045f * t, t); // turns to 45 deg, then jams
    // The last progress step (every 3 deg) is logged a little before t = 1000 ms, so the stall time counts from there.
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 45.0f, 1000 + MANIFOLD_STALL_MS - 100));
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_STALLED, manifold_guard_step(&g, params(), 45.0f, 1001 + MANIFOLD_STALL_MS));
}

void test_jitter_below_min_progress_is_a_stall(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 100.0f, 0);
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_STALLED,
                          manifold_guard_step(&g, params(), 100.0f + MANIFOLD_STALL_MIN_DEG * 0.5f, MANIFOLD_STALL_MS + 1));
}

void test_over_travel_either_way(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 100.0f, 0);
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 100.0f + MANIFOLD_MAX_TRAVEL_DEG, 10));
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OVERTRAVEL, manifold_guard_step(&g, params(), 100.1f + MANIFOLD_MAX_TRAVEL_DEG, 20));
    manifold_guard_init(&g, 100.0f, 0);
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OVERTRAVEL, manifold_guard_step(&g, params(), 99.9f - MANIFOLD_MAX_TRAVEL_DEG, 20));
}

void test_clock_wrap_of_the_timer(void)
{
    ManifoldGuard g;
    manifold_guard_init(&g, 0.0f, 0xFFFFFF00u);
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_OK, manifold_guard_step(&g, params(), 0.0f, 0xFFFFFF00u + 100u));
    TEST_ASSERT_EQUAL_INT(MANIFOLD_GUARD_STALLED, manifold_guard_step(&g, params(), 0.0f, 0xFFFFFF00u + MANIFOLD_STALL_MS + 1u));
}

void test_limits_are_inside_the_timeout(void)
{
    // the guard must trip long before the 15 s timeout, and a full turn must fit the over-travel limit
    TEST_ASSERT_TRUE(MANIFOLD_STALL_MS * 4 < MANIFOLD_ROTATE_TIMEOUT_MS);
    TEST_ASSERT_TRUE(MANIFOLD_MAX_TRAVEL_DEG > 360.0f);
    TEST_ASSERT_EQUAL_STRING("stalled", manifold_guard_name(MANIFOLD_GUARD_STALLED));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_turning_normally_is_ok);
    RUN_TEST(test_turning_the_other_way_is_ok);
    RUN_TEST(test_no_movement_stalls_after_stall_ms);
    RUN_TEST(test_a_stop_after_turning_stalls);
    RUN_TEST(test_jitter_below_min_progress_is_a_stall);
    RUN_TEST(test_over_travel_either_way);
    RUN_TEST(test_clock_wrap_of_the_timer);
    RUN_TEST(test_limits_are_inside_the_timeout);
    return UNITY_END();
}
