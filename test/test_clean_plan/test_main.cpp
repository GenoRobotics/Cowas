// Native unit tests for src/Core/clean_plan (run: pio test -e native).
// Which slots the ethanol cleaning visits: the operator's list of unused slots, and the fill-order walk without them.
#include <unity.h>

#include <stdint.h>

#include "clean_plan.h"
#include "Settings.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static bool u[NB_SLOT];

static int parse(const char *text, int *bad = nullptr)
{
    return parse_unused_slots(text, NB_SLOT, PURGE_SLOT, u, bad);
}

void test_empty_and_none_mean_no_slot(void)
{
    const char *none[] = {"", "   ", "none", "NONE", "no", "n", "skip=", "skip=none"};
    for (unsigned i = 0; i < sizeof none / sizeof none[0]; i++)
    {
        TEST_ASSERT_EQUAL_INT(0, parse(none[i]));
        for (int s = 0; s < NB_SLOT; s++)
            TEST_ASSERT_FALSE(u[s]);
    }
    TEST_ASSERT_EQUAL_INT(0, parse_unused_slots(nullptr, NB_SLOT, PURGE_SLOT, u, nullptr));
}

void test_commas_spaces_and_semicolons(void)
{
    TEST_ASSERT_EQUAL_INT(3, parse("3,7,12"));
    TEST_ASSERT_TRUE(u[3] && u[7] && u[12]);
    TEST_ASSERT_FALSE(u[1]);
    TEST_ASSERT_EQUAL_INT(3, parse("3 7 12"));
    TEST_ASSERT_EQUAL_INT(3, parse(" 3 , 7;12 \r\n"));
    TEST_ASSERT_TRUE(u[3] && u[7] && u[12]);
}

void test_prefix_and_ranges(void)
{
    TEST_ASSERT_EQUAL_INT(4, parse("skip=3-5,9"));
    TEST_ASSERT_TRUE(u[3] && u[4] && u[5] && u[9]);
    TEST_ASSERT_EQUAL_INT(2, parse("UNUSED=1,14"));
    TEST_ASSERT_TRUE(u[1] && u[14]);
}

void test_duplicates_count_once_and_state_is_cleared(void)
{
    TEST_ASSERT_EQUAL_INT(3, parse("5,5,3-5")); // 3, 4, 5
    TEST_ASSERT_EQUAL_INT(0, parse("none")); // a new parse starts from nothing
    for (int s = 0; s < NB_SLOT; s++)
        TEST_ASSERT_FALSE(u[s]);
}

void test_purge_and_beyond_last_are_refused(void)
{
    int bad = -1;
    TEST_ASSERT_EQUAL_INT(UNUSED_OUT_OF_RANGE, parse("3,0", &bad));
    TEST_ASSERT_EQUAL_INT(PURGE_SLOT, bad);
    TEST_ASSERT_EQUAL_INT(UNUSED_OUT_OF_RANGE, parse("2,15", &bad));
    TEST_ASSERT_EQUAL_INT(NB_SLOT, bad);
    TEST_ASSERT_EQUAL_INT(UNUSED_OUT_OF_RANGE, parse("12-16", &bad));
    TEST_ASSERT_EQUAL_INT(NB_SLOT, bad);
    TEST_ASSERT_EQUAL_INT(UNUSED_OUT_OF_RANGE, parse("99999999999", &bad));
}

void test_bad_text_and_bad_ranges(void)
{
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_TEXT, parse("3,x"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_TEXT, parse("3a"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_TEXT, parse("all"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_TEXT, parse("3.5"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_RANGE, parse("5-"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_RANGE, parse("-5"));
    TEST_ASSERT_EQUAL_INT(UNUSED_BAD_RANGE, parse("9-3"));
}

void test_nothing_unused_cleans_every_slot_in_fill_order(void)
{
    parse("none");
    int out[NB_SLOT];
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    const int n = build_clean_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, u, out, NB_SLOT);
    TEST_ASSERT_EQUAL_INT(len, n);
    for (int i = 0; i < n; i++)
        TEST_ASSERT_EQUAL_INT(FILL_ORDER[i], out[i]);
}

void test_unused_slots_are_left_out_and_order_is_kept(void)
{
    parse("1,14,8");
    int out[NB_SLOT];
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    const int n = build_clean_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, u, out, NB_SLOT);
    TEST_ASSERT_EQUAL_INT(len - 3, n);
    int k = 0;
    for (int i = 0; i < len; i++)
    {
        if (FILL_ORDER[i] == 1 || FILL_ORDER[i] == 14 || FILL_ORDER[i] == 8)
            continue;
        TEST_ASSERT_EQUAL_INT(FILL_ORDER[i], out[k++]);
    }
}

void test_all_unused_gives_an_empty_list_and_small_buffer_is_reported(void)
{
    parse("1-14");
    int out[NB_SLOT];
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    TEST_ASSERT_EQUAL_INT(0, build_clean_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, u, out, NB_SLOT));
    parse("none");
    TEST_ASSERT_EQUAL_INT(-1, build_clean_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, u, out, 3));
}

void test_purge_slot_in_the_order_is_ignored(void)
{
    parse("none");
    const uint8_t order[3] = {2, 0, 1};
    int out[4];
    TEST_ASSERT_EQUAL_INT(2, build_clean_list(order, 3, NB_SLOT, PURGE_SLOT, u, out, 4));
    TEST_ASSERT_EQUAL_INT(2, out[0]);
    TEST_ASSERT_EQUAL_INT(1, out[1]);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_empty_and_none_mean_no_slot);
    RUN_TEST(test_commas_spaces_and_semicolons);
    RUN_TEST(test_prefix_and_ranges);
    RUN_TEST(test_duplicates_count_once_and_state_is_cleared);
    RUN_TEST(test_purge_and_beyond_last_are_refused);
    RUN_TEST(test_bad_text_and_bad_ranges);
    RUN_TEST(test_nothing_unused_cleans_every_slot_in_fill_order);
    RUN_TEST(test_unused_slots_are_left_out_and_order_is_kept);
    RUN_TEST(test_all_unused_gives_an_empty_list_and_small_buffer_is_reported);
    RUN_TEST(test_purge_slot_in_the_order_is_ignored);
    return UNITY_END();
}
