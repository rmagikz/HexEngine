#include "opengl_backend.h"

#include "opengl_types.inl"

#include "opengl_buffer.h"

#include "core/logger.h"
#include "core/event.h"
#include "core/application.h"

#include "memory/hmemory.h"

#include "platform/platform.h"

#include "shaders/opengl_material_shader.h"

static opengl_context context;
static b8 size_dirty_flag = FALSE;

#ifdef _DEBUG
void GLAPIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name)
{
    context.instance = platform_opengl_context_create();
    if (context.instance == 0)
    {
        HFATAL("OpenGL context creation failed.");
        return FALSE;
    }

    if (glewInit() != GLEW_OK)
    {
        HFATAL("Could not initialize OpenGL extension entry points!");
        return FALSE;
    }

    GLint version_major;
    GLint version_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);

    if (version_major < 3 || version_minor < 3)
    {
        HFATAL("OpenGL version not met. Minimum version required: 3.3");
        return FALSE;
    }

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // TODO: Verify if needed.
    glDebugMessageCallback(opengl_debug_callback, 0);
#endif

    glEnable(GL_CULL_FACE);

    opengl_buffer_create(&context, GL_ARRAY_BUFFER, GL_STATIC_DRAW, TRUE, &context.object_vertex_buffer);
    opengl_buffer_create(&context, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, TRUE, &context.object_index_buffer);

    HINFO("OpenGL renderer initialized. Version: %s", glGetString(GL_VERSION));

    glClearColor(0.2f, 0.2f, 0.2, 1.0f);

    if (!opengl_material_shader_create(&context, &context.material_shader))
    {
        HERROR("Error loading built-in basic_lighting shader.");
        return FALSE;
    }

    vertex_3d verts[4];

    const f32 f = 10.0f;

    verts[0].position.x = -0.5 * f;
    verts[0].position.y = -0.5 * f;
    verts[0].position.z = 0.0 * f;

    verts[1].position.x = 0.5 * f;
    verts[1].position.y = 0.5 * f;
    verts[1].position.z = 0.0 * f;

    verts[2].position.x = -0.5 * f;
    verts[2].position.y = 0.5 * f;
    verts[2].position.z = 0.0 * f;

    verts[3].position.x = 0.5 * f;
    verts[3].position.y = -0.5 * f;
    verts[3].position.z = 0.0 * f;

    verts[0].texcoord.x = 0.0f;
    verts[0].texcoord.y = 0.0f;

    verts[1].texcoord.x = 1.0f;
    verts[1].texcoord.y = 1.0f;

    verts[2].texcoord.x = 0.0f;
    verts[2].texcoord.y = 1.0f;

    verts[3].texcoord.x = 1.0;
    verts[3].texcoord.y = 0.0f;

    u32 indices[6] = {0, 1, 2, 0, 3, 1};

    opengl_buffer_data(&context, &context.object_vertex_buffer, sizeof(verts), verts);
    opengl_buffer_add_layout(&context, &context.object_vertex_buffer, 0, 3, GL_FLOAT, FALSE, sizeof(vertex_3d), 0);
    opengl_buffer_add_layout(&context, &context.object_vertex_buffer, 1, 2, GL_FLOAT, FALSE, sizeof(vertex_3d), (const void*)sizeof(vec3));

    opengl_buffer_data(&context, &context.object_index_buffer, sizeof(indices), indices);

    return TRUE;
}

void opengl_backend_shutdown(renderer_backend* backend)
{
    opengl_material_shader_destroy(&context, &context.material_shader);
    platform_opengl_context_delete();

    HINFO("OpenGL renderer shut down successfully.");
}

void opengl_backend_on_resized(renderer_backend* backend, u16 width, u16 height)
{
    glViewport(0,0,width,height);
    size_dirty_flag = TRUE;
}

b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time)
{
    context.frame_delta_time = delta_time;

    if (size_dirty_flag)
    {
        // Not rendering this frame since a resize just happened.
        size_dirty_flag = FALSE;
        return FALSE;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return TRUE;
}

void opengl_backend_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, i32 mode)
{
    opengl_material_shader_use(&context, &context.material_shader);

    context.material_shader.global_ubo.projection = projection;
    context.material_shader.global_ubo.view = view;

    opengl_material_shader_update_global_state(&context, &context.material_shader, context.frame_delta_time);
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time)
{
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    return platform_swap_buffers();
}

void opengl_backend_update_object(geometry_render_data data)
{
    opengl_material_shader_update_object(&context, &context.material_shader, data);
}

void opengl_backend_create_texture (
    const char* name,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    texture* out_texture)
{
    out_texture->width = width;
    out_texture->height = height;
    out_texture->channel_count = channel_count;

    i32 image_size = width * height * channel_count;

    u32 imagine_format = 0;

    glGenTextures(1, &out_texture->handle);
    glActiveTexture((GL_TEXTURE0 - 1) + out_texture->handle);
    glBindTexture(GL_TEXTURE_2D, out_texture->handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, out_texture->width, out_texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void opengl_backend_destroy_texture (texture* texture)
{
    glDeleteTextures(1, &texture->handle);
    texture->handle = INVALID_ID;
}

#ifdef _DEBUG // OpenGL Debug Callback
const char* source_string(GLenum source)
{
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            return "[OpenGL]";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "[Window System]";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "[Shader]";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "[Third Party]";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "[Application]";
        case GL_DEBUG_SOURCE_OTHER:
            return "[Other]";
        default:
            return "[Unknown]";
    };
}
const char* type_string(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            return "Type Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "Deprecated Behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "Performance";
        case GL_DEBUG_TYPE_MARKER:
            return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "Push Group";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "Pop Group";
        case GL_DEBUG_TYPE_OTHER:
            return "Other";
        default:
            return "Unknown";
    };
}

void GLAPIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            HERROR("%s Severity: %s, Type: %s, Message: %s", source_string(source), "High", type_string(type), message);
            return;
        case GL_DEBUG_SEVERITY_MEDIUM:
            HWARN("%s Severity: %s, Type: %s, Message: %s", source_string(source), "Medium", type_string(type), message);
            return;
        case GL_DEBUG_SEVERITY_LOW:
            HINFO("%s Severity: %s, Type: %s, Message: %s", source_string(source), "Info", type_string(type), message);
            return;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            HINFO("%s Severity: %s, Type: %s, Message: %s", source_string(source), "Notification", type_string(type), message);
            return;
        default:
            HERROR("%s Severity: %s, Type: %s, Message: %s", source_string(source), "UNKNOWN", type_string(type), message);
            return;
    }
}

#endif