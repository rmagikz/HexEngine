#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "memory/hmemory.h"

#include "math/hmath.h"

#include "core/logger.h"

#include "resources/resource_types.h"

// TODO: Temporary
#include "core/hstring.h"
#include "core/event.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// TODO: End temporary

typedef struct renderer_system_state
{
    renderer_backend backend;
    mat4 projection;
    mat4 view;
    f32 near_clip;
    f32 far_clip;

    texture default_texture;
    texture test_diffuse;
} renderer_system_state;

static renderer_system_state* state_ptr;

void create_texture(texture* t)
{
    hzero_memory(t, sizeof(texture));
}

b8 load_texture(const char* texture_name, texture* t)
{
    char* format_str = "assets/textures/%s.%s";
    const i32 required_channel_count = 4;
    stbi_set_flip_vertically_on_load(TRUE);
    char full_file_path[512];

    string_format(full_file_path, format_str, texture_name, "png");

    texture temp_texture;

    u8* data = stbi_load(full_file_path, (i32*)&temp_texture.width, (i32*)&temp_texture.height, (i32*)temp_texture.channel_count, required_channel_count);

    temp_texture.channel_count = required_channel_count;

    if (!data) { HERROR("Unable to load texture at path: %s", full_file_path); }

    u64 total_size = temp_texture.width * temp_texture.height * required_channel_count;

    b32 has_transparency = FALSE;
    for (u64 i = 0; i < total_size; ++i)
    {
        u8 a = data[i + 3];
        if (a < 255)
        {
            has_transparency = TRUE;
            break;
        }
    }

    if (stbi_failure_reason())
    {
        HWARN("load_texture() failed to load file '%s': %s", full_file_path, stbi_failure_reason());
    }

    renderer_create_texture(texture_name, TRUE, temp_texture.width, temp_texture.height, temp_texture.channel_count, data, has_transparency, &temp_texture);

    texture old = *t;

    *t = temp_texture;

    renderer_destroy_texture(&old);

    stbi_image_free(data);

    return TRUE;
}

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
    choice++;
    choice %= 3;

    load_texture(names[choice], &state_ptr->test_diffuse);

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

    state_ptr->backend.default_diffuse = &state_ptr->default_texture;

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

    //Generate default texture
    const u32 tex_dimension = 16;
    const u32 channels = 4;
    const u32 pixel_count = tex_dimension * tex_dimension;
    
    u8 pixels[pixel_count * channels];
    hset_memory(pixels, 0, sizeof(u8) * pixel_count * channels);

    for (u64 row = 0; row < tex_dimension; ++row)
    {
        for (u64 col = 0; col < tex_dimension; ++col)
        {
            u64 index = (row * tex_dimension) + col;
            u64 index_channel = index * channels;

            if (row % 2)
            {
                if (col % 2)
                {
                    pixels[index_channel + 0] = 255;
                    pixels[index_channel + 2] = 255;
                }
            }
            else
            {
                if (!(col % 2))
                {
                    pixels[index_channel + 0] = 255;
                    pixels[index_channel + 2] = 255;
                }
            }
        }
    }

    renderer_create_texture("default", FALSE, tex_dimension, tex_dimension, channels, pixels, FALSE, &state_ptr->default_texture);

    create_texture(&state_ptr->test_diffuse);

    return TRUE;
}

void renderer_shutdown(void* state)
{
    if (state_ptr)
    {
        // TODO: Temporary
        event_unregister(EVENT_DEBUG0, state_ptr, event_on_debug_event);
        // TODO: End temporary

        renderer_destroy_texture(&state_ptr->default_texture);
        renderer_destroy_texture(&state_ptr->test_diffuse);
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
        data.textures[0] = &state_ptr->test_diffuse;

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
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    struct texture* out_texture)
{
    state_ptr->backend.create_texture(name, auto_release, width, height, channel_count, pixels, has_transparency, out_texture);
}

void renderer_destroy_texture (struct texture* texture)
{
    state_ptr->backend.destroy_texture(texture);
}