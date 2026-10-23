// Native unit tests for src/Core/lock_rules (run: pio test -e native).
// Truth tables for the O-ring lock rules (FW2, F18): every state, every home input.
#include <unity.h>

#include <math.h>

#include "lock_rules.h"
#include "Settings.h"

using namespace core;

static const LockState ALL_STATES[] = {LOCK_STATE_UNKNOWN, LOCK_STATE_UNLOCKED, LOCK_STATE_LOCKED};

void setUp(void) {}
void tearDown(void) {}

void test_state_names(void)
{
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", lock_state_name(LOCK_STATE_UNKNOWN));
    TEST_ASSERT_EQUAL_STRING("UNLOCKED", lock_state_name(LOCK_STATE_UNLOCKED));
    TEST_ASSERT_EQUAL_STRING("LOCKED", lock_state_name(LOCK_STATE_LOCKED));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", lock_state_name((LockState)42)); // corrupted value
}

void test_boot_state_is_unknown(void)
{
    // The zero value (what a static enum starts as) must be UNKNOWN.
    LockState zero = (LockState)0;
    TEST_ASSERT_EQUAL_INT(LOCK_STATE_UNKNOWN, zero);
    TEST_ASSERT_FALSE(may_rotate(zero));
}

void test_may_rotate_truth_table(void)
{
    const bool expected[] = {false, true, false};
    for (int i = 0; i < 3; i++)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expected[i], may_rotate(ALL_STATES[i]), lock_state_name(ALL_STATES[i]));
    }
    TEST_ASSERT_FALSE(may_rotate((LockState)42));
}

void test_may_lock_and_unlock_truth_table(void)
{
    const bool expected[] = {false, true, true};
    for (int i = 0; i < 3; i++)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expected[i], may_lock(ALL_STATES[i]), lock_state_name(ALL_STATES[i]));
        TEST_ASSERT_EQUAL_MESSAGE(expected[i], may_unlock(ALL_STATES[i]), lock_state_name(ALL_STATES[i]));
    }
    TEST_ASSERT_FALSE(may_lock((LockState)42));
    TEST_ASSERT_FALSE(may_unlock((LockState)42));
}

void test_may_home_encoder_failure_always_refuses(void)
{
    const float errors[] = {0.0f, 1.0f, -1.0f, 100.0f, NAN};
    for (unsigned i = 0; i < sizeof errors / sizeof errors[0]; i++)
    {
        TEST_ASSERT_EQUAL_INT(HOME_REFUSED_ENCODER, may_home(false, errors[i], 3.0f));
    }
}

void test_may_home_boundaries(void)
{
    const float tol = 3.0f;
    TEST_ASSERT_EQUAL_INT(HOME_ALLOWED, may_home(true, 0.0f, tol));
    TEST_ASSERT_EQUAL_INT(HOME_ALLOWED, may_home(true, 2.99f, tol));    // just below
    TEST_ASSERT_EQUAL_INT(HOME_ALLOWED, may_home(true, 3.0f, tol));     // at
    TEST_ASSERT_EQUAL_INT(HOME_ALLOWED, may_home(true, -3.0f, tol));    // at, other side
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, 3.01f, tol));  // just above
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, -3.01f, tol));
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, 11.25f, tol)); // between two slots
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, 180.0f, tol));
}

void test_may_home_nan_and_bad_tolerance_refuse(void)
{
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, NAN, 3.0f));
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, INFINITY, 3.0f));
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, 0.0f, NAN));
    TEST_ASSERT_EQUAL_INT(HOME_REFUSED_NOT_ALIGNED, may_home(true, 0.0f, -1.0f));
    TEST_ASSERT_EQUAL_INT(HOME_ALLOWED, may_home(true, 0.0f, 0.0f)); // zero tolerance, exact
}

void test_home_tolerance_setting_is_sane(void)
{
    // PLACEHOLDER value (3 deg, user 2026-09-25): must stay below half the 22.5 deg pitch,
    // or "aligned to a slot" would accept a position between two holes.
    TEST_ASSERT_TRUE(ACTUATOR_HOME_SLOT_TOLERANCE_DEG > 0);
    TEST_ASSERT_TRUE(ACTUATOR_HOME_SLOT_TOLERANCE_DEG < 22.5f / 2);
}

void test_lock_move(void)
{
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_NONE, lock_move_to(LOCK_STATE_LOCKED, LOCK_STATE_LOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_NONE, lock_move_to(LOCK_STATE_UNLOCKED, LOCK_STATE_UNLOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_TO_LOCKED, lock_move_to(LOCK_STATE_UNLOCKED, LOCK_STATE_LOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_TO_UNLOCKED, lock_move_to(LOCK_STATE_LOCKED, LOCK_STATE_UNLOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_REFUSE, lock_move_to(LOCK_STATE_UNKNOWN, LOCK_STATE_LOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_REFUSE, lock_move_to(LOCK_STATE_UNKNOWN, LOCK_STATE_UNLOCKED));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_REFUSE, lock_move_to(LOCK_STATE_LOCKED, LOCK_STATE_UNKNOWN));
    TEST_ASSERT_EQUAL_INT(LOCK_MOVE_REFUSE, lock_move_to((LockState)42, LOCK_STATE_LOCKED));
}

void test_home_decision_names(void)
{
    TEST_ASSERT_EQUAL_STRING("allowed", home_decision_name(HOME_ALLOWED));
    TEST_ASSERT_EQUAL_STRING("encoder", home_decision_name(HOME_REFUSED_ENCODER));
    TEST_ASSERT_EQUAL_STRING("not_aligned", home_decision_name(HOME_REFUSED_NOT_ALIGNED));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_state_names);
    RUN_TEST(test_boot_state_is_unknown);
    RUN_TEST(test_may_rotate_truth_table);
    RUN_TEST(test_may_lock_and_unlock_truth_table);
    RUN_TEST(test_may_home_encoder_failure_always_refuses);
    RUN_TEST(test_may_home_boundaries);
    RUN_TEST(test_may_home_nan_and_bad_tolerance_refuse);
    RUN_TEST(test_home_tolerance_setting_is_sane);
    RUN_TEST(test_home_decision_names);
    RUN_TEST(test_lock_move);
    return UNITY_END();
}
