#include "clean_plan.h"

namespace core
{

static bool is_separator(char c)
{
    return c == ',' || c == ';' || c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

// text starts with `word` (case-insensitive)
static bool starts_with(const char *text, const char *word)
{
    while (*word)
    {
        if (lower(*text) != *word)
        {
            return false;
        }
        text++;
        word++;
    }
    return true;
}

// Reads a run of digits at *p. False when there is none, or the number is absurdly long (kept below 1e6).
static bool read_number(const char *&p, long &value)
{
    if (!is_digit(*p))
    {
        return false;
    }
    value = 0;
    while (is_digit(*p))
    {
        value = value * 10 + (*p - '0');
        if (value > 999999L)
        {
            value = 999999L;
        }
        p++;
    }
    return true;
}

int parse_unused_slots(const char *text, int nb_slots, int purge_slot, bool *unused, int *bad_value)
{
    for (int i = 0; i < nb_slots; i++)
    {
        unused[i] = false;
    }
    if (bad_value)
    {
        *bad_value = -1;
    }
    if (!text)
    {
        return 0;
    }

    const char *p = text;
    while (is_separator(*p))
    {
        p++;
    }
    if (starts_with(p, "skip="))
    {
        p += 5;
    }
    else if (starts_with(p, "unused="))
    {
        p += 7;
    }

    int count = 0;
    for (;;)
    {
        while (is_separator(*p))
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        if (is_digit(*p))
        {
            long first = 0;
            long last = 0;
            read_number(p, first);
            last = first;
            if (*p == '-')
            {
                p++;
                if (!read_number(p, last) || last < first)
                {
                    return UNUSED_BAD_RANGE;
                }
            }
            if (*p != '\0' && !is_separator(*p))
            {
                return UNUSED_BAD_TEXT;
            }
            for (long s = first; s <= last; s++)
            {
                if (s < 0 || s >= nb_slots || s == purge_slot)
                {
                    if (bad_value)
                    {
                        *bad_value = (int)s;
                    }
                    return UNUSED_OUT_OF_RANGE;
                }
                if (!unused[s])
                {
                    unused[s] = true;
                    count++;
                }
            }
        }
        else if (*p == '-')
        {
            return UNUSED_BAD_RANGE;
        }
        else
        {
            // the only words: none / no / n, alone
            const char *w = p;
            int len = 0;
            while (*w && !is_separator(*w))
            {
                w++;
                len++;
            }
            const bool is_none = (len == 4 && starts_with(p, "none")) || (len == 2 && starts_with(p, "no")) ||
                                 (len == 1 && starts_with(p, "n"));
            if (!is_none)
            {
                return UNUSED_BAD_TEXT;
            }
            p = w;
        }
    }
    return count;
}

int build_clean_list(const uint8_t *fill_order, int order_len, int nb_slots, int purge_slot, const bool *unused, int *out,
                     int capacity)
{
    int n = 0;
    for (int i = 0; i < order_len; i++)
    {
        const int slot = fill_order[i];
        if (slot < 0 || slot >= nb_slots || slot == purge_slot || unused[slot])
        {
            continue;
        }
        if (n >= capacity)
        {
            return -1;
        }
        out[n++] = slot;
    }
    return n;
}

} // namespace core
