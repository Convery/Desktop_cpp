/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides a single include-file for all modules.
*/

#pragma once

// Our configuration-settings.
#include "Configuration/Buildsettings.hpp"
#include "Configuration/Definitions.hpp"
#include "Configuration/Macros.hpp"

// Ignore warnings from third-party code.
#pragma warning(push, 0)

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
#include <atomic>

// Platform-specific libraries.
#if defined(_WIN32)
    #include <Windowsx.h>
    #include <Windows.h>
    #include <direct.h>
    #include <intrin.h>
    #include <ObjIdl.h>
    #include <gdiplus.h>
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

// Restore warnings.
#pragma warning(pop)

// Utility includes.
#include "Utility/Variadicstring.hpp"
#include "Utility/FNV1Hash.hpp"
#include "Utility/Logfile.hpp"
#include "Utility/Base64.hpp"

// Our components.
#include "Commontypes.hpp"
//#include "Corecomponents/Input.hpp"
//#include "Corecomponents/Rendering.hpp"
