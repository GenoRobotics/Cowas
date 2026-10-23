#include "test_args.h"

#include <string.h>
#include <stdlib.h>

namespace core
{

const char *args_error_name(ArgsError e)
{
    switch (e)
    {
    case ARGS_OK:
        return "ok";
    case ARGS_TOO_MANY:
        return "too many arguments";
    case ARGS_TOKEN_TOO_LONG:
        return "argument too long";
    case ARGS_EMPTY_KEY:
        return "empty key before '='";
    case ARGS_EMPTY_VALUE:
        return "empty value after '='";
    default:
        return "key given twice";
    }
}

static bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Copies [begin, end) into dst (capacity cap). False if it does not fit.
static bool copy_span(char *dst, int cap, const char *begin, const char *end)
{
    int len = (int)(end - begin);
    if (len >= cap)
    {
        return false;
    }
    memcpy(dst, begin, len);
    dst[len] = 0;
    return true;
}

ArgsError parse_args(const char *line, char *command, int command_cap, Args *out)
{
    out->count = 0;
    if (command_cap > 0)
    {
        command[0] = 0;
    }

    const char *p = line;
    bool first = true;
    while (*p)
    {
        while (*p && is_space(*p))
        {
            p++;
        }
        if (!*p)
        {
            break;
        }
        const char *begin = p;
        while (*p && !is_space(*p))
        {
            p++;
        }
        const char *end = p;

        if (first)
        {
            first = false;
            int len = (int)(end - begin);
            if (command_cap > 0)
            {
                if (len > command_cap - 1)
                {
                    len = command_cap - 1;
                }
                memcpy(command, begin, len);
                command[len] = 0;
            }
            continue;
        }

        if (out->count >= ARGS_MAX_TOKENS)
        {
            return ARGS_TOO_MANY;
        }
        int i = out->count;
        const char *eq = begin;
        while (eq < end && *eq != '=')
        {
            eq++;
        }
        if (eq == end)
        {
            out->key[i][0] = 0;
            if (!copy_span(out->value[i], ARGS_TOKEN_LEN, begin, end))
            {
                return ARGS_TOKEN_TOO_LONG;
            }
        }
        else
        {
            if (eq == begin)
            {
                return ARGS_EMPTY_KEY;
            }
            if (eq + 1 == end)
            {
                return ARGS_EMPTY_VALUE;
            }
            if (!copy_span(out->key[i], ARGS_TOKEN_LEN, begin, eq) ||
                !copy_span(out->value[i], ARGS_TOKEN_LEN, eq + 1, end))
            {
                return ARGS_TOKEN_TOO_LONG;
            }
            for (int j = 0; j < i; j++)
            {
                if (out->key[j][0] && strcmp(out->key[j], out->key[i]) == 0)
                {
                    return ARGS_DUPLICATE_KEY;
                }
            }
        }
        out->count++;
    }
    return ARGS_OK;
}

const char *args_positional(const Args &a, int i)
{
    for (int t = 0; t < a.count; t++)
    {
        if (a.key[t][0] == 0)
        {
            if (i == 0)
            {
                return a.value[t];
            }
            i--;
        }
    }
    return 0;
}

bool args_has_flag(const Args &a, const char *word)
{
    for (int t = 0; t < a.count; t++)
    {
        if (a.key[t][0] == 0 && strcmp(a.value[t], word) == 0)
        {
            return true;
        }
    }
    return false;
}

const char *args_value(const Args &a, const char *key)
{
    for (int t = 0; t < a.count; t++)
    {
        if (a.key[t][0] && strcmp(a.key[t], key) == 0)
        {
            return a.value[t];
        }
    }
    return 0;
}

static bool in_list(const char *s, const char *const *list, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(s, list[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

int args_positional_count(const Args &a)
{
    int n = 0;
    for (int t = 0; t < a.count; t++)
    {
        if (a.key[t][0] == 0)
        {
            n++;
        }
    }
    return n;
}

int args_first_unknown(const Args &a, const char *const *keys, int nkeys,
                       const char *const *flags, int nflags, int max_positionals)
{
    int positionals = 0;
    for (int t = 0; t < a.count; t++)
    {
        if (a.key[t][0])
        {
            if (!in_list(a.key[t], keys, nkeys))
            {
                return t;
            }
        }
        else if (!in_list(a.value[t], flags, nflags))
        {
            positionals++;
            if (positionals > max_positionals)
            {
                return t;
            }
        }
    }
    return -1;
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool parse_long_strict(const char *s, long *out)
{
    if (!s)
    {
        return false;
    }
    const char *p = s;
    bool negative = false;
    if (*p == '+' || *p == '-')
    {
        negative = (*p == '-');
        p++;
    }
    int digits = 0;
    long value = 0;
    while (is_digit(*p))
    {
        if (++digits > 9)
        {
            return false;
        }
        value = value * 10 + (*p - '0');
        p++;
    }
    if (digits == 0 || *p)
    {
        return false;
    }
    *out = negative ? -value : value;
    return true;
}

bool parse_float_strict(const char *s, float *out)
{
    if (!s)
    {
        return false;
    }
    const char *p = s;
    if (*p == '+' || *p == '-')
    {
        p++;
    }
    int digits = 0;
    while (is_digit(*p))
    {
        p++;
        digits++;
    }
    if (*p == '.')
    {
        p++;
        while (is_digit(*p))
        {
            p++;
            digits++;
        }
    }
    if (digits == 0 || *p || digits > 12)
    {
        return false;
    }
    *out = (float)strtod(s, 0);
    return true;
}

static char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

bool parse_yes_no(const char *s, bool *out)
{
    if (!s)
    {
        return false;
    }
    char buf[4];
    int n = 0;
    while (s[n] && n < 3)
    {
        buf[n] = lower(s[n]);
        n++;
    }
    if (s[n])
    {
        return false; // longer than "yes"
    }
    buf[n] = 0;
    if (strcmp(buf, "y") == 0 || strcmp(buf, "yes") == 0)
    {
        *out = true;
        return true;
    }
    if (strcmp(buf, "n") == 0 || strcmp(buf, "no") == 0)
    {
        *out = false;
        return true;
    }
    return false;
}

bool parse_ml_or_seconds(const char *s, float *ml, long *seconds)
{
    if (!s || !*s)
    {
        return false;
    }
    int len = (int)strlen(s);
    if (lower(s[len - 1]) == 's')
    {
        char buf[ARGS_TOKEN_LEN];
        if (len - 1 >= ARGS_TOKEN_LEN || len == 1)
        {
            return false;
        }
        memcpy(buf, s, len - 1);
        buf[len - 1] = 0;
        long value;
        if (!parse_long_strict(buf, &value) || value <= 0)
        {
            return false;
        }
        *seconds = value;
        *ml = 0;
        return true;
    }
    float value;
    if (!parse_float_strict(s, &value) || !(value > 0))
    {
        return false;
    }
    *ml = value;
    *seconds = 0;
    return true;
}

bool selection_from_args(const Args &a, SlotSelection base, SlotSelection *out, const char **error)
{
    SlotSelection sel = base;
    const char *v;
    long n;

    if ((v = args_value(a, "from")) != 0)
    {
        if (!parse_long_strict(v, &n) || n < 0)
        {
            *error = "from=N needs a slot number";
            return false;
        }
        sel.from_slot = (int)n;
    }
    if ((v = args_value(a, "count")) != 0)
    {
        if (!parse_long_strict(v, &n) || n < 1)
        {
            *error = "count=M needs a number >= 1";
            return false;
        }
        sel.count = (int)n;
    }
    if ((v = args_value(a, "order")) != 0)
    {
        if (strcmp(v, "index") == 0)
        {
            sel.index_order = true;
        }
        else if (strcmp(v, "fill") == 0)
        {
            sel.index_order = false;
        }
        else
        {
            *error = "order= must be index or fill";
            return false;
        }
    }
    *out = sel;
    return true;
}

} // namespace core
