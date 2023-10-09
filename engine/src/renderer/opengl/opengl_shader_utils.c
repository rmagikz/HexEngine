#include "opengl_shader_utils.h"

#include "core/logger.h"
#include "core/hstring.h"
#include "memory/hmemory.h"

#include "systems/resource_system.h"

b8 validate_shader_module(opengl_context* context, const char* type_str, opengl_shader_stage* shader_stage)
{
    int result;
    glGetShaderiv(shader_stage->shader_handle, GL_COMPILE_STATUS, &result);

    if (result == FALSE)
    {
        int length;
        glGetShaderiv(shader_stage->shader_handle, GL_INFO_LOG_LENGTH, &length);
        
        char* message = (char*)_alloca(length * sizeof(char));
        glGetShaderInfoLog(shader_stage->shader_handle, length, &length, message);

        HERROR("[OpenGL Shader] Error (%s shader) %s", type_str, message);

        return FALSE;
    }

    HDEBUG("[OpenGL Shader] %s shader compiled successfully.", type_str);

    return TRUE;
}

b8 create_shader_module(opengl_context* context, const char* name, const char* type_str, u32 shader_stage_flag, u32 stage_index, opengl_shader_stage* shader_stages)
{
    char file_name[512];
    string_format(file_name, "shaders/%s.%s.glsl", name, type_str);

    resource text_resource;
    if (!resource_system_load(file_name, RESOURCE_TYPE_TEXT, &text_resource))
    {
        HERROR("Unable to read shader module: %s.", file_name);
        return FALSE;
    }

    shader_stages[stage_index].shader_source = text_resource.data;
    shader_stages[stage_index].source_length = text_resource.data_size;

    shader_stages[stage_index].shader_handle = glCreateShader(shader_stage_flag);

    glShaderSource(shader_stages[stage_index].shader_handle, 1, (const char* const*)&text_resource.data, 0);
    glCompileShader(shader_stages[stage_index].shader_handle);

    validate_shader_module(context, type_str, &shader_stages[stage_index]);

    resource_system_unload(&text_resource);

    return TRUE;
}