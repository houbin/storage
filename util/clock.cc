#include <sys/time.h>
#include "clock.h"

namespace storage
{

UTime GetClockNow()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	UTime n(&tv);

	return n;
}

}
