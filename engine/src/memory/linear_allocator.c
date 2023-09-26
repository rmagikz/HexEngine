#include "linear_allocator.h"

#include "core/hmemory.h"
#include "core/logger.h"

void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator)
{
    if (!out_allocator) return;

    out_allocator->total_size = total_size;
    out_allocator->allocated = 0;
    out_allocator->owns_memory = memory == 0;

    if (memory)
    {
        out_allocator->memory = memory;
        return;
    }

    out_allocator->memory = hallocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
}

void linear_allocator_destroy(linear_allocator* allocator)
{
    if (!allocator) return;

    if (allocator->owns_memory && allocator->memory)
    {
        hfree(allocator->memory, allocator->total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }

    allocator->memory = 0;
    allocator->total_size = 0;
    allocator->allocated = 0;
    allocator->owns_memory = FALSE;
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size)
{
    if (!allocator || !allocator->memory)
    {
        HERROR("linear_allocator_allocate: provided allocator not initialized.");
        return 0;
    }

    if (allocator->allocated + size > allocator->total_size)
    {
        u64 remaining = allocator->total_size - allocator->allocated;
        HERROR("linear_allocator_allocate: attempted to allocate %lluB, only %lluB remaining.", size, remaining);
        return 0;
    }

    void* block = allocator->memory + allocator->allocated;

    allocator->allocated += size;

    return block;
}

void linear_allocator_free_all(linear_allocator* allocator)
{
    if (!allocator || !allocator->memory) return;

    allocator->allocated = 0;
    //hzero_memory(allocator->memory, allocator->total_size);
}