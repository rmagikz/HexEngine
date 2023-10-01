#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef int b32;
typedef char b8;

#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

#define TRUE 1
#define FALSE 0

#define INVALID_ID 4294967295U

// platform windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define HPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit version of Windows is required!"
#endif
// platform linux
#elif defined(__linux__) || defined(__gnu_linux__)
#define HPLATFORM_LINUX 1
#elif defined(__unix__)
#define HPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
#define HPLATFORM_POSIX 1
// platform apple
#elif __APPLE__
#define HPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define HPLATFORM_IOS 1
#define HPLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define HPLATFORM_IOS 1
#elif TARGET_OS_MAC
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform"
#endif

#ifdef HEXPORT
#ifdef _MSC_VER
#define HAPI __declspec(dllexport)
#else
#define HAPI __attribute__((visibility("default")))
#endif
#else

#ifdef _MSC_VER
#define HAPI __declspec(dllimport)
#else
#define HAPI
#endif
#endif


#define HCLAMP(value, min, max) ((value <= min) ? min : (value >= max) ? max \
                                                                       : value)

// Inline
#ifdef _MSC_VER
#define HINLINE __forceinline
#define HNOINLINE __declspec(noinline)
#else
#define HINLINE static inline
#define HNOINLINE
#endif