// Native unit tests for src/Core/pid_metrics (run: pio test -e native).
// The numbers that tell whether a pressure PID got better or worse (pressure_ramp, S1-1b).
#include <unity.h>

#include "pid_metrics.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static PidRunStats fresh(float target = 2.0f)
{
    PidRunStats s;
    pid_stats_begin(&s, target, 0.9f, 5.0f, 100.0f);
    return s;
}

void test_overshoot_and_pmax(void)
{
    PidRunStats s = fresh();
    pid_stats_add(&s, 0, 0.0f, 10, false);
    pid_stats_add(&s, 100, 2.3f, 50, false);
    pid_stats_add(&s, 200, 2.0f, 40, false);
    TEST_ASSERT_EQUAL_FLOAT(2.3f, s.p_max);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.3f, pid_stats_overshoot(&s));
}

void test_no_overshoot_when_below_target(void)
{
    PidRunStats s = fresh();
    pid_stats_add(&s, 0, 1.5f, 10, false);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pid_stats_overshoot(&s));
}

void test_rise_time_is_first_crossing_of_90_percent(void)
{
    PidRunStats s = fresh();
    pid_stats_add(&s, 0, 0.0f, 10, false);
    pid_stats_add(&s, 1000, 1.79f, 10, false); // just below 1.8
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, s.rise_s);
    pid_stats_add(&s, 2500, 1.8f, 10, false);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, s.rise_s);
    pid_stats_add(&s, 3000, 0.5f, 10, false); // a later dip does not move it
    TEST_ASSERT_EQUAL_FLOAT(2.5f, s.rise_s);
}

void test_late_hold_statistics(void)
{
    PidRunStats s = fresh();
    pid_stats_add(&s, 0, 0.0f, 10, false); // not late: ignored by the hold numbers
    pid_stats_add(&s, 100, 1.9f, 40, true);
    pid_stats_add(&s, 200, 2.1f, 60, true);
    pid_stats_add(&s, 300, 2.0f, 50, true);
    TEST_ASSERT_TRUE(pid_stats_has_late(&s));
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.0f, pid_stats_mean_error(&s));
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.2f, pid_stats_ripple(&s));
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 50.0f, pid_stats_power_mean(&s));
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 20.0f, pid_stats_power_p2p(&s));
}

void test_no_late_samples_gives_zeros(void)
{
    PidRunStats s = fresh();
    pid_stats_add(&s, 0, 1.0f, 10, false);
    TEST_ASSERT_FALSE(pid_stats_has_late(&s));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pid_stats_ripple(&s));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pid_stats_mean_error(&s));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pid_stats_saturated_pct(&s));
}

void test_steady_power_has_no_reversals(void)
{
    PidRunStats s = fresh();
    for (int i = 0; i < 50; i++)
    {
        pid_stats_add(&s, i * 100, 2.0f, 50.0f + (i % 2) * 2.0f, true); // 2 % wiggle: below the 5 % threshold
    }
    TEST_ASSERT_EQUAL_UINT32(0, s.reversals);
}

void test_chatter_counts_reversals(void)
{
    PidRunStats s = fresh();
    // power swings 40 <-> 60 every sample: every swing beyond 5 % is a reversal
    for (int i = 0; i < 11; i++)
    {
        pid_stats_add(&s, i * 100, 2.0f, (i % 2) ? 60.0f : 40.0f, true);
    }
    TEST_ASSERT_EQUAL_UINT32(9, s.reversals); // first sample sets the start, the first move sets the direction
}

void test_slow_drift_is_not_chatter(void)
{
    PidRunStats s = fresh();
    for (int i = 0; i < 40; i++)
    {
        pid_stats_add(&s, i * 100, 2.0f, 30.0f + i, true); // monotonic rise
    }
    TEST_ASSERT_EQUAL_UINT32(0, s.reversals);
}

void test_saturation_share(void)
{
    PidRunStats s;
    pid_stats_begin(&s, 2.0f, 0.9f, 5.0f, 60.0f);
    pid_stats_add(&s, 0, 1.0f, 60.0f, true);
    pid_stats_add(&s, 100, 1.0f, 59.7f, true); // within 0.5 of the limit counts
    pid_stats_add(&s, 200, 1.0f, 30.0f, true);
    pid_stats_add(&s, 300, 1.0f, 30.0f, true);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 50.0f, pid_stats_saturated_pct(&s));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_overshoot_and_pmax);
    RUN_TEST(test_no_overshoot_when_below_target);
    RUN_TEST(test_rise_time_is_first_crossing_of_90_percent);
    RUN_TEST(test_late_hold_statistics);
    RUN_TEST(test_no_late_samples_gives_zeros);
    RUN_TEST(test_steady_power_has_no_reversals);
    RUN_TEST(test_chatter_counts_reversals);
    RUN_TEST(test_slow_drift_is_not_chatter);
    RUN_TEST(test_saturation_share);
    return UNITY_END();
}
