#ifndef ORING_LOCK_H
#define ORING_LOCK_H

#include <Arduino.h>
#include "../src/Core/lock_rules.h"

// O-ring lock state (FW2, F18): LOCKED / UNLOCKED / UNKNOWN, UNKNOWN at every boot.
// The rules (may rotate / lock / unlock / home) are pure functions in src/Core/lock_rules.

bool lock_oring();   // UNLOCKED -> LOCKED (no-op if LOCKED). Refuses (false) while UNKNOWN
bool unlock_oring(); // LOCKED -> UNLOCKED (no-op if UNLOCKED). Refuses (false) while UNKNOWN
bool is_oring_locked();
core::LockState oring_lock_state();
const char *oring_lock_state_name();

// Makes the state known again: only if the encoder reads the manifold within
// ACTUATOR_HOME_SLOT_TOLERANCE_DEG of a slot, runs the overdrive lock (a stall against
// the physical stop), then the normal unlock -> UNLOCKED. Otherwise refuses (false) and
// leaves the state as it was. Prints why. slot_out / error_out: the slot and signed
// encoder error it measured (-1 / NaN if the encoder failed).
bool actuator_home(int *slot_out = nullptr, float *error_out = nullptr);

// Forget the state after a move the lock code did not make (manual jog, calibration).
void oring_mark_unknown(const char *why);

#endif
