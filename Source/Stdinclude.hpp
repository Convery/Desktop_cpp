/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#pragma once

// Our configuration-settings.
#include "Config.hpp"

// Ignore warnings from third-party code.
#pragma warning(push, 0)

// Standard-library includes.
#include <unordered_map>
#include <string_view>
#include <functional>
#include <algorithm>
#include <typeindex>
#include <typeinfo>
#include <cassert>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <queue>
#include <any>

// Platform-specific libraries.
#if defined(_WIN32)
#include <Windows.h>
#include <shlwapi.h>
#include <ObjIdl.h>
#include <gdiplus.h>
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

// Third-party includes.
#include <absl/container/inlined_vector.h>
#include <parallel_hashmap/phmap.h>
#include <nlohmann/json.hpp>

// Restore warnings.
#pragma warning(pop)

// Utility includes.
#include "Utilities/Variadicstring.hpp"
#include "Utilities/FNV1Hash.hpp"
#include "Utilities/Logging.hpp"
#include "Utilities/Archive.hpp"
#include "Utilities/Base64.hpp"

// Our components.
#include "Global.hpp"
