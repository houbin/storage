#include "logger.h"

namespace util
{

void Logger::Logv(const char* format, va_list ap)
{
    char buffer[500];
    char *base;
    int32_t size;

    const uint64_t thread_id = pthread_self();
    for (int iter = 0; iter < 2; iter++)
    {
        if (iter == 0)
        {
            size = sizeof(buffer);
            base = buffer;
        }
        else
        {
            size = 30000;
            base = new char[30000];
        }

        char *p = base;
        char *limit = p + size;
        
        struct timeval now;
        gettimeofday(&now, NULL);
        time_t sec = now.tv_sec;
        struct tm t;
        localtime_r(&sec, &t);
        
        p += snprintf(p, limit - p, 
                     "%04d/%02d/%02d %02d:%02d:%02d.%06d %llx ",
                     t.tm_year + 1900,
                     t.tm_mon + 1,
                     t.tm_mday,
                     t.tm_hour,
                     t.tm_min,
                     t.tm_sec,
                     static_cast<int>(now.tv_usec),
                     static_cast<long long unsigned int>(thread_id)
                    );
        
        if (p < limit)
        {
            va_list backup_ap;
            va_copy(backup_ap, ap);
            p += vsnprintf(p, limit - p, format, backup_ap);
            va_end(backup_ap);
        }

        if (p >= limit)
        {
            if (iter == 0)
            {
                continue;
            }
            else
            {
                p = limit - 1;
            }
        }
        
        if (p == base || p[-1] != '\n')
        {
            *p++ = '\n';
        }

        assert(p <= limit);
        fwrite(base, 1, p - base, file_);
        fflush(file_);
        
        if (base != buffer)
        {
            delete[] base;
        }
        
        break;
    }

    return;
}

int32_t NewLogger(const char* name, Logger **logger)
{
    FILE *f = fopen(name, "w");
    if (NULL == f)
    {
        return -ERR_OPEN_FILE;
    }

    *logger = new Logger(f);
    return OK;
}

void LogFunc(Logger *info_log, const char* format, ...)
{
    if (NULL != info_log)
    {
        va_list ap;
        va_start(ap, format);
        info_log->Logv(format, ap);
        va_end(ap);
    }

    return;
}

}

