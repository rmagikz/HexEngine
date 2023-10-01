#pragma once

#include "renderer/opengl/opengl_types.inl"
#include "renderer/renderer_types.inl"

b8 opengl_object_shader_create(opengl_context* context, texture* default_diffuse, opengl_object_shader* out_shader);

void opengl_object_shader_destroy(opengl_context* context, struct opengl_object_shader* shader);

void opengl_object_shader_use(opengl_context* context, struct opengl_object_shader* shader);

void opengl_object_shader_update_global_state(opengl_context* context, struct opengl_object_shader* shader, f32 delta_time);

void opengl_object_shader_update_object(opengl_context* context, struct opengl_object_shader* shader, geometry_render_data data);