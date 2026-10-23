// Native unit tests for src/Core/sample_record (run: pio test -e native).
// The pressure statistics of a pump run, the end-reason names, and the JSON line of a sample record (golden string).
#include <unity.h>
#include <math.h>
#include <string.h>

#include "sample_record.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_pressure_stats_empty_is_nan(void)
{
    PressureStats s;
    TEST_ASSERT_TRUE(isnan(s.mean()));
    TEST_ASSERT_TRUE(isnan(s.peak()));
    TEST_ASSERT_EQUAL(0, s.count());
}

void test_pressure_stats_mean_and_peak(void)
{
    PressureStats s;
    s.add(1.0f);
    s.add(3.0f);
    s.add(2.0f);
    TEST_ASSERT_EQUAL(3, s.count());
    TEST_ASSERT_FLOAT_WITHIN(1e-5, 2.0f, s.mean());
    TEST_ASSERT_FLOAT_WITHIN(1e-5, 3.0f, s.peak());
}

void test_pressure_stats_reset(void)
{
    PressureStats s;
    s.add(3.5f);
    s.reset();
    TEST_ASSERT_TRUE(isnan(s.peak()));
    s.add(0.5f);
    TEST_ASSERT_FLOAT_WITHIN(1e-5, 0.5f, s.peak()); // a peak from before the reset is gone
}

void test_end_reason_names(void)
{
    TEST_ASSERT_EQUAL_STRING("volume", sample_end_name("cond"));
    TEST_ASSERT_EQUAL_STRING("pressure", sample_end_name("cap"));
    TEST_ASSERT_EQUAL_STRING("stop_key", sample_end_name("aborted"));
    TEST_ASSERT_EQUAL_STRING("timeout", sample_end_name("timeout"));
    TEST_ASSERT_EQUAL_STRING("clogged", sample_end_name("clogged"));
}

static SampleRecord example_record(void)
{
    static const char *const flags[] = {"placeholder:ABP_FULL_SCALE_PSI"};
    SampleRecord r;
    r.fw = "S1-1b-dev";
    r.id = "auto-3";
    r.purpose = "sample";
    r.site = NULL; // not written when NULL (F199)
    r.date = NULL;
    r.condition = NULL;
    r.slot = 5;
    r.start_ms = 1000;
    r.end_ms = 5000;
    r.target_ml = 100.0f;
    r.vol_ml = 101.25f;
    r.vol_trusted_ml = 88.5f;
    r.purge_ml = NAN;
    r.end_reason = "volume";
    r.p_mean_bar = 1.2346f;
    r.p_max_bar = 1.5f;
    r.p_cap_bar = 3.6f;
    r.lock_before = "UNLOCKED";
    r.lock_after = "LOCKED";
    r.shield_ml = NAN;
    r.flow_factor = 77.0f;
    r.p_gain = 1.0413f;
    r.p_offset_bar = -0.0311f;
    r.abp_full_scale_psi = 60.0f;
    r.linear_act_locked_steps = 210;
    r.flags = flags;
    r.n_flags = 1;
    return r;
}

void test_record_golden_line(void)
{
    char buf[700];
    SampleRecord r = example_record();
    int n = format_sample_record(r, buf, sizeof(buf));
    const char *golden =
        "{\"t\":\"sample\",\"fw\":\"S1-1b-dev\",\"id\":\"auto-3\",\"purpose\":\"sample\",\"slot\":5,"
        "\"start_ms\":1000,\"end_ms\":5000,\"target_ml\":100.000,\"vol_ml\":101.250,\"vol_trusted_ml\":88.500,\"purge_ml\":null,"
        "\"end_reason\":\"volume\",\"p_mean_bar\":1.235,\"p_max_bar\":1.500,\"p_cap_bar\":3.600,"
        "\"lock_before\":\"UNLOCKED\",\"lock_after\":\"LOCKED\",\"shield_ml\":null,"
        "\"calib\":{\"flow_factor\":77.000,\"p_gain\":1.041,\"p_offset_bar\":-0.031,"
        "\"abp_full_scale_psi\":60.000,\"linear_act_locked_steps\":210},\"flags\":[\"placeholder:ABP_FULL_SCALE_PSI\"]}";
    TEST_ASSERT_EQUAL((int)strlen(golden), n);
    TEST_ASSERT_EQUAL_STRING(golden, buf);
}

void test_record_nan_is_null_and_no_flags(void)
{
    char buf[600];
    SampleRecord r = example_record();
    r.p_mean_bar = NAN;
    r.p_max_bar = NAN;
    r.n_flags = 0;
    TEST_ASSERT_TRUE(format_sample_record(r, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"p_mean_bar\":null,\"p_max_bar\":null"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"flags\":[]}"));
}

void test_record_writes_site_date_condition(void)
{
    char buf[700];
    SampleRecord r = example_record();
    r.site = "BoisChamblard";
    r.date = "2026-10-23";
    r.condition = "control";
    TEST_ASSERT_TRUE(format_sample_record(r, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"purpose\":\"sample\",\"site\":\"BoisChamblard\",\"date\":\"2026-10-23\",\"condition\":\"control\",\"slot\":5"));
}

void test_record_escapes_id(void)
{
    char buf[600];
    SampleRecord r = example_record();
    r.id = "a\"b\\c";
    TEST_ASSERT_TRUE(format_sample_record(r, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"id\":\"a\\\"b\\\\c\""));
}

void test_record_negative_and_rounding(void)
{
    char buf[600];
    SampleRecord r = example_record();
    r.p_offset_bar = -0.0004f; // rounds to -0.000
    r.vol_ml = 0.9996f;        // rounds to 1.000
    TEST_ASSERT_TRUE(format_sample_record(r, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"p_offset_bar\":-0.000"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"vol_ml\":1.000"));
}

void test_record_too_small_buffer(void)
{
    char buf[40];
    SampleRecord r = example_record();
    TEST_ASSERT_EQUAL(-1, format_sample_record(r, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL(-1, format_sample_record(r, buf, 0));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_pressure_stats_empty_is_nan);
    RUN_TEST(test_pressure_stats_mean_and_peak);
    RUN_TEST(test_pressure_stats_reset);
    RUN_TEST(test_end_reason_names);
    RUN_TEST(test_record_golden_line);
    RUN_TEST(test_record_nan_is_null_and_no_flags);
    RUN_TEST(test_record_writes_site_date_condition);
    RUN_TEST(test_record_escapes_id);
    RUN_TEST(test_record_negative_and_rounding);
    RUN_TEST(test_record_too_small_buffer);
    return UNITY_END();
}
