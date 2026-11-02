// Native unit tests for src/Core/spool_fill (run: pio test -e native).
// The session's deepest depth of the water fill (2026-11-02).
#include <unity.h>

#include "spool_fill.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_empty_session_depth_means_the_default(void)
{
    float m = -1;
    TEST_ASSERT_EQUAL_INT(0, spool_session_depth_parse("", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(0, spool_session_depth_parse("   ", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(0, spool_session_depth_parse(nullptr, 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, m); // untouched
}

void test_session_depth_reads_numbers_with_a_point_or_a_comma(void)
{
    float m = 0;
    TEST_ASSERT_EQUAL_INT(1, spool_session_depth_parse("40", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_FLOAT(40.0f, m);
    TEST_ASSERT_EQUAL_INT(1, spool_session_depth_parse("12,5", 0.5f, 40.0f, &m));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.5f, m);
    TEST_ASSERT_EQUAL_INT(1, spool_session_depth_parse(" 2.5 ", 0.5f, 40.0f, &m));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, m);
}

void test_session_depth_refuses_out_of_range_and_words(void)
{
    float m = 0;
    TEST_ASSERT_EQUAL_INT(-1, spool_session_depth_parse("40.5", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(-1, spool_session_depth_parse("0.2", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(-1, spool_session_depth_parse("deep", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(-1, spool_session_depth_parse("20m", 0.5f, 40.0f, &m));
    TEST_ASSERT_EQUAL_INT(-1, spool_session_depth_parse("-3", 0.5f, 40.0f, &m));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_empty_session_depth_means_the_default);
    RUN_TEST(test_session_depth_reads_numbers_with_a_point_or_a_comma);
    RUN_TEST(test_session_depth_refuses_out_of_range_and_words);
    return UNITY_END();
}
