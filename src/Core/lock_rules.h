#ifndef CORE_LOCK_RULES_H
#define CORE_LOCK_RULES_H

// O-ring lock rules (FW2, F18): pure decisions, no Arduino.h. Used by
// src/Hardware/Oring_lock.cpp and rotateMotor(), unit-tested with `pio test -e native`
// (test/test_lock_rules/).

namespace core
{

// What the firmware knows about the O-ring. The actuator is open-loop (no sensor), so
// the state is only known after a move the firmware made itself. UNKNOWN at every boot,
// and after any manual jog (linear_actuator, calibrate_actuator).
enum LockState
{
    LOCK_STATE_UNKNOWN = 0,
    LOCK_STATE_UNLOCKED,
    LOCK_STATE_LOCKED
};

const char *lock_state_name(LockState s); // "UNKNOWN", "UNLOCKED", "LOCKED"

// The manifold may rotate only when the O-ring is known to be unlocked (ROADMAP §6 rule 4).
bool may_rotate(LockState s);

// lock_oring() / unlock_oring() move a fixed number of steps from a known position, so
// they need a known state. From UNKNOWN only actuator_home may move the actuator.
bool may_lock(LockState s);
bool may_unlock(LockState s);

enum HomeDecision
{
    HOME_ALLOWED = 0,
    HOME_REFUSED_ENCODER,     // no valid encoder reading
    HOME_REFUSED_NOT_ALIGNED  // the rotor is not within the tolerance of a slot
};

const char *home_decision_name(HomeDecision d); // "allowed", "encoder", "not_aligned"

// Target-based moves (oring_go_to): what has to happen to take the O-ring to a target state.
// Used by every function that pumps (target LOCKED) or turns the manifold (target UNLOCKED).
enum LockMove
{
    LOCK_MOVE_NONE = 0,    // already in the target state
    LOCK_MOVE_TO_LOCKED,   // UNLOCKED -> LOCKED
    LOCK_MOVE_TO_UNLOCKED, // LOCKED -> UNLOCKED
    LOCK_MOVE_REFUSE       // UNKNOWN (run actuator_home), or a target that is neither LOCKED nor UNLOCKED
};
LockMove lock_move_to(LockState current, LockState target);

// actuator_home runs the overdrive lock (a stall against the physical stop), which is
// only safe with a rotor hole in front of the O-ring. Allowed when the encoder reads and
// |slot_error_deg| <= tolerance_deg. A NaN error or a NaN / negative tolerance refuses.
HomeDecision may_home(bool encoder_ok, float slot_error_deg, float tolerance_deg);

} // namespace core

#endif
