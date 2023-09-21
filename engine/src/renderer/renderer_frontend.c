#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/hmemory.h"

static renderer_backend* backend = 0;

b8 renderer_initialize(const char* application_name, struct platform_state* platform_state)
{
    backend = hallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    renderer_backend_create(RENDERER_BACKEND_OPENGL, platform_state, backend);
    backend->frame_number = 0;

    if (!backend->initialize(backend, application_name, platform_state))
    {
        HFATAL("Renderer backend failed to initialzie. Shutting down.");
        return FALSE;
    }

    return TRUE;
}

void renderer_shutdown()
{
    backend->shutdown(backend);
    renderer_backend_destroy(backend);
    hfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

void renderer_on_resized(u16 width, u16 height)
{

}

b8 renderer_begin_frame(f32 delta_time)
{
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time)
{
    backend->frame_number++;
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_draw_frame(render_packet* packet)
{
    if (renderer_begin_frame(packet->delta_time))
    {
        if (!renderer_end_frame(packet->delta_time))
        {
            HFATAL("renderer_end_frame failed. Shutting down.");
            return FALSE;
        }
    }
        
    return TRUE;
}