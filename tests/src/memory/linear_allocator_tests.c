#include "linear_allocator_tests.h"
#include "../test_manager.h"
#include "../expect.h"

#include <defines.h>

#include <memory/linear_allocator.h>

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

void linear_allocator_register_tests()
{
    test_manager_register_test(linear_allocator_should_create_and_destroy, "Linear allocator should create and destroy.");
}