#ifndef STORAGE_LOGGER_H_
#define STORAGE_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <stdarg.h>
#include <assert.h>
#include "errcode.h"

namespace util {

class Logger
{
private:
    FILE *file_;
public:
    Logger(FILE *f): file_(f) {}

    virtual ~Logger()
    {
        fclose(file_);
    }

    void Logv(const char* format, va_list ap);
};

int32_t NewLogger(const char* name, Logger **logger);
void LogFunc(Logger *info_log, const char* format, ...);
#define Log(info_log, format, ...) LogFunc(info_log, "[File: %s, Func: %s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

}

#endif
