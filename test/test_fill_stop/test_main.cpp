// Native unit tests for src/Core/fill_stop (run: pio test -e native).
// The stop criteria of fill_test: stall windows of pumping time, the clog rule against the first window, the pressure rule.
#include <unity.h>

#include "fill_stop.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static const float DT = 100.0f;    // ms, the fill_test update
static const float STALL = 60000.0f;
static const float MIN_ML = 0.5f; // low, so the clog tests reach the clog rule (the stall minimum is 3 mL in the firmware)
static const float STUCK = 20000.0f;
static const float CLOG = 0.3f;
static const float RESUME = 3.15f;

// Feeds one full pumping window (600 steps of 100 ms) that counts `gain` mL; returns the reason of its last step.
static FillStop run_window(FillWatch &w, float gain, float &ml, float clog)
{
    FillStop r = FILL_GOING_ON;
    for (int i = 0; i < 600; i++)
    {
        ml += gain / 600.0f;
        r = fill_watch_step(w, DT, false, ml, 2.0f, RESUME, STALL, MIN_ML, STUCK, clog);
    }
    return r;
}

void test_stall_after_a_full_window_without_flow(void)
{
    FillWatch w;
    fill_watch_start(w, 10.0f);
    FillStop r = FILL_GOING_ON;
    for (int i = 0; i < 599 && r == FILL_GOING_ON; i++)
    {
        r = fill_watch_step(w, DT, false, 10.0f, 2.0f, RESUME, STALL, MIN_ML, STUCK, CLOG);
    }
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, r);
    r = fill_watch_step(w, DT, false, 10.0f, 2.0f, RESUME, STALL, MIN_ML, STUCK, CLOG);
    TEST_ASSERT_EQUAL_INT(FILL_STOP_STALLED, r);
}

void test_enough_flow_keeps_running(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    float ml = 0.0f;
    for (int i = 0; i < 2000; i++)
    {
        ml += 0.5f; // 5 mL per second: far above the minimum, and steady
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, false, ml, 2.0f, RESUME, STALL, MIN_ML, STUCK, CLOG));
    }
}

// F194: a filter that is slow from the start (24 mL in the first minute, the 2026-10-23 run) stops at once.
void test_slow_first_window_stops_at_once(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    w.slow_ml = 50.0f;
    float ml = 0.0f;
    TEST_ASSERT_EQUAL_INT(FILL_STOP_CLOGGED, run_window(w, 24.0f, ml, CLOG));
}

// F194: a healthy first window (115 mL per minute, the new-filter S1-A rate) does not stop, and the floor is off by default.
void test_healthy_window_passes_the_floor(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    w.slow_ml = 50.0f;
    float ml = 0.0f;
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 115.0f, ml, CLOG));
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 110.0f, ml, CLOG));
    FillWatch off;
    fill_watch_start(off, 0.0f);
    float ml2 = 0.0f;
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(off, 24.0f, ml2, CLOG)); // slow_ml 0 = off, the old rule decides
}

void test_pauses_do_not_restart_the_stall_window(void)
{
    // the pump pauses 2 s of every 3 s: the pumping time still adds up to a window
    FillWatch w;
    fill_watch_start(w, 0.0f);
    FillStop r = FILL_GOING_ON;
    int steps = 0;
    while (r == FILL_GOING_ON && steps < 5000)
    {
        bool paused = (steps % 30) >= 10;
        r = fill_watch_step(w, DT, paused, 0.0f, 2.0f, RESUME, STALL, MIN_ML, STUCK, CLOG);
        steps++;
    }
    TEST_ASSERT_EQUAL_INT(FILL_STOP_STALLED, r);
    // 60 s of pumping = 600 pumping steps = 60 blocks of 30 steps, the last pumped step is number 1780
    TEST_ASSERT_EQUAL_INT(1780, steps);
}

void test_pause_alone_never_stalls(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    for (int i = 0; i < 6000; i++)
    {
        // paused for 10 minutes, but the pressure falls back below the resume level every 2 s
        float p = (i % 20 == 0) ? 2.0f : 3.3f;
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, true, 0.0f, p, RESUME, STALL, MIN_ML, STUCK, CLOG));
    }
}

void test_pressure_stuck_while_paused(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    FillStop r = FILL_GOING_ON;
    int steps = 0;
    while (r == FILL_GOING_ON && steps < 1000)
    {
        r = fill_watch_step(w, DT, true, 0.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG);
        steps++;
    }
    TEST_ASSERT_EQUAL_INT(FILL_STOP_PRESSURE, r);
    TEST_ASSERT_EQUAL_INT(200, steps); // 20 s at 100 ms
}

void test_pressure_stuck_resets_when_it_falls(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    for (int i = 0; i < 150; i++)
    {
        fill_watch_step(w, DT, true, 0.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG);
    }
    fill_watch_step(w, DT, true, 0.0f, 2.90f, RESUME, STALL, MIN_ML, STUCK, CLOG); // falls back: counter resets
    for (int i = 0; i < 150; i++)
    {
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, true, 0.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG));
    }
}

void test_pumping_resets_the_pressure_counter(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    for (int i = 0; i < 150; i++)
    {
        fill_watch_step(w, DT, true, 0.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG);
    }
    fill_watch_step(w, DT, false, 1.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG);
    for (int i = 0; i < 150; i++)
    {
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, true, 1.0f, 3.30f, RESUME, STALL, MIN_ML, STUCK, CLOG));
    }
}

void test_clog_needs_two_windows_below_the_first(void)
{
    // 10, 10, 1, 1 mL per window: the single 1 mL window is a warning, the pair after it stops (mean 1 < 0.3 * 10)
    FillWatch w;
    fill_watch_start(w, 0.0f);
    float ml = 0.0f;
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 10.0f, ml, CLOG));
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 10.0f, ml, CLOG));
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 1.0f, ml, CLOG)); // mean of 10 and 1 = 5.5, above 3
    TEST_ASSERT_EQUAL_INT(FILL_STOP_CLOGGED, run_window(w, 1.0f, ml, CLOG)); // mean of 1 and 1 = 1, below 3
}

void test_one_good_window_after_a_dip_does_not_stop(void)
{
    // 10, 10, 2, 8: the mean of the last two is 5, above 3
    FillWatch w;
    fill_watch_start(w, 0.0f);
    float ml = 0.0f;
    run_window(w, 10.0f, ml, CLOG);
    run_window(w, 10.0f, ml, CLOG);
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 2.0f, ml, CLOG));
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 8.0f, ml, CLOG));
}

void test_clog_rule_can_be_turned_off(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    float ml = 0.0f;
    run_window(w, 10.0f, ml, 0.0f);
    run_window(w, 10.0f, ml, 0.0f);
    run_window(w, 1.0f, ml, 0.0f);
    TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, run_window(w, 1.0f, ml, 0.0f));
}

void test_pair_and_warning_helpers(void)
{
    TEST_ASSERT_TRUE(fill_pair_clogged(1.0f, 1.0f, 10.0f, 0.3f));
    TEST_ASSERT_FALSE(fill_pair_clogged(10.0f, 1.0f, 10.0f, 0.3f));
    TEST_ASSERT_FALSE(fill_pair_clogged(0.0f, 0.0f, 0.0f, 0.3f)); // no reference yet
    TEST_ASSERT_TRUE(fill_gain_low(1.0f, 10.0f, 0.3f));
    TEST_ASSERT_FALSE(fill_gain_low(4.0f, 10.0f, 0.3f));
}

void test_window_close_is_reported_with_its_gain(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    float ml = 0.0f;
    run_window(w, 6.0f, ml, CLOG);
    TEST_ASSERT_TRUE(w.window_closed);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.0f, w.last_gain_ml);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.0f, w.first_gain_ml);
    TEST_ASSERT_EQUAL_INT(1, w.windows);
}

void test_rules_can_be_turned_off(void)
{
    FillWatch w;
    fill_watch_start(w, 0.0f);
    for (int i = 0; i < 2000; i++)
    {
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, false, 0.0f, 2.0f, RESUME, 0.0f, MIN_ML, 0.0f, 0.0f));
        TEST_ASSERT_EQUAL_INT(FILL_GOING_ON, fill_watch_step(w, DT, true, 0.0f, 3.3f, RESUME, 0.0f, MIN_ML, 0.0f, 0.0f));
    }
}

void test_reason_names(void)
{
    TEST_ASSERT_EQUAL_STRING("stalled", fill_stop_name(FILL_STOP_STALLED));
    TEST_ASSERT_EQUAL_STRING("pressure", fill_stop_name(FILL_STOP_PRESSURE));
    TEST_ASSERT_EQUAL_STRING("clogged", fill_stop_name(FILL_STOP_CLOGGED));
    TEST_ASSERT_EQUAL_STRING("running", fill_stop_name(FILL_GOING_ON));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_stall_after_a_full_window_without_flow);
    RUN_TEST(test_enough_flow_keeps_running);
    RUN_TEST(test_slow_first_window_stops_at_once);
    RUN_TEST(test_healthy_window_passes_the_floor);
    RUN_TEST(test_pauses_do_not_restart_the_stall_window);
    RUN_TEST(test_pause_alone_never_stalls);
    RUN_TEST(test_pressure_stuck_while_paused);
    RUN_TEST(test_pressure_stuck_resets_when_it_falls);
    RUN_TEST(test_pumping_resets_the_pressure_counter);
    RUN_TEST(test_clog_needs_two_windows_below_the_first);
    RUN_TEST(test_one_good_window_after_a_dip_does_not_stop);
    RUN_TEST(test_clog_rule_can_be_turned_off);
    RUN_TEST(test_pair_and_warning_helpers);
    RUN_TEST(test_window_close_is_reported_with_its_gain);
    RUN_TEST(test_rules_can_be_turned_off);
    RUN_TEST(test_reason_names);
    return UNITY_END();
}
