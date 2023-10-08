#pragma once

#include "defines.h"

#include "math/math_types.h"
#include "renderer/renderer_types.inl"

#define GLEW_STATIC

#include <GL/glew.h>

typedef struct opengl_buffer
{
    u32 handle;
    GLenum type;
    GLenum usage;
} opengl_buffer;

typedef struct opengl_shader_stage
{
    u32 shader_handle;
    u32 source_length;
    char* shader_source;
} opengl_shader_stage;

typedef struct opengl_material_shader
{
    u32 program_handle;
    opengl_shader_stage stages[2];

    global_uniform_object global_ubo;
    opengl_buffer global_uniform_buffer;

    opengl_buffer object_uniform_buffer;
} opengl_material_shader;

#define OPENGL_MAX_GEOMETRY_COUNT 4096
typedef struct opengl_geometry_data
{
    u32 id;
    u32 vertex_count;
    u32 vertex_size;
    u32 vertex_buffer_offset;
    u32 index_count;
    u32 index_size;
    u32 index_buffer_offset;
} opengl_geometry_data;

typedef struct opengl_context
{
    void* instance;

    f32 frame_delta_time;

    opengl_buffer object_vertex_buffer;

    opengl_buffer object_index_buffer;

    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

    opengl_material_shader material_shader;

    opengl_geometry_data geometries[OPENGL_MAX_GEOMETRY_COUNT];
} opengl_context;