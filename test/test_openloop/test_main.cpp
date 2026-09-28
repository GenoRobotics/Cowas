// Native unit tests for src/Core/openloop (run: pio test -e native).
// The stop decision of the sensorless pump test mode (FW2, F17): every stop condition,
// its boundaries, the priority between them, NaN, and the power / time clamps.
#include <unity.h>

#include <math.h>

#include "openloop.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

// A running pump, nothing reached yet.
static OpenLoopInputs running(void)
{
    OpenLoopInputs in;
    in.elapsed_ms = 1000;
    in.volume_ml = 10.0f;
    in.target_ml = 50.0f;
    in.max_ms = 60000;
    in.ms_since_pulse = 100;
    in.no_flow_ms = 5000;
    in.stop_on_no_flow = true;
    in.keypress = false;
    return in;
}

void test_keeps_running(void)
{
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(running()));
}

void test_volume_boundaries(void)
{
    OpenLoopInputs in = running();
    in.volume_ml = 49.99f;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.volume_ml = 50.0f;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_VOLUME, openloop_stop_decision(in));
    in.volume_ml = 50.01f;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_VOLUME, openloop_stop_decision(in));
}

void test_no_volume_stop_without_target(void)
{
    OpenLoopInputs in = running();
    in.volume_ml = 1e6f;
    in.target_ml = 0.0f;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.target_ml = -5.0f;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.target_ml = NAN;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
}

void test_nan_volume_never_stops_on_volume_but_time_still_does(void)
{
    OpenLoopInputs in = running();
    in.volume_ml = NAN;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.elapsed_ms = in.max_ms;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_MAX_TIME, openloop_stop_decision(in));
}

void test_time_boundaries(void)
{
    OpenLoopInputs in = running();
    in.elapsed_ms = in.max_ms - 1;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.elapsed_ms = in.max_ms;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_MAX_TIME, openloop_stop_decision(in));
    in.elapsed_ms = in.max_ms + 1;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_MAX_TIME, openloop_stop_decision(in));
    in = running();
    in.max_ms = 0;
    in.elapsed_ms = 0;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_MAX_TIME, openloop_stop_decision(in)); // 0 = stop at once
}

void test_no_flow_boundaries(void)
{
    OpenLoopInputs in = running();
    in.ms_since_pulse = in.no_flow_ms - 1;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.ms_since_pulse = in.no_flow_ms;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_NO_FLOW, openloop_stop_decision(in));
    in.ms_since_pulse = in.no_flow_ms + 1;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_NO_FLOW, openloop_stop_decision(in));
}

void test_air_push_ignores_no_flow(void)
{
    OpenLoopInputs in = running();
    in.stop_on_no_flow = false;
    in.target_ml = 0.0f;
    in.ms_since_pulse = 1000000;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_RUN, openloop_stop_decision(in));
    in.elapsed_ms = in.max_ms;
    TEST_ASSERT_EQUAL_INT(OPENLOOP_MAX_TIME, openloop_stop_decision(in));
}

void test_priority_every_combination(void)
{
    // 4 conditions -> 16 combinations; the first true one in the order
    // keypress, volume, max time, no flow must win.
    for (int mask = 0; mask < 16; mask++)
    {
        OpenLoopInputs in = running();
        bool key = mask & 1, vol = mask & 2, time = mask & 4, flow = mask & 8;
        in.keypress = key;
        if (vol)
            in.volume_ml = in.target_ml;
        if (time)
            in.elapsed_ms = in.max_ms;
        if (flow)
            in.ms_since_pulse = in.no_flow_ms;
        OpenLoopStop expected = key ? OPENLOOP_KEYPRESS
                                : vol ? OPENLOOP_VOLUME
                                : time ? OPENLOOP_MAX_TIME
                                : flow ? OPENLOOP_NO_FLOW
                                       : OPENLOOP_RUN;
        TEST_ASSERT_EQUAL_INT_MESSAGE(expected, openloop_stop_decision(in), "priority");
    }
}

void test_clamp_power(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, openloop_clamp_power(-10, 30));
    TEST_ASSERT_EQUAL_UINT8(0, openloop_clamp_power(0, 30));
    TEST_ASSERT_EQUAL_UINT8(29, openloop_clamp_power(29, 30));
    TEST_ASSERT_EQUAL_UINT8(30, openloop_clamp_power(30, 30));
    TEST_ASSERT_EQUAL_UINT8(30, openloop_clamp_power(31, 30));
    TEST_ASSERT_EQUAL_UINT8(30, openloop_clamp_power(100000, 30));
    TEST_ASSERT_EQUAL_UINT8(100, openloop_clamp_power(250, 200)); // max itself capped at 100
}

void test_clamp_max_ms(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, openloop_clamp_max_ms(0, 120000));
    TEST_ASSERT_EQUAL_UINT32(119999, openloop_clamp_max_ms(119999, 120000));
    TEST_ASSERT_EQUAL_UINT32(120000, openloop_clamp_max_ms(120000, 120000));
    TEST_ASSERT_EQUAL_UINT32(120000, openloop_clamp_max_ms(120001, 120000));
    TEST_ASSERT_EQUAL_UINT32(120000, openloop_clamp_max_ms(0xFFFFFFFFu, 120000));
}

void test_settings_stay_low_power_and_time_limited(void)
{
    // ROADMAP §6 rule 4: the sensorless mode is low power and time-limited.
    TEST_ASSERT_TRUE(TEST_OPENLOOP_MAX_POWER > 0);
    TEST_ASSERT_TRUE(TEST_OPENLOOP_MAX_POWER <= 50);
    TEST_ASSERT_TRUE(TEST_OPENLOOP_MAX_MS_LIMIT > 0);
    TEST_ASSERT_TRUE(TEST_OPENLOOP_MAX_MS_LIMIT <= 5UL * 60 * 1000);
    TEST_ASSERT_EQUAL_UINT32(5000, TEST_OPENLOOP_NO_FLOW_MS);
    TEST_ASSERT_TRUE(TEST_OPENLOOP_AIR_PUSH_MS <= TEST_OPENLOOP_MAX_MS_LIMIT);
}

void test_stop_names(void)
{
    TEST_ASSERT_EQUAL_STRING("run", openloop_stop_name(OPENLOOP_RUN));
    TEST_ASSERT_EQUAL_STRING("keypress", openloop_stop_name(OPENLOOP_KEYPRESS));
    TEST_ASSERT_EQUAL_STRING("volume", openloop_stop_name(OPENLOOP_VOLUME));
    TEST_ASSERT_EQUAL_STRING("max_time", openloop_stop_name(OPENLOOP_MAX_TIME));
    TEST_ASSERT_EQUAL_STRING("no_flow", openloop_stop_name(OPENLOOP_NO_FLOW));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_keeps_running);
    RUN_TEST(test_volume_boundaries);
    RUN_TEST(test_no_volume_stop_without_target);
    RUN_TEST(test_nan_volume_never_stops_on_volume_but_time_still_does);
    RUN_TEST(test_time_boundaries);
    RUN_TEST(test_no_flow_boundaries);
    RUN_TEST(test_air_push_ignores_no_flow);
    RUN_TEST(test_priority_every_combination);
    RUN_TEST(test_clamp_power);
    RUN_TEST(test_clamp_max_ms);
    RUN_TEST(test_settings_stay_low_power_and_time_limited);
    RUN_TEST(test_stop_names);
    return UNITY_END();
}
