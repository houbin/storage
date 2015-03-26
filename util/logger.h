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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace util {


class Logger
{
private:
    FILE *file_;
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(FILE *f): file_(f) {}

    virtual ~Logger()
    {
        fclose(file_);
    }

    void Logv(const char* format, va_list ap);

    static LogLevel GetLogLevel();

    static void SetLogLevel(Logger::LogLevel level);

};

int32_t NewLogger(const char* name, Logger **logger);
void LogFunc(Logger *info_log, const char* format, ...);
#define Log(info_log, format, ...) LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

extern Logger::LogLevel g_loglevel;
inline Logger::LogLevel Logger::GetLogLevel()
{
    return g_loglevel;
}

#define LOG_TRACE(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::TRACE) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::DEBUG) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::INFO) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::WARN) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::ERROR) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define LOG_FATAL(info_log, format, ...) if (util::Logger::GetLogLevel() <= Logger::FATAL) \
    LogFunc(info_log, "[%s:%s:%d]: "format, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)
}


#endif
