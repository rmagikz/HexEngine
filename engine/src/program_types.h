#pragma once

#include "core/application.h"

typedef struct program 
{
    application_config app_config;

    b8 (*initialize)(struct program* program_inst);

    b8 (*update)(struct program* program_inst, f32 delta_time);
    
    b8 (*render)(struct program* program_inst, f32 delta_time);

    void (*on_resize)(struct program* program_inst, u32 width, u32 height);

    void* state;
} program;