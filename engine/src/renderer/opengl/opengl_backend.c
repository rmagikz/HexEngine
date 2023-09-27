#include "opengl_backend.h"

#include "opengl_types.inl"

#include "core/logger.h"
#include "core/event.h"
#include "core/application.h"

#include "platform/platform.h"

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

    glClearColor(0.2f, 0.2f, 0.2, 1.0f);

    HINFO("OpenGL renderer initialized. Version: %s", glGetString(GL_VERSION));
    return TRUE;
}

void opengl_backend_shutdown(renderer_backend* backend)
{
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
    if (size_dirty_flag)
    {
        // Not rendering this frame since a resize just happened.
        size_dirty_flag = FALSE;
        return FALSE;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return platform_swap_buffers();
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time)
{
    return TRUE;
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
    HERROR("%s OpenGL Error %d: Type: %s, Message: %s", source_string(source), type, type_string(type), message);
}
#endif