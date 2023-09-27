#include "program.h"

#include <memory/hmemory.h>

#include <core/logger.h>
#include <core/input.h>


b8 program_initialize (struct program* program_inst)
{
    return TRUE;
}

b8 program_update (struct program* program_inst, f32 delta_time)
{
    static u64 alloc_count = 0;
    u64 previous_alloc_count = alloc_count;

    alloc_count = get_total_memory_allocations();

    if (input_is_key_up('A') && input_was_key_down('A'))
    {
        HDEBUG("Total allocations: %llu (%llu this frame)", alloc_count, alloc_count - previous_alloc_count);
    }

    return TRUE;
}

b8 program_render (struct program* program_inst, f32 delta_time)
{
    return TRUE;
}

void program_on_resize (struct program* program_inst, u32 width, u32 height)
{

}