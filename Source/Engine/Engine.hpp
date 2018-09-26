/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-08-2018
    License: MIT

    Provides a platform abstraction for the application core.
*/

#pragma once
#include "../Stdinclude.hpp"

#pragma region Datatypes
#pragma pack(push, 1)
#pragma warning(disable: 4201)

// Primary coordinate-system.
using point3_t = struct { union { struct { int16_t x, y, z; }; int16_t Raw[3]; }; };
using point2_t = struct { union { struct { int16_t x, y; }; struct { int16_t first, second; }; int16_t Raw[2]; }; };
using point4_t = struct { union { struct { int16_t x, y, z, w; }; struct { int16_t x0, y0, x1, y1; }; int16_t Raw[4]; }; };

// Vertice and sub-pixel coordinate-system.
using vec2_t = struct { union { struct { float x, y; }; float Raw[2]; }; };
using vec3_t = struct { union { struct { float x, y, z; }; float Raw[3]; }; };
using vec4_t = struct { union { struct { float x, y, z, w; }; struct { float x0, y0, x1, y1; }; float Raw[4]; }; };

// By platform format for the pixels.
using pixel8_t  = struct { uint8_t PalletID; };
using pixel24_t = struct { union { struct { uint8_t R, G, B; } RGB; struct { uint8_t B, G, R; } BGR; uint8_t Raw[3]; }; };
using pixel32_t = struct { union { struct { uint8_t R, G, B, A; } RGBA; struct { uint8_t B, G, R, A; } BGRA; uint8_t Raw[4]; }; };

// In-flight representation, will be optimized out (probably).
using rgb_t = struct { union { struct { float R, G, B; }; float Raw[3]; }; };
using rgba_t = struct { union { struct { float R, G, B, A; }; float Raw[4]; }; };

// Textures, images, pre-rendered anything that should not be modified at runtime.
using texture_t = struct { const point2_t Dimensions; uint8_t Pixelsize; const uint8_t *Data; };

// Element state for IO notifications.
using elementstate_t = struct { unsigned char Focused : 1, Clicked : 1, ExclusiveIO : 1; };

#pragma pack(pop)
#pragma endregion

// All menu-components should derive from this base element.
struct Element_t
{
    /*
        TODO(Convery):
        We need to investigate this fuckery.
        sizeof(std::function<void()>) == 40
        sizeof(std::function<void>) == 1

        Maybe:
        using Callback = void(Element_t *Caller, elementstate_t Newstate);
        Callback *onStatechange;
    */

    elementstate_t Properties{};
    point4_t Dimensions{};
    vec4_t Margins{};

    // The children inherit the parents dimensions - margins.
    std::vector<Element_t *> Childelements{};
    void addChild(Element_t *Child)
    {
        std::function<void(Element_t *)> Recalculate = [&](Element_t *Target) -> void
        {
            const double DeltaX = std::abs(Target->Dimensions.x1 - Target->Dimensions.x0);
            const double DeltaY = std::abs(Target->Dimensions.y1 - Target->Dimensions.y0);

            for (auto &Child : Target->Childelements)
            {
                // Margins are given in pixels.
                if (std::abs(Child->Margins.x0) > 1.0) Child->Dimensions.x0 = int16_t(Target->Dimensions.x0 + std::round(Child->Margins.x0));
                if (std::abs(Child->Margins.y0) > 1.0) Child->Dimensions.y0 = int16_t(Target->Dimensions.y0 + std::round(Child->Margins.y0));
                if (std::abs(Child->Margins.x1) > 1.0) Child->Dimensions.x1 = int16_t(Target->Dimensions.x1 - std::round(Child->Margins.x1));
                if (std::abs(Child->Margins.y1) > 1.0) Child->Dimensions.y1 = int16_t(Target->Dimensions.y1 - std::round(Child->Margins.y1));

                // Margins are given in percentages.
                if (Child->Margins.x0 <= 1.0) Child->Dimensions.x0 = int16_t(std::round(Target->Dimensions.x0 + DeltaX * Child->Margins.x0));
                if (Child->Margins.y0 <= 1.0) Child->Dimensions.y0 = int16_t(std::round(Target->Dimensions.y0 + DeltaY * Child->Margins.y0));
                if (Child->Margins.x1 <= 1.0) Child->Dimensions.x1 = int16_t(std::round(Target->Dimensions.x1 - DeltaX * Child->Margins.x1));
                if (Child->Margins.y1 <= 1.0) Child->Dimensions.y1 = int16_t(std::round(Target->Dimensions.y1 - DeltaY * Child->Margins.y1));

                // Recurse into the sub-elements.
                Recalculate(Child);
            }
        };
        Childelements.push_back(Child);
        Recalculate(this);
    }

    // Callbacks on user-interaction, exclusive state-changes are consumed by the called element.
    std::function<void(const Element_t *Caller, const elementstate_t Newstate)> onStatechange{};
    std::function<bool(const Element_t *Caller, const elementstate_t Newstate)> isExclusive{};

    // Callbacks from the engine for every frame.
    std::function<void(const Element_t *Caller, double Deltatime)> onFrame{};
    std::function<void(const Element_t *Caller)> onRender{};

    // While debugging, add an identifier.
#if !defined(NDEBUG)
    std::string Elementname;
    Element_t(std::string &&Identity) : Elementname(Identity) {};
#else
    Element_t(std::string &&Identity) { (void)Identity; };
#endif
};

// Core properties.
namespace Engine
{
    point2_t getWindowposition();
    void setErrno(uint32_t Code);
    point2_t getMouseposition();
    uint32_t getErrno();

    extern const void *gWindowhandle;
    extern Element_t *gRootelement;
    extern uint32_t gCurrentmenuID;
    extern point2_t gWindowsize;
}

// Window management.
namespace Engine::Window
{
    // Modify the windows visible state and notify the composition-manager.
    void Move(point2_t Position, bool Deferupdate = false);
    void Resize(point2_t Size, bool Deferupdate = false);
    void Centerwindow(bool Deferupdate = false);
    void Togglevisibility();
}

// Render into a scanline and present.
namespace Engine::Rendering
{
    // onWindowresize().
    void Recalculatebuffers();

    // MArk a region as dirty.
    void Invalidateregion(const point4_t Area);

    // Process elements, render, and present to the context.
    void onFrame();
}

// Draw calls for the elements that are called every frame.
namespace Engine::Rendering::Draw
{
    // Outline == if we should only draw the outermost pixels for the specified shape.
    template <bool Outline = false> void Circle(const texture_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Circle(const rgba_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Quad(const texture_t Color, const point4_t Area);
    template <bool Outline = false> void Quad(const rgba_t Color, const point4_t Area);
}
namespace Draw = Engine::Rendering::Draw;

// Manage the scenes and compositing.
namespace Engine::Compositing
{
    // Callbacks for composition creation.
    void Registercomposer(std::string &&Name, std::function<void(Element_t *)> Callback);

    // Recreate the root element with another focus.
    void Switchcomposition(std::string &&Name);

    // Recalculate the elements dimensions.
    void Recalculate();

    // Notify the elements about a tick.
    void onFrame(const double Deltatime);
}

// Forward user input to the elements.
namespace Engine::Input
{
    // Process any and all system-events.
    void onFrame();
}
