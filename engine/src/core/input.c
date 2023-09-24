#include "input.h"

#include "core/hmemory.h"
#include "core/logger.h"
#include "core/event.h"

typedef struct keyboard_state
{
    b8 keys[KEYS_MAX_KEYS];
} keyboard_state;

typedef struct mouse_state
{
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state
{
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

static b8 is_initialized = FALSE;
static input_state state = {};

b8 input_initialize()
{
    if (is_initialized == TRUE)
    {
        HERROR("input_initialize() called more than once!");
        return FALSE;
    }
    HINFO("Input subsystem initialized.");

    is_initialized = TRUE;
    return TRUE;
}

void input_shutdown()
{
    HINFO("Input subsystem shut down successfully.");
}

void input_update(f64 delta_time)
{
    if (!is_initialized)
    {
        return;
    }

    hcopy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    hcopy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

b8 input_is_key_down(keys key)
{
    if (!is_initialized)
    {
        return FALSE;
    }

    return state.keyboard_current.keys[key] == TRUE;
}

b8 input_is_key_up(keys key)
{
    if (!is_initialized)
    {
        return TRUE;
    }

    return state.keyboard_current.keys[key] == FALSE;
}

b8 input_was_key_down(keys key)
{
    if (!is_initialized)
    {
        return FALSE;
    }

    return state.keyboard_previous.keys[key] == TRUE;
}

b8 input_was_key_up(keys key)
{
    if (!is_initialized)
    {
        return TRUE;
    }

    return state.keyboard_previous.keys[key] == FALSE;
}

void input_process_key(keys key, b8 pressed)
{
    if (state.keyboard_current.keys[key] == pressed)
    {
        return;
    }

    state.keyboard_current.keys[key] = pressed;

    event_context context;
    context.data.u16[0] = key;
    event_post(pressed ? EVENT_KEY_PRESSED : EVENT_KEY_RELEASED, 0, context);
}

b8 input_is_button_down(buttons button)
{
    if (!is_initialized)
    {
        return FALSE;
    }

    return state.mouse_current.buttons[button] == TRUE;
}

b8 input_is_button_up(buttons button)
{
    if (!is_initialized)
    {
        return TRUE;
    }

    return state.mouse_current.buttons[button] == FALSE;
}

b8 input_was_button_down(buttons button)
{
    if (!is_initialized)
    {
        return FALSE;
    }

    return state.mouse_previous.buttons[button] == TRUE;
}

b8 input_was_button_up(buttons button)
{
    if (!is_initialized)
    {
        return TRUE;
    }

    return state.mouse_previous.buttons[button] == FALSE;
}

void input_get_mouse_position(i32* x, i32* y)
{
    if (!is_initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }


    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y)
{
    if (!is_initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input_process_button(buttons button, b8 pressed)
{
    if (state.mouse_current.buttons[button] == pressed)
    {
        return;
    }

    state.mouse_current.buttons[button] = pressed;

    event_context context;
    context.data.u16[0] = button;
    event_post(pressed ? EVENT_BUTTON_PRESSED : EVENT_BUTTON_RELEASED, 0, context);
}

void input_process_mouse_move(i16 x, i16 y)
{
    if (state.mouse_current.x == x && state.mouse_current.y == y)
    {
        return;
    }

    state.mouse_current.x = x;
    state.mouse_current.y = y;

    event_context context;
    context.data.i16[0] = x;
    context.data.i16[1] = y;
    event_post(EVENT_MOUSE_MOVED, 0, context);
}

void input_process_mouse_wheel(i8 z_delta)
{
    event_context context;
    context.data.i8[0] = z_delta;
    event_post(EVENT_MOUSE_WHEEL, 0, context);
}