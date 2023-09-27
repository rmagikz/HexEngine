#include <entry.h>

#include "program.h"

b8 create_program(program* out_program)
{
    out_program->app_config.start_x = 100;
    out_program->app_config.start_y = 100;

    out_program->app_config.start_width = 640;
    out_program->app_config.start_height = 480;

    out_program->app_config.name = "Hex Engine Sandbox";

    out_program->initialize = program_initialize;
    out_program->update = program_update;
    out_program->render = program_render;
    out_program->on_resize = program_on_resize;

    out_program->program_state = hallocate(sizeof(program_state), MEMORY_TAG_PROGRAM);
    out_program->application_state = 0;

    return TRUE;
}