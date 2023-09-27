#include "linear_allocator_tests.h"
#include "../test_manager.h"
#include "../expect.h"

#include <defines.h>

#include <memory/linear_allocator.h>
#include <core/logger.h>

u8 linear_allocator_should_create_and_destroy()
{
    linear_allocator allocator;

    linear_allocator_create(sizeof(u64), 0, &allocator);

    expect_should_not_be(0, allocator.memory);
    expect_should_be(sizeof(u64), allocator.total_size);
    expect_should_be(0, allocator.allocated);

    linear_allocator_destroy(&allocator);

    expect_should_be(0, allocator.memory);
    expect_should_be(0, allocator.total_size);
    expect_should_be(0, allocator.allocated);

    return TRUE;
}

u8 linear_allocator_single_allocation_all_space()
{
    linear_allocator allocator;
    linear_allocator_create(sizeof(u64), 0, &allocator);

    void* block = linear_allocator_allocate(&allocator, sizeof(u64));

    expect_should_not_be(0, allocator.allocated);
    expect_should_be(sizeof(u64), allocator.allocated);

    linear_allocator_destroy(&allocator);

    return TRUE;
}

u8 linear_allocator_multi_allocation_all_space()
{
    u64 max_allocs = 1024;
    linear_allocator allocator;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

    void* block;
    for (u64 i = 0; i < max_allocs; ++i)
    {
        block = linear_allocator_allocate(&allocator, sizeof(64));

        expect_should_not_be(0, block);
        expect_should_be(sizeof(64) * (i + 1), allocator.allocated);
    }

    linear_allocator_destroy(&allocator);

    return TRUE;
}

u8 linear_allocator_multi_allocation_over_allocate()
{
    u64 max_allocs = 3;
    linear_allocator allocator;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

    void* block;
    for (u64 i = 0; i < max_allocs; ++i)
    {
        block = linear_allocator_allocate(&allocator, sizeof(u64));

        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i + 1), allocator.allocated);
    }

    HINFO("The following error is intentionally triggered.");

    block = linear_allocator_allocate(&allocator, sizeof(u64));

    expect_should_be(0, block);
    expect_should_be(sizeof(u64) * max_allocs, allocator.allocated);

    linear_allocator_destroy(&allocator);

    return TRUE;
}

u8 linear_allocator_multi_allocation_all_space_then_free()
{
    u64 max_allocs = 1024;
    linear_allocator allocator;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

    void* block;
    for (u64 i = 0; i < max_allocs; ++i)
    {
        block = linear_allocator_allocate(&allocator, sizeof(u64));

        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i + 1), allocator.allocated);
    }

    linear_allocator_free_all(&allocator);
    expect_should_be(0, allocator.allocated);

    linear_allocator_destroy(&allocator);

    return TRUE;
}

void linear_allocator_register_tests()
{
    test_manager_register_test(linear_allocator_should_create_and_destroy, "Linear allocator should create and destroy.");
    test_manager_register_test(linear_allocator_single_allocation_all_space, "Linear allocator should allocate all space.");
    test_manager_register_test(linear_allocator_multi_allocation_all_space, "Linear allocator should allocate all space in multiple allocations.");
    test_manager_register_test(linear_allocator_multi_allocation_over_allocate, "Linear allocator should allocate and prevent over allocating");
    test_manager_register_test(linear_allocator_multi_allocation_all_space_then_free, "Linear allocator should allocate and free memory");
}