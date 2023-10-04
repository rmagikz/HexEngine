#include "hashtable.h"

#include "memory/hmemory.h"
#include "core/logger.h"

u64 hash_name(const char* name, u32 element_count)
{
    static const u64 multiplier = 97;

    unsigned const char* us;
    u64 hash = 0;

    for (us = (unsigned const char*)name; *us; us++)
    {
        hash = hash * multiplier + *us;
    }

    hash %= element_count;

    return hash;
}

void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hashtable* out_hashtable)
{
    if (!memory || !out_hashtable)
    {
        HERROR("hashtable_create failed! Pointer to memory and out_hashtable are required.");
        return;
    }

    if (!element_count || !element_size)
    {
        HERROR("element_size and element_count must be a positive non-zero value.");
        return;
    }

    out_hashtable->memory = memory;
    out_hashtable->element_count = element_count;
    out_hashtable->element_size = element_size;
    out_hashtable->is_pointer_type = is_pointer_type;
    hzero_memory(out_hashtable->memory, element_count * element_size);
}

void hashtable_destroy(hashtable* table)
{
    if (table)
    {
        hzero_memory(table, sizeof(hashtable));
    }
}

b8 hashtable_set(hashtable* table, const char* name, void* value)
{
    if (!table || !name || !value)
    {
        HERROR("hashtable_set requires hashtable, name and value to exist.");
        return FALSE;
    }

    if (table->is_pointer_type)
    {
        HERROR("hashtable_set should not be used with tables holding pointer types. Use hashtable_set_ptr instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    hcopy_memory(table->memory + (table->element_size * hash), value, table->element_size);
    return TRUE;
}

b8 hashtable_set_ptr(hashtable* table, const char* name, void** value)
{
    if (!table || !name)
    {
        HERROR("hashtable_set_ptr requires hashtable and name to exist.");
        return FALSE;
    }

    if (!table->is_pointer_type)
    {
        HERROR("hashtable_set_ptr should not be used with tables holding non-pointer types. Use hashtable_set instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    ((void**)table->memory)[hash] = value ? *value : 0;
    return TRUE;
}

b8 hashtable_get(hashtable* table, const char* name, void* out_value)
{
    if (!table || !name || !out_value)
    {
        HERROR("hashtable_get requires hashtable, name and value to exist.");
        return FALSE;
    }

    if (table->is_pointer_type)
    {
        HERROR("hashtable_get should not be used with tables holding pointer types. Use hashtable_get_ptr instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    hcopy_memory(out_value, table->memory + (table->element_size * hash), table->element_size);
    return TRUE;
}

b8 hashtable_get_ptr(hashtable* table, const char* name, void** out_value)
{
    if (!table || !name || !out_value)
    {
        HERROR("hashtable_get_ptr requires hashtable, name and value to exist.");
        return FALSE;
    }

    if (!table->is_pointer_type)
    {
        HERROR("hashtable_get_ptr should not be used with tables holding non-pointer types. Use hashtable_get instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    *out_value = ((void**)table->memory)[hash];
    return *out_value != 0;
}

b8 hashtable_fill(hashtable* table, void* value)
{
    if (!table || !value)
    {
        HERROR("hashtable_fill requires hashtable and value to exist.");
        return FALSE;
    }

    if (table->is_pointer_type)
    {
        HERROR("hashtable_fill should not be used with tables holding non-pointer types.");
        return FALSE;
    }

    for (u32 i = 0; i < table->element_count; ++i)
    {
        hcopy_memory(table->memory + (table->element_size * i), value, table->element_size);
    }

    return TRUE;
}