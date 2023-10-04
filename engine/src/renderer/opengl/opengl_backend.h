#pragma once

#include "renderer/renderer_backend.h"
#include "resources/resource_types.h"

struct platform_state;

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name);

void opengl_backend_shutdown(renderer_backend* backend);

void opengl_backend_on_resized(renderer_backend* backend, u16 width, u16 height);

b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time);

void opengl_backend_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, i32 mode);

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time);

void opengl_backend_update_object(geometry_render_data data);

void opengl_backend_create_texture (
    const char* name,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    texture* out_texture);

void opengl_backend_destroy_texture (texture* texture);