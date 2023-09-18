#pragma once

#include <defines.h>
#include <program_types.h>

typedef struct program_state
{
    f32 delta_time;
} program_state;

b8 program_initialize (struct program* program_inst);

b8 program_update (struct program* program_inst, f32 delta_time);

b8 program_render (struct program* program_inst, f32 delta_time);

void program_on_resize (struct program* program_inst, u32 width, u32 height);