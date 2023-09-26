#pragma once

#include "defines.h"

HAPI u64 string_length(const char* str);

HAPI char* string_duplicate(const char* str);

HAPI b8 strings_equal(const char* str0, const char* str1);

HAPI i32 string_format(char* dest, const char* format, ...);

HAPI i32 string_format_v(char* dest, const char* format, void* va_listp);