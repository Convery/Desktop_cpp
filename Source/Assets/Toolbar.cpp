/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createtoolbar()
{
    // Root-level component.
    static auto Element = Global.Rootelement->Children.emplace_back(std::make_shared<Element_t>());
    Element->Properties.push_back({ "Margins", "[0.0, 0.944, 0.0, 0.0 ]" });

    Element->onRender = [](const vec4_t Viewport) -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Element), { 0x11, 0x0f, 0x0c, 1 });
    };
    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double)
    {
        static vec2_t Previous{};

        if (Element->State.isLeftclicked)
        {
            if (Previous.x + Previous.y == 0) Previous = Global.Mouse.Position;

            Window::Move(
            {
                Global.Windowposition.x + Global.Mouse.Position.x - Previous.x,
                Global.Windowposition.y + Global.Mouse.Position.y - Previous.y
            }, false);

            Previous = Global.Mouse.Position;
        }
        else
        {
            Previous = {};
        }
    });

    Composition::Registerelement("Toolbar", Element);
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createtoolbar); } }; static Startup Loader{}; }
