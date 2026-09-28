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
