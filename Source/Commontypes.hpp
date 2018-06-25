/*
    Initial author: Convery (tcn@ayria.se)
    Started: 20-06-2018
    License: MIT

    Provides simple types used throughout the components.
*/

#pragma once
#include <cstdint>

#pragma pack(1)
#pragma warning(disable: 4201)

using point2_t = struct { union { struct { uint16_t x, y; }; uint16_t Raw[2]; }; };
using point3_t = struct { union { struct { uint16_t x, y, z; }; uint16_t Raw[3]; }; };
using point4_t = struct { union { struct { uint16_t x, y, z, w; }; struct { uint16_t x0, y0, x1, y1; }; uint16_t Raw[4]; }; };

using vec2_t = struct { union { struct { float x, y; }; float Raw[2]; }; };
using vec3_t = struct { union { struct { float x, y, z; }; float Raw[3]; }; };
using vec4_t = struct { union { struct { float x, y, z, w; }; struct { float x0, y0, x1, y1; }; float Raw[4]; }; };

using pixel24_t = struct { union { struct { uint8_t R, G, B; } RGB; struct { uint8_t B, G, R; } BGR; uint8_t Raw[3]; }; };
using pixel32_t = struct { union { struct { uint8_t R, G, B, A; } RGBA; struct { uint8_t B, G, R, A; } BGRA; uint8_t Raw[4]; }; };

using rgb_t = struct { union { struct { float R, G, B; }; float Raw[3]; }; };
using rgba_t = struct { union { struct { float R, G, B, A; }; float Raw[4]; }; };

#pragma pack()
