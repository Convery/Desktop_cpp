/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides a configuration for the current build.
*/

#pragma once
#include <cstdint>

// Fixup for Visual Studio 2015 no longer defining this.
#if !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

namespace Build
{
    // Architecture settings.
    #if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
    constexpr bool is64bit = true;
    #else
    constexpr bool is64bit = false;
    #endif

    // Used for logging and filestorage.
    constexpr const char *Modulename = "Desktop_cpp";

    // Debugging settings.
    namespace Debug
    {
        #if defined(NDEBUG)
        constexpr bool isDebugging = false;
        #else
        constexpr bool isDebugging = true;
        #endif
    }

    // Utility settings.
    namespace Utility
    {
        constexpr size_t VABlocksize = 2048;
    }
}
