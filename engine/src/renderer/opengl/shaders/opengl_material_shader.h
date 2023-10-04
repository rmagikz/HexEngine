#pragma once

#include "renderer/opengl/opengl_types.inl"
#include "renderer/renderer_types.inl"

b8 opengl_material_shader_create(opengl_context* context, opengl_material_shader* out_shader);

void opengl_material_shader_destroy(opengl_context* context, struct opengl_material_shader* shader);

void opengl_material_shader_use(opengl_context* context, struct opengl_material_shader* shader);

void opengl_material_shader_update_global_state(opengl_context* context, struct opengl_material_shader* shader, f32 delta_time);

void opengl_material_shader_update_object(opengl_context* context, struct opengl_material_shader* shader, geometry_render_data data);