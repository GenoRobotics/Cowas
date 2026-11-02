#include "spool_fill.h"

#include <string.h>

#include "test_args.h"

namespace core
{

int spool_session_depth_parse(const char *text, float lo_m, float hi_m, float *m)
{
    if (text == nullptr)
    {
        return 0;
    }
    // trim the blanks at both ends: an empty answer (ENTER) means the default
    const char *begin = text;
    while (*begin == ' ' || *begin == '\t')
    {
        begin++;
    }
    const char *end = begin + strlen(begin);
    while (end > begin && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
    {
        end--;
    }
    if (end == begin)
    {
        return 0;
    }
    char buf[32];
    const size_t n = (size_t)(end - begin);
    if (n >= sizeof buf)
    {
        return -1;
    }
    memcpy(buf, begin, n);
    buf[n] = '\0';
    for (size_t i = 0; i < n; i++)
    {
        if (buf[i] == ',')
        {
            buf[i] = '.';
        }
    }
    float value = 0;
    if (!parse_float_strict(buf, &value) || value < lo_m || value > hi_m)
    {
        return -1;
    }
    *m = value;
    return 1;
}

} // namespace core
