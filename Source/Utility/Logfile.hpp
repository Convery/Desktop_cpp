/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides basic logging.
*/

#pragma once
#include "Variadicstring.hpp"
#include <string_view>
#include <cstdio>
#include <ctime>

// Output to file, assumes null-terminated strings.
inline void Logprint(std::string_view Message)
{
    // Open the logfile.
    static std::FILE *Filehandle{};
    #pragma warning(suppress: 4996)
    if (!Filehandle) Filehandle = std::fopen(va("./%s.log", Build::Modulename).c_str(), "a");

    // Append to the logfile.
    std::fputs(Message.data(), Filehandle);
    std::fputs("\n", Filehandle);
    std::fflush(Filehandle);

    // Duplicate to stderr if debugging.
    #if !defined(NDEBUG)
    {
        std::fputs(Message.data(), stderr);
        std::fputs("\n", stderr);
    }
    #endif
}

// Formatted output, [Type][Time][Message]
inline void Logformatted(std::string_view Message, char Prefix)
{
    auto Now = std::time(NULL);
    char Buffer[80]{};

    #pragma warning(suppress: 4996)
    std::strftime(Buffer, 80, "%H:%M:%S", std::localtime(&Now));
    Logprint(va("[%c][%-8s] %*s", Prefix, Buffer, static_cast<int>(Message.size()), Message.data()));
}

// Delete the log and create a new one.
inline void Clearlog()
{
    std::remove(va("./%s.log", Build::Modulename).c_str());

    // NOTE(Convery): We might not want to create a log by default.
    // Logformatted(MODULENAME " - Starting up..", 'I');
}
