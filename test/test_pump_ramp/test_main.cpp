// Native unit tests for src/Core/pump_ramp (run: pio test -e native).
// The soft start schedule of the pump output (2026-10-12): every start and every big change ramps.
#include <unity.h>

#include "pump_ramp.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_ramp_ends_at_both_values(void)
{
    TEST_ASSERT_EQUAL_INT(0, pump_ramp_percent(0, 100, 0, 200));
    TEST_ASSERT_EQUAL_INT(100, pump_ramp_percent(0, 100, 200, 200));
    TEST_ASSERT_EQUAL_INT(20, pump_ramp_percent(100, 20, 200, 200));
}

void test_ramp_is_linear(void)
{
    TEST_ASSERT_EQUAL_INT(50, pump_ramp_percent(0, 100, 100, 200));
    TEST_ASSERT_EQUAL_INT(25, pump_ramp_percent(0, 100, 50, 200));
    TEST_ASSERT_EQUAL_INT(60, pump_ramp_percent(100, 20, 100, 200));
}

void test_ramp_never_overshoots(void)
{
    // every step stays between from and to, for an up and a down ramp
    for (int i = 0; i <= 200; i++)
    {
        int up = pump_ramp_percent(0, 50, i, 200);
        TEST_ASSERT_TRUE(up >= 0 && up <= 50);
        int down = pump_ramp_percent(80, 20, i, 200);
        TEST_ASSERT_TRUE(down >= 20 && down <= 80);
    }
}

void test_ramp_degenerate_inputs(void)
{
    TEST_ASSERT_EQUAL_INT(70, pump_ramp_percent(0, 70, 5, 0)); // no steps: go straight to the target
    TEST_ASSERT_EQUAL_INT(0, pump_ramp_percent(0, 70, -3, 200));
    TEST_ASSERT_EQUAL_INT(70, pump_ramp_percent(0, 70, 999, 200));
}

void test_start_always_ramps(void)
{
    TEST_ASSERT_TRUE(pump_ramp_needed(0, 0, true, 10)); // even a 0 % start
    TEST_ASSERT_TRUE(pump_ramp_needed(0, 5, true, 10));
    TEST_ASSERT_TRUE(pump_ramp_needed(0, 100, true, 10));
}

void test_live_change_ramps_only_when_big(void)
{
    TEST_ASSERT_FALSE(pump_ramp_needed(50, 55, false, 10)); // PID trim: written at once
    TEST_ASSERT_FALSE(pump_ramp_needed(50, 50, false, 10));
    TEST_ASSERT_TRUE(pump_ramp_needed(50, 60, false, 10));  // exactly the minimum step ramps
    TEST_ASSERT_TRUE(pump_ramp_needed(60, 50, false, 10));  // down as well as up
    TEST_ASSERT_TRUE(pump_ramp_needed(20, 100, false, 10));
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_ramp_ends_at_both_values);
    RUN_TEST(test_ramp_is_linear);
    RUN_TEST(test_ramp_never_overshoots);
    RUN_TEST(test_ramp_degenerate_inputs);
    RUN_TEST(test_start_always_ramps);
    RUN_TEST(test_live_change_ramps_only_when_big);
    return UNITY_END();
}
