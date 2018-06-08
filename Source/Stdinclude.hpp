/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Provides a single include-file for all modules.
*/

#pragma once

// Our configuration-settings.
#include "Configuration/Buildsettings.hpp"
#include "Configuration/Definitions.hpp"
#include "Configuration/Macros.hpp"

// Standard-library includes.
#include <unordered_map>
#include <string_view>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>

// Platform-specific libraries.
#if defined(_WIN32)
    #include <Windows.h>
    #include <direct.h>
    #include <intrin.h>
    #undef min
    #undef max
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <dlfcn.h>
#endif

// Utility includes.
#include "Utility/Variadicstring.hpp"
#include "Utility/FNV1Hash.hpp"
#include "Utility/Logfile.hpp"
#include "Utility/Base64.hpp"

// Third-party libraries.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Our components.
#include "Menucomponents/Components.hpp"
#include "Graphicsbackend/Graphics.hpp"
#include "Application/Application.hpp"
