// Native unit tests for src/Core/pump_power (run: pio test -e native).
// The power limit of the operation pump: water 60 %, air 100 %, and the purge exception at 100 % (user 2026-10-26).
#include <unity.h>

#include "pump_power.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

// the firmware values: PUMP_MAX_POWER_PERCENT 60, PUMP_AIR_MAX_POWER_PERCENT 100, PUMP_PURGE_MAX_POWER_PERCENT 100
void test_water_is_limited_to_60(void)
{
    TEST_ASSERT_EQUAL_FLOAT(60.0f, pump_power_limit_percent(false, false, 60.0f, 100.0f, 100.0f));
}

void test_air_is_full_power(void)
{
    TEST_ASSERT_EQUAL_FLOAT(100.0f, pump_power_limit_percent(true, false, 60.0f, 100.0f, 100.0f));
}

void test_purge_is_full_power_on_water(void)
{
    TEST_ASSERT_EQUAL_FLOAT(100.0f, pump_power_limit_percent(false, true, 60.0f, 100.0f, 100.0f));
}

// the exception is for the purge only: a run without the flag keeps the water limit
void test_full_power_flag_is_needed(void)
{
    TEST_ASSERT_EQUAL_FLOAT(60.0f, pump_power_limit_percent(false, false, 60.0f, 100.0f, 80.0f));
    TEST_ASSERT_EQUAL_FLOAT(80.0f, pump_power_limit_percent(false, true, 60.0f, 100.0f, 80.0f));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_water_is_limited_to_60);
    RUN_TEST(test_air_is_full_power);
    RUN_TEST(test_purge_is_full_power_on_water);
    RUN_TEST(test_full_power_flag_is_needed);
    return UNITY_END();
}
