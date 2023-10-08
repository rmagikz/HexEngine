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

void opengl_backend_create_texture(const u8* pixels, texture* texture);

void opengl_backend_destroy_texture(texture* texture);

b8 opengl_backend_create_geometry (geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices);

void opengl_backend_destroy_geometry (geometry* geometry);

void opengl_backend_draw_geometry(geometry_render_data data);