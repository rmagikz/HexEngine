#include "program.h"

#include <memory/hmemory.h>

#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>

// HACK: this should not be available.
#include <renderer/renderer_frontend.h>
#include <math/hmath.h>

void recalculate_view_matrix(program_state* state)
{
    if (!state->camera_view_dirty) return;

    mat4 rotation = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
    mat4 translation = mat4_translation(state->camera_position);

    state->view = mat4_mul(rotation, translation);
    state->view = mat4_inverse(state->view);

    state->camera_view_dirty = FALSE;
}

void camera_yaw(program_state* state, f32 amount)
{
    state->camera_euler.y += amount;
    state->camera_view_dirty = TRUE;
}

void camera_pitch(program_state* state, f32 amount)
{
    state->camera_euler.x += amount;

    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = HCLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = TRUE;
}

b8 program_initialize (struct program* program_inst)
{

    program_state* state= (program_state*)program_inst->program_state;

    state->camera_position = (vec3){0, 0, 30.0f};
    state->camera_euler = vec3_zero();

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);

    state->camera_view_dirty = TRUE;

    return TRUE;
}

b8 program_update (struct program* program_inst, f32 delta_time)
{
    program_state* state = (program_state*)program_inst->program_state;

    static u64 alloc_count = 0;
    u64 previous_alloc_count = alloc_count;

    alloc_count = get_total_memory_allocations();

    if (input_is_key_up('M') && input_was_key_down('M'))
    {
        HDEBUG("Total allocations: %llu (%llu this frame)", alloc_count, alloc_count - previous_alloc_count);
    }

    // TODO: Temporary
    if (input_is_key_up('T') && input_was_key_down('T'))
    {
        HDEBUG("Swapping textures.");
        event_context context = {};
        event_post(EVENT_DEBUG0, program_inst, context);
    }
    // TODO: End temporary

    if (input_is_key_down(KEY_LEFT))
    {
        camera_yaw(state, 1000000.0f * delta_time);
    }

    if (input_is_key_down(KEY_RIGHT))
    {
        camera_yaw(state, -1000000.0f * delta_time);
    }

    if (input_is_key_down(KEY_UP))
    {
        camera_pitch(state, 1000000.0f * delta_time);
    }

    if (input_is_key_down(KEY_DOWN))
    {
        camera_pitch(state, -1000000.0f * delta_time);
    }

    f32 temp_move_speed = 10.0f;
    vec3 velocity = vec3_zero();

    if (input_is_key_down('W'))
    {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }

    if (input_is_key_down('S'))
    {
        vec3 back = mat4_back(state->view);
        velocity = vec3_add(velocity, back);
    }

    if (input_is_key_down('A'))
    {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }

    if (input_is_key_down('D'))
    {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    if (input_is_key_down(KEY_SPACE))
    {
        velocity.y += 1.0f;
    }

    if (input_is_key_down(KEY_LCONTROL))
    {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f))
    {
        vec3_normalize(&velocity);

        state->camera_position.x += velocity.x;
        state->camera_position.y += velocity.y;
        state->camera_position.z += velocity.z;

        state->camera_view_dirty = TRUE;
    }

    recalculate_view_matrix(state);

    // HACK: naughty naughty >;]
    renderer_set_view(state->view);

    return TRUE;
}

b8 program_render (struct program* program_inst, f32 delta_time)
{
    return TRUE;
}

void program_on_resize (struct program* program_inst, u32 width, u32 height)
{

}