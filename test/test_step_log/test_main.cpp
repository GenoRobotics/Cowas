// Native unit tests for src/Core/step_log (run: pio test -e native).
// The step log of one run: begin/end/add, the full log, and the JSON line (golden string, escaping, size limit).
#include <unity.h>
#include <string.h>

#include "step_log.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

void test_begin_end_gives_start_end(void)
{
    StepLog log;
    log.reset();
    const int i = log.begin("purge", 1000);
    TEST_ASSERT_EQUAL(0, i);
    log.end(i, 4500);
    TEST_ASSERT_EQUAL(1, log.count());
    TEST_ASSERT_EQUAL_STRING("purge", log.at(0).name);
    TEST_ASSERT_EQUAL_UINT32(1000, log.at(0).start_ms);
    TEST_ASSERT_EQUAL_UINT32(4500, log.at(0).end_ms);
}

void test_steps_keep_their_order(void)
{
    StepLog log;
    log.reset();
    log.add("spool_fill", 0, 100);
    log.add("purge", 100, 400);
    log.add("sample", 400, 1200);
    TEST_ASSERT_EQUAL(3, log.count());
    TEST_ASSERT_EQUAL_STRING("spool_fill", log.at(0).name);
    TEST_ASSERT_EQUAL_STRING("purge", log.at(1).name);
    TEST_ASSERT_EQUAL_STRING("sample", log.at(2).name);
}

void test_end_of_missing_step_is_ignored(void)
{
    StepLog log;
    log.reset();
    log.end(-1, 500);
    log.end(7, 500);
    TEST_ASSERT_EQUAL(0, log.count());
    TEST_ASSERT_EQUAL(0, log.dropped());
}

void test_full_log_counts_dropped_steps(void)
{
    StepLog log;
    log.reset();
    for (int i = 0; i < StepLog::CAPACITY; i++)
    {
        TEST_ASSERT_TRUE(log.begin("step", (uint32_t)i) >= 0);
    }
    TEST_ASSERT_EQUAL(-1, log.begin("late", 99));
    log.add("later", 1, 2);
    TEST_ASSERT_EQUAL(StepLog::CAPACITY, log.count());
    TEST_ASSERT_EQUAL(2, log.dropped());
}

void test_reset_clears_steps_and_dropped(void)
{
    StepLog log;
    log.reset();
    log.add("purge", 0, 1);
    log.reset();
    TEST_ASSERT_EQUAL(0, log.count());
    TEST_ASSERT_EQUAL(0, log.dropped());
}

void test_long_name_is_cut_not_overflowed(void)
{
    StepLog log;
    log.reset();
    log.add("a_name_that_is_much_longer_than_the_field_holds", 0, 1);
    TEST_ASSERT_EQUAL(23, (int)strlen(log.at(0).name));
}

void test_json_line_golden(void)
{
    StepLog log;
    log.reset();
    log.add("spool_fill", 1000, 61000);
    log.add("purge", 61000, 451500);
    char buf[400];
    const int n = format_steps_record(log, "E2_T2_20M_R1", 10, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL(n, (int)strlen(buf));
    TEST_ASSERT_EQUAL_STRING(
        "{\"t\":\"steps\",\"id\":\"E2_T2_20M_R1\",\"slot\":10,\"steps\":["
        "{\"name\":\"spool_fill\",\"start_ms\":1000,\"end_ms\":61000,\"dur_ms\":60000},"
        "{\"name\":\"purge\",\"start_ms\":61000,\"end_ms\":451500,\"dur_ms\":390500}"
        "],\"steps_dropped\":0}",
        buf);
}

void test_json_line_empty_log(void)
{
    StepLog log;
    log.reset();
    char buf[120];
    TEST_ASSERT_TRUE(format_steps_record(log, "auto-1", 0, buf, sizeof(buf)) > 0);
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"steps\",\"id\":\"auto-1\",\"slot\":0,\"steps\":[],\"steps_dropped\":0}", buf);
}

void test_json_line_escapes_id_and_counts_dropped(void)
{
    StepLog log;
    log.reset();
    for (int i = 0; i < StepLog::CAPACITY + 3; i++)
    {
        log.add("s", 0, 1);
    }
    char buf[6000]; // 51 steps of about 70 characters
    TEST_ASSERT_TRUE(format_steps_record(log, "a\"b\\c", 3, buf, sizeof(buf)) > 0);
    TEST_ASSERT_TRUE(strstr(buf, "\"id\":\"a\\\"b\\\\c\"") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "\"steps_dropped\":3}") != NULL);
}

void test_json_line_too_small_is_refused(void)
{
    StepLog log;
    log.reset();
    log.add("purge", 0, 1);
    char buf[20];
    TEST_ASSERT_EQUAL(-1, format_steps_record(log, "auto-1", 1, buf, sizeof(buf)));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_begin_end_gives_start_end);
    RUN_TEST(test_steps_keep_their_order);
    RUN_TEST(test_end_of_missing_step_is_ignored);
    RUN_TEST(test_full_log_counts_dropped_steps);
    RUN_TEST(test_reset_clears_steps_and_dropped);
    RUN_TEST(test_long_name_is_cut_not_overflowed);
    RUN_TEST(test_json_line_golden);
    RUN_TEST(test_json_line_empty_log);
    RUN_TEST(test_json_line_escapes_id_and_counts_dropped);
    RUN_TEST(test_json_line_too_small_is_refused);
    return UNITY_END();
}
