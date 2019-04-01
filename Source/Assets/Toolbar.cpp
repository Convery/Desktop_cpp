/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

inline void Createbuttons()
{
    static auto Closebutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Closebutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 1.0, 0.0 ]" });
    Closebutton->onRender = [](const vec4_t Viewport) -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0x00, 0xED, 0xFF, 1 });
    };
    Closebutton->isExclusive = [](Elementstate_t State) -> bool
    {
        return State.isLeftclicked;
    };
    Closebutton->onStatechange = [](Elementstate_t State) -> void
    {
        if(State.isLeftclicked && !State.isHoveredover && Closebutton->State.isLeftclicked && Closebutton->State.isHoveredover)
        {
            Global.Errorno = Hash::FNV1a_32("Toolbar.Closebutton");
        }
    };
    Composition::Registerelement("Toolbar.Closebutton", Closebutton);

    static auto Maxbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Maxbutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 0.972, 0.0 ]" });
    Maxbutton->onRender = [](const vec4_t Viewport) -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Maxbutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Maxbutton", Maxbutton);

    static auto Minbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Minbutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 0.945, 0.0 ]" });
    Minbutton->onRender = [](const vec4_t Viewport) -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Minbutton), { 0xFF, 0x00, 0xFF, 1 });
    };
    Composition::Registerelement("Toolbar.Minbutton", Minbutton);
}
static void Createtoolbar()
{
    // Root-level component.
    static auto Toolbar = Global.Rootelement->Children.emplace_back(std::make_shared<Element_t>());
    Toolbar->Properties.push_back({ "Margins", "[0.0, 0.944, 0.0, 0.0 ]" });

    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double)
    {
        static vec2_t Previous{};

        if (Toolbar->State.isLeftclicked)
        {
            auto Windowpos{ Global.Windowposition };
            const auto Current{ Global.Mouse.Position };
            if (Previous.x + Previous.y == 0) Previous = Current;

            Windowpos.x += (Current.x - Previous.x);
            Windowpos.y += (Current.y - Previous.y);
            Window::Move(Windowpos);
            Previous = Current;
        }
        else
        {
            Previous = {};
        }
    });
    Toolbar->onRender = [](const vec4_t Viewport) -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Toolbar), { 0x11, 0x0f, 0x0c, 1 });
    };
    Composition::Registerelement("Toolbar", Toolbar);

    Createbuttons();
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createtoolbar); } }; static Startup Loader{}; }
