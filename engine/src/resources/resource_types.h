#pragma once

#include "math/math_types.h"

typedef enum resource_type
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_CUSTOM
} resource_type;

typedef struct resource
{
    u32 loader_id;
    const char* name;
    char* full_path;
    u64 data_size;
    void* data;
} resource;

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

typedef struct image_resource_data
{
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;

#define MATERIAL_NAME_MAX_LENGTH 256
typedef struct material
{
    u32 handle;
    u32 internal_id;
    char name[MATERIAL_NAME_MAX_LENGTH];
    vec4 diffuse_color;
    texture* diffuse;
} material;

typedef struct material_resource_data
{
    char name[MATERIAL_NAME_MAX_LENGTH];
    b8 auto_release;
    vec4 diffuse_color;
    char diffuse_name[TEXTURE_NAME_MAX_LENGTH];
} material_resource_data;

#define GEOMETRY_NAME_MAX_LENGTH 256
typedef struct geometry
{
    u32 handle;
    u32 internal_id;
    char name[GEOMETRY_NAME_MAX_LENGTH];
    material* material;
} geometry;