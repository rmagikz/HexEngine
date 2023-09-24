#include "hmemory.h"

#include "core/logger.h"
#include "core/hstring.h"
#include "platform/platform.h"

#include <string.h>
#include <stdio.h>

struct memory_stats
{
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = 
{
    "UNKNOWN    ",
    "ARRAY      ",
    "LIST       ",
    "DICT       ",
    "STRING     ",
    "APPLICATION",
    "PROGRAM    ",
    "RENDERER   ",
};

static b8 is_initialized = FALSE;
static struct memory_stats stats;

b8 memory_initialize()
{
    if (is_initialized == TRUE)
    {
        HERROR("memory_initialize() called more than once!");
        return FALSE;
    }
    HINFO("Memory subsystem initialized.");

    platform_zero_memory(&stats, sizeof(stats));

    return TRUE;
}

void memory_shutdown()
{
    // TODO: eventual cleanup.

    HINFO("Memory subsystem shut down successfully.");
}

void* hallocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        HWARN("hallocate called using MEMORY_TAG_UNKNOWN.");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(block, size);

    return block;
}

void hfree(void* block, u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        HWARN("hfree called using MEMORY_TAG_UNKNOWN.");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    platform_free(block, FALSE);
}

void* hzero_memory(void* block, u64 size)
{
    return platform_zero_memory(block, size);
}

void* hcopy_memory(void* dest, const void* src, u64 size)
{
    return platform_copy_memory(dest, src, size);
}

void* hset_memory(void* block, i32 value, u64 size)
{
    return platform_set_memory(block, value, size);
}

#ifdef _DEBUG

char* get_memory_usage_str()
{
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
    {
        char unit[4] = "Xib";
        float amount = 1.0f;

        if (stats.tagged_allocations[i] >= gib)
        {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (float)gib;
        }
        else if (stats.tagged_allocations[i] >= mib)
        {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (float)mib;
        }
        else if (stats.tagged_allocations[i] >= kib)
        {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (float)kib;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.tagged_allocations[i];
        }

        offset += snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
    }

    offset += snprintf(buffer + offset, 8000, "Total memory usage (including this string): %lldB", stats.total_allocated += string_length(buffer));

    char* out_string = string_duplicate(buffer);
    return out_string;
}

#endif