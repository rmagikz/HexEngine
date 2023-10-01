#include "opengl_buffer.h"

void opengl_buffer_create(opengl_context* context, GLenum type, GLenum usage, b8 bind_on_create, opengl_buffer* out_buffer)
{
    glGenBuffers(1, &out_buffer->handle);
    out_buffer->type = type;
    out_buffer->usage = usage;

    if (bind_on_create)
    {
        glBindBuffer(out_buffer->type, out_buffer->handle);
    }
}

void opengl_buffer_data(opengl_context* context, opengl_buffer* buffer, u64 size, const void* data)
{
    glBindBuffer(buffer->type, buffer->handle);
    glBufferData(buffer->type, size, data, buffer->usage);
}

void opengl_buffer_add_layout(opengl_context* context, opengl_buffer* buffer, u32 index, u32 count, GLenum type, b8 normalized, u32 stride, const void* pointer)
{
    glBindBuffer(buffer->type, buffer->handle);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, count, type, normalized, stride, pointer);
}

void opengl_buffer_destroy(opengl_context* context, opengl_buffer* buffer)
{
    glDeleteBuffers(1, &buffer->handle);
    buffer->handle = 0;
    buffer->type = 0;
    buffer->usage = 0;
}

void opengl_buffer_bind(opengl_context* context, opengl_buffer* buffer)
{
    glBindBuffer(buffer->type, buffer->handle);
}