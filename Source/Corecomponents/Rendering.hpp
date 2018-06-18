/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#pragma once
#include "Input.hpp"
#include <functional>
#include <cstdint>
#include <string>
#include <vector>

// Some common types.
using vec3_t = struct { double x, y, z; };
using rgba_t = struct { double r, g, b, a; };
using rect_t = struct { double x0, y0, x1, y1; };

// A virtual element that's the core of all menus.
// The state is largely for the system, not user-code.
struct Element_t
{
    rect_t Margin{};
    rect_t Renderbox{};
    rect_t Boundingbox{};
    rect_t Worlddimensions{};
    rect_t Renderdimensions{};
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

    // Recalculate the properties and render them.
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
    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle);
    Element_t *getRootelement();
    vec2_t getResolution();
    void onRender();

    // User-code interaction.
    namespace Draw
    {
        void Quad(rgba_t Color, rect_t Box);
        void Line(rgba_t Color, rect_t Box);
        void Border(rgba_t Color, rect_t Box);

        void Quadgradient(std::vector<rgba_t> Colors, rect_t Box);
        void Linegradient(std::vector<rgba_t> Colors, rect_t Box);
        void Bordergradient(std::vector<rgba_t> Colors, rect_t Box);
    }
    std::vector<rgba_t> Creategradient(size_t Steps, rgba_t Color1, rgba_t Color2);

    // Scene-management.
    namespace Menu
    {
        void Register(std::string Name, std::function<void()> onChange);
        void Switch(std::string Name);
    }
}
