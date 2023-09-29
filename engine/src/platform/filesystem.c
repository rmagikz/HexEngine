#include "filesystem.h"

#include "core/logger.h"

#include "memory/hmemory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystem_exists(const char* path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

b8 filesystem_open(const char* path, file_modes mode, b8 binary, file_handle* out_handle)
{
    out_handle->is_valid = FALSE;
    out_handle->handle = 0;
    const char* mode_str;

    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0)
    {
        mode_str = binary ? "w+b" : "w+";
    }
    else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0)
    {
        mode_str = binary ? "rb" : "r";
    }
    else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0)
    {
        mode_str = binary ? "wb" : "w";
    }
    else
    {
        HERROR("Invalid mode passed while trying to open file: '%s'", path);
        return FALSE;
    }

    FILE* file = fopen(path, mode_str);
    if (!file)
    {
        HERROR("Error opening file: '%s'", path);
        return FALSE;
    }

    out_handle->handle = file;
    out_handle->is_valid = TRUE;

    return TRUE;
}

void filesystem_close(file_handle* handle)
{
    if (handle->handle)
    {
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->is_valid = FALSE;
    }
}

b8 filesystem_read_line(file_handle* handle, char** line_buf)
{
    if (!handle->handle) return FALSE;

    char buffer[32000];
    if (fgets(buffer, 32000, (FILE*)handle->handle) != 0)
    {
        u64 length = strlen(buffer);
        *line_buf = hallocate((sizeof(char) * length) + 1, MEMORY_TAG_STRING);
        strcpy(*line_buf, buffer);
        return TRUE;
    }

    return FALSE;
}

b8 filesystem_read_file(file_handle* handle, char** file_buf, u64* out_read)
{
    if (!handle->handle) return FALSE;

    fseek((FILE*)handle->handle, 0, SEEK_END);
    *out_read = ftell(handle->handle);
    fseek((FILE*)handle->handle, 0, SEEK_SET);

    *file_buf = hallocate(sizeof(char) * (*out_read + 1), MEMORY_TAG_STRING);
    fread(*file_buf, 1, *out_read, (FILE*)handle->handle);

    return TRUE;
}

b8 filesystem_write_line(file_handle* handle, const char* text)
{
    if (!handle->handle) return FALSE;

    i32 result = fputs(text, (FILE*)handle->handle);
    if (result != EOF)
    {
        result = fputc('\n', (FILE*)handle->handle);
    }

    fflush((FILE*)handle->handle);
    return result != EOF;
}

b8 filesystem_read(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read)
{
    if (!handle->handle || !out_data) return FALSE;

    *out_bytes_read = fread(out_data, 1, data_size, (FILE*)handle->handle);
    if (*out_bytes_read != data_size)
    {
        return FALSE;
    }
    return TRUE;
}

b8 filesystem_read_all_bytes(file_handle* handle, u8** out_bytes, u64* out_bytes_read)
{
    if (!handle->handle) return FALSE;

    fseek((FILE*)handle->handle, 0, SEEK_END);
    u64 size = ftell((FILE*)handle->handle);
    rewind((FILE*)handle->handle);

    *out_bytes = hallocate(sizeof(u8) * size, MEMORY_TAG_STRING);
    *out_bytes_read = fread(*out_bytes, 1, size, (FILE*)handle->handle);
    if (*out_bytes_read != size)
    {
        return FALSE;
    }
    return TRUE;
}

b8 filesystem_write(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written)
{
    if (!handle->handle) return FALSE;

    *out_bytes_written = fwrite(data, 1, data_size, (FILE*)handle->handle);
    if (*out_bytes_written != data_size)
    {
        return FALSE;
    }

    fflush((FILE*)handle->handle);

    return TRUE;
}