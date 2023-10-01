#version 450
#extension GL_ARB_separate_shader_objects: enable



layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform local_uniform_object
{
    vec4 diffuse_color;
} object_ubo;

layout(location = 4) in struct dto
{
    vec2 tex_coord;
} in_dto;

layout(location = 4) uniform sampler2D diffuse_sampler;

void main()
{
    out_color = object_ubo.diffuse_color * texture(diffuse_sampler, in_dto.tex_coord);
}