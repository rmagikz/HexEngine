#include "logger.h"
#include "asserts.h"

#include "core/hstring.h"

#include "memory/hmemory.h"

#include "platform/platform.h"
#include "platform/filesystem.h"

#include <stdarg.h>

typedef struct logger_system_state
{
    file_handle log_file_handle;
} logger_system_state;

static logger_system_state* state_ptr;

static const char* log_level_strings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};

void append_to_log_file(const char* message)
{
    if (!state_ptr || !state_ptr->log_file_handle.is_valid) return;

    u64 length = string_length(message);
    u64 written = 0;
    if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written))
    {
        platform_console_write_error("ERROR: Unable to write to console.log", LOG_ERROR);
    }
}

b8 logger_initialize(u64* memory_requirement, void* state)
{
    *memory_requirement = sizeof(logger_system_state);

    if (!state) return FALSE;
    
    state_ptr = state;

    if (!filesystem_open("console.log", FILE_MODE_WRITE, FALSE, &state_ptr->log_file_handle))
    {
        platform_console_write_error("ERROR: Unable to open console.log for writing", LOG_ERROR);
        return FALSE;
    }

    HINFO("Logger subsystem initialized successfully.");

    

    return TRUE;
}

void logger_shutdown(void* state)
{
    filesystem_close(&state_ptr->log_file_handle);

    state_ptr = 0;

    HINFO("Logger subsystem shut down successfully.");
}

void logger_log(log_level level, const char* message, ...)
{
    b8 is_error = level < 2;

    char out_message[32000];
    hzero_memory(out_message, sizeof(out_message));

    va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    string_format(out_message, "%s %s\n", log_level_strings[level], out_message);

    if (is_error)
    {
        platform_console_write_error(out_message, level);
    }
    else
    {
        platform_console_write(out_message, level);
    }

    append_to_log_file(out_message);

}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line)
{
    logger_log(LOG_FATAL, "Assertion failure: %s, message: %s, in file: %s, line: %d\n", expression, message, file, line);
}