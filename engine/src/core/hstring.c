#include "core/hstring.h"
#include "core/hmemory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 string_length(const char* str)
{
    return strlen(str);
}

char* string_duplicate(const char* str)
{
    u64 length = string_length(str);
    char* copy = hallocate(length + 1, MEMORY_TAG_STRING);
    hcopy_memory(copy, str, length + 1);
    return copy;
}

b8 strings_equal(const char* str0, const char* str1)
{
    return strcmp(str0, str1);
}

i32 string_format(char* dest, const char* format, ...)
{
    if (!dest) return -1;

    __builtin_va_list arg_ptr;

    va_start(arg_ptr, format);
    i32 written = string_format_v(dest, format, arg_ptr);
    va_end(arg_ptr);

    return written;
}

i32 string_format_v(char* dest, const char* format, void* va_listp)
{
    if (!dest) return -1;

    char buffer[32000];

    i32 written = vsnprintf(buffer, 32000, format, va_listp);
    buffer[written] = 0;

    hcopy_memory(dest, buffer, written + 1);

    return written;
}