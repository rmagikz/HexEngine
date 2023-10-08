#include "renderer_backend.h"

#include "opengl/opengl_backend.h"

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* out_renderer_backend)
{
    if (type == RENDERER_BACKEND_OPENGL)
    {
        out_renderer_backend->initialize = opengl_backend_initialize;
        out_renderer_backend->shutdown = opengl_backend_shutdown;
        out_renderer_backend->resized = opengl_backend_on_resized;
        out_renderer_backend->begin_frame = opengl_backend_begin_frame;
        out_renderer_backend->update_global_state = opengl_backend_update_global_state;
        out_renderer_backend->end_frame = opengl_backend_end_frame;
        out_renderer_backend->draw_geometry = opengl_backend_draw_geometry;
        out_renderer_backend->create_texture = opengl_backend_create_texture;
        out_renderer_backend->destroy_texture = opengl_backend_destroy_texture;
        out_renderer_backend->create_geometry = opengl_backend_create_geometry;
        out_renderer_backend->destroy_geometry = opengl_backend_destroy_geometry;

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
    renderer_backend->update_global_state = 0;
    renderer_backend->resized = 0;
    renderer_backend->draw_geometry = 0;
    renderer_backend->create_texture = 0;
    renderer_backend->destroy_texture = 0;
    renderer_backend->create_geometry = 0;
    renderer_backend->destroy_geometry = 0;
}