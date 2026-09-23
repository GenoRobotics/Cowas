/**
 * @file Oring_lock.cpp
 * @brief Production (unattended) lock/unlock of the manifold o-ring via the linear
 * actuator - the automated counterpart to Tests.cpp's interactive calibration/test
 * tools (calibrate_linear_actuator(), test_linear_actuator()), which stay separate
 * since they need operator keypresses at each step; this runs with no operator
 * watching, so on failure it logs and returns false/no-op rather than waiting.
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
 * (non-overdriven) step count back from that known-good stop. This is why
 * Linear_actuator::position_steps is expected to drift out of sync with the real
 * position once locking happens - that's fine, nothing here reads it back.
 *
 * Tracks locked/unlocked state itself (oring_locked_state) since the actuator has
 * no position feedback of its own - callers (Step_functions.cpp) MUST unlock before
 * any manifold rotation and MUST re-lock after arriving at a slot that will be
 * pumped through (water or air), regardless of slot (filter or purge) - see
 * goto_slot_locked() in Step_functions.cpp.
 */
#include <Arduino.h>
#include "Oring_lock.h"
#include "Linear_actuator.h"
#include "Settings.h"
#include "C_output.h"

extern Linear_actuator linear_actuator;
extern C_output output;

static bool oring_locked_state = false;

bool is_oring_locked()
{
    return oring_locked_state;
}

/**
 * @brief Reserved hook for a future physical contact-confirm button (see
 * ORING_CONTACT_BUTTON_INSTALLED in Settings.h) - not implemented yet, so this
 * always reports "contact confirmed" and lock_oring() stays fully open-loop. Once a
 * button is wired up, replace the body with an actual read and flip the Settings.h
 * flag; nothing else in lock_oring() needs to change.
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

/**
 * @brief Compresses the o-ring: moves LINEAR_ACT_LOCKED_STEPS * LINEAR_ACT_LOCK_OVERDRIVE_FACTOR
 * backward from the unlocked reference - deliberately overshooting into stall so
 * the physical end position is self-correcting regardless of step drift (see file
 * header). No-op if already locked. No manual abort - this runs unattended.
 */
bool lock_oring()
{
    if (oring_locked_state)
    {
        return true;
    }

    linear_actuator.enable();
    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);
    linear_actuator.move_steps(lround(LINEAR_ACT_LOCKED_STEPS * LINEAR_ACT_LOCK_OVERDRIVE_FACTOR), backward);

    if (!confirm_lock_contact())
    {
        output.println("ERROR | lock_oring: contact not confirmed by the confirm button");
        return false;
    }

    output.println("O-ring locked");
    oring_locked_state = true;
    return true;
}

/**
 * @brief Backs the o-ring off LINEAR_ACT_LOCKED_STEPS forward, back to the unlocked
 * reference position. No-op if not currently locked, so it's always safe to call
 * before a rotation regardless of prior state.
 *
 * No blind pressure-settle delay here: in the actual sampling sequence, most
 * unlocks happen after step_sampling()'s air-emptying (CtrlPumpNoWater), which
 * already waits for measured pressure to drop rather than guessing a fixed time.
 * If a specific call site rotates away right after a pressurized fill with no
 * prior air-emptying, add a wait there deliberately instead of blanket-delaying
 * every unlock regardless of whether it's needed.
 */
void unlock_oring()
{
    if (!oring_locked_state)
    {
        return;
    }

    linear_actuator.set_speed(LINEAR_ACT_SLOW_STEPS_PER_SEC);
    linear_actuator.move_steps(LINEAR_ACT_LOCKED_STEPS, forward);
    linear_actuator.disable();

    output.println("O-ring unlocked");
    oring_locked_state = false;
}
