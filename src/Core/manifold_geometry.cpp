// Pure manifold math, moved out of src/Hardware/Manifold.cpp in FW0 without any change
// in behaviour. The float/double mix of the original expressions is kept on purpose
// (e.g. `+= 360.0` and `60.0 / positions` are double): changing it would change the
// rounding, and the golden tests would catch it.
#include "manifold_geometry.h"

#include <math.h>

namespace core
{

static bool raw_in_range(int raw, int raw_positions)
{
    return raw >= 0 && raw < raw_positions;
}

int slot_count(const ManifoldGeometry &g)
{
    return raw_in_range(g.omitted_raw, g.raw_positions) ? g.raw_positions - 1 : g.raw_positions;
}

int build_slot_angle_table(const ManifoldGeometry &g, float *angles, int *raw_of_slot, int capacity)
{
    // Walks outward from the purge position (purge_raw_offset raw steps from the
    // calibrated reference), so index 0 = purge and 1..N-1 = samples, skipping the
    // no-hole position (omitted_raw) wherever it falls along that walk.
    int human_idx = 0;
    for (int offset = 0; offset < g.raw_positions; offset++)
    {
        int slot = (g.purge_raw_offset + offset) % g.raw_positions;
        if (slot == g.omitted_raw)
        {
            continue; // no hole at this physical position
        }
        if (human_idx >= capacity)
        {
            break;
        }
        float angle = g.reference_angle_deg - slot * g.slot_pitch_deg;
        if (angle < 0)
        {
            angle += 360.0;
        }
        angles[human_idx] = angle;
        raw_of_slot[human_idx] = slot;
        human_idx++;
    }
    return human_idx;
}

int clock_minutes_for_raw(int raw, int raw_positions, bool raw_index_increases_cw)
{
    // 16 raw positions * 3.75 min/position = 60 min, the same scale as a clock face.
    float minutes = raw * (60.0 / raw_positions);
    if (!raw_index_increases_cw)
    {
        minutes = 60.0 - minutes;
    }

    int rounded = (int)lround(minutes) % 60;
    if (rounded < 0)
    {
        rounded += 60;
    }
    return rounded;
}

int clock_minutes_for_slot(const int *raw_of_slot, int nb_slots, int human_slot,
                           int raw_positions, bool raw_index_increases_cw)
{
    if (human_slot < 0 || human_slot >= nb_slots)
    {
        return -1;
    }
    return clock_minutes_for_raw(raw_of_slot[human_slot], raw_positions, raw_index_increases_cw);
}

int slot_for_clock_minutes(const int *raw_of_slot, int nb_slots, int minutes,
                           int raw_positions, bool raw_index_increases_cw)
{
    for (int human_slot = 0; human_slot < nb_slots; human_slot++)
    {
        if (clock_minutes_for_slot(raw_of_slot, nb_slots, human_slot, raw_positions, raw_index_increases_cw) == minutes)
        {
            return human_slot;
        }
    }
    return -1;
}

float scale_angle(float angle, float reference_angle_deg, float slot_pitch_deg)
{
    // The original wrote `+ 11.25`, i.e. half of the 22.5 deg pitch; 0.5 * pitch is the
    // same double value, so the result is bit-identical.
    float new_angle = angle - reference_angle_deg + 0.5 * slot_pitch_deg;

    // Unbounded, exactly like the original: its `loop_nb < 3` guard was never
    // incremented. Terminates for any finite angle (NaN exits at once); +/-inf would
    // loop forever. Not reachable from the encoder path. Kept as is in FW0 (no
    // behaviour change); see the FW0 report.
    while (new_angle < -180 || new_angle >= 180)
    {
        if (new_angle < -180)
        {
            new_angle += 360.0;
        }
        else
        {
            new_angle -= 360.0;
        }
    }

    return new_angle;
}

} // namespace core
