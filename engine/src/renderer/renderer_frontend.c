#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "memory/hmemory.h"

#include "math/hmath.h"

#include "core/logger.h"

#include "resources/resource_types.h"

#include "systems/texture_system.h"
#include "systems/material_system.h"

// TODO: Temporary
#include "core/hstring.h"
#include "core/event.h"
// TODO: End temporary

typedef struct renderer_system_state
{
    renderer_backend backend;
    mat4 projection;
    mat4 view;
    f32 near_clip;
    f32 far_clip;
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

    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), 640/480.0f, state_ptr->near_clip, state_ptr->far_clip);

    state_ptr->view = mat4_translation((vec3){0, 0, -20.0f});
    state_ptr->view = mat4_inverse(state_ptr->view);

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
        state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), width/(f32)height, state_ptr->near_clip, state_ptr->far_clip);
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
        state_ptr->backend.update_global_state(state_ptr->projection, state_ptr->view, vec3_zero(), vec4_one(), 0);

        u32 count = packet->geometry_count;
        for (u32 i = 0; i < count; ++i)
        {
            state_ptr->backend.draw_geometry(packet->geometries[i]);
        }

        if (!renderer_end_frame(packet->delta_time))
        {
            HFATAL("renderer_end_frame failed. Shutting down.");
            return FALSE;
        }
    }
        
    return TRUE;
}

void renderer_set_view(mat4 view)
{
    state_ptr->view = view;
}

void renderer_create_texture (const u8* pixels, struct texture* texture)
{
    state_ptr->backend.create_texture(pixels, texture);
}

void renderer_destroy_texture (struct texture* texture)
{
    state_ptr->backend.destroy_texture(texture);
}

b8 renderer_create_geometry(geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices)
{
    return state_ptr->backend.create_geometry(geometry, vertex_count, vertices, index_count, indices);
}

void renderer_destroy_geometry(geometry* geometry)
{
    state_ptr->backend.destroy_geometry(geometry);
}