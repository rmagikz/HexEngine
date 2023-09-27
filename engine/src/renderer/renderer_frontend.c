#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "memory/hmemory.h"

#include "core/logger.h"

typedef struct renderer_system_state
{
    renderer_backend backend;
} renderer_system_state;

static renderer_system_state* state_ptr;

b8 renderer_initialize(u64* memory_requirement, void* state, const char* application_name)
{
    *memory_requirement = sizeof(renderer_system_state);

    if (!state) return FALSE;

    state_ptr = state;

    renderer_backend_create(RENDERER_BACKEND_OPENGL, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name))
    {
        HFATAL("Renderer backend failed to initialzie. Shutting down.");
        return FALSE;
    }

    return TRUE;
}

void renderer_shutdown(void* state)
{
    if (state_ptr)
    {
        state_ptr->backend.shutdown(&state_ptr->backend);
    }

    state_ptr = 0;

    HINFO("Renderer subsystem shut down successfully.");
}

void renderer_on_resized(u16 width, u16 height)
{
    if (state_ptr)
    {
        state_ptr->backend.resized(&state_ptr->backend, width, height);
        return;
    }
}

b8 renderer_begin_frame(f32 delta_time)
{
    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time)
{
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return result;
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