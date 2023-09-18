#pragma once

#include "defines.h"

// forward declared to avoid circular dependecy
struct program;

typedef struct application_config
{
    i16 start_x;
    i16 start_y;

    i16 start_width;
    i16 start_height;

    char* name;
} application_config;

HAPI b8 application_initialize(struct program* program_inst);

HAPI b8 application_run();