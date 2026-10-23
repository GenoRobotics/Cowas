#ifndef CORE_MANIFOLD_GEOMETRY_H
#define CORE_MANIFOLD_GEOMETRY_H

// Pure manifold math: no Arduino.h, no globals, no hardware. Built into the firmware
// (called from src/Hardware/Manifold.cpp) and unit-tested on the host with
// `pio test -e native` (test/test_manifold_geometry/). The results are asserted bit
// for bit against the outputs of the original code (golden_manifold.h).

namespace core
{

// Layout of the rotor: `raw_positions` evenly spaced positions around the circle,
// counted from the calibrated reference (raw 0). One of them has no hole.
struct ManifoldGeometry
{
    int raw_positions;         // 16 today (15 holes + 1 no-hole position)
    int purge_raw_offset;      // raw steps from the reference to the purge port (human slot 0)
    int omitted_raw;           // raw position with no hole; out of range = every position has a hole
    float reference_angle_deg; // encoder angle of raw 0 (purge_angle in Manifold.h)
    float slot_pitch_deg;      // angle between neighbouring raw positions (22.5 today)
};

// Number of human slots (purge + samples) the geometry has.
int slot_count(const ManifoldGeometry &g);

// Builds the slot-angle table of Manifold::begin(): walks the raw positions starting at
// the purge port, skipping the no-hole position, so index 0 = purge and 1..N-1 = samples.
// angles[i] = encoder angle of human slot i (0..360), raw_of_slot[i] = its raw position.
// Writes at most `capacity` entries and returns how many it wrote.
int build_slot_angle_table(const ManifoldGeometry &g, float *angles, int *raw_of_slot, int capacity);

// Clock-face label of a raw position: top of the manifold (raw 0) = minute 0, minutes
// increase like a clock's minute hand. Result 0..59.
int clock_minutes_for_raw(int raw, int raw_positions, bool raw_index_increases_cw);

// Human slot (0 = purge) -> clock minutes, or -1 if the slot is out of range.
// raw_of_slot / nb_slots: the table built by build_slot_angle_table().
int clock_minutes_for_slot(const int *raw_of_slot, int nb_slots, int human_slot,
                           int raw_positions, bool raw_index_increases_cw);

// Inverse of clock_minutes_for_slot(): the human slot with that exact label, or -1.
int slot_for_clock_minutes(const int *raw_of_slot, int nb_slots, int minutes,
                           int raw_positions, bool raw_index_increases_cw);

// Maps an encoder angle (may be beyond 0..360 after several turns) to [-180, 180),
// measured from the reference and shifted by half a slot pitch. NaN for a non-finite input.
float scale_angle(float angle, float reference_angle_deg, float slot_pitch_deg);
const int SCALE_ANGLE_MAX_TURNS = 1024; // bit-exact +/- 360 steps up to here, fmod() beyond

// Signed shortest difference a - b, in [-180, 180). NaN if either is NaN.
float angle_diff_deg(float a, float b);

// The slot whose angle (the table of build_slot_angle_table()) is closest to angle_deg,
// and the signed error angle_deg - angles[slot] in *error_deg (may be 0). Returns -1 (and
// NaN) if nb_slots < 1 or angle_deg is NaN.
int nearest_slot(const float *angles, int nb_slots, float angle_deg, float *error_deg);

} // namespace core

#endif
