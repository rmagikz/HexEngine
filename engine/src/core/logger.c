#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static b8 is_initialized = FALSE;
static const char* log_level_strings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};

b8 logger_initialize()
{
    if (is_initialized == TRUE)
    {
        HERROR("logger_initialize() called more than once!");
        return FALSE;
    }
    HINFO("Logger subsystem initialized.");

    // TODO: create log file.
    return TRUE;
}

void logger_shutdown()
{
    // TODO: clean up logger.
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