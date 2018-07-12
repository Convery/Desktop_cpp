/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Provides a platform abstraction for the application.
*/

#pragma once
#include "../Stdinclude.hpp"

// All components derive from the base element.
struct Element_t
{
    // Dimensions relative to the renderer and screen.
    point4_t Worldbox, Localbox;
    vec4_t Bordermargin;

    // State so callbacks only fire when changed.
    struct
    {
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Reserved : 6;
    } Elementstate;

    // The children inherit the parents Worldbox.
    std::vector<Element_t *> Childelements;
    void addChild(Element_t *Child);

    // Callbacks on user-interaction, returns if the event was consumed.
    std::function<bool(Element_t *Caller, bool Released)> onClicked;
    std::function<bool(Element_t *Caller, bool Released)> onHoover;

    // Update the state and draw, the renderer will do any needed culling.
    std::function<void(Element_t *Caller, double Deltatime)> onFrame;

    // Debugging helpers.
    #if !defined(NDEBUG)
    std::string Elementname;
    Element_t(std::string &&Identity) : Elementname(Identity) {};
    #else
    Element_t(std::string &&Identity) {};
    #endif
};

// Global engine-variables.
namespace Engine
{
    constexpr point2_t gRenderingresolution{ 1920, 1080 };
    extern point2_t gWindowsize, gWindowposition;
    extern point4_t gDisplayrectangle;
    extern Element_t *gRootelement;
    extern void  *gWindowhandle;
    extern bool gShouldquit;
    extern uint32_t gErrno;
}

// Window management.
namespace Engine::Window
{
    // Create a new window and switch the focus to it, deletes any existing.
    bool Createwindow();

    // Modify the windows visible state.
    void Move(point2_t Position);
    void Resize(point2_t Size);
    void Togglevisibility();

    // Process any window-events.
    void onFrame();
}

// Render into a framebuffer and present.
namespace Engine::Rendering
{
    // Process elements, render, and present to the context.
    void onRender(const void *Context);

    // Invalidate the area that needs to be redrawn.
    void Invalidatearea(point4_t Area);
}

// Draw calls for the elements that are called every frame.
namespace Engine::Rendering::Draw
{
    template <bool Outline = false> void Circle(const texture_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Circle(const rgba_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices);
    template <bool Outline = false> void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices);
    template <bool Outline = false> void Quad(const texture_t Color, const point4_t Area);
    template <bool Outline = false> void Quad(const rgba_t Color, const point4_t Area);
    void Line(const texture_t Color, const point2_t Start, const point2_t Stop);
    void Line(const rgba_t Color, const point2_t Start, const point2_t Stop);
}
namespace Draw = Engine::Rendering::Draw;
