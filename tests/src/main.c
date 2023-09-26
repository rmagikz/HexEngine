#include "test_manager.h"

#include "memory/linear_allocator_tests.h"

#include <core/logger.h>

int main()
{
    test_manager_init();

    // Register all tests...
    linear_allocator_register_tests();

    HDEBUG("Starting tests...");

    test_manager_run_tests();

    return 0;
}