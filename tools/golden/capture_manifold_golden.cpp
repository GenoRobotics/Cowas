// Golden-value capture for FW0. Compiles the ORIGINAL manifold math, pasted
// verbatim (via #include of slices cut from `git show d508f41:src/Hardware/Manifold.cpp`
// by capture_manifold_golden.sh), with the constants of that commit's Manifold.h /
// Settings.h. Prints the raw values; the script turns them into golden_manifold.h.
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef CW
#define CW true
#endif

// --- constants copied from HEAD include/Manifold.h and include/Settings.h ---
const float purge_angle = 342.07;
const float angle_between_slots = 22.5;
const int omitted_angle_nb = 12;
const int PURGE_RAW_OFFSET = 8;
const int NB_SLOT = 15;
const bool MANIFOLD_RAW_INDEX_INCREASES_CW = CW;
const float encoder_to_deg = 360.0 / 4096.0;

float sterivex_angle[15];
int raw_slot_of_human_idx[15];

void manifold_begin_table()
{
#include "slice_table_walk.inc"
}

#include "slice_clock.inc"

#include "slice_scale.inc"

static uint32_t bits(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }

int main()
{
    manifold_begin_table();
    printf("// cw=%d\n", (int)CW);
    printf("TABLE\n");
    for (int i = 0; i < 15; i++)
        printf("  {0x%08Xu, %d}, // [%d] %.9g deg\n", bits(sterivex_angle[i]), raw_slot_of_human_idx[i], i, sterivex_angle[i]);
    printf("CLOCK_FOR_SLOT (human -1..16)\n");
    for (int s = -1; s <= 16; s++)
        printf("  %d,", clock_minutes_for_slot(s));
    printf("\nSLOT_FOR_CLOCK (minutes -1..60)\n");
    for (int m = -1; m <= 60; m++)
        printf("  %d,", slot_for_clock_minutes(m));
    printf("\nSCALE\n");
    // inputs: encoder-derived angles exactly as readEncoder() builds them
    // (current_angle = pos * encoder_to_deg + 360 * nb_turns), plus hand-picked values
    for (int turns = -2; turns <= 2; turns++) {
        int step = (turns == 0) ? 4 : 64;
        for (int pos = 0; pos < 4096; pos += step) {
            uint16_t p = (uint16_t)pos;
            float deg = p * encoder_to_deg;
            float current = deg + 360 * turns;
            printf("  {0x%08Xu, 0x%08Xu},\n", bits(current), bits(scale_angle(current)));
        }
    }
    const float extra[] = {0.0f, 360.0f, -360.0f, 720.0f, 1000.5f, -1000.5f, 3600.0f, -3600.0f,
                           purge_angle, purge_angle - 11.25f, purge_angle + 168.75f, purge_angle - 191.25f,
                           161.32f, 180.0f, -180.0f, 11.25f, 353.32f, 170.07f, 190.0f, 522.07f,
                           -17.93f, 0.1f, 359.9f, 1e-7f, -1e-7f};
    for (unsigned i = 0; i < sizeof(extra) / sizeof(extra[0]); i++)
        printf("  {0x%08Xu, 0x%08Xu},\n", bits(extra[i]), bits(scale_angle(extra[i])));
    for (int i = 0; i < 15; i++)
        printf("  {0x%08Xu, 0x%08Xu},\n", bits(sterivex_angle[i]), bits(scale_angle(sterivex_angle[i])));
    return 0;
}
