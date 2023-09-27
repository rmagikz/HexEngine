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

/**
 * @brief Initializes logger subsystem. Call twice; once with state = 0 to get
 * required memory size, then a second time passing allocated memory for internal state.
 * 
 * @param memory_requirement a pointer to hold the required size for the internal state.
 * @param state the pointer in which to store the internal state.
 * @return b8
*/
HAPI b8 logger_initialize(u64* memory_requirement, void* state);

/**
 * @brief Shuts down the logger subsystems and flushes any queued entries to the log file.
 * 
 * @param state the pointer to the internal state for the logger.
*/
HAPI void logger_shutdown(void* state);

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