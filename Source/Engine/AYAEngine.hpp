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
    point4_t Dimensions{};
    vec4_t Margins{};
    struct
    {
        unsigned char Hoover : 1;
        unsigned char Clicked : 1;
        unsigned char Fixedwidth : 1;
        unsigned char Fixedheight : 1;
        unsigned char Reserved : 4;
    } Properties{};

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
                // Margins relative to the edges.
                Child->Dimensions.x0 = int16_t(std::round(Target->Dimensions.x0 + DeltaX * Child->Margins.x0));
                Child->Dimensions.y0 = int16_t(std::round(Target->Dimensions.y0 + DeltaY * Child->Margins.y0));
                Child->Dimensions.x1 = int16_t(std::round(Target->Dimensions.x1 - DeltaX * Child->Margins.x1));
                Child->Dimensions.y1 = int16_t(std::round(Target->Dimensions.y1 - DeltaY * Child->Margins.y1));

                // Margins relative to (0, 0).
                if (Child->Properties.Fixedwidth)
                {
                    Child->Dimensions.x0 = int16_t(std::round(Target->Dimensions.x0 + Child->Margins.x0));
                    Child->Dimensions.x1 = int16_t(std::round(Target->Dimensions.x0 + Child->Margins.x1));
                }
                if (Child->Properties.Fixedheight)
                {
                    Child->Dimensions.y0 = int16_t(std::round(Target->Dimensions.y0 + Child->Margins.y0));
                    Child->Dimensions.y1 = int16_t(std::round(Target->Dimensions.y0 + Child->Margins.y1));
                }
 
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
    Element_t(std::string &&Identity) { (void)Identity; };
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
    // Process elements, render, and present to the context.
    void onRender(const void *Context);

    // Mark a span of lines as dirty.
    void Invalidatespan(point2_t Span);
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
}
