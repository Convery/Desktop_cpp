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
    // State so callbacks only fire when changed.
    struct
    {
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Reserved : 6;
    } Elementstate{};
    point4_t Dimensions{};
    vec4_t Margins{};

    // The children inherit the parents dimensions - margins.
    std::vector<Element_t *> Childelements{};
    void addChild(Element_t *Child)
    {
        std::function<void(Element_t *)> Recalc = [&Recalc](Element_t *Target) -> void
        {
            double DeltaX = std::abs(Target->Dimensions.x1 - Target->Dimensions.x0) / 2;
            double DeltaY = std::abs(Target->Dimensions.y1 - Target->Dimensions.y0) / 2;

            for (auto &Child : Target->Childelements)
            {
                Child->Dimensions.x0 = int16_t(std::round(Target->Dimensions.x0 + DeltaX * Child->Margins.x0));
                Child->Dimensions.x1 = int16_t(std::round(Target->Dimensions.x1 - DeltaX * Child->Margins.x1));
                Child->Dimensions.y0 = int16_t(std::round(Target->Dimensions.y0 + DeltaY * Child->Margins.y0));
                Child->Dimensions.y1 = int16_t(std::round(Target->Dimensions.y1 - DeltaY * Child->Margins.y1));

                Recalc(Child);
            }
        };

        Childelements.push_back(Child);
        Recalc(this);
    }

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
    extern point4_t gDisplayrectangle;
    extern Element_t *gRootelement;
    extern void *gWindowhandle;
    extern uint32_t gErrno;
}

// Window management.
namespace Engine::Window
{
    // Modify the windows visible state and notify the composition-manager.
    void Move(point2_t Position, bool Deferupdate = false);
    void Resize(point2_t Size, bool Deferupdate = false);
    void Centerwindow(bool Deferupdate = false);
    void Togglevisibility();

    // Process any window-events.
    void onFrame();
}

// Render into a framebuffer and present.
namespace Engine::Rendering
{
    extern int16_t Currentline;
    extern uint8_t *Scanline;

    // Process elements, render, and present to the context.
    void onRender(const void *Context);
}

// Draw calls for the elements that are called every frame.
namespace Engine::Rendering::Draw
{
    #if 0

    // Outline == if we should only draw the outermost pixels for the specified shape.
    template <bool Outline = false> void Circle(const texture_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Circle(const rgba_t Color, const point2_t Position, const float Radius);
    template <bool Outline = false> void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices);
    template <bool Outline = false> void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices);
    template <bool Outline = false> void Quad(const texture_t Color, const point4_t Area);
    template <bool Outline = false> void Quad(const rgba_t Color, const point4_t Area);
    void Line(const texture_t Color, const point2_t Start, const point2_t Stop);
    void Line(const rgba_t Color, const point2_t Start, const point2_t Stop);

    // Special drawing.
    void PNG(const unsigned char *Data, const size_t Size, const point4_t Area);
    void PNGFile(const std::string &&Filename, const point4_t Area);

    #endif
}
namespace Draw = Engine::Rendering::Draw;

// Manage the scenes and compositing.
namespace Engine::Compositing
{





    // Recalculate all elements dimensions for when the window changes.
    void Recalculateroot();

    // Remove the old root and recreate, or just append.
    void Switchscene(std::string &&Name);
    void Appendscene(std::string &&Name);

    // Register callbacks for scene-creation.
    void Registercomposer(std::string &&Name, std::function<void(Element_t *Target)> Callback);
}
