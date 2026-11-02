#ifndef CORE_CLEAN_PLAN_H
#define CORE_CLEAN_PLAN_H

// Pure logic of the ethanol cleaning of the tubing (`ethanol_purge`): which slots are cleaned. No Arduino.h; unit-tested with
// `pio test -e native` (test/test_clean_plan/). Works for any slot count: nothing here knows 14 or 15.

#include <stdint.h>

namespace core
{

enum UnusedParseError
{
    UNUSED_BAD_TEXT = -1,     // a character that is not a digit, a separator, '-' or the word "none"
    UNUSED_OUT_OF_RANGE = -2, // a slot number that is the purge slot or beyond the last slot (the number is in *bad_value)
    UNUSED_BAD_RANGE = -3     // "5-", "-5", "9-3"
};

// Reads the slots the operator did not use: numbers separated by commas, semicolons or spaces, ranges like "3-5".
// "", "none", "no" and "n" mean no slot. A leading "skip=" or "unused=" is ignored.
// Writes unused[0..nb_slots-1] (true = not used, no need to clean) and returns how many distinct slots, or a negative
// UnusedParseError. On UNUSED_OUT_OF_RANGE, *bad_value is the offending number (bad_value may be nullptr).
int parse_unused_slots(const char *text, int nb_slots, int purge_slot, bool *unused, int *bad_value);

// The slots to clean, in `fill_order` (the manifold visits them with the least rotation), without the unused ones.
// Returns how many (may be 0), or -1 if `out` is too small. A slot of the fill order outside 0..nb_slots-1 or equal to
// purge_slot is ignored.
int build_clean_list(const uint8_t *fill_order, int order_len, int nb_slots, int purge_slot, const bool *unused, int *out,
                     int capacity);

} // namespace core

#endif
