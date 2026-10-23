#include "run_keys.h"

#include <string.h>

namespace core
{

static bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool same_word(const char *a, const char *b)
{
    // case-insensitive compare of two NUL-terminated words
    while (*a && *b)
    {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z')
            x = x - 'A' + 'a';
        if (y >= 'A' && y <= 'Z')
            y = y - 'A' + 'a';
        if (x != y)
            return false;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

RunKey skip_or_abort(const char *line)
{
    if (line == nullptr)
        return RUN_KEY_NONE;
    // trim: copy the word without the surrounding spaces (a typed line is short, the buffer is small)
    char word[16];
    size_t start = 0;
    size_t len = strlen(line);
    while (start < len && is_space(line[start]))
        start++;
    size_t end = len;
    while (end > start && is_space(line[end - 1]))
        end--;
    if (end - start == 0 || end - start >= sizeof word)
        return RUN_KEY_NONE;
    memcpy(word, line + start, end - start);
    word[end - start] = 0;

    if (same_word(word, "s") || same_word(word, "skip"))
        return RUN_KEY_SKIP;
    if (same_word(word, "x") || same_word(word, "abort") || same_word(word, "stop"))
        return RUN_KEY_ABORT;
    return RUN_KEY_NONE;
}

float purge_volume_ml(bool spool_installed, float no_spool_ml, float spool_ml)
{
    return spool_installed ? spool_ml : no_spool_ml;
}

} // namespace core
