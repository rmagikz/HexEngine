#include "material_system.h"

#include "core/logger.h"
#include "core/hstring.h"
#include "containers/hashtable.h"
#include "math/hmath.h"
#include "renderer/renderer_frontend.h"
#include "systems/texture_system.h"

// TODO: Temporary
#include "platform/filesystem.h"

typedef struct material_system_state
{
    material_system_config config;

    material default_material;

    material* registered_materials;

    hashtable registered_materials_table;
} material_system_state;

typedef struct material_reference
{
    u64 reference_count;
    u32 id;
    b8 auto_release;
} material_reference;

static material_system_state* state_ptr = 0;

b8 create_default_material(material_system_state* state);
b8 load_material(material_config config, material* m);
void destroy_material(material* m);
b8 load_configuration_file(const char* path, material_config* out_config);

b8 material_system_initialize(u64* memory_requirement, void* state, material_system_config config)
{
    if (config.max_material_count == 0)
    {
        HFATAL("material_system_initialize - config.max_material_count must be > 0.");
        return FALSE;
    }

    u64 struct_requirement = sizeof(material_system_state);
    u64 array_requirement = sizeof(material) * config.max_material_count;
    u64 hashtable_requirement = sizeof(material_reference) * config.max_material_count;
    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;

    if (!state)
    {
        return FALSE;
    }

    state_ptr = state;
    state_ptr->config = config;

    void* array_block = state + struct_requirement;
    state_ptr->registered_materials = array_block;

    void* hashtable_block = array_block + hashtable_requirement;
    hashtable_create(sizeof(material_reference), config.max_material_count, hashtable_block, FALSE, &state_ptr->registered_materials_table);

    material_reference invalid_ref;
    invalid_ref.auto_release = FALSE;
    invalid_ref.id = INVALID_ID;
    invalid_ref.reference_count = 0;
    hashtable_fill(&state_ptr->registered_materials_table, &invalid_ref);

    u32 count = state_ptr->config.max_material_count;
    for (u32 i = 0; i < count; ++i)
    {
        state_ptr->registered_materials[i].handle = INVALID_ID;
        state_ptr->registered_materials[i].internal_id = INVALID_ID;
    }

    if (!create_default_material(state_ptr))
    {
        HFATAL("Failed to create default material. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void material_system_shutdown(void* state)
{
    material_system_state* s = (material_system_state*)state;

    if (!s) { return; }

    u32 count = s->config.max_material_count;
    for (u32 i = 0; i < count; ++i)
    {
        if (s->registered_materials[i].handle != INVALID_ID)
        {
            destroy_material(&s->registered_materials[i]);
        }
    }

    destroy_material(&s->default_material);

    state_ptr = 0;
}

material* material_system_acquire(const char* name)
{
    material_config config;

    char* format_str = "assets/materials/%s.%s";
    char full_file_path[512];

    string_format(full_file_path, format_str, name, "hmt");
    if (!load_configuration_file(full_file_path, &config))
    {
        HERROR("Failed to load material file: '%s'.", full_file_path);
        return 0;
    }

    return material_system_acquire_from_config(config);
}

material* material_system_acquire_from_config(material_config config)
{
    if (strings_equali(config.name, DEFAULT_MATERIAL_NAME))
    {
        return &state_ptr->default_material;
    }

    material_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_materials_table, config.name, &ref))
    {
        if (ref.reference_count == 0)
        {
            ref.auto_release = config.auto_release;
        }

        ref.reference_count++;

        if (ref.id == INVALID_ID)
        {
            u32 count = state_ptr->config.max_material_count;
            material* m = 0;
            for (u32 i = 0; i < count; ++i)
            {
                if (state_ptr->registered_materials[i].handle == INVALID_ID)
                {
                    ref.id = i;
                    m = &state_ptr->registered_materials[i];
                    break;
                }
            }

            if (!m || ref.id == INVALID_ID)
            {
                HFATAL("material_system_acquire - Material system cannot hold any more materials. Adjust configuration to allow more.");
                return 0;
            }

            if (!load_material(config, m))
            {
                HERROR("Failed to load material '%s'.", config.name);
                return 0;
            }

            HTRACE("Material '%s' doesn ot exist. Created, and ref_count is now %i.", config.name, ref.reference_count);
        }
        else
        {
            HTRACE("Material '%s' already exists, ref_count increased to %i.", config.name, ref.reference_count);
        }

        hashtable_set(&state_ptr->registered_materials_table, config.name, &ref);
        return &state_ptr->registered_materials[ref.id];
    }

    HERROR("material_system_acquire_from_config failed to acquire material '%s'.", config.name);
    return 0;
}

void material_system_release(const char* name)
{
    if (strings_equali(name, DEFAULT_MATERIAL_NAME))
    {
        return;
    }

    material_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_materials_table, name, &ref))
    {
        if (ref.reference_count == 0)
        {
            HWARN("Tried to release a material that does not exist: '%s'", name);
            return;
        }

        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release)
        {
            material* m = &state_ptr->registered_materials[ref.id];

            destroy_material(m);

            ref.id = INVALID_ID;
            ref.auto_release = FALSE;
            HTRACE("Released material '%s' because reference count = 0 and auto_release = true.", name);
        }
        else
        {
            HTRACE("Released material '%s', reference count is %i. (auto_release=%s)", name, ref.reference_count, ref.auto_release ? "true" : "false")
        }

        hashtable_set(&state_ptr->registered_materials_table, name, &ref);
    }
    else
    {
        HERROR("material_system_release failed to release material '%s'", name);
    }
}

b8 create_default_material(material_system_state* state)
{
    hzero_memory(&state->default_material, sizeof(material));
    state->default_material.handle = INVALID_ID;
    
    string_ncopy(state->default_material.name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    state->default_material.diffuse_color = vec4_one();
    state->default_material.diffuse = texture_system_get_default_texture();

    return TRUE;
}

b8 load_material(material_config config, material* m)
{
    hzero_memory(m, sizeof(material));

    string_ncopy(m->name, config.name, MATERIAL_NAME_MAX_LENGTH);

    m->diffuse_color = config.diffuse_color;

    if (string_length(config.diffuse_name) > 0)
    {
        m->diffuse = texture_system_acquire(config.diffuse_name, TRUE);
        if (!m->diffuse)
        {
            HWARN("Unable to load texture '%s' for material '%s', using default.", config.diffuse_name, m->name);
            m->diffuse = texture_system_get_default_texture();
        }
    }

    return TRUE;
}

void destroy_material(material* m)
{
    HTRACE("Destroying material '%s'...", m->name);

    if (m->diffuse)
    {
        texture_system_release(m->diffuse->name);
    }

    hzero_memory(m, sizeof(material));
    m->handle = INVALID_ID;
    m->internal_id = INVALID_ID;
}

b8 load_configuration_file(const char* path, material_config* out_config)
{
    file_handle f;
    if (!filesystem_open(path, FILE_MODE_READ, FALSE, &f))
    {
        HERROR("load_configuration_file - unable to open material file for reading: '%s'.", path);
        return FALSE;
    }

    char line_buf[512] = "";
    char* p = &line_buf[0];
    u64 line_length = 0;
    u32 line_number = 1;
    while (filesystem_read_line(&f, 511, &p, &line_length))
    {
        char* trimmed = string_trim(line_buf);

        line_length = string_length(trimmed);

        if (line_length < 1 || trimmed[0] == '#')
        {
            line_number++;
            continue;
        }

        i32 equal_index = string_index_of(trimmed, '=');
        if (equal_index == -1)
        {
            HWARN("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", path, line_number);
            line_number++;
            continue;
        }

        char raw_var_name[64];
        hzero_memory(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char* trimmed_var_name = string_trim(raw_var_name);

        char raw_value[446];
        hzero_memory(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1);
        char* trimmed_value = string_trim(raw_value);

        if (strings_equali(trimmed_var_name, "version"))
        {
            // TODO: version
        }
        else if (strings_equali(trimmed_var_name, "name"))
        {
            string_ncopy(out_config->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_name"))
        {
            string_ncopy(out_config->diffuse_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_color"))
        {
            if (!string_to_vec4(trimmed_value, &out_config->diffuse_color))
            {
                HWARN("Error parsing diffuse_color in file '%s'. Using default of white instead.", path);
                out_config->diffuse_color = vec4_one();
            }
        }

        hzero_memory(line_buf, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&f);

    return TRUE;
}

material* material_system_get_default()
{
    if (state_ptr)
    {
        return &state_ptr->default_material;
    }

    HFATAL("material_system_get_default called before material system initialization.");
    return 0;
}