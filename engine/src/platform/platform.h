#pragma once

#include "defines.h"

typedef struct platform_system_state
{
    void* internal_state;
} platform_system_state;

b8 platform_initialize(u64* memory_requirement, void* platform_state, const char* application_name, i32 x, i32 y, i32 width, i32 height);

void platform_shutdown(void* platform_state);

b8 platform_pump_messages(void* platform_state);

void* platform_opengl_context_create();
void platform_opengl_context_delete();
b8 platform_swap_buffers();

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* src, u64 size);
void* platform_set_memory(void* block, i32 value, u64 size);

void platform_console_write(const char* message, u8 color);
void platform_console_write_error(const char* message, u8 color);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);