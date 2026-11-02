#ifndef CORE_SPOOL_MOTION_H
#define CORE_SPOOL_MOTION_H

// Pure logic for the spool module (depth): no Arduino.h, no globals, no hardware. Unit-tested on the host with
// `pio test -e native` (test/test_spool_motion/). The motor loop that calls it is Motor::drive() in src/Hardware/Motor.cpp.
//
// Lengths are centimetres of tube unrolled from the origin (the top end-stop). The depth of the intake is
// length - height, where height is the distance from the origin to the water surface.

#include <stdint.h>

namespace core
{

// Depth in metres (as typed in the schedule or a command) -> length to unroll in cm.
// False when depth is NaN, negative, or the length (depth + height) is above max_length_cm.
bool spool_depth_to_length_cm(float depth_m, float height_cm, float max_length_cm, float *length_cm);

// What a move should do next. Checked every millisecond by the motor loop.
enum SpoolMove
{
    SPOOL_MOVE_GO = 0,    // keep going
    SPOOL_MOVE_ARRIVED,   // the goal length is reached (never returned when going to the switch)
    SPOOL_MOVE_STALLED,   // the encoder has not counted for stall_ms: encoder or motor dead, tube jammed
    SPOOL_MOVE_WRONG_WAY, // the length went the other way by more than wrong_way_cm: encoder wired the other way round
    SPOOL_MOVE_TIMEOUT    // the move lasts longer than max_ms
};

struct SpoolMoveParams
{
    int dir;              // +1 = unrolling (the length must grow), -1 = rolling up (it must shrink)
    bool to_switch;       // true: going to the origin switch, the goal and the wrong-way test do not apply
    float goal_cm;        // length to reach (ignored when to_switch)
    uint32_t max_ms;      // longest move
    uint32_t stall_ms;    // longest time with no encoder pulse
    float wrong_way_cm;   // how far the wrong way is allowed before the move stops
};

// start_cm = length at the start of the move, now_cm = length now, elapsed_ms = time since the start,
// still_ms = time since the encoder last counted. Order: arrived, wrong way, stalled, timeout.
SpoolMove spool_move_state(const SpoolMoveParams &p, float start_cm, float now_cm, uint32_t elapsed_ms, uint32_t still_ms);

const char *spool_move_name(SpoolMove m);

// Speed for the approach: slow_speed once the remaining distance (cm, > 0) is inside slow_zone_cm, else fast_speed.
uint8_t spool_approach_speed(float remaining_cm, float slow_zone_cm, uint8_t fast_speed, uint8_t slow_speed);

// Calibration: one stop = the length the encoder said it had unrolled, and the length measured with a tape.
struct SpoolStop
{
    float reading_cm;  // encoder length at the stop (with the diameter used during the move)
    float measured_cm; // tape measure, from the origin mark
};

struct SpoolFit
{
    bool valid;          // false: fewer than 1 stop, a non-positive value, or a ratio outside 0.5..2
    int n;               // stops used
    float scale;         // measured / reading, least squares through the origin (1 = the diameter is right)
    float worst_dev_pct; // largest |measured - scale * reading| / measured, in %: linearity of the spool
};

SpoolFit spool_fit_scale(const SpoolStop *stops, int n);

// The diameter that gives the measured lengths: old_diameter_cm x scale.
float spool_new_diameter_cm(float old_diameter_cm, float scale);

} // namespace core

#endif
