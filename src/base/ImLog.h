//
// Created by benben on 7/29/15.
//

#ifndef BASE_IMLOG_H
#define BASE_IMLOG_H

#include <stdarg.h>
#include <stdio.h>
#define NO_IM_ERROR_LOG
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#ifndef NO_IM_INFO_LOG
#define log(fmt, args...) ImLog::Logi("<%s>|<%d>|<%s>--INFO--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define logi(fmt, args...) ImLog::Logi("<%s>|<%d>|<%s>--INFO--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
#define log(fmt, args...)
#define logi(fmt, args...)
#endif

#ifndef NO_IM_DEBUG_LOG
#define logd(fmt, args...) ImLog::Logd("<%s>|<%d>|<%s>--DEBUG--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
#define logd(fmt, args...)
#endif

#ifndef NO_IM_WARN_LOG
#define logw(fmt, args...) ImLog::Logw("<%s>|<%d>|<%s>--WARN--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
#define logw(fmt, args...)
#endif

#ifndef NO_IM_ERROR_LOG
#define loge(fmt, args...) ImLog::Loge("<%s>|<%d>|<%s>--ERROR--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
#define loge(fmt, args...)
#endif

#ifndef NO_IM_TRACE_LOG
#define logt(fmt, args...) ImLog::Logt("<%s>|<%d>|<%s>--TRACE--" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
#define logt(fmt, args...)
#endif


class ImLog
{
public:
    static void Logi(const char *format, ...) {
        va_list args;
        va_start(args, format);
        char szBuffer[4096];
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);
        va_end(args);
        printf("%s\n", szBuffer);
    }

    static void Loge(const char *format, ...) {
        va_list args;
        va_start(args, format);
        char szBuffer[4096];
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);
        va_end(args);
        printf("%s\n", szBuffer);
    }

    static void Logd(const char *format, ...) {
        va_list args;
        va_start(args, format);
        char szBuffer[4096];
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);
        va_end(args);
        printf("%s\n", szBuffer);
    }

    static void Logw(const char *format, ...) {
        va_list args;
        va_start(args, format);
        char szBuffer[4096];
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);
        va_end(args);
        printf("%s\n", szBuffer);
    }

    static void Logt(const char *format, ...) {
        va_list args;
        va_start(args, format);
        char szBuffer[4096];
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);
        va_end(args);
        printf("%s\n", szBuffer);
    }
};


#endif //BASE_IMLOG_H
