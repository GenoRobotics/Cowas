#include "test_line.h"

namespace core
{

const char *test_result_name(TestResult r)
{
    switch (r)
    {
    case TEST_PASS:
        return "PASS";
    case TEST_FAIL:
        return "FAIL";
    default:
        return "SKIP";
    }
}

static bool is_structural(char c, bool kv)
{
    if (c == '|' || c == '\r' || c == '\n')
    {
        return true;
    }
    return !kv && (c == ';' || c == '=');
}

// Appends s to out at *pos; false once the buffer is full.
static bool append(char *out, int cap, int *pos, const char *s, bool sanitize, bool kv)
{
    for (; s && *s; s++)
    {
        if (*pos >= cap - 1)
        {
            return false;
        }
        out[(*pos)++] = (sanitize && is_structural(*s, kv)) ? '_' : *s;
    }
    return true;
}

int format_test_line(char *out, int cap, const char *name, const char *id, TestResult r, const char *kv)
{
    if (cap <= 0)
    {
        return -1;
    }
    int pos = 0;
    bool ok = append(out, cap, &pos, "TEST|", false, false) &&
              append(out, cap, &pos, name, true, false) &&
              append(out, cap, &pos, "|", false, false) &&
              append(out, cap, &pos, id, true, false) &&
              append(out, cap, &pos, "|", false, false) &&
              append(out, cap, &pos, test_result_name(r), false, false) &&
              append(out, cap, &pos, "|", false, false) &&
              append(out, cap, &pos, kv, true, true);
    out[pos] = 0;
    return ok ? pos : -1;
}

void sanitize_kv_value(char *s)
{
    for (; s && *s; s++)
    {
        if (is_structural(*s, false))
        {
            *s = '_';
        }
    }
}

} // namespace core
