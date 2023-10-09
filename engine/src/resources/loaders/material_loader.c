#include "material_loader.h"

#include "core/logger.h"
#include "core/hstring.h"

#include "memory/hmemory.h"

#include "resources/resource_types.h"

#include "systems/resource_system.h"

#include "math/hmath.h"

#include "platform/filesystem.h"

b8 material_loader_load(struct resource_loader* self, const char* name, resource* out_resource)
{
    if (!self || !name || !out_resource)
    {
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char full_file_path[512];

    string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, ".hmt");

    out_resource->full_path = string_duplicate(full_file_path);

    file_handle f;
    if (!filesystem_open(full_file_path, FILE_MODE_READ, FALSE, &f))
    {
        HERROR("material_loader_load - unable to open material file for reading: '%s'.", full_file_path);
        return FALSE;
    }

    material_resource_data* resource_data = hallocate(sizeof(material_resource_data), MEMORY_TAG_MATERIAL);
    resource_data->auto_release = TRUE;
    resource_data->diffuse_color = vec4_one();
    resource_data->diffuse_name[0] = 0;
    string_ncopy(resource_data->name, name, MATERIAL_NAME_MAX_LENGTH);

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
            HWARN("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", full_file_path, line_number);
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
            string_ncopy(resource_data->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_name"))
        {
            string_ncopy(resource_data->diffuse_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_color"))
        {
            if (!string_to_vec4(trimmed_value, &resource_data->diffuse_color))
            {
                HWARN("Error parsing diffuse_color in file '%s'. Using default of white instead.", full_file_path);
            }
        }

        hzero_memory(line_buf, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&f);

    out_resource->data = resource_data;
    out_resource->data_size = sizeof(material_resource_data);
    out_resource->name = name;
    
    return TRUE;
}

void material_loader_unload(struct resource_loader* self, resource* resource)
{
    if (!self || !resource)
    {
        HWARN("material_loader_unload called with nullptr for self or resource.");
        return;
    }

    u32 path_length = string_length(resource->full_path);
    if (path_length)
    {
        hfree(resource->full_path, sizeof(char) * path_length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data)
    {
        hfree(resource->data, resource->data_size, MEMORY_TAG_MATERIAL);
        resource->data = 0;
        resource->data_size = 0;
        resource->loader_id = INVALID_ID;
    }
}

resource_loader material_resource_loader_create()
{
    resource_loader loader;
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.custom_type = 0;
    loader.load = material_loader_load;
    loader.unload = material_loader_unload;
    loader.type_path = "materials";

    return loader;
}