#pragma once

#include "defines.h"

typedef enum memory_tag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LIST,
    MEMORY_TAG_DICT,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_PROGRAM,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_LINEAR_ALLOCATOR,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

HAPI b8 memory_initialize();

HAPI void memory_shutdown();

HAPI void* hallocate(u64 size, memory_tag tag);

HAPI void hfree(void* block, u64 size, memory_tag tag);

HAPI void* hzero_memory(void* block, u64 size);

HAPI void* hcopy_memory(void* dest, const void* src, u64 size);

HAPI void* hset_memory(void* block, i32 value, u64 size);

#ifdef _DEBUG

HAPI char* get_memory_usage_str();

#endif