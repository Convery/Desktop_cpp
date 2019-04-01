/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

inline void Createnavigation()
{
    static auto Navigation = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Navigation->Properties.push_back({ "Margins", "[0.8, 0.0, 0.0, 0.0 ]" });
    Navigation->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Navigation), { 0x00, 0xED, 0xFF, 1 });
    };
    Composition::Registerelement("Toolbar.Navigation", Navigation);

    static auto Backbutton = Navigation->Children.emplace_back(std::make_shared<Element_t>());
    Backbutton->Properties.push_back({ "Margins", "[0.7, 0.0, 0.0, 0.0 ]" });
    Backbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Backbutton), { 0xFF, 0x33, 0xFF, 1 });
    };
    Composition::Registerelement("Toolbar.Navigation.Backbutton", Backbutton);

    static auto Frontbutton = Navigation->Children.emplace_back(std::make_shared<Element_t>());
    Frontbutton->Properties.push_back({ "Margins", "[0.7, 0.0, 1.0, 0.0 ]" });
    Frontbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Frontbutton), { 0xFF, 0xEE, 0xFF, 1 });
    };
    Composition::Registerelement("Toolbar.Navigation.Frontbutton", Frontbutton);
}
inline void Createbuttons()
{
    static auto Closebutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Closebutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 1.0, 0.0 ]" });
    Closebutton->onRender = []() -> void
    {
        if (Closebutton->State.isLeftclicked) Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0xFF, 0x00, 0x00, 0.3 });
        if (Closebutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Closebutton), { 0x00, 0xFF, 0x00, 1 });
        // TODO(tcn): Render texture / font 'X'
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

        // Invalidate the area if hovering changed or clicked.
        if (State.isLeftclicked || State.isHoveredover) Invalidatewindow();
    };
    Composition::Registerelement("Toolbar.Closebutton", Closebutton);

    static auto Maxbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Maxbutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 0.972, 0.0 ]" });
    Maxbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Maxbutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Maxbutton", Maxbutton);

    static auto Minbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Minbutton->Properties.push_back({ "Margins", "[ 0.975, 0.5, 0.945, 0.0 ]" });
    Minbutton->onRender = []() -> void
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
    Toolbar->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Toolbar), { 0x11, 0x0f, 0x0c, 1 });
    };
    Composition::Registerelement("Toolbar", Toolbar);

    Createbuttons();
    Createnavigation();
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createtoolbar); } }; static Startup Loader{}; }
