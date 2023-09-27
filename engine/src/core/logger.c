#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct logger_system_state
{
    b8 is_initialized;
} logger_system_state;

static logger_system_state* state_ptr;

static const char* log_level_strings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};

b8 logger_initialize(u64* memory_requirement, void* state)
{
    *memory_requirement = sizeof(logger_system_state);

    if (!state) return FALSE;
    
    state_ptr = state;
    state_ptr->is_initialized = TRUE;

    HINFO("Logger subsystem initialized successfully.");

    // TODO: create log file.
    return TRUE;
}

void logger_shutdown(void* state)
{
    // TODO: clean up logger.

    state_ptr = 0;

    HINFO("Logger subsystem shut down successfully.");
}

void logger_log(log_level level, const char* message, ...)
{
    b8 is_error = level < LOG_WARN;

    // TODO: dynamic list.
    const i32 message_length = 32000;

    char formatted_message[message_length];
    memset(formatted_message, 0, sizeof(formatted_message));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(formatted_message, message_length, message, arg_ptr);
    va_end(arg_ptr);

    char out_message[message_length];
    sprintf(out_message, "%s %s\n", log_level_strings[level], formatted_message);

    if (is_error)
    {
        platform_console_write_error(out_message, level);
    }
    else
    {
        platform_console_write(out_message, level);
    }

}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line)
{
    logger_log(LOG_FATAL, "Assertion failure: %s, message: %s, in file: %s, line: %d\n", expression, message, file, line);
}