// Native unit tests for src/Core/run_keys (run: pio test -e native).
// The keys typed during a pump run, and the purge volume with and without the spool.
#include <unity.h>

#include "run_keys.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_skip_keys(void)
{
    TEST_ASSERT_EQUAL(RUN_KEY_SKIP, skip_or_abort("s"));
    TEST_ASSERT_EQUAL(RUN_KEY_SKIP, skip_or_abort("S"));
    TEST_ASSERT_EQUAL(RUN_KEY_SKIP, skip_or_abort("skip"));
    TEST_ASSERT_EQUAL(RUN_KEY_SKIP, skip_or_abort(" s\r"));
}

void test_abort_keys(void)
{
    TEST_ASSERT_EQUAL(RUN_KEY_ABORT, skip_or_abort("x"));
    TEST_ASSERT_EQUAL(RUN_KEY_ABORT, skip_or_abort("X"));
    TEST_ASSERT_EQUAL(RUN_KEY_ABORT, skip_or_abort("abort"));
    TEST_ASSERT_EQUAL(RUN_KEY_ABORT, skip_or_abort("stop"));   // STOP is always accepted
    TEST_ASSERT_EQUAL(RUN_KEY_ABORT, skip_or_abort("STOP\r\n"));
}

void test_other_lines_are_ignored(void)
{
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort(""));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("   "));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort(nullptr));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("status"));  // a typed command is not a skip
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("st"));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("sx"));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("full_cycle 1 volume=1000"));
    TEST_ASSERT_EQUAL(RUN_KEY_NONE, skip_or_abort("skipped_step_now_long_word"));
}

// the firmware values: PURGE_NO_SPOOL_ML 500, PURGE_SPOOL_ML 7000 (user 2026-10-23)
void test_purge_volume_without_spool(void)
{
    TEST_ASSERT_EQUAL_FLOAT(500.0f, purge_volume_ml(false, 500.0f, 7000.0f));
}

void test_purge_volume_with_spool(void)
{
    TEST_ASSERT_EQUAL_FLOAT(7000.0f, purge_volume_ml(true, 500.0f, 7000.0f));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_skip_keys);
    RUN_TEST(test_abort_keys);
    RUN_TEST(test_other_lines_are_ignored);
    RUN_TEST(test_purge_volume_without_spool);
    RUN_TEST(test_purge_volume_with_spool);
    return UNITY_END();
}
