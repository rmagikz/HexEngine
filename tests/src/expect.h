#include <core/logger.h>

#include <math/hmath.h>

#define expect_should_be(expected, actual)                                                              \
    if (actual != expected)                                                                             \
    {                                                                                                   \
        HERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                                   \
    }

#define expect_should_not_be(expected, actual)                                                                      \
    if (actual == expected)                                                                                         \
    {                                                                                                               \
        HERROR("--> Expected %lld != %lld, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__);\
        return FALSE;                                                                                               \
    }

#define expect_floats_to_be(expected, actual)                                                       \
    if (kabs(expected - actual) > 0.001f)                                                           \
    {                                                                                               \
        HERROR("--> Expected %f, but got %f. File: %s:%d.", expected, actual, __FILE__, __LINE__);  \
        return FALSE;                                                                               \
    }

#define expect_to_be_true(actual)                                                               \
    if (actual != TRUE)                                                                         \
    {                                                                                           \
        HERROR("--> Expected true, but got: false. File %s:%d.", actual, __FILE__, __LINE__);   \
        return FALSE;                                                                           \
    }

#define expect_to_be_false(actual)                                                              \
    if (actual != FALSE)                                                                        \
    {                                                                                           \
        HERROR("--> Expected false, but got: true. File %s:%d.", actual, __FILE__, __LINE__);   \
        return FALSE;                                                                           \
    }