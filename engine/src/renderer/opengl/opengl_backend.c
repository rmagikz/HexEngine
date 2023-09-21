#include "opengl_backend.h"

#include "opengl_types.inl"

#include <core/logger.h>

#include <platform/platform.h>

static opengl_context context;

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name, platform_state* platform_state)
{
    context.instance = platform_opengl_context_create(platform_state);
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

    HINFO("OpenGL renderer context initialized.");
    return TRUE;
}

void opengl_backend_shutdown(renderer_backend* backend)
{
    platform_opengl_context_delete(context.instance);
}

void opengl_backend_on_resized(renderer_backend* backend, u16 width, u16 height)
{

}

b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time)
{
    return TRUE;
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time)
{
    return TRUE;
}