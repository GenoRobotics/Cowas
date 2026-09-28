// Native unit tests for src/Core/slot_iteration (run: pio test -e native).
// The slot list of the FW2 multi-slot tests: from/count/order, with the firmware's
// FILL_ORDER (15 slots) and with synthetic 24-slot layouts. No hard-coded slot count.
#include <unity.h>

#include <stdint.h>

#include "slot_iteration.h"
#include "Settings.h"

using namespace core;

static const int CAP = 64;

void setUp(void) {}
void tearDown(void) {}

// A 24-slot fill order (purge = 0): a different permutation from index order.
static void make_order_24(uint8_t *order)
{
    // 23 sample slots: odd ones ascending, then even ones descending
    int k = 0;
    for (int s = 1; s < 24; s += 2)
        order[k++] = (uint8_t)s;
    for (int s = 22; s >= 2; s -= 2)
        order[k++] = (uint8_t)s;
}

void test_firmware_fill_order_is_valid(void)
{
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    TEST_ASSERT_TRUE(fill_order_is_valid(FILL_ORDER, len, NB_SLOT, PURGE_SLOT));
}

void test_fill_order_validation_rejects(void)
{
    uint8_t ok[4] = {3, 1, 4, 2}; // 5 slots, purge 0
    TEST_ASSERT_TRUE(fill_order_is_valid(ok, 4, 5, 0));

    uint8_t dup[4] = {3, 1, 3, 2};
    TEST_ASSERT_FALSE(fill_order_is_valid(dup, 4, 5, 0));
    uint8_t has_purge[4] = {0, 1, 3, 2};
    TEST_ASSERT_FALSE(fill_order_is_valid(has_purge, 4, 5, 0));
    uint8_t out_of_range[4] = {5, 1, 3, 2};
    TEST_ASSERT_FALSE(fill_order_is_valid(out_of_range, 4, 5, 0));
    TEST_ASSERT_FALSE(fill_order_is_valid(ok, 3, 5, 0)); // too short
    TEST_ASSERT_FALSE(fill_order_is_valid(ok, 4, 6, 0)); // missing a slot
    TEST_ASSERT_FALSE(fill_order_is_valid(ok, 4, 5, 7)); // purge out of range
    TEST_ASSERT_FALSE(fill_order_is_valid(ok, 4, 0, 0)); // no slots

    uint8_t purge_last[4] = {0, 1, 2, 3}; // purge = 4
    TEST_ASSERT_TRUE(fill_order_is_valid(purge_last, 4, 5, 4));
}

void test_default_selection_is_the_whole_fill_order(void)
{
    int out[CAP];
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    int n = build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, default_selection(), out, CAP);
    TEST_ASSERT_EQUAL_INT(NB_SLOT - 1, n);
    for (int i = 0; i < n; i++)
    {
        TEST_ASSERT_EQUAL_INT(FILL_ORDER[i], out[i]);
    }
}

// Checks every (order, include_purge, from, count) combination against a reference
// built by hand from the base sequence.
static void check_all_combinations(const uint8_t *order, int nb_slots, int purge_slot)
{
    int base_fill[CAP], base_index[CAP];
    int m = 0;
    for (int i = 0; i < nb_slots - 1; i++)
        base_fill[m++] = order[i];
    m = 0;
    for (int s = 0; s < nb_slots; s++)
        if (s != purge_slot)
            base_index[m++] = s;

    for (int idx = 0; idx < 2; idx++)
    {
        for (int purge = 0; purge < 2; purge++)
        {
            int seq[CAP];
            int total = 0;
            if (purge)
                seq[total++] = purge_slot;
            for (int i = 0; i < nb_slots - 1; i++)
                seq[total++] = idx ? base_index[i] : base_fill[i];

            for (int from = -1; from <= nb_slots; from++)
            {
                for (int count = -1; count <= total + 1; count++)
                {
                    SlotSelection sel = default_selection();
                    sel.index_order = idx != 0;
                    sel.include_purge = purge != 0;
                    sel.from_slot = from;
                    sel.count = count;
                    int out[CAP];
                    int n = build_slot_list(order, nb_slots - 1, nb_slots, purge_slot, sel, out, CAP);

                    if (count == 0)
                    {
                        TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_COUNT, n);
                        continue;
                    }
                    int start = 0;
                    if (from != -1)
                    {
                        start = -1;
                        for (int i = 0; i < total; i++)
                            if (seq[i] == from)
                                start = i;
                        if (start < 0)
                        {
                            TEST_ASSERT_EQUAL_INT(SLOTLIST_FROM_NOT_FOUND, n);
                            continue;
                        }
                    }
                    int expected_n = total - start;
                    if (count != -1 && count < expected_n)
                        expected_n = count;
                    TEST_ASSERT_EQUAL_INT(expected_n, n);
                    for (int k = 0; k < n; k++)
                        TEST_ASSERT_EQUAL_INT(seq[start + k], out[k]);
                }
            }
        }
    }
}

void test_all_combinations_15_slots_firmware(void)
{
    check_all_combinations(FILL_ORDER, NB_SLOT, PURGE_SLOT);
}

void test_all_combinations_24_slots(void)
{
    uint8_t order[23];
    make_order_24(order);
    TEST_ASSERT_TRUE(fill_order_is_valid(order, 23, 24, 0));
    check_all_combinations(order, 24, 0);
}

void test_all_combinations_24_slots_purge_in_the_middle(void)
{
    uint8_t order[23];
    int k = 0;
    for (int s = 23; s >= 0; s--)
        if (s != 12)
            order[k++] = (uint8_t)s;
    check_all_combinations(order, 24, 12);
}

void test_index_order_skips_purge(void)
{
    uint8_t order[4] = {4, 3, 1, 0}; // 5 slots, purge = 2
    SlotSelection sel = default_selection();
    sel.index_order = true;
    int out[CAP];
    int n = build_slot_list(order, 4, 5, 2, sel, out, CAP);
    TEST_ASSERT_EQUAL_INT(4, n);
    const int expected[] = {0, 1, 3, 4};
    TEST_ASSERT_EQUAL_INT_ARRAY(expected, out, 4);
}

void test_from_purge_needs_include_purge(void)
{
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    SlotSelection sel = default_selection();
    sel.from_slot = PURGE_SLOT;
    int out[CAP];
    TEST_ASSERT_EQUAL_INT(SLOTLIST_FROM_NOT_FOUND, build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, sel, out, CAP));
    sel.include_purge = true;
    TEST_ASSERT_EQUAL_INT(NB_SLOT, build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, sel, out, CAP));
    TEST_ASSERT_EQUAL_INT(PURGE_SLOT, out[0]);
}

void test_errors(void)
{
    const int len = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
    int out[CAP];
    SlotSelection sel = default_selection();

    sel.count = -2;
    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_COUNT, build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, sel, out, CAP));

    sel = default_selection();
    TEST_ASSERT_EQUAL_INT(SLOTLIST_CAPACITY, build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, sel, out, NB_SLOT - 2));
    TEST_ASSERT_EQUAL_INT(NB_SLOT - 1, build_slot_list(FILL_ORDER, len, NB_SLOT, PURGE_SLOT, sel, out, NB_SLOT - 1));

    uint8_t bad[3] = {1, 1, 2};
    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_FILL_ORDER, build_slot_list(bad, 3, 4, 0, sel, out, CAP));
    sel.index_order = true; // still validated: a broken FILL_ORDER is a config error
    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_FILL_ORDER, build_slot_list(bad, 3, 4, 0, sel, out, CAP));

    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_SLOTS, build_slot_list(bad, 3, 0, 0, sel, out, CAP));
    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_SLOTS, build_slot_list(bad, 3, 4, 4, sel, out, CAP));
    TEST_ASSERT_EQUAL_INT(SLOTLIST_BAD_SLOTS, build_slot_list(bad, 3, 4, -1, sel, out, CAP));
}

void test_single_slot_machine(void)
{
    // Only a purge slot: nothing to sample, but a rotation test can still visit purge.
    uint8_t none[1] = {0};
    SlotSelection sel = default_selection();
    int out[CAP];
    TEST_ASSERT_EQUAL_INT(0, build_slot_list(none, 0, 1, 0, sel, out, CAP));
    sel.include_purge = true;
    TEST_ASSERT_EQUAL_INT(1, build_slot_list(none, 0, 1, 0, sel, out, CAP));
    TEST_ASSERT_EQUAL_INT(0, out[0]);
}

void test_error_names(void)
{
    TEST_ASSERT_EQUAL_STRING("bad_fill_order", slot_list_error_name(SLOTLIST_BAD_FILL_ORDER));
    TEST_ASSERT_EQUAL_STRING("from_not_in_order", slot_list_error_name(SLOTLIST_FROM_NOT_FOUND));
    TEST_ASSERT_EQUAL_STRING("bad_count", slot_list_error_name(SLOTLIST_BAD_COUNT));
    TEST_ASSERT_EQUAL_STRING("too_many_slots", slot_list_error_name(SLOTLIST_CAPACITY));
    TEST_ASSERT_EQUAL_STRING("bad_slot_config", slot_list_error_name(SLOTLIST_BAD_SLOTS));
    TEST_ASSERT_EQUAL_STRING("ok", slot_list_error_name(3));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_firmware_fill_order_is_valid);
    RUN_TEST(test_fill_order_validation_rejects);
    RUN_TEST(test_default_selection_is_the_whole_fill_order);
    RUN_TEST(test_all_combinations_15_slots_firmware);
    RUN_TEST(test_all_combinations_24_slots);
    RUN_TEST(test_all_combinations_24_slots_purge_in_the_middle);
    RUN_TEST(test_index_order_skips_purge);
    RUN_TEST(test_from_purge_needs_include_purge);
    RUN_TEST(test_errors);
    RUN_TEST(test_single_slot_machine);
    RUN_TEST(test_error_names);
    return UNITY_END();
}
