#include "errcode.h"
#include "logger.h"

using namespace storage;

int main()
{
    int32_t ret;
    struct timeval now;
    gettimeofday(&now, NULL);
    time_t sec = now.tv_sec;
    struct tm t;
    localtime_r(&sec, &t);
    char timestr[256] = {0};
    char *p = timestr;
    p += snprintf(timestr, 255, "/tmp/%04d-%02d-%02d_%02d:%02d:%02d.log",
                    t.tm_year + 1900,
                    t.tm_mon + 1,
                    t.tm_mday,
                    t.tm_hour,
                    t.tm_min,
                    t.tm_sec);
    *p = '\0';

    Logger *info_log;
    ret = NewLogger(timestr, &info_log);
    if (ret != OK)
    {
        printf("NewLogger error, error is %d\n", ret);
        return -1;
    }

    int i = 0;
    for (i = 0; i < 100; i++)
    {
        Log(info_log, "sequence %d\n", i);
    }

    return 0;
}
