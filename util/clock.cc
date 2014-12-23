#include <sys/time.h>
#include "clock.h"
#include <cstddef>

namespace util
{

UTime GetClockNow()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    UTime n(tv);

    return n;
}

}
