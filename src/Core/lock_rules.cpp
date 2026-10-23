#include "lock_rules.h"

#include <math.h>

namespace core
{

const char *lock_state_name(LockState s)
{
    switch (s)
    {
    case LOCK_STATE_UNLOCKED:
        return "UNLOCKED";
    case LOCK_STATE_LOCKED:
        return "LOCKED";
    default:
        return "UNKNOWN";
    }
}

bool may_rotate(LockState s)
{
    return s == LOCK_STATE_UNLOCKED;
}

bool may_lock(LockState s)
{
    return s == LOCK_STATE_UNLOCKED || s == LOCK_STATE_LOCKED;
}

bool may_unlock(LockState s)
{
    return s == LOCK_STATE_UNLOCKED || s == LOCK_STATE_LOCKED;
}

LockMove lock_move_to(LockState current, LockState target)
{
    if (target != LOCK_STATE_LOCKED && target != LOCK_STATE_UNLOCKED)
        return LOCK_MOVE_REFUSE;
    if (current != LOCK_STATE_LOCKED && current != LOCK_STATE_UNLOCKED)
        return LOCK_MOVE_REFUSE;
    if (current == target)
        return LOCK_MOVE_NONE;
    return (target == LOCK_STATE_LOCKED) ? LOCK_MOVE_TO_LOCKED : LOCK_MOVE_TO_UNLOCKED;
}

const char *home_decision_name(HomeDecision d)
{
    switch (d)
    {
    case HOME_ALLOWED:
        return "allowed";
    case HOME_REFUSED_ENCODER:
        return "encoder";
    default:
        return "not_aligned";
    }
}

HomeDecision may_home(bool encoder_ok, float slot_error_deg, float tolerance_deg)
{
    if (!encoder_ok)
    {
        return HOME_REFUSED_ENCODER;
    }
    // Written so that NaN in either value fails the comparison and refuses.
    if (!(tolerance_deg >= 0) || !(fabsf(slot_error_deg) <= tolerance_deg))
    {
        return HOME_REFUSED_NOT_ALIGNED;
    }
    return HOME_ALLOWED;
}

} // namespace core
