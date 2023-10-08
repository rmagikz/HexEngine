#pragma once

#include "math/math_types.h"

#define TEXTURE_NAME_MAX_LENGTH 512
typedef struct texture
{
    u32 handle;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    char name[TEXTURE_NAME_MAX_LENGTH];
    void* internal_data;
} texture;

typedef enum texture_use
{
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USEMAP_DIFFUSE = 0x01
} texture_use;

#define MATERIAL_NAME_MAX_LENGTH 256
typedef struct material
{
    u32 handle;
    u32 internal_id;
    char name[MATERIAL_NAME_MAX_LENGTH];
    vec4 diffuse_color;
    texture* diffuse;
} material;

#define GEOMETRY_NAME_MAX_LENGTH 256
typedef struct geometry
{
    u32 handle;
    u32 internal_id;
    char name[GEOMETRY_NAME_MAX_LENGTH];
    material* material;
} geometry;