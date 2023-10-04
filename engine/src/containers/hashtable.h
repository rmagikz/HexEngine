#pragma once

#include "defines.h"

typedef struct hashtable
{
    u64 element_size;
    u64 element_count;
    b8 is_pointer_type;
    void* memory;
} hashtable;

HAPI void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hashtable* out_hashtable);

HAPI void hashtable_destroy(hashtable* hashtable);

HAPI b8 hashtable_set(hashtable* hashtable, const char* name, void* value);

HAPI b8 hashtable_set_ptr(hashtable* hashtable, const char* name, void** value);

HAPI b8 hashtable_get(hashtable* hashtable, const char* name, void* out_value);

HAPI b8 hashtable_get_ptr(hashtable* hashtable, const char* name, void** out_value);

HAPI b8 hashtable_fill(hashtable* hashtable, void* value);