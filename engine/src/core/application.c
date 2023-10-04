#include "application.h"
#include "program_types.h"

#include "platform/platform.h"

#include "memory/hmemory.h"

#include "core/logger.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/linear_allocator.h"

#include "renderer/renderer_frontend.h"

#include "systems/texture_system.h"

typedef struct application_state
{
    b8 is_running;
    b8 is_suspended;

    i16 width;
    i16 height;

    clock clock;
    f64 last_time;

    program* program_inst;

    linear_allocator systems_allocator;

    //u64 logger_subsystem_memory_requirement;
    void* logger_subsystem_state;

    //u64 input_subsystem_memory_requirement;
    void* input_subsystem_state;

    //u64 event_subsystem_memory_requirement;
    void* event_subsystem_state;

    //u64 memory_subsystem_memory_requirement;
    void* memory_subsystem_state;

    //u64 platform_subsystem_memory_requirement;
    void* platform_subsystem_state;

    //u64 renderer_subsystem_memory_requirement;
    void* renderer_subsystem_state;

    void* texture_system_state;
} application_state;

static application_state* app_state;

b8 application_on_quit(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_initialize(program* program_inst)
{
    if (program_inst->application_state)
    {
        HERROR("application_create called more than once.");
        return FALSE;
    }

    program_inst->application_state = hallocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    app_state = program_inst->application_state;
    app_state->program_inst = program_inst;
    app_state->is_running = FALSE;
    app_state->is_suspended = FALSE;

    u64 systems_allocator_total_size = 64 * 1024 * 1024; // 64mb

    linear_allocator_create(systems_allocator_total_size, 0, &app_state->systems_allocator);

    // Initialize memory.
    u64 memory_memory_requirement;
    memory_initialize(&memory_memory_requirement, 0);
    app_state->memory_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, memory_memory_requirement);
    if (!memory_initialize(&memory_memory_requirement, app_state->memory_subsystem_state))
    {
        HERROR("Failed to initialize memory subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize logger.
    u64 logger_memory_requirement;
    logger_initialize(&logger_memory_requirement, 0);
    app_state->logger_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, logger_memory_requirement);
    if (!logger_initialize(&logger_memory_requirement, app_state->logger_subsystem_state))
    {
        HERROR("Failed to initialize logger subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize input.
    u64 input_memory_requirement;
    input_initialize(&input_memory_requirement, 0);
    app_state->input_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, input_memory_requirement);
    if (!input_initialize(&input_memory_requirement, app_state->input_subsystem_state))
    {
        HERROR("Failed to initialize input subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize event.
    u64 event_memory_requirement;
    event_initialize(&event_memory_requirement, 0);
    app_state->event_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, event_memory_requirement);
    if (!event_initialize(&event_memory_requirement, app_state->event_subsystem_state))
    {
        HERROR("Failed to initialize event subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize platform.
    u64 platform_memory_requirement;
    platform_initialize(&platform_memory_requirement, 0, 0, 0, 0, 0, 0);
    app_state->platform_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, platform_memory_requirement);
    if (!platform_initialize(
        &platform_memory_requirement,
        app_state->platform_subsystem_state,
        program_inst->app_config.name,
        program_inst->app_config.start_x,
        program_inst->app_config.start_y,
        program_inst->app_config.start_width,
        program_inst->app_config.start_height))
    {
        HERROR("Failed to initialize platform subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize renderer.
    u64 renderer_memory_requirement;
    renderer_initialize(&renderer_memory_requirement, 0, 0);
    app_state->renderer_subsystem_state = linear_allocator_allocate(&app_state->systems_allocator, renderer_memory_requirement);
    if (!renderer_initialize(&renderer_memory_requirement, app_state->renderer_subsystem_state, app_state->program_inst->app_config.name))
    {
        HFATAL("Failed to initialize renderer subsystem. Shutting down.");
        return FALSE;
    }

    // Initialize texture system.
    u64 texture_system_memory_requirement;
    texture_system_config texture_system_config;
    texture_system_config.max_texture_count = 65536;
    texture_system_initialize(&texture_system_memory_requirement, 0, texture_system_config);
    app_state->texture_system_state = linear_allocator_allocate(&app_state->systems_allocator, texture_system_memory_requirement);
    if (!texture_system_initialize(&texture_system_memory_requirement, app_state->texture_system_state, texture_system_config))
    {
        HFATAL("Failed to initialize renderer subsystem. Shutting down.");
    }

    // Initialize client program.
    if (!app_state->program_inst->initialize(app_state->program_inst))
    {
        HFATAL("Program failed to initialize. Shutting Down.");
        return FALSE;
    }

    event_register(EVENT_APPLICATION_QUIT, 0, application_on_quit);
    event_register(EVENT_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_RESIZED, 0, application_on_resized);

    app_state->program_inst->on_resize(app_state->program_inst, app_state->width, app_state->height);

    return TRUE;
}

b8 application_run()
{
    app_state->is_running = TRUE;
    app_state->is_suspended = FALSE;

    HINFO(get_memory_usage_str());

    clock_start(&app_state->clock);
    clock_update(&app_state->clock);

    app_state->last_time = app_state->clock.elapsed;

    f64 running_time = 0;
    f64 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    while (app_state->is_running)
    {
        if (!platform_pump_messages(&app_state->platform_subsystem_state))
        {
            app_state->is_running = FALSE;
        }

        if (!app_state->is_suspended)
        {
            clock_update(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta = current_time - app_state->last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if (!app_state->program_inst->update(app_state->program_inst, (f32)delta))
            {
                HFATAL("Program update failed, shutting down.");
                app_state->is_running = FALSE;
                break;
            }

            if (!app_state->program_inst->render(app_state->program_inst, (f32)delta))
            {
                HFATAL("Program render failed, shutting down.");
                app_state->is_running = FALSE;
                break;
            }

            render_packet packet;
            packet.delta_time = delta;
            renderer_draw_frame(&packet);

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0)
            {
                u64 remaining_ms = remaining_seconds * 1000;

                b8 limit_frames = FALSE;
                if (remaining_ms > 0 && limit_frames)
                {
                    platform_sleep(remaining_ms - 1);
                }

                frame_count++;
            }

            input_update(delta);

            app_state->last_time = current_time;
        }
    }

    app_state->is_running = FALSE;

    event_unregister(EVENT_APPLICATION_QUIT, 0, application_on_quit);
    event_unregister(EVENT_KEY_PRESSED, 0, application_on_key);

    texture_system_shutdown(app_state->texture_system_state);

    renderer_shutdown(app_state->renderer_subsystem_state);

    // Shutdown subsystems.
    logger_shutdown(app_state->logger_subsystem_state);
    event_shutdown(app_state->event_subsystem_state);
    input_shutdown(app_state->input_subsystem_state);

    platform_shutdown(&app_state->platform_subsystem_state);

    memory_shutdown(&app_state->event_subsystem_state);

    return TRUE;
}

b8 application_on_quit(u16 code, void* sender, void* listener_inst, event_context context)
{
    switch (code)
    {
        case EVENT_APPLICATION_QUIT:
            HINFO("EVENT_APPLICATION_QUIT received, shutting down");
            app_state->is_running = FALSE;
            return TRUE;
    }

    return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context)
{
    if (code == EVENT_KEY_PRESSED)
    {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE)
        {
            event_context data = {};
            event_post(EVENT_APPLICATION_QUIT, 0, data);

            return TRUE;
        }
    }

    return FALSE;
}

b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context)
{
    if (code != EVENT_RESIZED) return FALSE;

    u16 width = context.data.u16[0];
    u16 height = context.data.u16[1];

    if (width != app_state->width || height != app_state->height) // Only update if size changed.
    {
        app_state->width = width;
        app_state->height = height;

        if (width == 0 || height == 0)
        {
            // Window is minimized, suspending.
            app_state->is_suspended = TRUE;
            HDEBUG("Window minimized, suspending.");
        }
        else
        {
            // Window is not minimized, resize program and renderer.
            app_state->is_suspended = FALSE;
            app_state->program_inst->on_resize(app_state->program_inst, width, height);
            renderer_on_resized(width, height);
        }
    }

    return FALSE;
}