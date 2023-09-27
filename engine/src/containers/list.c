#include "containers/list.h"

#include "memory/hmemory.h"

#include "core/logger.h"

void* _list_create(u64 count, u64 stride)
{
    u64 header_size = LIST_FIELD_LENGTH * sizeof(u64);
    u64 list_size = count * stride;

    u64* new_list = hallocate(header_size + list_size, MEMORY_TAG_LIST);
    hset_memory(new_list, 0, header_size + list_size);

    new_list[LIST_CAPACITY] = count;
    new_list[LIST_COUNT] = 0;
    new_list[LIST_STRIDE] = stride;

    return (void*)(new_list + LIST_FIELD_LENGTH);
}

void _list_destroy(void* list)
{
    u64* header = (u64*)list - LIST_FIELD_LENGTH;
    u64 header_size = LIST_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[LIST_CAPACITY] * header[LIST_STRIDE];
    hfree(header, total_size, MEMORY_TAG_LIST);
}

u64 _list_field_get(void* list, u64 field)
{
    u64* header = (u64*)list - LIST_FIELD_LENGTH;
    return header[field];
}

void _list_field_set(void* list, u64 field, u64 value)
{
    u64* header = (u64*)list - LIST_FIELD_LENGTH;
    header[field] = value;
}

void* _list_resize(void* list)
{
    u64 count = list_count(list);
    u64 stride = list_stride(list);

    // copy from old list into new list
    void* temp = _list_create(list_capacity(list) * LIST_RESIZE_FACTOR, stride);
    hcopy_memory(temp, list, count * stride);

    _list_field_set(temp, LIST_COUNT, count);
    _list_destroy(list);

    return temp;
}

void* _list_push(void* list, const void* value_ptr)
{
    u64 count = list_count(list);
    u64 stride = list_stride(list);

    if (count >= list_capacity(list))
    {
        list = _list_resize(list);
    }

    u64 address = (u64)list;

    // go to end of list
    address += (count * stride);

    hcopy_memory((void*)address, value_ptr, stride);
    _list_field_set(list, LIST_COUNT, count + 1);

    return list;
}

void _list_pop(void* list, void* dest)
{
    u64 count = list_count(list);
    u64 stride = list_stride(list);

    u64 address = (u64)list;

    // go to end of list
    address += ((count - 1) * stride);

    hcopy_memory(dest, (void*)address, stride);
    _list_field_set(list, LIST_COUNT, count - 1);
}

void* _list_insert_at(void* list, u64 index, void* value_ptr)
{
    u64 count = list_count(list);
    u64 stride = list_stride(list);

    if (index >= count)
    {
        HERROR("Index is outside the bounds of this list! count: %d, index: %d", count, index);
        return list;
    }

    if (count >= list_capacity(list))
    {
        list = _list_resize(list);
    }

    u64 address = (u64)list;

    if (index != count - 1)
    {
        hcopy_memory((void*)(address + ((index + 1) * stride)), (void*)(address + (index * stride)), stride * (count - index));
    }

    hcopy_memory((void*)(address + (index * stride)), value_ptr, stride);

    _list_field_set(list, LIST_COUNT, count + 1);

    return list;
}

void* _list_pop_at(void* list, u64 index, void* dest)
{
    u64 count = list_count(list);
    u64 stride = list_stride(list);

    if (index >= count)
    {
        HERROR("Index is outside the bounds of this list! count: %d, index: %d", count, index);
        return list;
    }

    u64 address = (u64)list;
    hcopy_memory(dest, (void*)(address + (index * stride)), stride);

    if (index != count - 1)
    {
        hcopy_memory((void*)(address + (index * stride)), (void*)(address + (index + 1) * stride), stride * (count - index));
    }

    _list_field_set(list, LIST_COUNT, count - 1);
    return list;
}