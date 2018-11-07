/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides definitions for the compiler.
*/

#pragma once

// Fixup for Visual Studio 2015 no longer defining this.
#if !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

// Platform identification.
#if defined(_MSC_VER)
    #define EXPORT_ATTR __declspec(dllexport)
    #define IMPORT_ATTR __declspec(dllimport)
#elif defined(__GNUC__)
    #define EXPORT_ATTR __attribute__((visibility("default")))
    #define IMPORT_ATTR
#else
    #define EXPORT_ATTR
    #define IMPORT_ATTR
    #error Compiling for unknown platform.
#endif

// Remove some Windows annoyance.
#if defined(_WIN32)
    #define _CRT_SECURE_NO_WARNINGS
    #define WIN32_LEAN_AND_MEAN
#endif

// Build information.
namespace Build
{
    constexpr bool is64bit = sizeof(void *) == 8;
    constexpr const char *Modulename = MODULENAME;

}
