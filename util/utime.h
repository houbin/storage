#ifndef STORAGE_UTIME_H_
#define STORAGE_UTIME_H_

#include <sys/time.h>
#include <math.h>
#include <stdint.h>

namespace util
{

class UTime
{
public:
    uint32_t tv_sec;
    uint32_t tv_nsec;

    bool IsZero()
    {
        return ((tv_sec == 0) && (tv_nsec == 0));
    }

    void normalize()
    {
        if (tv_nsec > 1000000000ul)
        {
            tv_sec += (tv_nsec / 1000000000ul);
            tv_nsec = tv_nsec % 1000000000ul;
        }
    }

    UTime() { tv_sec = 0; tv_nsec = 0;}
    UTime(uint32_t s, uint32_t n) { tv_sec = s; tv_nsec = n; normalize(); }
    UTime(struct timeval &time) { tv_sec = time.tv_sec; tv_nsec = (time.tv_usec * 1000); }
    UTime(struct timespec &time) { tv_sec = time.tv_sec; tv_nsec = time.tv_nsec; }

    UTime(const UTime &time) { tv_sec = time.tv_sec; tv_nsec = time.tv_nsec; }
    UTime& operator=(const UTime& time)
    {
        this->tv_sec = time.tv_sec;
        this->tv_nsec = time.tv_nsec;

        return *this;
    }

    UTime& operator+=(const UTime &time)
    {
        this->tv_sec += time.tv_sec;
        this->tv_nsec += time.tv_nsec;
        normalize();
        
        return *this;
    }

    UTime& operator+=(const double f)
    {
        double fs = trunc(f);
        double ns = (f - fs) * (double)1000000000.0;
        this->tv_sec += fs;
        this->tv_nsec += ns;
        normalize();

        return *this;
    }

    UTime& operator-=(const UTime &time)
    {
        this->tv_sec = this->tv_sec - time.tv_sec - (this->tv_nsec < time.tv_nsec ? 1 : 0);
        this->tv_nsec = (this->tv_nsec < time.tv_nsec) ? (this->tv_nsec + 1000000000ul - time.tv_nsec) 
                            : (this->tv_nsec - time.tv_nsec);
        return *this;
    }

    void ToTimeSpec(struct timespec *ts) const
    {
        ts->tv_sec = tv_sec;
        ts->tv_nsec = tv_nsec;
    }
};

inline UTime operator+(const UTime &l, const UTime &r)
{
    return UTime(l.tv_sec + r.tv_sec, l.tv_nsec + r.tv_nsec);
}

inline UTime operator-(const UTime &l, const UTime &r)
{
    return UTime(l.tv_sec - r.tv_sec - (l.tv_nsec < r.tv_nsec ? 1 : 0), 
                (l.tv_nsec < r.tv_nsec) ? (l.tv_nsec + 1000000000ul - r.tv_nsec) : (l.tv_nsec - r.tv_nsec));
}

inline bool operator>(const UTime &a, const UTime &b)
{
    return (a.tv_sec > b.tv_sec) || ((a.tv_sec == b.tv_sec) && (a.tv_nsec > b.tv_nsec));
}

inline bool operator<(const UTime &a, const UTime &b)
{
    return ((a.tv_sec < b.tv_sec) || ((a.tv_sec == b.tv_sec) && (a.tv_nsec < b.tv_nsec)));
}

inline bool operator==(const UTime &a, const UTime &b)
{
    return ((a.tv_sec == b.tv_sec) && (a.tv_nsec == b.tv_nsec));
}

inline bool operator>=(const UTime &a, const UTime &b)
{
    return !(operator<(a, b));
}

inline bool operator<=(const UTime &a, const UTime &b)
{
    return !(operator>(a, b));
}

inline bool operator!=(const UTime &a, const UTime &b)
{
    return !(operator==(a, b));
}

}

#endif
