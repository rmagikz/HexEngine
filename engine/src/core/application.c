#include "application.h"
#include "program_types.h"

#include "platform/platform.h"

#include "core/logger.h"
#include "core/hmemory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "renderer/renderer_frontend.h"

typedef struct application_state
{
    b8 is_running;
    b8 is_suspended;

    i16 width;
    i16 height;

    clock clock;
    f64 last_time;

    program* program_inst;
    platform_state platform;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_initialize(program* program_inst)
{
    if (initialized)
    {
        HERROR("application_create called more than once.");
        return FALSE;
    }

    app_state.program_inst = program_inst;
    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    // Initialize subsystems.
    if (!logger_initialize())
    {
        HERROR("Logger system failed to initialize.");
    }

    if (!input_initialize())
    {
        HERROR("Input system failed to initialize. Application cannot continue.");
        return FALSE;
    }

    if (!event_initialize())
    {
        HERROR("Event system failed to initialize. Application cannot continue.");
        return FALSE;
    }

    event_register(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_KEY_PRESSED, 0, application_on_key);

    // Initialize platform.
    if (!platform_initialize(
        &app_state.platform,
        program_inst->app_config.name,
        program_inst->app_config.start_x,
        program_inst->app_config.start_y,
        program_inst->app_config.start_width,
        program_inst->app_config.start_height))
    {
        return FALSE;
    }

    // Initialize renderer.
    if (!renderer_initialize(program_inst->app_config.name, &app_state.platform))
    {
        HFATAL("Failed to initialize renderer. Shutting down.");
        return FALSE;
    }

    // Initialize client program.
    if (!app_state.program_inst->initialize(app_state.program_inst))
    {
        HFATAL("Program failed to initialize. Shutting Down.");
        return FALSE;
    }

    app_state.program_inst->on_resize(app_state.program_inst, app_state.width, app_state.height);

    initialized = TRUE;
    return TRUE;
}

b8 application_run()
{
    HINFO(get_memory_usage_str());

    clock_start(&app_state.clock);
    clock_update(&app_state.clock);

    app_state.last_time = app_state.clock.elapsed;

    f64 running_time = 0;
    f64 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    while (app_state.is_running)
    {
        if (!platform_pump_messages(&app_state.platform))
        {
            app_state.is_running = FALSE;
        }

        if (!app_state.is_suspended)
        {
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta = current_time - app_state.last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if (!app_state.program_inst->update(app_state.program_inst, (f32)delta))
            {
                HFATAL("Program update failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }

            if (!app_state.program_inst->render(app_state.program_inst, (f32)delta))
            {
                HFATAL("Program render failed, shutting down.");
                app_state.is_running = FALSE;
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

            app_state.last_time = current_time;
        }
    }

    app_state.is_running = FALSE;

    event_unregister(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_KEY_PRESSED, 0, application_on_key);

    // Shutdown subsystems.
    logger_shutdown();
    event_shutdown();
    input_shutdown();

    renderer_shutdown();

    platform_shutdown(&app_state.platform);

    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context)
{
    switch (code)
    {
        case EVENT_APPLICATION_QUIT:
            HINFO("EVENT_APPLICATION_QUIT received, shutting down");
            app_state.is_running = FALSE;
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