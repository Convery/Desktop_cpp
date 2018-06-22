/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#pragma once
#include "../Stdinclude.hpp"

// A virtual element that's the core of all menus.
// The state is largely for the system, not user-code.
struct Element_t
{
    vec4_t Margin{};
    vec4_t Renderbox{};
    vec4_t Boundingbox{};
    vec4_t Worlddimensions{};
    vec4_t Renderdimensions{};
    std::string Identifier;
    struct
    {
        unsigned int Hidden : 1;
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Noinput : 1;
        unsigned int Reserved : 4;
    } State{};
    rgba_t Backgroundcolor{};
    std::vector<Element_t *> Children{};

    // Callbacks on user-interaction, returns if the event is handled.
    std::function<bool (Element_t *Caller, bool Released)> onClicked;
    std::function<bool (Element_t *Caller, bool Released)> onHoover;
    std::function<void(Element_t *Caller)> onModifiedstate;
    std::function<void(Element_t *Caller)> onRender;

    // Inlined modifiers.
    void Show() { State.Hidden = false; }
    void Hide() { State.Hidden = true; }

    // Elements require some sort of identifier.
    Element_t(std::string Identifier);
};

namespace Rendering
{
    // Internal rendering-resolution and format to use.
    constexpr vec2_t Resolution{ 1280, 720 };
    using Pixel_t = pixel24_t;
    extern Pixel_t *Canvas;

    // Internal state-access.
    void Invalidatearea(const vec4_t Area);
    void setClipping(const vec4_t Area);
    void onPresent(const void *Handle);
    void onRender();

    // Internal drawing, no boundary-checking.
    namespace Internal
    {
        using onDrawcallback = std::function<void(const size_t X, const size_t Y)>;

        void setPixel(const size_t X, const size_t Y, const Pixel_t Pixel, const float Alpha);
        void fillPoly(const vec2_t *Vertices, const size_t Count, onDrawcallback Callback);
        Pixel_t blendPixel(const Pixel_t Base, const Pixel_t Overlay, const float Alpha);
        void setPixel(const size_t X, const size_t Y, const Pixel_t Pixel);
        Pixel_t fromRGBA(const rgba_t Color);
        vec4_t clipArea(const vec4_t Area);
    }

    // Basic drawing, converts the color to internal format.
    namespace Soliddraw
    {
        template <bool Filled = true> void Triangle(const rgba_t Color, const vec2_t a, const vec2_t b, const vec2_t c);
        template <bool Filled = true> void Quad(const rgba_t Color, const vec4_t Area);
        void Line(const rgba_t Color, const vec2_t Start, const vec2_t Stop);
        void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices);
        void Circle(const rgba_t Color, const vec2_t Position, float Radius);
    }
    namespace Textureddraw
    {
        template <bool Filled = true> void Triangle(const texture_t Color, const vec2_t a, const vec2_t b, const vec2_t c);
        template <bool Filled = true> void Quad(const texture_t Color, const vec4_t Area);
        void Line(const texture_t Color, const vec2_t Start, const vec2_t Stop);
        void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices);
        void Circle(const texture_t Color, const vec2_t Position, float Radius);
    }

    // Basic textures.
    namespace Texture
    {
        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2);
    }

    // Scene-management.
    namespace Scene
    {
        void Register(std::string Name, std::function<void()> onChange);    // Register elements.
        void Switch(std::string Name);                                      // Change scenes and redraw.
        Element_t *getRootelement();                                        // Get the "ui" element.
    }
}
