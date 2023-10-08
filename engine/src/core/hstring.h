#pragma once

#include "defines.h"
#include "math/math_types.h"

HAPI u64 string_length(const char* str);

HAPI char* string_duplicate(const char* str);

HAPI b8 strings_equal(const char* str0, const char* str1);

HAPI b8 strings_equali(const char* str0, const char* str1);

HAPI i32 string_format(char* dest, const char* format, ...);

HAPI i32 string_format_v(char* dest, const char* format, void* va_listp);

HAPI char* string_empty(char* str);

HAPI char* string_copy(char* dest, const char* source);

HAPI char* string_ncopy(char* dest, const char* source, i64 length);

HAPI char* string_trim(char* str);

HAPI void string_mid(char* dest, const char* source, i32 start, i32 length);

HAPI i32 string_index_of(char* str, char c);

HAPI b8 string_to_vec4(char* str, vec4* out_vector);

HAPI b8 string_to_vec3(char* str, vec3* out_vector);

HAPI b8 string_to_vec2(char* str, vec2* out_vector);

HAPI b8 string_to_f32(char* str, f32* f);

HAPI b8 string_to_f64(char* str, f64* f);

HAPI b8 string_to_i8(char* str, i8* i);

HAPI b8 string_to_i16(char* str, i16* i);

HAPI b8 string_to_i32(char* str, i32* i);

HAPI b8 string_to_i64(char* str, i64* i);

HAPI b8 string_to_u8(char* str, u8* u);

HAPI b8 string_to_u16(char* str, u16* u);

HAPI b8 string_to_u32(char* str, u32* u);

HAPI b8 string_to_u64(char* str, u64* u);

HAPI b8 string_to_bool(char* str, b8* b);