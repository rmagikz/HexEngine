#include "renderer_backend.h"

#include "opengl/opengl_backend.h"

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* platform_state, renderer_backend* out_renderer_backend)
{
    out_renderer_backend->platform_state = platform_state;
    
    if (type == RENDERER_BACKEND_OPENGL)
    {
        out_renderer_backend->initialize = opengl_backend_initialize;
        out_renderer_backend->shutdown = opengl_backend_shutdown;
        out_renderer_backend->resized = opengl_backend_on_resized;
        out_renderer_backend->begin_frame = opengl_backend_begin_frame;
        out_renderer_backend->end_frame = opengl_backend_end_frame;

        return TRUE;
    }

    return FALSE;
}

void renderer_backend_destroy(renderer_backend* renderer_backend)
{
    renderer_backend->initialize = 0;
    renderer_backend->shutdown = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->end_frame = 0;
    renderer_backend->resized = 0;
}