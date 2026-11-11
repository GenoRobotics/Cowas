#include "step_log.h"

#include <string.h>

namespace core
{

namespace
{

// Appends to buf with a size limit; ok_ goes false on the first character that does not fit.
class Out
{
public:
    Out(char *buf, size_t size) : p_(buf), left_(size), ok_(size > 0 && buf != NULL) {}

    void raw(const char *s)
    {
        while (ok_ && *s != '\0')
        {
            ch(*s++);
        }
    }

    void ch(char c)
    {
        if (!ok_)
        {
            return;
        }
        if (left_ < 2) // keep one for the terminating 0
        {
            ok_ = false;
            return;
        }
        *p_++ = c;
        left_--;
    }

    void str(const char *s)
    {
        ch('"');
        for (; s != NULL && *s != '\0'; s++)
        {
            if (*s == '"' || *s == '\\')
            {
                ch('\\');
            }
            ch(*s);
        }
        ch('"');
    }

    void uint(uint32_t v)
    {
        char digits[12];
        int n = 0;
        do
        {
            digits[n++] = (char)('0' + v % 10);
            v /= 10;
        } while (v > 0);
        while (n > 0)
        {
            ch(digits[--n]);
        }
    }

    bool ok() const { return ok_; }
    int written(const char *start) const { return (int)(p_ - start); }
    char *pos() const { return p_; }

private:
    char *p_;
    size_t left_;
    bool ok_;
};

void copy_name(char *dst, size_t size, const char *src)
{
    size_t i = 0;
    for (; src != NULL && src[i] != '\0' && i + 1 < size; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

} // namespace

void StepLog::reset()
{
    n_ = 0;
    dropped_ = 0;
}

int StepLog::begin(const char *name, uint32_t now_ms)
{
    if (n_ >= CAPACITY)
    {
        dropped_++;
        return -1;
    }
    copy_name(items_[n_].name, sizeof(items_[n_].name), name);
    items_[n_].start_ms = now_ms;
    items_[n_].end_ms = now_ms;
    return n_++;
}

void StepLog::end(int index, uint32_t now_ms)
{
    if (index >= 0 && index < n_)
    {
        items_[index].end_ms = now_ms;
    }
}

void StepLog::add(const char *name, uint32_t start_ms, uint32_t end_ms)
{
    const int i = begin(name, start_ms);
    end(i, end_ms);
}

int format_steps_record(const StepLog &log, const char *id, int slot, char *buf, size_t size)
{
    Out w(buf, size);
    const char *start = buf;

    w.raw("{\"t\":\"steps\",\"id\":");
    w.str(id);
    w.raw(",\"slot\":");
    w.uint((uint32_t)slot); // manifold slots are 0 to 14
    w.raw(",\"steps\":[");
    for (int i = 0; i < log.count(); i++)
    {
        const StepTime &s = log.at(i);
        if (i > 0)
        {
            w.ch(',');
        }
        w.raw("{\"name\":");
        w.str(s.name);
        w.raw(",\"start_ms\":");
        w.uint(s.start_ms);
        w.raw(",\"end_ms\":");
        w.uint(s.end_ms);
        w.raw(",\"dur_ms\":");
        w.uint(s.end_ms - s.start_ms);
        w.ch('}');
    }
    w.raw("],\"steps_dropped\":");
    w.uint((uint32_t)log.dropped());
    w.ch('}');

    if (!w.ok())
    {
        return -1;
    }
    *w.pos() = '\0';
    return w.written(start);
}

} // namespace core
