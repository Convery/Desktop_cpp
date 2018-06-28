/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides an engine system to be platform independent.
*/

#pragma once
#include "../Stdinclude.hpp"

// Will represent an object in the compositions.
#pragma pack(push, 1)
struct Element_t
{
    // The dimensions of the element.
    point4_t Localbox{};
    point4_t Worldbox{};
    vec4_t Margin{};

    // State so we only update when needed.
    struct
    {
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Reserved : 6;
    } State{};

    // Children inherit the parents worldbox.
    std::vector<Element_t *> Children{};

    // Callbacks on user-interaction, returns if the event is handled.
    std::function<void(Element_t *Caller, double Deltatime)> onFrame;
    std::function<bool(Element_t *Caller, bool Released)> onHoover;
    std::function<bool(Element_t *Caller, bool Released)> onClick;

    #if !defined(NDEBUG)
    std::string Identity;
    Element_t(std::string ID) : Identity(ID) {}
    #else
    Element_t(std::string ID) {}
    #endif
};
#pragma pack(pop)

namespace Engine
{
    // Global engine-variables.
    extern point2_t gWindowsize, gWindowposition;
    extern point2_t gRenderingresolution;
    extern point4_t gDisplayrectangle;
    extern Element_t *gRootelement;
    extern void  *gWindowhandle;

    // Main-loop for the application, returns error.
    bool doFrame(double Deltatime);

    // Manage the window area.
    namespace Window
    {
        // Create a new window centered and switch focus.
        bool Create(point2_t Windowsize);

        // Modify the window.
        void Move(point2_t Position);
        void Resize(point2_t Size);
        void Togglevisibility();
    }

    // Manage the compositions and assets.
    namespace Compositions
    {
        // Read the layout from disk.
        void Parseblueprint();

        // Switch focus to another composition.
        void Switch(const std::string &&Name);

        // Dump an asset if available.
        std::string *Findasset(const std::string &&Name);

        // Register callbacks.
        void addFrame(const std::string &&Name, std::function<void(Element_t *Caller, double Deltatime)> Callback);
        void addHoover(const std::string &&Name, std::function<bool(Element_t *Caller, bool Released)> Callback);
        void addClick(const std::string &&Name, std::function<bool(Element_t *Caller, bool Released)> Callback);

        // We need to update all coordinates when the window changes.
        void onWindowchange();
    }

    // Get the compositions to the screen.
    namespace Rendering
    {
        // Create and invalidate part of a framebuffer.
        void Createframebuffer(point2_t Size);
        void Invalidatearea(point4_t Area);

        // Callback on when to process elements.
        void onPresent(const void *Context);
        void onRender();

        // Basic drawing.
        namespace Draw
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
    }
}
