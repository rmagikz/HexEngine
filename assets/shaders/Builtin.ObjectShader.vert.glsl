#version 450
#extension GL_ARB_separate_shader_objects: enable

// ============== INPUT ==============
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 0) uniform global_uniform_object
{
    mat4 projection;
    mat4 view;
} global_ubo;

layout(location = 2) uniform mat4 model;

// ============== OUTPUT ==============
layout(location = 3) out int out_mode;

layout(location = 4) out struct dto
{
    vec2 tex_coord;
} out_dto;

void main()
{
    out_dto.tex_coord = in_texcoord;
    gl_Position = global_ubo.projection * global_ubo.view * model * vec4(in_position, 1.0);
}