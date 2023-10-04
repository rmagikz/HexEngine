#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "memory/hmemory.h"

#include "math/hmath.h"

#include "core/logger.h"

#include "resources/resource_types.h"

#include "systems/texture_system.h"

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
    
    texture* test_diffuse;
} renderer_system_state;

static renderer_system_state* state_ptr;

// TODO: Temporary
b8 event_on_debug_event(u16 code, void* sender, void* listener_inst, event_context data)
{
    const char* names[3] = 
    {
        "armor",
        "items",
        "weapons"
    };

    static i8 choice = 2;

    const char* old_name = names[choice];

    choice++;
    choice %= 3;

    state_ptr->test_diffuse = texture_system_acquire(names[choice], FALSE);
    HINFO("%i", state_ptr->test_diffuse->handle);

    texture_system_release(old_name);

    return TRUE;
}
// TODO: End temporary

b8 renderer_initialize(u64* memory_requirement, void* state, const char* application_name)
{
    *memory_requirement = sizeof(renderer_system_state);

    if (!state) return FALSE;

    state_ptr = state;

    // TODO: Temporary
    event_register(EVENT_DEBUG0, state_ptr, event_on_debug_event);
    // TODO: End temporary

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
        // TODO: Temporary
        event_unregister(EVENT_DEBUG0, state_ptr, event_on_debug_event);
        // TODO: End temporary

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

        mat4 model = mat4_identity();

        geometry_render_data data = {};
        data.object_id = 0;
        data.model = model;

        if (!state_ptr->test_diffuse)
        {
            state_ptr->test_diffuse = texture_system_get_default_texture();
        }

        data.textures[0] = state_ptr->test_diffuse;

        state_ptr->backend.update_object(data);

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

void renderer_create_texture (
    const char* name,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    struct texture* out_texture)
{
    state_ptr->backend.create_texture(name, width, height, channel_count, pixels, has_transparency, out_texture);
}

void renderer_destroy_texture (struct texture* texture)
{
    state_ptr->backend.destroy_texture(texture);
}