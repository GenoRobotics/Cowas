// Native unit tests for src/Core/shield_calibration (run: pio test -e native).
// The LA3 console procedure `shield_calibrate` (Tests.cpp) is a thin wrapper around these.
#include <unity.h>

#include <math.h>

#include "shield_calibration.h"
#include "Settings.h"

using namespace core;

void setUp() {}
void tearDown() {}

// --- mass -> volume, tube volume ---

static void test_mass_to_ml()
{
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.5f, shield_mass_to_ml(1.5f, 1.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.0f, shield_mass_to_ml(1.05f, 1.05f));
    TEST_ASSERT_TRUE(isnan(shield_mass_to_ml(1.0f, 0.0f)));
    TEST_ASSERT_TRUE(isnan(shield_mass_to_ml(1.0f, -1.0f)));
    TEST_ASSERT_TRUE(isnan(shield_mass_to_ml(NAN, 1.0f)));
    TEST_ASSERT_TRUE(isnan(shield_mass_to_ml(1.0f, NAN)));
}

static void test_tube_volume()
{
    // 1 m of 2 mm ID: pi x 1^2 x 1000 mm^3 = 3.1416 mL
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 3.14159f, tube_volume_ml(2.0f, 1.0f));
    // the ROADMAP §3.7 example: 5 m of 6 mm ID = 141 mL
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 141.4f, tube_volume_ml(6.0f, 5.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, tube_volume_ml(2.0f, 0.0f));
    TEST_ASSERT_TRUE(isnan(tube_volume_ml(-1.0f, 1.0f)));
    TEST_ASSERT_TRUE(isnan(tube_volume_ml(2.0f, -1.0f)));
}

// --- fit ---

static void test_fit_exact_line()
{
    // 0.05 mL/s, offset -0.1 mL (spin-up loss), density 1.0
    const ShieldRun runs[] = {{10000, 0.4f}, {20000, 0.9f}, {40000, 1.9f},
                              {10000, 0.4f}, {20000, 0.9f}, {40000, 1.9f}};
    ShieldFit f = fit_shield_rate(runs, 6, 1.0f);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_EQUAL_INT(6, f.n_runs);
    TEST_ASSERT_EQUAL_INT(3, f.n_durations);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.05f, f.rate_ml_per_s);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, -0.1f, f.offset_ml);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, f.r2);
}

static void test_fit_uses_density()
{
    // same volumes as above, weighed with a 1.1 g/mL product
    const ShieldRun runs[] = {{10000, 0.44f}, {20000, 0.99f}, {40000, 2.09f}};
    ShieldFit f = fit_shield_rate(runs, 3, 1.1f);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.05f, f.rate_ml_per_s);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, -0.1f, f.offset_ml);
}

static void test_fit_noise_lowers_r2()
{
    const ShieldRun runs[] = {{10000, 0.50f}, {20000, 0.90f}, {40000, 2.10f},
                              {10000, 0.30f}, {20000, 1.00f}, {40000, 1.80f}};
    ShieldFit f = fit_shield_rate(runs, 6, 1.0f);
    TEST_ASSERT_TRUE(f.valid);
    TEST_ASSERT_TRUE(f.r2 < 0.99f);
    TEST_ASSERT_TRUE(f.r2 > 0.9f);
}

static void test_fit_needs_two_durations()
{
    const ShieldRun runs[] = {{10000, 0.5f}, {10000, 0.51f}, {10000, 0.49f}};
    ShieldFit f = fit_shield_rate(runs, 3, 1.0f);
    TEST_ASSERT_FALSE(f.valid);
    TEST_ASSERT_EQUAL_INT(1, f.n_durations);
}

static void test_fit_rejects_bad_inputs()
{
    const ShieldRun ok[] = {{10000, 0.5f}, {20000, 1.0f}};
    TEST_ASSERT_FALSE(fit_shield_rate(ok, 0, 1.0f).valid);
    TEST_ASSERT_FALSE(fit_shield_rate(0, 2, 1.0f).valid);
    TEST_ASSERT_FALSE(fit_shield_rate(ok, 2, 0.0f).valid); // no density
    const ShieldRun nan_mass[] = {{10000, NAN}, {20000, 1.0f}};
    TEST_ASSERT_FALSE(fit_shield_rate(nan_mass, 2, 1.0f).valid);
    const ShieldRun zero_time[] = {{0, 0.5f}, {20000, 1.0f}};
    TEST_ASSERT_FALSE(fit_shield_rate(zero_time, 2, 1.0f).valid);
    // more volume at the shorter time: a negative rate is not a calibration
    const ShieldRun falling[] = {{10000, 1.0f}, {20000, 0.5f}};
    TEST_ASSERT_FALSE(fit_shield_rate(falling, 2, 1.0f).valid);
}

// --- stats per duration ---

static void test_duration_stats()
{
    const ShieldRun runs[] = {{10000, 0.50f}, {20000, 1.00f}, {10000, 0.52f},
                              {20000, 1.00f}, {10000, 0.48f}, {20000, 1.00f}};
    ShieldDurationStats s[3];
    int n = shield_duration_stats(runs, 6, 1.0f, s, 3);
    TEST_ASSERT_EQUAL_INT(2, n);
    TEST_ASSERT_EQUAL_UINT32(10000, s[0].duration_ms); // order of first appearance
    TEST_ASSERT_EQUAL_INT(3, s[0].n);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.50f, s[0].mean_ml);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.02f, s[0].sd_ml);  // sample SD of 0.50, 0.52, 0.48
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 4.0f, s[0].cv_pct);
    TEST_ASSERT_EQUAL_UINT32(20000, s[1].duration_ms);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, s[1].sd_ml);
}

static void test_duration_stats_single_run_and_capacity()
{
    const ShieldRun runs[] = {{10000, 0.5f}, {20000, 1.0f}, {40000, 2.0f}};
    ShieldDurationStats s[2];
    int n = shield_duration_stats(runs, 3, 1.0f, s, 2);
    TEST_ASSERT_EQUAL_INT(2, n); // capacity respected
    TEST_ASSERT_EQUAL_INT(1, s[0].n);
    TEST_ASSERT_TRUE(isnan(s[0].sd_ml));
    TEST_ASSERT_TRUE(isnan(s[0].cv_pct));
}

// --- fill time ---

static void test_fill_ms()
{
    ShieldFit f = {true, 6, 3, 0.05f, -0.1f, 1.0f};
    // (1.5 + 0.1) / 0.05 = 32 s
    TEST_ASSERT_EQUAL_UINT32(32000, shield_fill_ms(f, 1.5f, 300000));
    // bound: exactly at max is accepted, just above is refused
    TEST_ASSERT_EQUAL_UINT32(32000, shield_fill_ms(f, 1.5f, 32000));
    TEST_ASSERT_EQUAL_UINT32(0, shield_fill_ms(f, 1.5f, 31999));
    // target at or below the offset: no time can give it
    ShieldFit g = {true, 6, 3, 0.05f, 0.2f, 1.0f};
    TEST_ASSERT_EQUAL_UINT32(0, shield_fill_ms(g, 0.2f, 300000));
    TEST_ASSERT_EQUAL_UINT32(0, shield_fill_ms(g, 0.1f, 300000));
    TEST_ASSERT_EQUAL_UINT32(0, shield_fill_ms(f, NAN, 300000));
    ShieldFit bad = f;
    bad.valid = false;
    TEST_ASSERT_EQUAL_UINT32(0, shield_fill_ms(bad, 1.5f, 300000));
}

static void test_fill_ms_round_trip()
{
    // the fill time, fed back into the line, gives the target
    const ShieldRun runs[] = {{10000, 0.61f}, {20000, 1.18f}, {40000, 2.37f},
                              {10000, 0.60f}, {20000, 1.20f}, {40000, 2.35f}};
    ShieldFit f = fit_shield_rate(runs, 6, 1.02f);
    uint32_t ms = shield_fill_ms(f, 1.5f, 300000);
    TEST_ASSERT_TRUE(ms > 0);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 1.5f, f.rate_ml_per_s * ms / 1000.0f + f.offset_ml);
}

// --- verdict ---

static void test_verdict_order_and_boundaries()
{
    ShieldFit f = {true, 6, 2, 0.05f, 0.0f, 0.999f};
    ShieldDurationStats s[2] = {{10000, 3, 0.5f, 0.01f, 2.0f}, {20000, 3, 1.0f, 0.02f, 2.0f}};
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_PASS, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));

    // CV exactly at the limit passes, just above fails
    s[1].cv_pct = 5.0f;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_PASS, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    s[1].cv_pct = 5.01f;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_FAIL_REPEATABILITY, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    s[1].cv_pct = 2.0f;

    // r2 exactly at the limit passes, just below fails; NaN fails
    f.r2 = 0.995f;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_PASS, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    f.r2 = 0.9949f;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_FAIL_LINEARITY, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    f.r2 = NAN;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_FAIL_LINEARITY, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    f.r2 = 0.999f;

    // too few repeats beats a bad CV; an invalid fit beats everything
    s[0].n = 2;
    s[0].cv_pct = 50.0f;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_FAIL_REPEATS, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
    f.valid = false;
    TEST_ASSERT_EQUAL_INT(SHIELD_CAL_FAIL_FIT, shield_cal_verdict(f, s, 2, 3, 5.0f, 0.995f));
}

static void test_verdict_names()
{
    TEST_ASSERT_EQUAL_STRING("PASS", shield_cal_verdict_name(SHIELD_CAL_PASS));
    TEST_ASSERT_EQUAL_STRING("FIT", shield_cal_verdict_name(SHIELD_CAL_FAIL_FIT));
    TEST_ASSERT_EQUAL_STRING("REPEATS", shield_cal_verdict_name(SHIELD_CAL_FAIL_REPEATS));
    TEST_ASSERT_EQUAL_STRING("REPEATABILITY", shield_cal_verdict_name(SHIELD_CAL_FAIL_REPEATABILITY));
    TEST_ASSERT_EQUAL_STRING("LINEARITY", shield_cal_verdict_name(SHIELD_CAL_FAIL_LINEARITY));
}

// --- the real Settings.h values ---

static void test_settings_are_consistent()
{
    const int n = sizeof(SHIELD_CAL_DEFAULT_MS) / sizeof(SHIELD_CAL_DEFAULT_MS[0]);
    TEST_ASSERT_TRUE(n >= 2 && n <= 3);
    for (int i = 0; i < n; i++)
    {
        TEST_ASSERT_TRUE(SHIELD_CAL_DEFAULT_MS[i] >= 1000);
        TEST_ASSERT_TRUE(SHIELD_CAL_DEFAULT_MS[i] <= SHIELD_CAL_MAX_RUN_MS);
        for (int j = 0; j < i; j++)
        {
            TEST_ASSERT_TRUE(SHIELD_CAL_DEFAULT_MS[i] != SHIELD_CAL_DEFAULT_MS[j]);
        }
    }
    TEST_ASSERT_TRUE(SHIELD_CAL_REPEATS >= 2); // an SD needs 2 runs
    TEST_ASSERT_TRUE(SHIELD_DENSITY_G_PER_ML > 0.5f && SHIELD_DENSITY_G_PER_ML < 2.0f);
    TEST_ASSERT_TRUE(SHIELD_TARGET_ML > 0.0f);
    TEST_ASSERT_TRUE(SHIELD_PRIME_MARGIN >= 1.0f);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_mass_to_ml);
    RUN_TEST(test_tube_volume);
    RUN_TEST(test_fit_exact_line);
    RUN_TEST(test_fit_uses_density);
    RUN_TEST(test_fit_noise_lowers_r2);
    RUN_TEST(test_fit_needs_two_durations);
    RUN_TEST(test_fit_rejects_bad_inputs);
    RUN_TEST(test_duration_stats);
    RUN_TEST(test_duration_stats_single_run_and_capacity);
    RUN_TEST(test_fill_ms);
    RUN_TEST(test_fill_ms_round_trip);
    RUN_TEST(test_verdict_order_and_boundaries);
    RUN_TEST(test_verdict_names);
    RUN_TEST(test_settings_are_consistent);
    return UNITY_END();
}
