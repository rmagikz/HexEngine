#pragma once

#include "defines.h"

#define GLEW_STATIC

#include <GL/glew.h>

typedef struct opengl_shader_stage
{
    u32 shader_handle;
    u32 source_length;
    char* shader_source;
} opengl_shader_stage;

typedef struct opengl_object_shader
{
    u32 program_handle;
    opengl_shader_stage stages[2];
} opengl_object_shader;

typedef struct opengl_context
{
    void* instance;

    opengl_object_shader object_shader;
} opengl_context;