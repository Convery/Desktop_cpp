/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createbuttons()
{
    static auto Closebutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Closebutton->Properties.push_back({ "Margins", "[ 0.97, 0.5, 1.0, 0.0 ]" });
    Closebutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0xFF, 0x00, 0x00, 0.6 });
        if (Closebutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0xFF, 0xFF, 0xFF, 0.3 });
        if (Closebutton->State.isLeftclicked) Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0xFF, 0xFF, 0xFF, 0.3 });

        // TODO(tcn): Render texture / font 'X'
    };
    Closebutton->isExclusive = [](Elementstate_t State) -> bool
    {
        return State.isLeftclicked;
    };
    Closebutton->onStatechange = [](Elementstate_t State) -> void
    {
        if (State.isLeftclicked && !State.isHoveredover && Closebutton->State.isLeftclicked && Closebutton->State.isHoveredover)
        {
            Global.Errorno = Hash::FNV1a_32("Toolbar.Closebutton");
        }

        // Invalidate the area if hovering changed or clicked.
        if (State.isLeftclicked || State.isHoveredover) Invalidatewindow();
    };
    Composition::Registerelement("Toolbar.Closebutton", Closebutton);

    static auto Maxbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Maxbutton->Properties.push_back({ "Margins", "[ 0.97, 0.5, 0.9685, 0.0 ]" });
    Maxbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Maxbutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Maxbutton", Maxbutton);

    static auto Minbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Minbutton->Properties.push_back({ "Margins", "[ 0.97, 0.5, 0.9365, 0.0 ]" });
    Minbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Minbutton), { 0xFF, 0x00, 0xFF, 1 });
    };
    Composition::Registerelement("Toolbar.Minbutton", Minbutton);
}
static void Createtoolbar()
{
    static bool shouldMove = false;
    static auto Toolbar = Composition::Getelement("Toolbar");
    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double) -> void
    {
        static vec2_t Previous{};

        if (shouldMove)
        {
            POINT Workaround{};
            GetCursorPos(&Workaround);
            auto Windowpos{ Global.Windowposition };
            const vec2_t Current = instantiate(vec2_t, $.x = (float)Workaround.x, $.y = (float)Workaround.y);
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
    Toolbar->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Toolbar), { 0x11, 0x0f, 0x0c, 1 });
    };
    Toolbar->onStatechange = [](const Elementstate_t State) -> void
    {
        if (State.isLeftclicked) shouldMove ^= 1;
    };

    Createbuttons();
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createtoolbar); } }; static Startup Loader{}; }
