#ifndef CORE_SLOT_ITERATION_H
#define CORE_SLOT_ITERATION_H

// Which slots a multi-slot test visits, and in what order (FW2): pure logic, no
// Arduino.h, unit-tested with `pio test -e native` (test/test_slot_iteration/).
// Works for any slot count: nothing here knows 14 or 15.

#include <stdint.h>

namespace core
{

// True if `order` holds every sample slot (0..nb_slots-1 except purge_slot) exactly once.
bool fill_order_is_valid(const uint8_t *order, int order_len, int nb_slots, int purge_slot);

// What the operator asked for: `order=index` / `order=fill`, `from=N`, `count=M`.
struct SlotSelection
{
    bool index_order;   // true: 0, 1, 2, ... ; false: the fill order
    bool include_purge; // true: the purge slot comes first (rotation tests)
    int from_slot;      // start at this slot (it must be in the sequence); -1 = the start
    int count;          // how many slots; -1 = all the rest. Clamped to what remains
};

SlotSelection default_selection(); // fill order, no purge, from the start, all

enum SlotListError
{
    SLOTLIST_BAD_FILL_ORDER = -1, // the fill order is not a permutation of the sample slots
    SLOTLIST_FROM_NOT_FOUND = -2, // from_slot is not in the sequence
    SLOTLIST_BAD_COUNT = -3,      // count is 0 or below -1
    SLOTLIST_CAPACITY = -4,       // `out` is too small
    SLOTLIST_BAD_SLOTS = -5       // nb_slots < 1 or purge_slot out of range
};

// Writes the slots to visit into `out` and returns how many (>= 1), or a negative
// SlotListError. The fill order is validated even when index order is asked for.
int build_slot_list(const uint8_t *fill_order, int order_len, int nb_slots, int purge_slot,
                    const SlotSelection &sel, int *out, int capacity);

const char *slot_list_error_name(int error); // "bad_fill_order", ...

} // namespace core

#endif
