#include "memory/hmemory.h"

#include "core/event.h"
#include "core/logger.h"

#include "containers/list.h"

typedef struct registered_event
{
    void* listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry
{
    registered_event* events;
} event_code_entry;

#define MAX_MESSAGE_CODES 16384

typedef struct event_system_state
{
    event_code_entry registered[MAX_MESSAGE_CODES];
    b8 is_initialized;
} event_system_state;

static event_system_state* state_ptr;

b8 event_initialize(u64* memory_requirement, void* state)
{
    *memory_requirement = sizeof(event_system_state);

    if (!state) return FALSE;

    state_ptr = state;
    state_ptr->is_initialized = TRUE;

    HINFO("Event subsystem initialized successfully.");

    return TRUE;
}

void event_shutdown(void* state)
{
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i)
    {
        if (state_ptr->registered[i].events != 0)
        {
            list_destroy(state_ptr->registered[i].events);
            state_ptr->registered[i].events = 0;
        }
    }

    state_ptr = 0;

    HINFO("Event subsystem shut down successfully.");
}

b8 event_register(u16 code, void* listener, PFN_on_event on_event)
{
    if (state_ptr->is_initialized == FALSE)
    {
        return FALSE;
    }

    if (state_ptr->registered[code].events == 0)
    {
        state_ptr->registered[code].events = list_create(registered_event);
    }

    u64 registered_count = list_count(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        if (state_ptr->registered[code].events[i].listener == listener)
        {
            return FALSE;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    list_push(state_ptr->registered[code].events, event);

    return TRUE;
}

b8 event_unregister(u16 code, void* listener, PFN_on_event on_event)
{
    if (state_ptr->is_initialized == FALSE)
    {
        return FALSE;
    }

    if (state_ptr->registered[code].events == 0)
    {
        return FALSE;
    }

    u64 registered_count = list_count(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state_ptr->registered[code].events[i];
        if (e.listener == listener && e.callback == on_event)
        {
            registered_event popped_event;
            list_pop_at(state_ptr->registered[code].events, i, &popped_event);
            return TRUE;
        }
    }

    return FALSE;
}

b8 event_post(u16 code, void* sender, event_context data)
{
    if (state_ptr->is_initialized == FALSE)
    {
        return FALSE;
    }

    if (state_ptr->registered[code].events == 0)
    {
        return FALSE;
    }

    u64 registered_count = list_count(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state_ptr->registered[code].events[i];
        if (e.callback(code, sender, e.listener, data))
        {
            return TRUE;
        }
    }

    return FALSE;
}