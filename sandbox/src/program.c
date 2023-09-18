#include "program.h"

#include <core/logger.h>


b8 program_initialize (struct program* program_inst)
{
    HDEBUG("program_initialize called successfully.");
    return TRUE;
}

b8 program_update (struct program* program_inst, f32 delta_time)
{
    return TRUE;
}

b8 program_render (struct program* program_inst, f32 delta_time)
{
    return TRUE;
}

void program_on_resize (struct program* program_inst, u32 width, u32 height)
{

}