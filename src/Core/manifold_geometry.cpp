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

    // F46 (fixed in FW2): the original loop was unbounded (its `loop_nb < 3` guard was
    // never incremented), so +/-inf, or a finite value so large that -= 360 no longer
    // changes it, looped forever. The same +/- 360 steps run first, so every input within
    // SCALE_ANGLE_MAX_TURNS turns gives the same bits as before (golden tests); beyond
    // that, fmod() finishes the job, and a non-finite input returns NaN.
    if (!isfinite(new_angle))
    {
        return NAN;
    }
    for (int turns = 0; turns < SCALE_ANGLE_MAX_TURNS; turns++)
    {
        if (new_angle < -180)
        {
            new_angle += 360.0;
        }
        else if (new_angle >= 180)
        {
            new_angle -= 360.0;
        }
        else
        {
            return new_angle;
        }
    }
    double reduced = fmod((double)new_angle + 180.0, 360.0);
    if (reduced < 0)
    {
        reduced += 360.0;
    }
    new_angle = (float)(reduced - 180.0);
    return new_angle >= 180 ? -180.0f : new_angle; // float rounding can land exactly on 180
}

float angle_diff_deg(float a, float b)
{
    double d = fmod((double)a - (double)b, 360.0); // (-360, 360), NaN stays NaN
    if (d >= 180.0)
    {
        d -= 360.0;
    }
    else if (d < -180.0)
    {
        d += 360.0;
    }
    return (float)d;
}

int nearest_slot(const float *angles, int nb_slots, float angle_deg, float *error_deg)
{
    int best = -1;
    float best_error = NAN;
    for (int i = 0; i < nb_slots; i++)
    {
        float e = angle_diff_deg(angle_deg, angles[i]);
        if (e != e)
        {
            continue; // NaN
        }
        if (best < 0 || fabsf(e) < fabsf(best_error))
        {
            best = i;
            best_error = e;
        }
    }
    if (error_deg)
    {
        *error_deg = best_error;
    }
    return best;
}

} // namespace core
