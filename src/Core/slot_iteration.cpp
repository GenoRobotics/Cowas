#include "slot_iteration.h"

namespace core
{

bool fill_order_is_valid(const uint8_t *order, int order_len, int nb_slots, int purge_slot)
{
    if (nb_slots < 1 || purge_slot < 0 || purge_slot >= nb_slots || order_len != nb_slots - 1)
    {
        return false;
    }
    for (int i = 0; i < order_len; i++)
    {
        int slot = order[i];
        if (slot >= nb_slots || slot == purge_slot)
        {
            return false;
        }
        for (int j = 0; j < i; j++)
        {
            if (order[j] == slot)
            {
                return false; // duplicate
            }
        }
    }
    return true; // nb_slots - 1 distinct sample slots = all of them
}

SlotSelection default_selection()
{
    SlotSelection sel;
    sel.index_order = false;
    sel.include_purge = false;
    sel.from_slot = -1;
    sel.count = -1;
    return sel;
}

// Slot at position `i` of the full sequence (purge first if asked, then samples).
static int sequence_at(const uint8_t *fill_order, int purge_slot,
                       const SlotSelection &sel, int i)
{
    if (sel.include_purge)
    {
        if (i == 0)
        {
            return purge_slot;
        }
        i--;
    }
    if (!sel.index_order)
    {
        return fill_order[i];
    }
    // i-th sample slot in ascending order, skipping purge
    return i < purge_slot ? i : i + 1;
}

int build_slot_list(const uint8_t *fill_order, int order_len, int nb_slots, int purge_slot,
                    const SlotSelection &sel, int *out, int capacity)
{
    if (nb_slots < 1 || purge_slot < 0 || purge_slot >= nb_slots)
    {
        return SLOTLIST_BAD_SLOTS;
    }
    if (!fill_order_is_valid(fill_order, order_len, nb_slots, purge_slot))
    {
        return SLOTLIST_BAD_FILL_ORDER;
    }
    if (sel.count == 0 || sel.count < -1)
    {
        return SLOTLIST_BAD_COUNT;
    }

    int total = (nb_slots - 1) + (sel.include_purge ? 1 : 0);
    int start = 0;
    if (sel.from_slot != -1)
    {
        start = -1;
        for (int i = 0; i < total; i++)
        {
            if (sequence_at(fill_order, purge_slot, sel, i) == sel.from_slot)
            {
                start = i;
                break;
            }
        }
        if (start < 0)
        {
            return SLOTLIST_FROM_NOT_FOUND;
        }
    }

    int n = total - start;
    if (sel.count != -1 && sel.count < n)
    {
        n = sel.count;
    }
    if (n > capacity)
    {
        return SLOTLIST_CAPACITY;
    }
    for (int k = 0; k < n; k++)
    {
        out[k] = sequence_at(fill_order, purge_slot, sel, start + k);
    }
    return n;
}

const char *slot_list_error_name(int error)
{
    switch (error)
    {
    case SLOTLIST_BAD_FILL_ORDER:
        return "bad_fill_order";
    case SLOTLIST_FROM_NOT_FOUND:
        return "from_not_in_order";
    case SLOTLIST_BAD_COUNT:
        return "bad_count";
    case SLOTLIST_CAPACITY:
        return "too_many_slots";
    case SLOTLIST_BAD_SLOTS:
        return "bad_slot_config";
    default:
        return error >= 0 ? "ok" : "unknown";
    }
}

} // namespace core
