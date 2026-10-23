/**
 * @file Oring_lock.cpp
 * @brief Operation (unattended) lock/unlock of the manifold o-ring via the linear
 * actuator - the automated counterpart to Tests.cpp's interactive calibration/test
 * tools (calibrate_linear_actuator(), test_linear_actuator()), which stay separate
 * since they need operator keypresses at each step; this runs with no operator
 * watching, so on failure it logs and returns false rather than waiting.
 *
 * Open-loop / dead-reckoning: there used to be a force sensor here to detect actual
 * o-ring contact, but its readings weren't reliable in practice, so it was removed.
 * lock_oring()/unlock_oring() now just move a fixed, calibrated step count
 * (LINEAR_ACT_LOCKED_STEPS) from the known "unlocked" reference position instead of
 * searching for contact - see calibrate_linear_actuator() (Tests.cpp) for how that
 * reference and step count are measured.
 *
 * Anti-drift: lock_oring() deliberately commands MORE steps than
 * LINEAR_ACT_LOCKED_STEPS (see LINEAR_ACT_LOCK_OVERDRIVE_FACTOR, Settings.h) so it
 * always drives the motor into stall against the o-ring, every cycle. A stalled
 * stepper just skips steps rather than overrunning, so the o-ring always ends up
 * compressed against the same real physical stop regardless of any step drift
 * accumulated over previous cycles - unlock_oring() then moves the plain
 * (non-overdriven) step count back from that known-good stop.
 *
 * Lock state (FW2, F18): a tri-state, LOCKED / UNLOCKED / UNKNOWN. The actuator has no
 * position feedback, so the state is only known after a move made here. It is UNKNOWN
 * at every boot (the O-ring may have been left locked) and after a manual jog. While
 * UNKNOWN, lock_oring()/unlock_oring() refuse - a fixed step count from an unknown
 * position could over-travel - and rotateMotor() refuses (core::may_rotate). The only
 * way out is actuator_home(): with a slot aligned, the overdrive lock stalls against the
 * physical stop wherever the actuator started, which is a known reference.
 *
 * Callers (Step_functions.cpp) MUST unlock before any manifold rotation and MUST re-lock
 * after arriving at a slot that will be pumped through - see goto_slot_locked().
 */
#include <Arduino.h>
#include "Oring_lock.h"
#include "Linear_actuator.h"
#include "Manifold.h"
#include "Settings.h"
#include "C_output.h"
#include "Tests.h" // real_keypress_waiting(): an 'x' aborts actuator_home

extern Linear_actuator linear_actuator;
extern C_output output;

static core::LockState oring_state = core::LOCK_STATE_UNKNOWN;

core::LockState oring_lock_state()
{
    return oring_state;
}

const char *oring_lock_state_name()
{
    return core::lock_state_name(oring_state);
}

bool oring_go_to(core::LockState target)
{
    switch (core::lock_move_to(oring_state, target))
    {
    case core::LOCK_MOVE_NONE:
        return true;
    case core::LOCK_MOVE_TO_LOCKED:
        return lock_oring();
    case core::LOCK_MOVE_TO_UNLOCKED:
        return unlock_oring();
    default:
        output.println(String("REFUSED | O-ring is ") + oring_lock_state_name() + ", needs " +
                       core::lock_state_name(target) + " - run actuator_home first");
        return false;
    }
}

OringScope::OringScope(core::LockState target) : ok_(oring_go_to(target))
{
}

OringScope::~OringScope()
{
    if (ok_)
    {
        oring_go_to(core::LOCK_STATE_LOCKED); // rest state
    }
}

bool is_oring_locked()
{
    return oring_state == core::LOCK_STATE_LOCKED;
}

void oring_mark_unknown(const char *why)
{
    if (oring_state != core::LOCK_STATE_UNKNOWN)
    {
        output.println(String("O-ring lock state -> UNKNOWN (") + why + "): run actuator_home before rotating");
    }
    oring_state = core::LOCK_STATE_UNKNOWN;
}

/**
 * @brief Reserved hook for a future physical contact-confirm button (see
 * ORING_CONTACT_BUTTON_INSTALLED in Settings.h) - not implemented yet, so this
 * always reports "contact confirmed" and lock_oring() stays fully open-loop.
 */
static bool confirm_lock_contact()
{
    if (!ORING_CONTACT_BUTTON_INSTALLED)
    {
        return true; // no button yet - trust the dead-reckoned move
    }

    // TODO: once ORING_CONTACT_BUTTON_INSTALLED is true, read the button here and
    // return whether it confirms contact.
    return true;
}

// The overdrive lock move: LINEAR_ACT_LOCKED_STEPS * LINEAR_ACT_LOCK_OVERDRIVE_FACTOR
// backward, into stall. Leaves the driver enabled (holding).
static void drive_lock_move()
{
    linear_actuator.enable();
    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);
    linear_actuator.move_steps(lround(LINEAR_ACT_LOCKED_STEPS * LINEAR_ACT_LOCK_OVERDRIVE_FACTOR), backward);
}

// The plain unlock move: LINEAR_ACT_LOCKED_STEPS forward from the stall, then coils off.
static void drive_unlock_move()
{
    linear_actuator.enable(); // no-op after a lock; needed if the coils were switched off meanwhile
    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);
    linear_actuator.move_steps(LINEAR_ACT_LOCKED_STEPS, forward);
    linear_actuator.disable();
}

/**
 * @brief Compresses the o-ring (overdrive into stall, see file header). No-op if already
 * locked. Refuses while the state is UNKNOWN (run actuator_home). No manual abort -
 * this runs unattended.
 */
bool lock_oring()
{
    if (oring_state == core::LOCK_STATE_LOCKED)
    {
        return true;
    }
    if (!core::may_lock(oring_state))
    {
        output.println(String("REFUSED | lock_oring: O-ring lock state is ") + oring_lock_state_name() + " - run actuator_home first");
        return false;
    }

    drive_lock_move();

    if (!confirm_lock_contact())
    {
        output.println("ERROR | lock_oring: contact not confirmed by the confirm button");
        oring_state = core::LOCK_STATE_UNKNOWN;
        return false;
    }

    output.println("O-ring locked");
    oring_state = core::LOCK_STATE_LOCKED;
    return true;
}

/**
 * @brief Backs the o-ring off to the unlocked reference. No-op if already unlocked.
 * Refuses while the state is UNKNOWN: moving LINEAR_ACT_LOCKED_STEPS forward from an
 * unknown position could drive the actuator past its unlocked end.
 *
 * No blind pressure-settle delay here: in the actual sampling sequence, most
 * unlocks happen after step_sampling()'s air-emptying (CtrlPumpNoWater), which
 * already waits for measured pressure to drop rather than guessing a fixed time.
 */
bool unlock_oring()
{
    if (oring_state == core::LOCK_STATE_UNLOCKED)
    {
        return true;
    }
    if (!core::may_unlock(oring_state))
    {
        output.println(String("REFUSED | unlock_oring: O-ring lock state is ") + oring_lock_state_name() + " - run actuator_home first");
        return false;
    }

    drive_unlock_move();

    output.println("O-ring unlocked");
    oring_state = core::LOCK_STATE_UNLOCKED;
    return true;
}

// The lock moves of actuator_home, one pulse at a time, so an 'x' typed on the console stops
// them. Only actuator_home uses these: lock_oring / unlock_oring run unattended. False = aborted.
static bool move_abortable(long steps, actuator_direction dir)
{
    linear_actuator.set_direction(dir);
    for (long i = 0; i < steps; i++)
    {
        char k = real_keypress_waiting();
        if (k == 'x' || k == 'X')
        {
            return false;
        }
        linear_actuator.step_pulses(1);
    }
    return true;
}

// The move stopped part way, so the O-ring may be part way too: the state is UNKNOWN
// (rotation stays refused) until actuator_home runs again.
static bool home_aborted(bool *aborted_out)
{
    oring_state = core::LOCK_STATE_UNKNOWN;
    if (aborted_out)
    {
        *aborted_out = true;
    }
    output.println("ABORT | actuator_home: stopped by x during a move. The O-ring may be part way, lock state UNKNOWN (no rotation). Run actuator_home again");
    return false;
}

bool actuator_home(int *slot_out, float *error_out, bool *aborted_out)
{
    if (aborted_out)
    {
        *aborted_out = false;
    }
    float error = NAN;
    bool encoder_ok = false;
    int slot = manifold_nearest_slot(&error, &encoder_ok);
    if (slot_out)
    {
        *slot_out = slot;
    }
    if (error_out)
    {
        *error_out = error;
    }

    core::HomeDecision d = core::may_home(encoder_ok, error, ACTUATOR_HOME_SLOT_TOLERANCE_DEG);
    if (d == core::HOME_REFUSED_ENCODER)
    {
        output.println("REFUSED | actuator_home: no valid encoder reading - check the AMT22 (CS D38)");
        return false;
    }
    // 2026-10-23 (user, D36): actuator_home runs whatever the angle. It moves only the O-ring, never the manifold.
    // Alignment is reported (and the homing test records it), it no longer blocks the lock.
    if (d == core::HOME_REFUSED_NOT_ALIGNED)
    {
        output.println("WARN | actuator_home: not aligned (nearest slot " + String(slot) + ", error " + String(error, 2) +
                       " deg, tolerance " + String(ACTUATOR_HOME_SLOT_TOLERANCE_DEG, 1) +
                       " deg). The manifold does not move: lock and unlock run anyway. Align with slotN before rotating-dependent tests");
    }
    else
    {
        output.println("actuator_home: slot " + String(slot) + " aligned (error " + String(error, 2) + " deg)");
    }
    output.println("actuator_home: overdrive lock to the physical stop, then unlock (x = abort)");
    linear_actuator.enable();
    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);
    if (!move_abortable(lround(LINEAR_ACT_LOCKED_STEPS * LINEAR_ACT_LOCK_OVERDRIVE_FACTOR), backward))
    {
        return home_aborted(aborted_out);
    }
    oring_state = core::LOCK_STATE_LOCKED;
    if (!move_abortable(LINEAR_ACT_LOCKED_STEPS, forward))
    {
        return home_aborted(aborted_out);
    }
    linear_actuator.disable();
    oring_state = core::LOCK_STATE_UNLOCKED;
    output.println("actuator_home: done, O-ring UNLOCKED (known)");
    return true;
}
