#include "opengl_material_shader.h"

#include "core/logger.h"

#include "renderer/opengl/opengl_shader_utils.h"

#include "renderer/opengl/opengl_buffer.h"

#include "math/hmath.h"

#include "systems/texture_system.h"

#define BUILTIN_SHADER_NAME_MATERIAL "Builtin.MaterialShader"

b8 opengl_material_shader_create(opengl_context* context, opengl_material_shader* out_shader)
{
    char stage_type_strs[2][5] = {"vert", "frag"};
    u32 stage_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

    out_shader->program_handle = glCreateProgram();

    for (u32 i = 0; i < 2; ++i)
    {
        if (create_shader_module(context, BUILTIN_SHADER_NAME_MATERIAL, stage_type_strs[i], stage_types[i], i, out_shader->stages))
        {
            glAttachShader(out_shader->program_handle, out_shader->stages[i].shader_handle);
        }
        else
        {
            HERROR("Unable to create %s shader module for '%s'.", stage_type_strs[i], BUILTIN_SHADER_NAME_MATERIAL);
            return FALSE;
        }
    }

    glLinkProgram(out_shader->program_handle);
    glValidateProgram(out_shader->program_handle);

    for (u32 i = 0; i < 2; ++i)
    {
        glDeleteShader(out_shader->stages[i].shader_handle);
    }

    opengl_buffer_create(context, GL_UNIFORM_BUFFER, GL_STATIC_DRAW, TRUE, &out_shader->global_uniform_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, out_shader->global_uniform_buffer.handle);

    opengl_buffer_create(context, GL_UNIFORM_BUFFER, GL_STATIC_DRAW, TRUE, &out_shader->object_uniform_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, out_shader->object_uniform_buffer.handle);

    return TRUE;
}

void opengl_material_shader_destroy(opengl_context* context, struct opengl_material_shader* shader)
{
    opengl_buffer_destroy(context, &shader->global_uniform_buffer);
    opengl_buffer_destroy(context, &shader->object_uniform_buffer);
    glDeleteProgram(shader->program_handle);
    shader->program_handle = 0;
}

void opengl_material_shader_use(opengl_context* context, struct opengl_material_shader* shader)
{
    glUseProgram(shader->program_handle);
}

void opengl_material_shader_update_global_state(opengl_context* context, struct opengl_material_shader* shader, f32 delta_time)
{
    opengl_buffer_add_data(context, &shader->global_uniform_buffer, sizeof(global_uniform_object), &shader->global_ubo);
}

void opengl_material_shader_set_model(opengl_context* context, struct opengl_material_shader* shader, mat4 model)
{
    if (context && shader)
    {
        glUniformMatrix4fv(2, 1, FALSE, (const GLfloat*)&model);
    }
}

void opengl_material_shader_apply_material(opengl_context* context, struct opengl_material_shader* shader, material* material)
{
    if (context && shader)
    {
        glUniform1i(4, material->diffuse->handle - 1);

        material_uniform_object obo;

        obo.diffuse_color = material->diffuse_color;

        opengl_buffer_add_data(context, &shader->object_uniform_buffer, sizeof(material_uniform_object), &obo);
    }
}