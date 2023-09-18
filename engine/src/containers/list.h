#pragma once

#include "defines.h"

/*
Layout
u64 capacity = number of elements that can be held
u64 count = number of elements currently contained
u64 stride = size of each element
void* elements
*/

enum
{
    LIST_CAPACITY,
    LIST_COUNT,
    LIST_STRIDE,
    LIST_FIELD_LENGTH
};

HAPI void* _list_create(u64 count, u64 stride);
HAPI void _list_destroy(void* list);

HAPI u64 _list_field_get(void* list, u64 field);
HAPI void _list_field_set(void* list, u64 field, u64 value);

HAPI void* _list_resize(void* list);

HAPI void* _list_push(void* list, const void* value_ptr);
HAPI void _list_pop(void* list, void* dest);

HAPI void* _list_insert_at(void* list, u64 index, void* value_ptr);
HAPI void* _list_pop_at(void* list, u64 index, void* dest);

#define LIST_DEFAULT_CAPACITY 1
#define LIST_RESIZE_FACTOR 2

#define list_create(type) \
    _list_create(LIST_DEFAULT_CAPACITY, sizeof(type))

#define list_reserve(type, capacity) \
    _list_create(capacity, sizeof(type))

#define list_destroy(list) _list_destroy(list)

#define list_push(list, value)              \
    {                                       \
        typeof(value) temp = value;         \
        list = _list_push(list, &temp);     \
    }

#define list_pop(list, value_ptr) \
    _list_pop(list, value_ptr)

#define list_insert_at(list, index, value_ptr)          \
    {                                                   \
        typeof(value) temp = value;                     \
        array = _list_insert_at(list, index, &temp);    \
    }

#define list_pop_at(list, index, value_ptr) \
    _list_pop_at(list, index, value_ptr)

#define list_clear(list) \
    _list_field_set(list, LIST_COUNT, 0)

#define list_capacity(list) \
    _list_field_get(list, LIST_CAPACITY)

#define list_count(list) \
_list_field_get(list, LIST_COUNT)

#define list_stride(list) \
_list_field_get(list, LIST_STRIDE)

#define list_count_set(array, value) \
    _list_field_set(list, LIST_CAPACITY, value)