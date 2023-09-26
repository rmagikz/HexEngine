#include "test_manager.h"

#include <containers/list.h>
#include <core/logger.h>
#include <core/hstring.h>
#include <core/clock.h>

typedef struct test_entry
{
    PFN_test func;
    char* desc;
} test_entry;

static test_entry* tests;

void test_manager_init()
{
    tests = list_create(test_entry);
}

void test_manager_register_test(u8 (*PFN_test)(), char* desc)
{
    test_entry test;
    test.func = PFN_test;
    test.desc = desc;
    list_push(tests, test);
}

void test_manager_run_tests()
{
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = list_count(tests);

    clock total_time;
    clock_start(&total_time);

    for (u32 i = 0; i < count; ++i)
    {
        clock test_time;
        clock_start(&test_time);
        u8 result = tests[i].func();
        clock_update(&test_time);

        if (result == TRUE)
        {
            passed++;
        }
        else if (result == BYPASS)
        {
            HWARN("[SKIPPED]: %s", tests[i].desc);
            skipped++;
        }
        else
        {
            HERROR("[FAILED]: %s", tests[i].desc);
            failed++;
        }

        char status[20];
        string_format(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        clock_update(&total_time);
        HINFO("Executed %d of %d (skipped %d) %s (%0.6f sec / %0.6f sec total)", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    clock_stop(&total_time);

    HINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}