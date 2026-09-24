// Native unit tests for src/Core/manifold_geometry (run: pio test -e native).
//
// 1. Golden tests: the Core functions, fed with the firmware geometry, must reproduce
//    the outputs of the original Manifold.cpp code bit for bit (golden_manifold.h).
// 2. Config tests: NB_SLOT, PURGE_SLOT and FILL_ORDER come from the real
//    include/Settings.h (compiled here through test/stubs/).
// 3. Property tests: the same builders with other geometries (16 and 24 positions).
#include <unity.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "manifold_geometry.h"
#include "Settings.h"
#include "golden_manifold.h"

using core::ManifoldGeometry;

// Mirror of include/Manifold.h (it pulls in Arduino-only headers, so it can't be
// included here): MANIFOLD_RAW_POSITIONS, PURGE_RAW_OFFSET, omitted_angle_nb,
// purge_angle, angle_between_slots. The goldens pin these exact values; if Manifold.h
// is recalibrated, update this mirror and regenerate the goldens deliberately
// (tools/golden/capture_manifold_golden.sh). Manifold.cpp static_asserts NB_SLOT.
static const ManifoldGeometry FIRMWARE = {16, 8, 12, 342.07f, 22.5f};

static const int MAX_SLOTS = 64;

static uint32_t bits(float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    return u;
}

static float from_bits(uint32_t u)
{
    float f;
    memcpy(&f, &u, sizeof f);
    return f;
}

struct Table
{
    float angles[MAX_SLOTS];
    int raw[MAX_SLOTS];
    int count;
};

static Table build(const ManifoldGeometry &g)
{
    Table t;
    t.count = core::build_slot_angle_table(g, t.angles, t.raw, MAX_SLOTS);
    return t;
}

// ---------------------------------------------------------------- golden values

void test_golden_angle_table_bit_exact(void)
{
    Table t = build(FIRMWARE);
    TEST_ASSERT_EQUAL_INT(15, t.count);
    for (int i = 0; i < 15; i++)
    {
        char msg[48];
        snprintf(msg, sizeof msg, "human slot %d", i);
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(GOLDEN_TABLE[i].angle_bits, bits(t.angles[i]), msg);
        TEST_ASSERT_EQUAL_INT_MESSAGE(GOLDEN_TABLE[i].raw, t.raw[i], msg);
    }
}

void test_golden_table_purge_at_index_0(void)
{
    Table t = build(FIRMWARE);
    TEST_ASSERT_EQUAL_INT(0, PURGE_SLOT);
    TEST_ASSERT_EQUAL_INT(FIRMWARE.purge_raw_offset, t.raw[PURGE_SLOT]);
    TEST_ASSERT_EQUAL_HEX32(GOLDEN_TABLE[0].angle_bits, bits(t.angles[PURGE_SLOT]));
}

void test_golden_table_never_produces_the_no_hole_position(void)
{
    Table t = build(FIRMWARE);
    float no_hole_angle = FIRMWARE.reference_angle_deg - FIRMWARE.omitted_raw * FIRMWARE.slot_pitch_deg;
    if (no_hole_angle < 0)
        no_hole_angle += 360.0;
    for (int i = 0; i < t.count; i++)
    {
        TEST_ASSERT_NOT_EQUAL(FIRMWARE.omitted_raw, t.raw[i]);
        TEST_ASSERT_NOT_EQUAL(bits(no_hole_angle), bits(t.angles[i]));
    }
}

static void check_clock_goldens(bool cw, const int *golden_clock_for_slot, const int *golden_slot_for_clock)
{
    Table t = build(FIRMWARE);
    for (int h = -1; h <= 16; h++)
    {
        char msg[48];
        snprintf(msg, sizeof msg, "cw=%d human slot %d", cw, h);
        TEST_ASSERT_EQUAL_INT_MESSAGE(golden_clock_for_slot[h + 1],
                                      core::clock_minutes_for_slot(t.raw, NB_SLOT, h, FIRMWARE.raw_positions, cw), msg);
    }
    for (int m = -1; m <= 60; m++)
    {
        char msg[48];
        snprintf(msg, sizeof msg, "cw=%d minutes %d", cw, m);
        TEST_ASSERT_EQUAL_INT_MESSAGE(golden_slot_for_clock[m + 1],
                                      core::slot_for_clock_minutes(t.raw, NB_SLOT, m, FIRMWARE.raw_positions, cw), msg);
    }
}

void test_golden_clock_labels_cw(void)
{
    check_clock_goldens(true, GOLDEN_CLOCK_FOR_SLOT_CW, GOLDEN_SLOT_FOR_CLOCK_CW);
}

void test_golden_clock_labels_ccw(void)
{
    check_clock_goldens(false, GOLDEN_CLOCK_FOR_SLOT_CCW, GOLDEN_SLOT_FOR_CLOCK_CCW);
}

void test_golden_scale_angle_bit_exact(void)
{
    const size_t n = sizeof GOLDEN_SCALE / sizeof GOLDEN_SCALE[0];
    TEST_ASSERT_TRUE(n > 1000);
    for (size_t i = 0; i < n; i++)
    {
        float in = from_bits(GOLDEN_SCALE[i].in_bits);
        float out = core::scale_angle(in, FIRMWARE.reference_angle_deg, FIRMWARE.slot_pitch_deg);
        char msg[64];
        snprintf(msg, sizeof msg, "scale_angle(%.9g), entry %u", in, (unsigned)i);
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(GOLDEN_SCALE[i].out_bits, bits(out), msg);
    }
}

// ---------------------------------------------------------------- firmware config

void test_nb_slot_matches_the_table(void)
{
    Table t = build(FIRMWARE);
    TEST_ASSERT_EQUAL_INT(NB_SLOT, t.count);
    TEST_ASSERT_EQUAL_INT(NB_SLOT, core::slot_count(FIRMWARE));
}

void test_fill_order_is_a_permutation_of_the_sample_slots(void)
{
    const int n = (int)(sizeof FILL_ORDER / sizeof FILL_ORDER[0]);
    TEST_ASSERT_EQUAL_INT(NB_SLOT - 1, n);

    bool seen[MAX_SLOTS] = {false};
    for (int i = 0; i < n; i++)
    {
        int slot = FILL_ORDER[i];
        char msg[48];
        snprintf(msg, sizeof msg, "FILL_ORDER[%d] = %d", i, slot);
        TEST_ASSERT_TRUE_MESSAGE(slot >= 0 && slot < NB_SLOT, msg);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(PURGE_SLOT, slot, msg);
        TEST_ASSERT_FALSE_MESSAGE(seen[slot], msg);
        seen[slot] = true;
    }
}

void test_slot_clock_round_trip_firmware(void)
{
    Table t = build(FIRMWARE);
    for (int cw = 0; cw <= 1; cw++)
    {
        for (int h = 0; h < NB_SLOT; h++)
        {
            int minutes = core::clock_minutes_for_slot(t.raw, NB_SLOT, h, FIRMWARE.raw_positions, cw);
            TEST_ASSERT_EQUAL_INT(h, core::slot_for_clock_minutes(t.raw, NB_SLOT, minutes, FIRMWARE.raw_positions, cw));
        }
    }
}

// ---------------------------------------------------------------- other geometries

// Checks what must hold for ANY geometry: slot count, purge first, the no-hole position
// never produced, a unique raw position per slot, angles in [0, 360) one pitch apart
// along the walk, unique clock labels in 0..59, and the slot <-> clock round trip.
static void check_geometry(const ManifoldGeometry &g, int expected_slots)
{
    Table t = build(g);
    TEST_ASSERT_EQUAL_INT(expected_slots, t.count);
    TEST_ASSERT_EQUAL_INT(expected_slots, core::slot_count(g));
    TEST_ASSERT_EQUAL_INT(g.purge_raw_offset, t.raw[0]);

    bool raw_seen[MAX_SLOTS] = {false};
    for (int i = 0; i < t.count; i++)
    {
        TEST_ASSERT_TRUE(t.raw[i] >= 0 && t.raw[i] < g.raw_positions);
        TEST_ASSERT_NOT_EQUAL(g.omitted_raw, t.raw[i]);
        TEST_ASSERT_FALSE(raw_seen[t.raw[i]]);
        raw_seen[t.raw[i]] = true;

        TEST_ASSERT_TRUE(t.angles[i] >= 0.0f && t.angles[i] < 360.0f);
        // angle = reference - raw * pitch, modulo 360
        float expected = fmodf(g.reference_angle_deg - t.raw[i] * g.slot_pitch_deg + 720.0f, 360.0f);
        TEST_ASSERT_FLOAT_WITHIN(1e-3f, expected, t.angles[i]);
    }

    for (int cw = 0; cw <= 1; cw++)
    {
        bool label_seen[60] = {false};
        for (int h = 0; h < t.count; h++)
        {
            int minutes = core::clock_minutes_for_slot(t.raw, t.count, h, g.raw_positions, cw);
            TEST_ASSERT_TRUE(minutes >= 0 && minutes < 60);
            TEST_ASSERT_FALSE(label_seen[minutes]);
            label_seen[minutes] = true;
            TEST_ASSERT_EQUAL_INT(h, core::slot_for_clock_minutes(t.raw, t.count, minutes, g.raw_positions, cw));
        }
        TEST_ASSERT_EQUAL_INT(-1, core::clock_minutes_for_slot(t.raw, t.count, -1, g.raw_positions, cw));
        TEST_ASSERT_EQUAL_INT(-1, core::clock_minutes_for_slot(t.raw, t.count, t.count, g.raw_positions, cw));
    }

    // scale_angle: always in [-180, 180); the reference maps to half a pitch.
    for (float a = -1000.0f; a <= 1000.0f; a += 7.3f)
    {
        float s = core::scale_angle(a, g.reference_angle_deg, g.slot_pitch_deg);
        TEST_ASSERT_TRUE(s >= -180.0f && s < 180.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.5f * g.slot_pitch_deg,
                             core::scale_angle(g.reference_angle_deg, g.reference_angle_deg, g.slot_pitch_deg));
}

void test_geometry_16_positions_firmware_layout(void)
{
    check_geometry(FIRMWARE, 15);
}

void test_geometry_16_positions_other_layout(void)
{
    // purge at the reference, no-hole just after it, reference near 0/360
    const ManifoldGeometry g = {16, 0, 1, 3.0f, 22.5f};
    check_geometry(g, 15);
}

void test_geometry_16_positions_all_holes(void)
{
    const ManifoldGeometry g = {16, 8, -1, 342.07f, 22.5f};
    check_geometry(g, 16);
}

void test_geometry_24_positions(void)
{
    const ManifoldGeometry g = {24, 12, 18, 200.0f, 15.0f};
    check_geometry(g, 23);
}

void test_geometry_24_positions_no_hole_before_purge(void)
{
    const ManifoldGeometry g = {24, 5, 4, 10.0f, 15.0f};
    check_geometry(g, 23);
    Table t = build(g);
    TEST_ASSERT_EQUAL_INT(3, t.raw[t.count - 1]); // the walk wraps around and stops before the no-hole position
}

void test_build_never_writes_past_capacity(void)
{
    float angles[8];
    int raw[8];
    const float SENTINEL = -12345.0f;
    for (int i = 0; i < 8; i++)
    {
        angles[i] = SENTINEL;
        raw[i] = -7;
    }
    int written = core::build_slot_angle_table(FIRMWARE, angles, raw, 5);
    TEST_ASSERT_EQUAL_INT(5, written);
    for (int i = 5; i < 8; i++)
    {
        TEST_ASSERT_EQUAL_FLOAT(SENTINEL, angles[i]);
        TEST_ASSERT_EQUAL_INT(-7, raw[i]);
    }
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_golden_angle_table_bit_exact);
    RUN_TEST(test_golden_table_purge_at_index_0);
    RUN_TEST(test_golden_table_never_produces_the_no_hole_position);
    RUN_TEST(test_golden_clock_labels_cw);
    RUN_TEST(test_golden_clock_labels_ccw);
    RUN_TEST(test_golden_scale_angle_bit_exact);
    RUN_TEST(test_nb_slot_matches_the_table);
    RUN_TEST(test_fill_order_is_a_permutation_of_the_sample_slots);
    RUN_TEST(test_slot_clock_round_trip_firmware);
    RUN_TEST(test_geometry_16_positions_firmware_layout);
    RUN_TEST(test_geometry_16_positions_other_layout);
    RUN_TEST(test_geometry_16_positions_all_holes);
    RUN_TEST(test_geometry_24_positions);
    RUN_TEST(test_geometry_24_positions_no_hole_before_purge);
    RUN_TEST(test_build_never_writes_past_capacity);
    return UNITY_END();
}
