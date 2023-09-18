#include "application.h"
#include "program_types.h"

#include "logger.h"

#include "platform/platform.h"
#include "core/hmemory.h"
#include "core/event.h"

typedef struct application_state
{
    b8 is_running;
    b8 is_suspended;

    i16 width;
    i16 height;
    f64 last_time;

    program* program_inst;
    platform_state platform;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

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

    if (!event_initialize())
    {
        HERROR("Event system failed to initialize. Application cannot continue.");
        return FALSE;
    }

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

    // Initialize client program.
    if (!app_state.program_inst->initialize(app_state.program_inst))
    {
        HFATAL("Program failed to initialize.");
        return FALSE;
    }

    app_state.program_inst->on_resize(app_state.program_inst, app_state.width, app_state.height);

    initialized = TRUE;
    return TRUE;
}

b8 application_run()
{
    HINFO(get_memory_usage_str());
    while (app_state.is_running)
    {
        if (!platform_pump_messages(&app_state.platform))
        {
            app_state.is_running = FALSE;
        }

        if (!app_state.is_suspended)
        {
            if (!app_state.program_inst->update(app_state.program_inst, (f32)0))
            {
                HFATAL("Program update failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }

            if (!app_state.program_inst->render(app_state.program_inst, (f32)0))
            {
                HFATAL("Program render failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }
        }
    }

    app_state.is_running = FALSE;

    event_shutdown();

    platform_shutdown(&app_state.platform);

    return TRUE;
}