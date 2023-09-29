#pragma once

#include "renderer/opengl/opengl_types.inl"

b8 create_shader_module(opengl_context* context, const char* name, const char* type_str, u32 shader_stage_flag, u32 stage_index, opengl_shader_stage* shader_stage);