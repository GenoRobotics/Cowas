// Native unit tests for src/Core/pressure_ramp (run: pio test -e native).
// The setpoint profile, plan check, slew limit and the pressure low-pass of the `pressure_ramp` console test.
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

void test_lowpass(void)
{
    TEST_ASSERT_EQUAL_FLOAT(3.0f, lowpass_step(1.0f, 3.0f, 0.1f, 0.0f)); // filter off: the raw value
    float f = 0.0f;
    for (int i = 0; i < 300; i++)
    {
        f = lowpass_step(f, 1.5f, 0.1f, 2.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 1.5f, f); // a constant input is reached
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.5f, lowpass_step(0.0f, 1.0f, 1.0f, 1.0f)); // dt = tau: halfway
}

void test_settings_are_consistent(void)
{
    // the pressure_ramp defaults pass the plan check against the real cap
    TEST_ASSERT_EQUAL(RAMP_PLAN_OK, ramp_plan_check(PRESSURE_RAMP_DEFAULT_TARGET_BAR, STX_MAX_PRESSURE, PRESSURE_RAMP_CAP_MARGIN_BAR,
                                                    PRESSURE_RAMP_DEFAULT_RATE_BAR_S, PRESSURE_RAMP_MIN_RATE_BAR_S, PRESSURE_RAMP_MAX_RATE_BAR_S));
    // fill_test: the target sits below the resume pressure, which sits below the pause pressure, which sits below the cap
    const float pause_bar = STX_MAX_PRESSURE - FILL_PAUSE_BELOW_CAP_BAR;
    const float resume_bar = pause_bar - FILL_RESUME_BELOW_PAUSE_BAR;
    TEST_ASSERT_TRUE(FILL_MAX_TARGET_BAR <= resume_bar);
    TEST_ASSERT_TRUE(FILL_RESUME_BAR == resume_bar && FILL_PAUSE_BAR == pause_bar);
    TEST_ASSERT_TRUE(resume_bar < pause_bar && pause_bar < STX_MAX_PRESSURE);
    TEST_ASSERT_TRUE(PUMP_MINIMUM_POWER_PERCENT == 10); // the deadband floor the controller relies on
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
    RUN_TEST(test_lowpass);
    RUN_TEST(test_settings_are_consistent);
    return UNITY_END();
}
