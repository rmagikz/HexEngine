#include "texture_system.h"

#include "core/logger.h"
#include "core/hstring.h"
#include "memory/hmemory.h"
#include "containers/hashtable.h"

#include "renderer/renderer_frontend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct texture_system_state
{
    texture_system_config config;
    texture default_texture;

    texture* registered_textures;

    hashtable registered_texture_table;
} texture_system_state;

typedef struct texture_reference
{
    u64 reference_count;
    u32 index;
    b8 auto_release;
} texture_reference;

static texture_system_state* state_ptr = 0;

b8 create_default_textures(texture_system_state* state);
void destroy_default_textures(texture_system_state* state);
b8 load_texture(const char* texture_name, texture* t);

b8 texture_system_initialize(u64* memory_requirement, void* state, texture_system_config config)
{
    if (config.max_texture_count == 0)
    {
        HFATAL("texture_system_initialize - config.max_texture_count must be > 0.");
        return FALSE;
    }

    u64 struct_requirement = sizeof(texture_system_state);
    u64 array_requirement = sizeof(texture) * config.max_texture_count;
    u64 hashtable_requirement = sizeof(texture_reference) * config.max_texture_count;

    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;

    if (!state)
    {
        return FALSE;
    }

    state_ptr = state;
    state_ptr->config = config;

    void* array_block = state + struct_requirement;
    state_ptr->registered_textures = array_block;

    void* hashtable_block = array_block + array_requirement;
    hashtable_create(sizeof(texture_reference), config.max_texture_count, hashtable_block, FALSE, &state_ptr->registered_texture_table);

    texture_reference invalid_ref;
    invalid_ref.auto_release = FALSE;
    invalid_ref.index = INVALID_ID;
    invalid_ref.reference_count = 0;
    hashtable_fill(&state_ptr->registered_texture_table, &invalid_ref);

    u32 count = state_ptr->config.max_texture_count;
    for (u32 i = 0; i < count; ++i)
    {
        state_ptr->registered_textures[i].handle = INVALID_ID;
    }

    create_default_textures(state_ptr);

    return TRUE;
}

void texture_system_shutdown(void* state)
{
    if (!state_ptr) return;

    for (u32 i = 0; i < state_ptr->config.max_texture_count; ++i)
    {
        texture* t = &state_ptr->registered_textures[i];
        if (t->handle != INVALID_ID)
        {
            renderer_destroy_texture(t);
        }
    }

    destroy_default_textures(state_ptr);

    state_ptr = 0;
}

texture* texture_system_acquire(const char* name, b8 auto_release)
{
    if (strings_equali(name, DEFAULT_TEXTURE_NAME))
    {
        HWARN("texture_system_acquire called for default texture. Use texture_system_get_default_texture for texture 'default'.");
        return &state_ptr->default_texture;
    }

    texture_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_texture_table, name, &ref))
    {
        if (ref.reference_count == 0)
        {
            ref.auto_release = auto_release;
        }

        ref.reference_count++;
        
        if (ref.index == INVALID_ID)
        {
            u32 count = state_ptr->config.max_texture_count;
            texture* t = 0;
            for (u32 i = 0; i < count; ++i)
            {
                if (state_ptr->registered_textures[i].handle == INVALID_ID)
                {
                    ref.index = i;
                    t = &state_ptr->registered_textures[i];
                    break;
                }
            }

            if (!t || ref.index == INVALID_ID)
            {
                HFATAL("texture_system_acquire - Texture system cannot hold any more textures. Adjust configuration to allow more.");
                return 0;
            }

            if (!load_texture(name, t))
            {
                HERROR("Failed to load texture '%s'.", name);
                return 0;
            }

            HTRACE("Texture '%s' does not exist yet. Created, and ref_count is now %i.", name, ref.reference_count);
        }
        else
        {
            HTRACE("Texture '%s' already exists, ref_count increased to %i.", name, ref.reference_count);
        }

        hashtable_set(&state_ptr->registered_texture_table, name, &ref);
        return &state_ptr->registered_textures[ref.index];
    }

    HERROR("texture_system_acquire failed to acquire texture '%s'.", name);
    return 0;
}

void texture_system_release(const char* name)
{
    if (strings_equali(name, DEFAULT_TEXTURE_NAME))
    {
        return;
    }

    texture_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_texture_table, name, &ref))
    {
        if (ref.reference_count == 0)
        {
            HWARN("Tried to release a texture that does not exist.");
            return;
        }

        ref.reference_count--;

        if (ref.reference_count == 0 && ref.auto_release)
        {
            texture* t = &state_ptr->registered_textures[ref.index];

            renderer_destroy_texture(t);

            hzero_memory(t, sizeof(texture));
            t->handle = INVALID_ID;

            ref.index = INVALID_ID;
            ref.auto_release = FALSE;
            
            HTRACE("Released texture '%s'. Texture unloaded because reference count = 0 and auto_release = true", name);
        }
        else
        {
            HTRACE("Released texture '%s'. Texture now has a reference count of '%i'. (auto_release = %s)", name, ref.reference_count, ref.auto_release ? "true" : "false");
        }

        hashtable_set(&state_ptr->registered_texture_table, name, &ref);
    }
    else
    {
        HERROR("texture_system_release failed to release texture '%s'.", name);
    }
}

texture* texture_system_get_default_texture()
{
    if (state_ptr)
    {
        return &state_ptr->default_texture;
    }

    HERROR("texture_system_get_default_texture called before textur system initialization!");
    return 0;
}

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

    renderer_create_texture(texture_name, temp_texture.width, temp_texture.height, temp_texture.channel_count, data, has_transparency, &temp_texture);

    texture old = *t;

    *t = temp_texture;

    renderer_destroy_texture(&old);

    stbi_image_free(data);

    return TRUE;
}

b8 create_default_textures(texture_system_state* state)
{
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

    renderer_create_texture(DEFAULT_TEXTURE_NAME, tex_dimension, tex_dimension, channels, pixels, FALSE, &state_ptr->default_texture);

    return TRUE;
}

void destroy_default_textures(texture_system_state* state)
{
    if (state)
    {
        renderer_destroy_texture(&state->default_texture);
    }
}