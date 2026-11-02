// Native unit tests for src/Core/spool_motion (run: pio test -e native).
// Depth to length, the move watchdog, the approach speed and the diameter calibration fit.
#include <math.h>
#include <unity.h>

#include "spool_motion.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static SpoolMoveParams down_to(float goal)
{
    SpoolMoveParams p = {+1, false, goal, 600000u, 4000u, 5.0f};
    return p;
}

static SpoolMoveParams up_to(float goal)
{
    SpoolMoveParams p = {-1, false, goal, 600000u, 4000u, 5.0f};
    return p;
}

void test_depth_to_length(void)
{
    float len = 0;
    TEST_ASSERT_TRUE(spool_depth_to_length_cm(2.5f, 0.0f, 1500.0f, &len));
    TEST_ASSERT_EQUAL_FLOAT(250.0f, len);
    TEST_ASSERT_TRUE(spool_depth_to_length_cm(2.5f, 40.0f, 1500.0f, &len)); // 40 cm between the origin and the water
    TEST_ASSERT_EQUAL_FLOAT(290.0f, len);
    TEST_ASSERT_TRUE(spool_depth_to_length_cm(0.0f, 0.0f, 1500.0f, &len));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, len);
}

void test_depth_to_length_refuses(void)
{
    float len = 123.0f;
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(15.01f, 0.0f, 1500.0f, &len)); // above the calibrated range
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(14.8f, 40.0f, 1500.0f, &len)); // the height counts
    TEST_ASSERT_TRUE(spool_depth_to_length_cm(15.0f, 0.0f, 1500.0f, &len));
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(-0.1f, 0.0f, 1500.0f, &len));
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(NAN, 0.0f, 1500.0f, &len));
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(1.0f, NAN, 1500.0f, &len));
    TEST_ASSERT_FALSE(spool_depth_to_length_cm(1.0f, 0.0f, 0.0f, &len));
    TEST_ASSERT_EQUAL_FLOAT(15.0f * 100.0f, len); // untouched by the refusals (last success)
}

void test_move_arrives(void)
{
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(down_to(250), 0, 100, 1000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_ARRIVED, spool_move_state(down_to(250), 0, 250, 9000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_ARRIVED, spool_move_state(down_to(250), 0, 262, 9000, 0)); // overshoot
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(up_to(100), 250, 180, 1000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_ARRIVED, spool_move_state(up_to(100), 250, 100, 9000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_ARRIVED, spool_move_state(up_to(100), 250, 96, 9000, 0));
}

void test_move_wrong_way(void)
{
    // unrolling, but the length shrinks by more than 5 cm: encoder wired the other way round
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(down_to(250), 100, 96, 500, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_WRONG_WAY, spool_move_state(down_to(250), 100, 94, 500, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_WRONG_WAY, spool_move_state(up_to(10), 100, 106, 500, 0));
}

void test_move_stalled_and_timeout(void)
{
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(down_to(250), 0, 10, 5000, 3999));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_STALLED, spool_move_state(down_to(250), 0, 10, 5000, 4000));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_TIMEOUT, spool_move_state(down_to(250), 0, 10, 600000u, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(down_to(250), 0, 10, 599999u, 0));
}

void test_move_to_switch_ignores_goal_and_direction(void)
{
    SpoolMoveParams p = {-1, true, 0.0f, 600000u, 4000u, 5.0f};
    // the length may be anything when the position is unknown, even growing (reset encoder, wrong start)
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(p, 0, -50, 1000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(p, 0, 80, 1000, 0));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_GO, spool_move_state(p, 100, 0, 1000, 0)); // at 0 and still going: only the switch ends it
    TEST_ASSERT_EQUAL(SPOOL_MOVE_STALLED, spool_move_state(p, 0, 0, 5000, 4000));
    TEST_ASSERT_EQUAL(SPOOL_MOVE_TIMEOUT, spool_move_state(p, 0, 0, 600000u, 0));
}

void test_approach_speed(void)
{
    TEST_ASSERT_EQUAL_UINT8(100, spool_approach_speed(200.0f, 30.0f, 100, 40));
    TEST_ASSERT_EQUAL_UINT8(100, spool_approach_speed(30.0f, 30.0f, 100, 40));
    TEST_ASSERT_EQUAL_UINT8(40, spool_approach_speed(29.9f, 30.0f, 100, 40));
}

void test_fit_exact(void)
{
    // the encoder says 200, 500, 1000, 1500 cm and the tape says 10 % more everywhere
    const SpoolStop stops[4] = {{200, 220}, {500, 550}, {1000, 1100}, {1500, 1650}};
    const SpoolFit fit = spool_fit_scale(stops, 4);
    TEST_ASSERT_TRUE(fit.valid);
    TEST_ASSERT_EQUAL_INT(4, fit.n);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.1f, fit.scale);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.0f, fit.worst_dev_pct);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 11.0f, spool_new_diameter_cm(10.0f, fit.scale));
}

void test_fit_shows_non_linearity(void)
{
    // the layers on the drum make the tube longer per turn at the end: 5 % more at 15 m than at 2 m
    const SpoolStop stops[4] = {{200, 200}, {500, 505}, {1000, 1020}, {1500, 1575}};
    const SpoolFit fit = spool_fit_scale(stops, 4);
    TEST_ASSERT_TRUE(fit.valid);
    TEST_ASSERT_TRUE(fit.scale > 1.0f && fit.scale < 1.05f);
    TEST_ASSERT_TRUE(fit.worst_dev_pct > 2.0f); // the operator sees that one diameter does not fit everywhere
}

void test_fit_refuses_bad_input(void)
{
    const SpoolStop none[1] = {{0, 0}};
    TEST_ASSERT_FALSE(spool_fit_scale(none, 0).valid);
    TEST_ASSERT_FALSE(spool_fit_scale(none, 1).valid);
    const SpoolStop zero_measure[2] = {{200, 200}, {500, 0}};
    TEST_ASSERT_FALSE(spool_fit_scale(zero_measure, 2).valid);
    const SpoolStop typo[2] = {{200, 200}, {500, 5000}}; // 50 m typed for 5 m
    TEST_ASSERT_FALSE(spool_fit_scale(typo, 2).valid);
    const SpoolStop nan_stop[1] = {{NAN, 200}};
    TEST_ASSERT_FALSE(spool_fit_scale(nan_stop, 1).valid);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_depth_to_length);
    RUN_TEST(test_depth_to_length_refuses);
    RUN_TEST(test_move_arrives);
    RUN_TEST(test_move_wrong_way);
    RUN_TEST(test_move_stalled_and_timeout);
    RUN_TEST(test_move_to_switch_ignores_goal_and_direction);
    RUN_TEST(test_approach_speed);
    RUN_TEST(test_fit_exact);
    RUN_TEST(test_fit_shows_non_linearity);
    RUN_TEST(test_fit_refuses_bad_input);
    return UNITY_END();
}
