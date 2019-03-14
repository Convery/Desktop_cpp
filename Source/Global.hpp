/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#pragma once
#include "Stdinclude.hpp"

// Disable padding.
#pragma pack(push, 1)

// Vertex and sub-pixel coordinate-system, could probably go down to half-precision floats.
using vec4_t = struct { union { struct { float x, y, z, w; }; struct { float x0, y0, x1, y1; }; float Raw[4]; }; };
using vec3_t = struct { union { struct { float x, y, z; }; float Raw[3]; }; };
using vec2_t = struct { union { struct { float x, y; }; float Raw[2]; }; };

// In-flight representation, will be optimized out (probably), should still be aligned to 16 bytes.
using rgba_t = struct { union { struct { float R, G, B, A; }; float Raw[4]; }; };
using rgb_t = struct { union { struct { float R, G, B; }; float Raw[3]; }; };

// By platform format for the pixels.
using pixel32_t = struct { union { struct { uint8_t R, G, B, A; } RGBA; struct { uint8_t B, G, R, A; } BGRA; uint8_t Raw[4]; }; };
using pixel24_t = struct { union { struct { uint8_t R, G, B; } RGB; struct { uint8_t B, G, R; } BGR; uint8_t Raw[3]; }; };

// Simple textures.
using Texture32_t = struct { vec2_t Size; pixel32_t *Data; };
using Texture24_t = struct { vec2_t Size; pixel24_t *Data; };

// Global state for storage on a line.
struct /*alignas(64)*/ Globalstate_t
{
    // Most commonly accessed property.
    uint32_t Errorno;

    // Note(tcn): Input information is only updated once per frame (~16ms), elements are notified about all events for the frame automatically.
    using Keyboardflags_t = struct { uint16_t Keydown : 1, Keyrepeat : 1, Keyup : 1, ALTMod : 1, CTRLMod : 1, SHIFTMod : 1, CaretMod : 1, OumlatMod : 1, TildeMod : 1; };
    using Mouseflags_t    = struct { uint16_t Rightdown : 1, Leftdown : 1, Middledown : 1; };
    struct { uint16_t Keycode; Keyboardflags_t Flags; } Keyboard;
    struct { vec2_t Position; Mouseflags_t Flags; } Mouse;

    // The current implementation only has a single window, create your own class for sub-windows.
    std::unique_ptr<Gdiplus::Graphics> Drawingcontext;
    const void *Windowhandle;
    vec2_t Windowposition;
    vec4_t Dirtyregion;

    /*
        TODO(tcn):
        14 free bytes here, use them or lose them.
    */
};
extern Globalstate_t Global;
constexpr size_t Bytesleft = 64 - sizeof(Globalstate_t);

// Events that panels (or anyone; really) can subscribe to.
namespace Events
{
    template <typename E> uint32_t Subscribe(E Event, std::function<void(void *Param)> Callback);
    template <typename E> void Unsubscribe(E Event, uint32_t Identifier);

    enum class Engineevent { PAINT, PRESENT, TICK, STARTUP, TERMINATION };
}

// The element-state is updated remotely.
using Elementstate_t = union { struct {
    unsigned char
    isHoovered : 1,
    hasCallbacks : 1,
    isLeftclicked : 1,
    isRightclicked : 1,
    isMiddleclicked : 1;
}; unsigned char Raw; };
struct Element_t
{
    // Evaluated properties.
    vec2_t Position, Size{};
    Elementstate_t State{};
    uint32_t TransformID{};

    // Keep all properties readable.
    absl::flat_hash_map<std::string, std::string> Properties{};

    // Child-elements, generally only a single one.
    absl::InlinedVector<Element_t *, 1> Children{};
};

// Callbacks for element-scripts.
namespace Commands
{
    using Callback_t = std::function<void(Element_t *Caller, std::vector<std::string_view> Arguments)>;
    void Add(std::string Name, Callback_t Function);
    Callback_t *Find(std::string_view Name);
}

// Re-enable padding.
#pragma pack(pop)
