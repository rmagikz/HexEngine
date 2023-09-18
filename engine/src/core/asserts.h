#pragma once

#include "defines.h"

#define HASSERTIONS_ENABLED

#ifdef HASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

HAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define HASSERT(expr)                                                       \
    {                                                                       \
        if (expr) {}                                                        \
        else                                                                \
        {                                                                   \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);        \
            debugBreak();                                                   \
        }                                                                   \
    }

#define HASSERT_MSG(expr, message)                                          \
    {                                                                       \
        if (expr) {}                                                        \
        else                                                                \
        {                                                                   \
            report_assertion_failure(#expr, message, __FILE__, __LINE__);   \
            debugBreak();                                                   \
        }                                                                   \
    }

#if _DEBUG
#define HASSERT_DEBUG(expr)                                                 \
    {                                                                       \
        if (expr) {}                                                        \
        else                                                                \
        {                                                                   \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);        \
            debugBreak();                                                   \
        }                                                                   \
    }
#else
#define HASSERT_DEBUG(expr)
#endif

#else
#define HASSERT(expr)
#define HASSERT_MSG(expr, message)
#define HASSERT_DEBUG(expr)
#endif