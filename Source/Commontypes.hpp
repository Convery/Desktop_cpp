/*
    Initial author: Convery (tcn@ayria.se)
    Started: 20-06-2018
    License: MIT

    Provides simple types used throughout the components.
*/

#pragma once
#include <cstdint>

#pragma pack(push, 1)
#pragma warning(disable: 4201)

using point2_t = struct { union { struct { int16_t x, y; }; int16_t Raw[2]; }; };
using point3_t = struct { union { struct { int16_t x, y, z; }; int16_t Raw[3]; }; };
using point4_t = struct { union { struct { int16_t x, y, z, w; }; struct { int16_t x0, y0, x1, y1; }; int16_t Raw[4]; }; };

using vec2_t = struct { union { struct { float x, y; }; float Raw[2]; }; };
using vec3_t = struct { union { struct { float x, y, z; }; float Raw[3]; }; };
using vec4_t = struct { union { struct { float x, y, z, w; }; struct { float x0, y0, x1, y1; }; float Raw[4]; }; };

using pixel24_t = struct { union { struct { uint8_t R, G, B; } RGB; struct { uint8_t B, G, R; } BGR; uint8_t Raw[3]; }; };
using pixel32_t = struct { union { struct { uint8_t R, G, B, A; } RGBA; struct { uint8_t B, G, R, A; } BGRA; uint8_t Raw[4]; }; };

using rgb_t = struct { union { struct { float R, G, B; }; float Raw[3]; }; };
using rgba_t = struct { union { struct { float R, G, B, A; }; float Raw[4]; }; };

using texture_t = struct { point2_t Size; float Alpha; const void *Data; };

namespace Pixelmask
{
    using segment_t = struct { uint8_t Data[3]; };
    inline uint16_t Filled(segment_t Segment) { return ((Segment.Data[1] & 0x000F) << 8) | Segment.Data[2];  };
    inline uint16_t Blank(segment_t Segment) { return (Segment.Data[0] << 4) | ((Segment.Data[1] & 0x00F0) >> 4);  };
    inline segment_t toSegment(uint16_t Blank, uint16_t Filled)
    {
        return { uint8_t((Blank >> 4)), uint8_t((Blank | 0x00F0) << 4 | (Filled | 0x000F) >> 8), uint8_t(Filled) };
    }
};

#pragma pack(pop)
