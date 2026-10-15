// Native unit tests for src/Core/pressure_ramp (run: pio test -e native).
// The setpoint profile, plan check, slew limit and soft limit of the `pressure_ramp` console test.
#include <unity.h>

#include "pressure_ramp.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_setpoint_starts_at_start_and_ends_at_target(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ramp_setpoint(0.0f, 2.8f, 0.05f, 0));
    TEST_ASSERT_EQUAL_FLOAT(2.8f, ramp_setpoint(0.0f, 2.8f, 0.05f, 56000));
    TEST_ASSERT_EQUAL_FLOAT(2.8f, ramp_setpoint(0.0f, 2.8f, 0.05f, 600000)); // stays there
}

void test_setpoint_is_linear(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.5f, ramp_setpoint(0.0f, 2.8f, 0.05f, 10000));
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.4f, ramp_setpoint(0.0f, 2.8f, 0.05f, 28000));
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.7f, ramp_setpoint(0.5f, 2.8f, 0.05f, 4000));
}

void test_setpoint_never_passes_the_target(void)
{
    for (uint32_t t = 0; t <= 120000; t += 250)
    {
        float up = ramp_setpoint(0.0f, 2.8f, 0.05f, t);
        TEST_ASSERT_TRUE(up >= 0.0f && up <= 2.8f);
        float down = ramp_setpoint(2.8f, 0.5f, 0.05f, t);
        TEST_ASSERT_TRUE(down >= 0.5f && down <= 2.8f);
    }
}

void test_setpoint_zero_rate_stays_at_start(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.3f, ramp_setpoint(0.3f, 2.8f, 0.0f, 100000));
}

void test_duration(void)
{
    TEST_ASSERT_EQUAL_UINT32(56000, ramp_duration_ms(0.0f, 2.8f, 0.05f));
    TEST_ASSERT_EQUAL_UINT32(28000, ramp_duration_ms(0.0f, 2.8f, 0.1f));
    TEST_ASSERT_EQUAL_UINT32(28000, ramp_duration_ms(2.8f, 0.0f, 0.1f)); // falling
    TEST_ASSERT_EQUAL_UINT32(0, ramp_duration_ms(1.0f, 1.0f, 0.1f));
    TEST_ASSERT_EQUAL_UINT32(0, ramp_duration_ms(0.0f, 2.0f, 0.0f));
    // the setpoint really is at the target at that time
    TEST_ASSERT_EQUAL_FLOAT(2.0f, ramp_setpoint(0.0f, 2.0f, 0.07f, ramp_duration_ms(0.0f, 2.0f, 0.07f)));
}

void test_plan_check(void)
{
    TEST_ASSERT_EQUAL(RAMP_PLAN_OK, ramp_plan_check(2.8f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_TARGET_TOO_HIGH, ramp_plan_check(2.9f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_TARGET_TOO_HIGH, ramp_plan_check(3.0f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_TARGET_TOO_LOW, ramp_plan_check(0.0f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_TARGET_TOO_LOW, ramp_plan_check(-1.0f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_RATE_OUT_OF_RANGE, ramp_plan_check(2.0f, 3.0f, 0.2f, 0.005f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_RATE_OUT_OF_RANGE, ramp_plan_check(2.0f, 3.0f, 0.2f, 0.6f, 0.01f, 0.5f));
    TEST_ASSERT_EQUAL(RAMP_PLAN_RATE_OUT_OF_RANGE, ramp_plan_check(2.0f, 3.0f, 0.2f, 0.0f, 0.01f, 0.5f));
    // NaN is refused, never accepted by a comparison that happens to be false
    TEST_ASSERT_EQUAL(RAMP_PLAN_TARGET_TOO_LOW, ramp_plan_check(0.0f / 0.0f, 3.0f, 0.2f, 0.05f, 0.01f, 0.5f));
}

void test_slew_limit(void)
{
    TEST_ASSERT_EQUAL_FLOAT(13.0f, slew_limit(10.0f, 50.0f, 3.0f));
    TEST_ASSERT_EQUAL_FLOAT(7.0f, slew_limit(10.0f, 0.0f, 3.0f));
    TEST_ASSERT_EQUAL_FLOAT(11.0f, slew_limit(10.0f, 11.0f, 3.0f)); // within a step: unchanged
    TEST_ASSERT_EQUAL_FLOAT(10.0f, slew_limit(10.0f, 10.0f, 3.0f));
}

void test_soft_limit(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 2.95f, ramp_soft_limit(2.8f, 0.15f, 3.5f, 0.1f)); // target + overshoot, far from the cap
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 2.9f, ramp_soft_limit(2.8f, 0.15f, 3.0f, 0.1f));  // the defaults: clipped to cap - margin
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 2.9f, ramp_soft_limit(2.8f, 0.5f, 3.0f, 0.1f));
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.15f, ramp_soft_limit(1.0f, 0.15f, 3.0f, 0.1f));
}

void test_settings_are_consistent(void)
{
    // the defaults pass the plan check against the real cap
    TEST_ASSERT_EQUAL(RAMP_PLAN_OK, ramp_plan_check(PRESSURE_RAMP_DEFAULT_TARGET_BAR, STX_MAX_PRESSURE, PRESSURE_RAMP_CAP_MARGIN_BAR,
                                                    PRESSURE_RAMP_DEFAULT_RATE_BAR_S, PRESSURE_RAMP_MIN_RATE_BAR_S, PRESSURE_RAMP_MAX_RATE_BAR_S));
    // the soft stop is above the target and below the cap
    float soft = ramp_soft_limit(PRESSURE_RAMP_DEFAULT_TARGET_BAR, PRESSURE_RAMP_OVERSHOOT_STOP_BAR, STX_MAX_PRESSURE, PRESSURE_RAMP_SOFT_CAP_MARGIN_BAR);
    TEST_ASSERT_TRUE(soft > PRESSURE_RAMP_DEFAULT_TARGET_BAR);
    TEST_ASSERT_TRUE(soft < STX_MAX_PRESSURE);
    // a PID step must never trigger the blocking 4 s ramp of Pump::set_power (F132)
    TEST_ASSERT_TRUE(PRESSURE_RAMP_MAX_STEP_PERCENT < PUMP_RAMP_MIN_STEP_PERCENT);
    TEST_ASSERT_TRUE(PRESSURE_RAMP_START_POWER <= PRESSURE_RAMP_MAX_POWER);
    TEST_ASSERT_TRUE(PRESSURE_RAMP_MAX_POWER <= 100);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_setpoint_starts_at_start_and_ends_at_target);
    RUN_TEST(test_setpoint_is_linear);
    RUN_TEST(test_setpoint_never_passes_the_target);
    RUN_TEST(test_setpoint_zero_rate_stays_at_start);
    RUN_TEST(test_duration);
    RUN_TEST(test_plan_check);
    RUN_TEST(test_slew_limit);
    RUN_TEST(test_soft_limit);
    RUN_TEST(test_settings_are_consistent);
    return UNITY_END();
}
