// Native unit tests for src/Core/pump_start (run: pio test -e native). F196: the pump rises from 0 % after a start.
#include <unity.h>

#include "pump_start.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_starts_at_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pump_start_limit(0, 5.0f));
}

void test_rises_at_the_rate(void)
{
    TEST_ASSERT_EQUAL_FLOAT(5.0f, pump_start_limit(1000, 5.0f));
    TEST_ASSERT_EQUAL_FLOAT(15.0f, pump_start_limit(3000, 5.0f)); // 15 % at 3 s, not at once
}

void test_fifteen_percent_is_not_reached_at_once(void)
{
    // the old start: 15 % at 0 s. The limit must be well under it in the first 100 ms (the first PID update)
    TEST_ASSERT_TRUE(pump_start_limit(100, 5.0f) < 1.0f);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_at_zero);
    RUN_TEST(test_rises_at_the_rate);
    RUN_TEST(test_fifteen_percent_is_not_reached_at_once);
    return UNITY_END();
}
