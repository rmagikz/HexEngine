#include "opengl_object_shader.h"

#include "core/logger.h"

#include "renderer/opengl/opengl_shader_utils.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 opengl_object_shader_create(opengl_context* context, opengl_object_shader* out_shader)
{
    char stage_type_strs[2][5] = {"vert", "frag"};
    u32 stage_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

    out_shader->program_handle = glCreateProgram();

    for (u32 i = 0; i < 2; ++i)
    {
        if (create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, out_shader->stages))
        {
            glAttachShader(out_shader->program_handle, out_shader->stages[i].shader_handle);
        }
        else
        {
            HERROR("Unable to create %s shader module for '%s'.", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return FALSE;
        }
    }

    glLinkProgram(out_shader->program_handle);
    glValidateProgram(out_shader->program_handle);

    for (u32 i = 0; i < 2; ++i)
    {
        glDeleteShader(out_shader->stages[i].shader_handle);
    }

    return TRUE;
}

void opengl_object_shader_destroy(opengl_context* context, struct opengl_object_shader* shader)
{
    
}

void opengl_object_shader_use(opengl_context* context, struct opengl_object_shader* shader)
{

}