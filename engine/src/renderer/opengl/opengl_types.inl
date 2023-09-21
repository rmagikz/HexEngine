#pragma once

#include "defines.h"

#define GLEW_STATIC

#include <GL/glew.h>

typedef struct opengl_context
{
    void* instance;
} opengl_context;