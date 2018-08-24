/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    auto String = va("Hello %s", "World");
    while(true) va(String, "\nIt's %llu sec since 1970", time(NULL));
*/

#pragma once
#include "../Configuration/Buildsettings.hpp"
#include <string_view>
#include <cstdarg>

inline std::string va(std::string_view Format, ...)
{
    auto Resultbuffer = std::make_unique<char[]>(Build::Utility::VABlocksize);
    std::va_list Varlist;

    // Create a new string from the arguments and truncate as needed.
    va_start(Varlist, Format);
    std::vsnprintf(Resultbuffer.get(), Build::Utility::VABlocksize, Format.data(), Varlist);
    va_end(Varlist);

    return std::move(Resultbuffer.get());
}
inline void va(std::string &Result, std::string_view Format, ...)
{
    auto Resultbuffer = std::make_unique<char[]>(Build::Utility::VABlocksize);
    std::va_list Varlist;

    // Create a new string from the arguments and truncate as needed.
    va_start(Varlist, Format);
    std::vsnprintf(Resultbuffer.get(), Build::Utility::VABlocksize, Format.data(), Varlist);
    va_end(Varlist);

    Result.append(Resultbuffer.get());
}
