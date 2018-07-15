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
    point4_t Dimensions;
    vec4_t Margins;

    // State so callbacks only fire when changed.
    struct
    {
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Reserved : 6;
    } Elementstate;

    // The children inherit the parents dimensions - margins.
    std::vector<Element_t *> Childelements;
    void addChild(Element_t *Child);

    // Callbacks on user-interaction, returns if the event was consumed.
    std::function<bool(Element_t *Caller, bool Released)> onClicked;
    std::function<bool(Element_t *Caller, bool Released)> onHoover;

    // Update the state and draw, the renderer will do any needed culling.
    std::function<void(Element_t *Caller, double Deltatime)> onFrame;
    std::function<void(Element_t *Caller)> onRender;

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
    extern point2_t gWindowsize, gWindowposition;
    extern point2_t gRenderingresolution;
    extern point4_t gDisplayrectangle;
    extern Element_t *gRootelement;
    extern void  *gWindowhandle;
    extern bool gShouldquit;
    extern uint32_t gErrno;
}

// Window management.
namespace Engine::Window
{
    // Modify the windows visible state and notify the composition-manager.
    void Move(point2_t Position, bool Deferupdate = false);
    void Resize(point2_t Size, bool Deferupdate = false);
    void Togglevisibility(bool Deferupdate = false);
    void Centerwindow(bool Deferupdate = false);

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

    // Create the framebuffer if needed.
    void Createframebuffer(const point2_t Size);
}

// Draw calls for the elements that are called every frame.
namespace Engine::Rendering::Draw
{
    // Outline == if we should only draw the outermost pixels for the specified shape.
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
