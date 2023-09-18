#pragma once

#include "defines.h"

typedef enum log_level
{
    LOG_FATAL   = 0,
    LOG_ERROR   = 1,
    LOG_WARN    = 2,
    LOG_INFO    = 3,
    LOG_DEBUG   = 4,
    LOG_TRACE   = 5
} log_level;

HAPI b8 logger_initialize();

HAPI void logger_shutdown();

HAPI void logger_log(log_level log_severity, const char* message, ...);

#define HFATAL(message, ...) logger_log(LOG_FATAL, message, ##__VA_ARGS__);
#define HERROR(message, ...) logger_log(LOG_ERROR, message, ##__VA_ARGS__);

#if HRELEASE == 1
#define HWARN (message, ...)
#define HINFO (message, ...)
#define HDEBUG(message, ...)
#define HTRACE(message, ...)
#else
#define HWARN(message, ...) logger_log(LOG_WARN, message, ##__VA_ARGS__);
#define HINFO(message, ...) logger_log(LOG_INFO, message, ##__VA_ARGS__);
#define HDEBUG(message, ...) logger_log(LOG_DEBUG, message, ##__VA_ARGS__);
#define HTRACE(message, ...) logger_log(LOG_TRACE, message, ##__VA_ARGS__);
#endif