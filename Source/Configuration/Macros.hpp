/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides preprocessor-macros.
*/

#pragma once

// Debug information logging.
#if defined(NDEBUG)
    #define Printfunction()
    #define Debugprint(string)
#else
    #define Printfunction() Logformatted(__FUNCTION__, 'T')
    #define Debugprint(string) Logformatted(string, 'D')
#endif

// General information.
#define Infoprint(string) Logformatted(string, 'I')
#define vaprint(format, ...) Logprint(va(format, __VA_ARGS__))

// Some performance tweaking.
#if defined(_MSC_VER)
    #define likely(x)       x
    #define unlikely(x)     x
    #define ainline         __forceinline
    #define ninline         __declspec(noinline)
#else
    #define likely(x)       __builtin_expect(!!(x), 1)
    #define unlikely(x)     __builtin_expect(!!(x), 0)
    #define ainline         inline __attribute__((__always_inline__))
    #define ninline         __attribute__((__noinline__))
#endif
