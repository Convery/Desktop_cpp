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

// The element-state is updated remotely.
using Elementstate_t = union { struct {
    unsigned char
    isHoveredover : 1,
    isLeftclicked : 1,
    isRightclicked : 1,
    isMiddleclicked : 1;
}; unsigned char Raw; };

// Element optimized for size.
struct Element_t
{
    // Evaluated properties.
    vec2_t Position, Size{};
    Elementstate_t State{};

    // We keep all properties as readable JSON. Vector is 16 bytes, unordered_map is 40.
    std::vector <std::pair<std::string, std::string>> Properties{};

    // We generally only have 1 child, inline it (12 bytes).
    absl::InlinedVector<std::shared_ptr<Element_t>, 1> Children{};

    // Note(tcn): std::function takes up too much memory to be used here.
    // Callbacks triggered from the engine if they are implemented.
    void (*onStatechange)(const Elementstate_t State) {};
    bool (*isExclusive)(const Elementstate_t State) {};
    void (*onRender)() {};

    /*
        NOTE(tcn): 7 bytes left on the line here.
    */
};
inline vec4_t Elementbox(const std::shared_ptr<Element_t> Item)
{
    return { Item->Position.x, Item->Position.y, Item->Position.x + Item->Size.x, Item->Position.y + Item->Size.y };
}

// Global state for storage on a line.
struct
#if defined(NDEBUG)
    alignas(64)
#endif
    Globalstate_t
{
    // Most commonly accessed property.
    uint32_t Errorno;

    // Note(tcn): Input information is only updated once per frame (~16ms), elements are notified about all events for the frame automatically.
    using Keyboardflags_t = struct { uint16_t isKeydown : 1, isKeyrepeat : 1, isKeyup : 1, ALTMod : 1, CTRLMod : 1, SHIFTMod : 1, CaretMod : 1, OumlatMod : 1, TildeMod : 1; };
    using Mouseflags_t    = struct { uint16_t isRightdown : 1, isLeftdown : 1, isMiddledown : 1; };
    struct { uint16_t Keycode; Keyboardflags_t Flags; } Keyboard;
    struct { vec2_t Position; Mouseflags_t Flags; } Mouse;

    // The current implementation only has a single window, create your own class for sub-windows.
    std::unique_ptr<Gdiplus::Graphics> Drawingcontext;
    std::unique_ptr<Element_t> Rootelement;
    const void *Windowhandle;
    vec2_t Windowposition;
    vec2_t Windowsize;
    bool Dirtyframe;

    /*
        NOTE(tcn): 17 free bytes here, use them or lose them.
    */
};
extern Globalstate_t Global;
inline void Invalidatewindow()
{
    Global.Dirtyframe = true;
}

// Events that elements (or anyone; really) can subscribe to.
namespace Events
{
    // A simple event-stack until we have something better.
    template<typename Event, typename... Callbacks> struct Eventstack_t
    {
        std::tuple<std::vector<std::function<Callbacks>>...> Internalstack;

        // Sanity-checking if we ever re-use this shit..
        static_assert(std::is_enum_v<Event>, "Eventstack error 1");
        static_assert(size_t(Event::MAX) != 0, "Eventstack error 2");
        static_assert(sizeof...(Callbacks) == size_t(Event::MAX), "Eventstack error 3");

        template<Event e, typename Function> const void Subscribe(Function &&Callback)
        {
            std::get<size_t(e)>(Internalstack).push_back(std::forward<Function>(Callback));
        }
        template<Event e, typename... Args>  const void Execute(const Args&... Arguments)
        {
            static_assert(std::is_invocable_v<typename std::tuple_element_t<size_t(e), decltype(Internalstack)>::value_type, Args...>, "Eventstack error 4");
            for (const auto &Item : std::get<size_t(e)>(Internalstack)) std::invoke(Item, Arguments...);
        }
    };

    // A separate stack for each category.
    enum class Engineevent { PAINT, PRESENT, TICK, STARTUP, TERMINATION, MAX };
    extern Eventstack_t<Engineevent, void(void), void(void), void(double), void(void), void(void)> *Enginestack;
    /* TODO(tcn): Add more stacks here */

    // Defines to ensure safe operations.
    #define Validatestack(Stack) { if(!Stack) Stack = new std::remove_pointer_t<decltype(Stack)>(); assert(Stack); }
    #define Subscribetostack(Stack, Event, Callback) { Validatestack(Stack); Stack->Subscribe<Event>(Callback); }
    #define Executeevent(Stack, Event, ...) { Stack->Execute<Event>(__VA_ARGS__); }
}

// Basic windowing operations.
namespace Window
{
    // Argument can be in pixels or percentage, with (<= 1.0).
    void Move(vec2_t Newposition, bool Deferredraw = false);
    void Resize(vec2_t Newsize, bool Deferredraw = false);
    void Togglevisibility();
    void Forceredraw();

    // Moved to Inputhandling.cpp
    void Processmessages();
}

//
namespace Composition
{
    // Load the settings from JSON.
    bool ParseJSON(const std::string_view JSON);

    // Add a name for an element (mostly for debugging).
    void Registerelement(std::string Name, std::shared_ptr<Element_t> Element);
    std::shared_ptr<Element_t> Getelement(std::string Name);
}

// Wrappers for converting code to pretty colors.
namespace Rendering
{
    void Buildcallgraph(Element_t *Node);
    void Clearcallgraph();
    void Renderframe();

    namespace Gradient
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Fillpolygon(std::vector<vec2_t> &&Points, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Line(vec2_t Start, vec2_t Stop, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Outlinerectangle(vec4_t Region, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Fillrectangle(vec4_t Region, rgba_t Color1, rgba_t Color2, uint32_t Steps);
    }

    namespace Textured
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, vec2_t Anchor, Texture32_t Texture);
        void Fillpolygon(std::vector<vec2_t> &&Points, vec2_t Anchor, Texture32_t Texture);
        void Line(vec2_t Start, vec2_t Stop, vec2_t Anchor, Texture32_t Texture);
        void Outlinerectangle(vec4_t Region, vec2_t Anchor, Texture32_t Texture);
        void Fillrectangle(vec4_t Region, vec2_t Anchor, Texture32_t Texture);
    }

    namespace Solid
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, rgba_t Color);
        void Fillpolygon(std::vector<vec2_t> &&Points, rgba_t Color);
        void Line(vec2_t Start, vec2_t Stop, rgba_t Color);
        void Outlinerectangle(vec4_t Region, rgba_t Color);
        void Fillrectangle(vec4_t Region, rgba_t Color);
    }
}

// Re-enable padding.
#pragma pack(pop)
