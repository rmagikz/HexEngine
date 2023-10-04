#pragma once

#include "math/math_types.h"

typedef struct texture
{
    u32 handle;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    void* internal_data;
} texture;