#pragma once

#include "memory/hmemory.h"

#include "core/application.h"
#include "core/logger.h"
#include "program_types.h"

extern b8 create_program(program* out_program);

// Main entry point.
int main(void)
{
    program program_inst;
    if (!create_program(&program_inst))
    {
        HFATAL("Could not create program.");
        return -1;
    }

    if (!program_inst.render || !program_inst.initialize || !program_inst.update || !program_inst.on_resize)
    {
        HFATAL("The program's function pointers must be assigned.");
        return -2;
    }

    if(!application_initialize(&program_inst))
    {
        HINFO("Application failed to create.");
        return 1;
    }

    if(!application_run())
    {
        HINFO("Application did not shut down correctly.");
        return 2;
    }

    return 0;
}