#include "sample_record.h"

#include <math.h>
#include <string.h>

namespace core
{

void PressureStats::reset()
{
    sum_ = 0;
    max_ = 0;
    n_ = 0;
}

void PressureStats::add(float bar)
{
    if (n_ == 0 || bar > max_)
    {
        max_ = bar;
    }
    sum_ += bar;
    n_++;
}

float PressureStats::mean() const
{
    return n_ > 0 ? (float)(sum_ / n_) : NAN;
}

float PressureStats::peak() const
{
    return n_ > 0 ? max_ : NAN;
}

const char *sample_end_name(const char *ctrl_end_reason)
{
    if (strcmp(ctrl_end_reason, "cond") == 0)
    {
        return "volume";
    }
    if (strcmp(ctrl_end_reason, "cap") == 0)
    {
        return "pressure";
    }
    if (strcmp(ctrl_end_reason, "aborted") == 0)
    {
        return "stop_key";
    }
    return ctrl_end_reason; // timeout, sensor, skipped, stalled, clogged, refused, ... as the control loop names them
}

namespace
{

// Writes into buf and remembers whether everything fit.
class Writer
{
public:
    Writer(char *buf, size_t size) : p_(buf), left_(size), ok_(size > 0)
    {
        if (ok_)
        {
            *p_ = '\0';
        }
    }

    bool ok() const { return ok_; }

    int written(char *start) const { return (int)(p_ - start); }

    void raw(const char *s)
    {
        while (*s)
        {
            ch(*s++);
        }
    }

    void ch(char c)
    {
        if (left_ < 2) // keep room for the terminating 0
        {
            ok_ = false;
            return;
        }
        *p_++ = c;
        *p_ = '\0';
        left_--;
    }

    // A quoted string; " and \ are escaped, control characters are replaced by a space.
    void str(const char *s)
    {
        ch('"');
        for (; *s; s++)
        {
            char c = *s;
            if (c == '"' || c == '\\')
            {
                ch('\\');
                ch(c);
            }
            else
            {
                ch(((unsigned char)c < 0x20) ? ' ' : c);
            }
        }
        ch('"');
    }

    void integer(long v)
    {
        char tmp[12];
        int n = 0;
        unsigned long u = v < 0 ? (unsigned long)(-v) : (unsigned long)v;
        do
        {
            tmp[n++] = (char)('0' + u % 10);
            u /= 10;
        } while (u > 0 && n < (int)sizeof(tmp));
        if (v < 0)
        {
            ch('-');
        }
        while (n > 0)
        {
            ch(tmp[--n]);
        }
    }

    // Three decimals, no printf float (see sample_record.h). A NaN is null.
    void number(float v)
    {
        if (isnan(v))
        {
            raw("null");
            return;
        }
        if (v < 0)
        {
            ch('-');
            v = -v;
        }
        unsigned long scaled = (unsigned long)(v * 1000.0f + 0.5f);
        integer((long)(scaled / 1000));
        ch('.');
        unsigned long frac = scaled % 1000;
        ch((char)('0' + frac / 100));
        ch((char)('0' + (frac / 10) % 10));
        ch((char)('0' + frac % 10));
    }

private:
    char *p_;
    size_t left_;
    bool ok_;
};

} // namespace

int format_sample_record(const SampleRecord &r, char *buf, size_t size)
{
    Writer w(buf, size);
    char *start = buf;

    w.raw("{\"t\":\"sample\",\"fw\":");
    w.str(r.fw);
    w.raw(",\"id\":");
    w.str(r.id);
    w.raw(",\"purpose\":");
    w.str(r.purpose);
    if (r.site != NULL)
    {
        w.raw(",\"site\":");
        w.str(r.site);
    }
    if (r.date != NULL)
    {
        w.raw(",\"date\":");
        w.str(r.date);
    }
    if (r.condition != NULL)
    {
        w.raw(",\"condition\":");
        w.str(r.condition);
    }
    w.raw(",\"slot\":");
    w.integer(r.slot);
    w.raw(",\"start_ms\":");
    w.integer((long)r.start_ms);
    w.raw(",\"end_ms\":");
    w.integer((long)r.end_ms);
    w.raw(",\"target_ml\":");
    w.number(r.target_ml);
    w.raw(",\"vol_ml\":");
    w.number(r.vol_ml);
    w.raw(",\"vol_trusted_ml\":");
    w.number(r.vol_trusted_ml);
    w.raw(",\"purge_ml\":");
    w.number(r.purge_ml);
    w.raw(",\"end_reason\":");
    w.str(r.end_reason);
    w.raw(",\"p_mean_bar\":");
    w.number(r.p_mean_bar);
    w.raw(",\"p_max_bar\":");
    w.number(r.p_max_bar);
    w.raw(",\"p_cap_bar\":");
    w.number(r.p_cap_bar);
    w.raw(",\"lock_before\":");
    w.str(r.lock_before);
    w.raw(",\"lock_after\":");
    w.str(r.lock_after);
    w.raw(",\"shield_ml\":");
    w.number(r.shield_ml);
    w.raw(",\"calib\":{\"flow_factor\":");
    w.number(r.flow_factor);
    w.raw(",\"p_gain\":");
    w.number(r.p_gain);
    w.raw(",\"p_offset_bar\":");
    w.number(r.p_offset_bar);
    w.raw(",\"abp_full_scale_psi\":");
    w.number(r.abp_full_scale_psi);
    w.raw(",\"linear_act_locked_steps\":");
    w.integer(r.linear_act_locked_steps);
    w.raw("},\"flags\":[");
    for (int i = 0; i < r.n_flags; i++)
    {
        if (i > 0)
        {
            w.ch(',');
        }
        w.str(r.flags[i]);
    }
    w.raw("]}");

    if (!w.ok())
    {
        return -1;
    }
    return w.written(start);
}

} // namespace core
