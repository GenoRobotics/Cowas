// Native unit tests for src/Core/pressure_calibration (run: pio test -e native).
// The console procedure `calibrate_pressure` (Tests.cpp) is a thin wrapper around these.
#include <unity.h>

#include <math.h>

#include "pressure_calibration.h"
#include "abp_convert.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static PressureCalVerdict verdict(const PressureCalFit &f)
{
    return pressure_cal_verdict(f, 3, 0.9f, 1.1f, 0.05f);
}

static void test_fit_exact_line()
{
    // the sensor reads 2 % low with a 0.03 bar zero: reference = 1.02 x (nominal - 0.03)
    PressureCalPoint p[5];
    const float nominal[5] = {0.03f, 0.52f, 1.01f, 1.50f, 1.99f};
    for (int i = 0; i < 5; i++)
    {
        p[i].nominal_bar = nominal[i];
        p[i].reference_bar = 1.02f * (nominal[i] - 0.03f);
    }
    PressureCalFit f = fit_pressure_cal(p, 5);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_EQUAL_INT(5, f.n);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.02f, f.gain);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.03f, f.zero_offset_bar);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, -1.02f * 0.03f, f.intercept_bar);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, f.r2);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.0f, f.max_residual_bar);
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_PASS, verdict(f));
}

static void test_fit_round_trip_through_the_driver_formula()
{
    // pasting the fitted gain and zero offset into the driver's formula gives the reference back
    const PressureCalPoint p[] = {{0.0f, -0.021f}, {0.5f, 0.468f}, {1.0f, 0.957f}, {2.0f, 1.935f}};
    PressureCalFit f = fit_pressure_cal(p, 4);
    TEST_ASSERT_TRUE(f.valid);
    for (int i = 0; i < 4; i++)
    {
        TEST_ASSERT_FLOAT_WITHIN(2e-3f, p[i].reference_bar,
                                 abp_apply_calibration(p[i].nominal_bar, f.gain, f.zero_offset_bar));
    }
}

static void test_fit_noise()
{
    // unity gain, reading errors of a few hundredths of a bar on the dial
    const PressureCalPoint p[] = {{0.0f, 0.00f}, {0.5f, 0.52f}, {1.0f, 0.98f}, {1.5f, 1.53f}, {2.0f, 1.99f}};
    PressureCalFit f = fit_pressure_cal(p, 5);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.03f, 1.0f, f.gain);
    TEST_ASSERT_TRUE(f.r2 > 0.995f && f.r2 < 1.0f);
    TEST_ASSERT_TRUE(f.max_residual_bar > 0.01f && f.max_residual_bar < 0.05f);
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_PASS, verdict(f));
    // the residual of one point matches the reported worst case in size
    float worst = 0;
    for (int i = 0; i < 5; i++)
    {
        const float r = fabsf(pressure_cal_residual(f, p[i]));
        worst = (r > worst) ? r : worst;
    }
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, f.max_residual_bar, worst);
}

static void test_fit_one_bad_reading_fails_residual()
{
    // the needle was misread by 0.2 bar at one point
    const PressureCalPoint p[] = {{0.0f, 0.0f}, {0.5f, 0.5f}, {1.2f, 1.0f}, {1.5f, 1.5f}, {2.0f, 2.0f}};
    PressureCalFit f = fit_pressure_cal(p, 5);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_TRUE(f.max_residual_bar > 0.05f);
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_FAIL_RESIDUAL, verdict(f));
}

static void test_fit_needs_two_points()
{
    const PressureCalPoint p[] = {{1.0f, 1.0f}, {2.0f, 2.0f}};
    TEST_ASSERT_FALSE(fit_pressure_cal(p, 0).valid);
    TEST_ASSERT_FALSE(fit_pressure_cal(p, 1).valid);
    TEST_ASSERT_FALSE(fit_pressure_cal(0, 2).valid);
    TEST_ASSERT_TRUE(isnan(fit_pressure_cal(p, 1).gain));
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_FAIL_FIT, verdict(fit_pressure_cal(p, 1)));

    PressureCalFit f = fit_pressure_cal(p, 2);
    TEST_ASSERT_TRUE(f.valid); // a line through 2 points...
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, f.gain);
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_FAIL_POINTS, verdict(f)); // ...is not a calibration
}

static void test_fit_rejects_bad_inputs()
{
    const PressureCalPoint same[] = {{0.0f, 1.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}}; // the sensor did not move
    TEST_ASSERT_FALSE(fit_pressure_cal(same, 3).valid);
    const PressureCalPoint with_nan[] = {{0.0f, 0.0f}, {1.0f, NAN}, {2.0f, 2.0f}};
    TEST_ASSERT_FALSE(fit_pressure_cal(with_nan, 3).valid);
    const PressureCalPoint with_inf[] = {{0.0f, 0.0f}, {INFINITY, 1.0f}, {2.0f, 2.0f}};
    TEST_ASSERT_FALSE(fit_pressure_cal(with_inf, 3).valid);
    const PressureCalPoint reversed[] = {{2.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 2.0f}}; // negative slope
    PressureCalFit f = fit_pressure_cal(reversed, 3);
    TEST_ASSERT_FALSE(f.valid);
    TEST_ASSERT_TRUE(isnan(pressure_cal_residual(f, reversed[0])));
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_FAIL_FIT, verdict(f));
}

static void test_gain_outside_bounds_is_refused()
{
    // a 60 psi part read with the 150 psi constant: gain 0.4. Perfect line, still refused.
    const PressureCalPoint wrong_part[] = {{0.0f, 0.0f}, {0.5f, 1.25f}, {1.0f, 2.5f}, {2.0f, 5.0f}};
    PressureCalFit f = fit_pressure_cal(wrong_part, 4);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.4f, f.gain);
    TEST_ASSERT_EQUAL_INT(PRESSURE_CAL_FAIL_GAIN, verdict(f));

    // just inside and just outside the bounds, on both sides
    const float gains[] = {0.905f, 1.095f, 0.895f, 1.105f};
    const PressureCalVerdict expected[] = {PRESSURE_CAL_PASS, PRESSURE_CAL_PASS, PRESSURE_CAL_FAIL_GAIN, PRESSURE_CAL_FAIL_GAIN};
    for (int g = 0; g < 4; g++)
    {
        PressureCalPoint p[3] = {{0.0f, 0.0f}, {gains[g] * 1.0f, 1.0f}, {gains[g] * 2.0f, 2.0f}};
        TEST_ASSERT_EQUAL_INT(expected[g], verdict(fit_pressure_cal(p, 3)));
    }
}

static void test_verdict_names()
{
    TEST_ASSERT_EQUAL_STRING("PASS", pressure_cal_verdict_name(PRESSURE_CAL_PASS));
    TEST_ASSERT_EQUAL_STRING("FIT", pressure_cal_verdict_name(PRESSURE_CAL_FAIL_FIT));
    TEST_ASSERT_EQUAL_STRING("POINTS", pressure_cal_verdict_name(PRESSURE_CAL_FAIL_POINTS));
    TEST_ASSERT_EQUAL_STRING("GAIN", pressure_cal_verdict_name(PRESSURE_CAL_FAIL_GAIN));
    TEST_ASSERT_EQUAL_STRING("RESIDUAL", pressure_cal_verdict_name(PRESSURE_CAL_FAIL_RESIDUAL));
}

static void test_settings_are_consistent()
{
    const int n = sizeof(CAL_PRESSURE_TARGETS_BAR) / sizeof(CAL_PRESSURE_TARGETS_BAR[0]);
    TEST_ASSERT_TRUE(n + 1 >= CAL_PRESSURE_MIN_POINTS); // the zero + the proposed targets can pass
    for (int i = 0; i < n; i++)
    {
        TEST_ASSERT_TRUE(CAL_PRESSURE_TARGETS_BAR[i] > 0.0f);
        TEST_ASSERT_TRUE(CAL_PRESSURE_TARGETS_BAR[i] < CAL_PRESSURE_MAX_REF_BAR); // on the manometer's scale
        if (i > 0)
        {
            TEST_ASSERT_TRUE(CAL_PRESSURE_TARGETS_BAR[i] > CAL_PRESSURE_TARGETS_BAR[i - 1]);
        }
    }
    TEST_ASSERT_TRUE(CAL_PRESSURE_MIN_POINTS >= 3);
    TEST_ASSERT_TRUE(CAL_PRESSURE_GAIN_MIN < 1.0f && CAL_PRESSURE_GAIN_MAX > 1.0f);
    TEST_ASSERT_TRUE(CAL_PRESSURE_MAX_RESIDUAL_BAR >= 0.05f && CAL_PRESSURE_MAX_RESIDUAL_BAR <= 0.1f);
    TEST_ASSERT_TRUE(CAL_PRESSURE_AVG_MS >= 1000 && PRESSURE_ZERO_AVG_MS >= 1000);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_fit_exact_line);
    RUN_TEST(test_fit_round_trip_through_the_driver_formula);
    RUN_TEST(test_fit_noise);
    RUN_TEST(test_fit_one_bad_reading_fails_residual);
    RUN_TEST(test_fit_needs_two_points);
    RUN_TEST(test_fit_rejects_bad_inputs);
    RUN_TEST(test_gain_outside_bounds_is_refused);
    RUN_TEST(test_verdict_names);
    RUN_TEST(test_settings_are_consistent);
    return UNITY_END();
}
