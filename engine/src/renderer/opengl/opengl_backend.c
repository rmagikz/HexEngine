#include "opengl_backend.h"

#include "opengl_types.inl"

#include "opengl_buffer.h"

#include "core/logger.h"
#include "core/event.h"
#include "core/application.h"

#include "memory/hmemory.h"

#include "platform/platform.h"

#include "shaders/opengl_material_shader.h"

#include "systems/material_system.h"

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
    glClearColor(0.2f, 0.2f, 0.2, 1.0f);

    opengl_buffer_create(&context, GL_ARRAY_BUFFER, GL_STATIC_DRAW, TRUE, &context.object_vertex_buffer);
    opengl_buffer_create(&context, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, TRUE, &context.object_index_buffer);

    if (!opengl_material_shader_create(&context, &context.material_shader))
    {
        HERROR("Error loading built-in basic_lighting shader.");
        return FALSE;
    }

    for (u32 i = 0; i < OPENGL_MAX_GEOMETRY_COUNT; ++i)
    {
        context.geometries[i].id = INVALID_ID;
    }

    HINFO("OpenGL renderer initialized. Version: %s", glGetString(GL_VERSION));

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
    return platform_swap_buffers();
}

void opengl_backend_create_texture (const u8* pixels, texture* texture)
{
    i32 image_size = texture->width * texture->height * texture->channel_count;

    glGenTextures(1, &texture->handle);
    glActiveTexture((GL_TEXTURE0 - 1) + texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void opengl_backend_destroy_texture (texture* texture)
{
    glDeleteTextures(1, &texture->handle);
    texture->handle = INVALID_ID;
}

b8 opengl_backend_create_geometry (geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices)
{
    if (!vertex_count || !vertices)
    {
        HERROR("opengl_backend_create_geometry requires vertex data and none was supplied. vertex_count=%d, vertices=%p", vertex_count, vertices);
        return FALSE;
    }

    b8 is_reupload = geometry->internal_id != INVALID_ID;
    opengl_geometry_data old_range;

    opengl_geometry_data* internal_data = 0;
    if (is_reupload)
    {
        internal_data = &context.geometries[geometry->internal_id];

        old_range.index_buffer_offset = internal_data->index_buffer_offset;
        old_range.index_count = internal_data->index_count;
        old_range.index_size = internal_data->index_size;
        old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
        old_range.vertex_count = internal_data->vertex_count;
        old_range.vertex_size = internal_data->vertex_size;
    }
    else
    {
        for (u32 i = 0; i < OPENGL_MAX_GEOMETRY_COUNT; ++i)
        {
            if (context.geometries[i].id == INVALID_ID)
            {
                geometry->internal_id = i;
                context.geometries[i].id = i;
                internal_data = &context.geometries[i];
                break;
            }
        }
    }

    if (!internal_data)
    {
        HFATAL("opengl_backend_create_geometry failed to find a free index for a new geometry upload. Adjust config to allow for more.");
        return FALSE;
    }

    internal_data->vertex_buffer_offset = context.geometry_vertex_offset;
    internal_data->vertex_count = vertex_count;
    internal_data->vertex_size = sizeof(vertex_3d) * vertex_count;
    opengl_buffer_add_data(&context, &context.object_vertex_buffer, internal_data->vertex_size, vertices);
    opengl_buffer_add_layout(&context, &context.object_vertex_buffer, 0, 3, GL_FLOAT, FALSE, sizeof(vertex_3d), 0);
    opengl_buffer_add_layout(&context, &context.object_vertex_buffer, 1, 2, GL_FLOAT, FALSE, sizeof(vertex_3d), (const void*)sizeof(vec3));
    context.geometry_vertex_offset += internal_data->vertex_size;

    if (index_count && indices)
    {
        internal_data->index_buffer_offset = context.geometry_index_offset;
        internal_data->index_count = index_count;
        internal_data->index_size = sizeof(u32) * index_count;
        opengl_buffer_add_data(&context, &context.object_index_buffer, internal_data->index_size, indices);
        context.geometry_index_offset += internal_data->index_size;
    }

    if (is_reupload)
    {
        opengl_buffer_free_data();

        if (old_range.index_size > 0)
        {
            opengl_buffer_free_data();
        }
    }

    return TRUE;
}

void opengl_backend_destroy_geometry (geometry* geometry)
{
    if (geometry && geometry->internal_id != INVALID_ID)
    {
        opengl_geometry_data* internal_data = &context.geometries[geometry->internal_id];

        opengl_buffer_free_data();

        if (internal_data->index_size > 0)
        {
            opengl_buffer_free_data();
        }

        hzero_memory(internal_data, sizeof(opengl_geometry_data));
        internal_data->id = INVALID_ID;
    }
}

void opengl_backend_draw_geometry(geometry_render_data data)
{
    if (data.geometry && data.geometry->internal_id == INVALID_ID)
    {
        return;
    }

    opengl_geometry_data* buffer_data = &context.geometries[data.geometry->internal_id];

    opengl_material_shader_use(&context, &context.material_shader);

    opengl_material_shader_set_model(&context, &context.material_shader, data.model);

    material* m = 0;

    if (data.geometry->material)
    {
        m = data.geometry->material;
    }
    else
    {
        m = material_system_get_default();
    }

    opengl_material_shader_apply_material(&context, &context.material_shader, m);

    glBindBuffer(context.object_vertex_buffer.type, context.object_vertex_buffer.handle);

    if (buffer_data->index_count > 0)
    {
        glBindBuffer(context.object_index_buffer.type, context.object_index_buffer.handle);

        glDrawElements(GL_TRIANGLES, buffer_data->index_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, buffer_data->vertex_count);
    }
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