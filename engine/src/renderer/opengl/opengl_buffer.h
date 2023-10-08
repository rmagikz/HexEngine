#pragma once

#include "opengl_types.inl"

void opengl_buffer_create(opengl_context* context, GLenum type, GLenum usage, b8 bind_on_create, opengl_buffer* out_buffer);

void opengl_buffer_add_data(opengl_context* context, opengl_buffer* buffer, u64 size, const void* data);

void opengl_buffer_free_data();

void opengl_buffer_add_layout(opengl_context* context, opengl_buffer* buffer, u32 index, u32 count, GLenum type, b8 normalized, u32 stride, const void* pointer);

void opengl_buffer_destroy(opengl_context* context, opengl_buffer* buffer);

void opengl_buffer_bind(opengl_context* context, opengl_buffer* buffer);