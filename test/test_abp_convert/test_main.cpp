// Native unit tests for src/Core/abp_convert (run: pio test -e native).
// The ABP driver (src/Hardware/Trustability_ABP_Gage.cpp) is a thin wrapper around these:
// frame classification, counts -> bar for 60 and 150 psi parts, gain / zero, and the rule
// that makes a dead or frozen sensor untrusted (S1-1, F27).
#include <unity.h>

#include <math.h>

#include "abp_convert.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

// --- frame classification ---

static void test_classify_status_bits()
{
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_VALID, abp_classify_frame(0x06, 0x66, 0x5A));        // 00
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_COMMAND_MODE, abp_classify_frame(0x46, 0x66, 0x5A)); // 01
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_STALE, abp_classify_frame(0x86, 0x66, 0x5A));        // 10
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_DIAGNOSTIC, abp_classify_frame(0xC6, 0x66, 0x5A));   // 11
}

static void test_classify_dead_lines()
{
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_NO_ANSWER, abp_classify_frame(0xFF, 0xFF, 0xFF));
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_STUCK_LOW, abp_classify_frame(0x00, 0x00, 0x00));
    // only the exact patterns: one differing byte is an ordinary frame
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_DIAGNOSTIC, abp_classify_frame(0xFF, 0xFF, 0xFE));
    TEST_ASSERT_EQUAL_INT(ABP_FRAME_VALID, abp_classify_frame(0x00, 0x00, 0x01));
}

static void test_status_names()
{
    TEST_ASSERT_EQUAL_STRING("valid", abp_frame_status_name(ABP_FRAME_VALID));
    TEST_ASSERT_EQUAL_STRING("stale", abp_frame_status_name(ABP_FRAME_STALE));
    TEST_ASSERT_EQUAL_STRING("command_mode", abp_frame_status_name(ABP_FRAME_COMMAND_MODE));
    TEST_ASSERT_EQUAL_STRING("diagnostic", abp_frame_status_name(ABP_FRAME_DIAGNOSTIC));
    TEST_ASSERT_EQUAL_STRING("no_answer", abp_frame_status_name(ABP_FRAME_NO_ANSWER));
    TEST_ASSERT_EQUAL_STRING("stuck_low", abp_frame_status_name(ABP_FRAME_STUCK_LOW));
}

static void test_frame_counts_masks_status()
{
    TEST_ASSERT_EQUAL_UINT16(1638, abp_frame_counts(0x06, 0x66));
    TEST_ASSERT_EQUAL_UINT16(1638, abp_frame_counts(0x86, 0x66)); // stale: same count
    TEST_ASSERT_EQUAL_UINT16(14745, abp_frame_counts(0x39, 0x99));
    TEST_ASSERT_EQUAL_UINT16(16383, abp_frame_counts(0xFF, 0xFF));
}

// --- counts -> bar ---

static void test_counts_to_bar_150_psi()
{
    const float fs = 150.0f * 0.0689476f; // 10.342 bar
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, abp_counts_to_bar(1638, 150.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, fs, abp_counts_to_bar(14745, 150.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, fs / 2, abp_counts_to_bar(8191, 150.0f)); // midpoint 8191.5
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, fs / 2, abp_counts_to_bar(8192, 150.0f));
    // the bench quick check: 1.0 and 2.0 bar
    TEST_ASSERT_FLOAT_WITHIN(2e-3f, 1.0f, abp_counts_to_bar(2905, 150.0f));
    TEST_ASSERT_FLOAT_WITHIN(2e-3f, 2.0f, abp_counts_to_bar(4173, 150.0f));
}

static void test_counts_to_bar_60_psi()
{
    const float fs = 60.0f * 0.0689476f; // 4.137 bar
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, abp_counts_to_bar(1638, 60.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, fs, abp_counts_to_bar(14745, 60.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, fs / 2, abp_counts_to_bar(8192, 60.0f));
}

static void test_counts_to_bar_below_zero_and_bad_scale()
{
    // below 10 %: a small negative gauge pressure (suction), not an error
    TEST_ASSERT_TRUE(abp_counts_to_bar(1600, 150.0f) < 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, -1.2925f, abp_counts_to_bar(0, 150.0f));
    TEST_ASSERT_TRUE(isnan(abp_counts_to_bar(2000, 0.0f)));
    TEST_ASSERT_TRUE(isnan(abp_counts_to_bar(2000, -150.0f)));
    TEST_ASSERT_TRUE(isnan(abp_counts_to_bar(2000, NAN)));
}

static void test_old_constants_read_low_on_150_psi()
{
    // F27: 7.63e-4 x counts - 1.25 assumed a 10.0 bar part; on the 150 psi part it read 3.3 % low
    const uint16_t counts = 4807; // 2.5 bar on a 150 psi part
    const float now = abp_counts_to_bar(counts, 150.0f);
    const float before = counts * 7.63e-4f - 1.25f;
    TEST_ASSERT_FLOAT_WITHIN(2e-3f, 2.5f, now);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.967f, before / now);
}

static void test_apply_calibration()
{
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.234f, abp_apply_calibration(1.234f, 1.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, abp_apply_calibration(0.03f, 1.05f, 0.03f)); // the zero stays the zero at any gain
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 2.1f, abp_apply_calibration(2.03f, 1.05f, 0.03f));
}

// --- trust ---

static void test_trust_valid_frames()
{
    AbpTrust t = abp_trust_init();
    TEST_ASSERT_FALSE(t.has_valid);
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_VALID, 10, 500));
    TEST_ASSERT_TRUE(t.has_valid);
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_VALID, 100000, 500)); // a long pause between polls is fine
}

static void test_trust_stale_keeps_value_then_times_out()
{
    AbpTrust t = abp_trust_init();
    abp_trust_update(t, ABP_FRAME_VALID, 1000, 500);
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 1002, 500));
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 1501, 500)); // 499 ms into the run
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_STALE, abp_trust_update(t, ABP_FRAME_STALE, 1502, 500));
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_STALE, abp_trust_update(t, ABP_FRAME_STALE, 9000, 500));
    // one valid frame restores it and restarts the run
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_VALID, 9002, 500));
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 9004, 500));
}

static void test_trust_stale_run_counts_from_the_first_stale_frame()
{
    // a single stale frame after a long idle time must not trip: the run starts there
    AbpTrust t = abp_trust_init();
    abp_trust_update(t, ABP_FRAME_VALID, 1000, 500);
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 60000, 500));
    // ...but a sensor that stays stale trips even when it is polled slowly
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_STALE, abp_trust_update(t, ABP_FRAME_STALE, 61000, 500));
}

static void test_trust_stale_without_a_valid_frame()
{
    AbpTrust t = abp_trust_init();
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_NEVER_VALID, abp_trust_update(t, ABP_FRAME_STALE, 5, 500));
}

static void test_trust_faults_are_immediate_and_need_a_new_valid_frame()
{
    const AbpFrameStatus faults[] = {ABP_FRAME_COMMAND_MODE, ABP_FRAME_DIAGNOSTIC, ABP_FRAME_NO_ANSWER, ABP_FRAME_STUCK_LOW};
    for (int i = 0; i < 4; i++)
    {
        AbpTrust t = abp_trust_init();
        abp_trust_update(t, ABP_FRAME_VALID, 1000, 500);
        TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_FAULT, abp_trust_update(t, faults[i], 1002, 500));
        // a stale frame right after a fault can't bring the old value back
        TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_NEVER_VALID, abp_trust_update(t, ABP_FRAME_STALE, 1004, 500));
        TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_VALID, 1006, 500));
    }
}

static void test_trust_millis_wrap()
{
    AbpTrust t = abp_trust_init();
    abp_trust_update(t, ABP_FRAME_VALID, 0xFFFFFF00u, 500);
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 0xFFFFFF38u, 500)); // run starts 200 ms before the wrap
    TEST_ASSERT_EQUAL_INT(ABP_TRUSTED, abp_trust_update(t, ABP_FRAME_STALE, 100, 500));         // 300 ms later
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_STALE, abp_trust_update(t, ABP_FRAME_STALE, 300, 500)); // 500 ms later
}

static void test_trust_zero_timeout()
{
    AbpTrust t = abp_trust_init();
    abp_trust_update(t, ABP_FRAME_VALID, 1000, 0);
    TEST_ASSERT_EQUAL_INT(ABP_UNTRUSTED_STALE, abp_trust_update(t, ABP_FRAME_STALE, 1002, 0));
}

static void test_trust_names()
{
    TEST_ASSERT_EQUAL_STRING("trusted", abp_trust_state_name(ABP_TRUSTED));
    TEST_ASSERT_EQUAL_STRING("fault", abp_trust_state_name(ABP_UNTRUSTED_FAULT));
    TEST_ASSERT_EQUAL_STRING("stale", abp_trust_state_name(ABP_UNTRUSTED_STALE));
    TEST_ASSERT_EQUAL_STRING("never_valid", abp_trust_state_name(ABP_UNTRUSTED_NEVER_VALID));
}

// --- Settings.h ---

static void test_settings_are_consistent()
{
    // the "not trusted" value must trip the existing plausibility checks
    TEST_ASSERT_TRUE(PRESSURE_SENSOR_INVALID_BAR < PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR);
    // ...and a real reading never looks implausible at rest: 0 counts is -1.29 bar on 150 psi,
    // the plausibility bound sits between that and 0
    TEST_ASSERT_TRUE(PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR < 0.0f);
    TEST_ASSERT_TRUE(ABP_FULL_SCALE_PSI > 0.0f);
    // the cap is inside the sensor's range, with room above it to see an overshoot
    TEST_ASSERT_TRUE(STX_MAX_PRESSURE > 0.0f);
    TEST_ASSERT_TRUE(STX_MAX_PRESSURE < 0.9f * abp_counts_to_bar(ABP_COUNTS_MAX, ABP_FULL_SCALE_PSI));
    TEST_ASSERT_TRUE(ABP_STALE_TIMEOUT_MS > 0 && ABP_STALE_TIMEOUT_MS <= 1000); // the dead-sensor stop is checked at 1 s
    TEST_ASSERT_TRUE(PRESSURE_GAIN >= CAL_PRESSURE_GAIN_MIN && PRESSURE_GAIN <= CAL_PRESSURE_GAIN_MAX);
    TEST_ASSERT_TRUE(fabsf(PRESSURE_ZERO_OFFSET_BAR) <= PRESSURE_ZERO_MAX_ABS_BAR);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_classify_status_bits);
    RUN_TEST(test_classify_dead_lines);
    RUN_TEST(test_status_names);
    RUN_TEST(test_frame_counts_masks_status);
    RUN_TEST(test_counts_to_bar_150_psi);
    RUN_TEST(test_counts_to_bar_60_psi);
    RUN_TEST(test_counts_to_bar_below_zero_and_bad_scale);
    RUN_TEST(test_old_constants_read_low_on_150_psi);
    RUN_TEST(test_apply_calibration);
    RUN_TEST(test_trust_valid_frames);
    RUN_TEST(test_trust_stale_keeps_value_then_times_out);
    RUN_TEST(test_trust_stale_run_counts_from_the_first_stale_frame);
    RUN_TEST(test_trust_stale_without_a_valid_frame);
    RUN_TEST(test_trust_faults_are_immediate_and_need_a_new_valid_frame);
    RUN_TEST(test_trust_millis_wrap);
    RUN_TEST(test_trust_zero_timeout);
    RUN_TEST(test_trust_names);
    RUN_TEST(test_settings_are_consistent);
    return UNITY_END();
}
