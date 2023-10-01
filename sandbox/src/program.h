#pragma once

#include <defines.h>
#include <program_types.h>
#include <math/math_types.h>

typedef struct program_state
{
    f32 delta_time;
    mat4 view;
    vec3 camera_position;
    vec3 camera_euler;
    b8 camera_view_dirty;
} program_state;

b8 program_initialize (struct program* program_inst);

b8 program_update (struct program* program_inst, f32 delta_time);

b8 program_render (struct program* program_inst, f32 delta_time);

void program_on_resize (struct program* program_inst, u32 width, u32 height);