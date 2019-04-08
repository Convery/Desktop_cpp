/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

static void Createnavigationbuttons()
{
    static auto Navigation = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Navigation->Margins = { 0.8, 0.0, 0.0, 0.0 };
    Navigation->onRender = []() -> void
    {
        Rendering::Drawimage(Navigation->Position, { 256, 40 }, Assets::NavigationBG.data());
    };
    Composition::Registerelement("Toolbar.Navigation", Navigation);

    static auto Backbutton = Navigation->Children.emplace_back(std::make_shared<Element_t>());
    Backbutton->Margins = { 0.7, 0.0, 0.0, 0.0 };
    Backbutton->onRender = []() -> void
    {
        if (Backbutton->State.isHoveredover)
        {
            Rendering::Drawimage(Backbutton->Position, { 77, 40 }, Assets::Navleft.data(), {3, 0});
        }
    };
    Backbutton->onStatechange = [](Elementstate_t State) { Invalidatewindow(); };
    Composition::Registerelement("Toolbar.Navigation.Backbutton", Backbutton);

    static auto Frontbutton = Navigation->Children.emplace_back(std::make_shared<Element_t>());
    Frontbutton->Margins = { 0.7, 0.0, 1.0, 0.0 };
    Frontbutton->onRender = []() -> void
    {
        if (Frontbutton->State.isHoveredover)
        {
            Rendering::Drawimage(Frontbutton->Position, { 77, 40 }, Assets::Navright.data(), { -3, 0 });
        }
    };
    Frontbutton->onStatechange = [](Elementstate_t State) { Invalidatewindow(); };
    Composition::Registerelement("Toolbar.Navigation.Frontbutton", Frontbutton);

    static auto Homebutton = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Homebutton->Margins = { 0.02, 0.95, 0.5, 0.001 };
    Homebutton->onRender = []() -> void
    {
        Rendering::Drawtextcentered({ Homebutton->Position.x + Homebutton->Size.x / 2, Homebutton->Position.y + 2 }, 32, { 192, 192, 122, 0.4 }, L"Ironclad Bold", L"HOME");
        if (Homebutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Homebutton), { 100, 100, 0, 0.2 });
        Rendering::Solid::Outlinerectangle(Elementbox(Homebutton), { 100, 100, 0, 0.6 });
    };
    Homebutton->onStatechange = [](Elementstate_t) {Invalidatewindow(); };
    Composition::Registerelement("Leftbar.Home", Homebutton);

    static auto Pluginsbutton = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Pluginsbutton->Margins = { 0.02, 0.95, 0.5, 0.057 };
    Pluginsbutton->onRender = []() -> void
    {
        Rendering::Drawtextcentered({ Pluginsbutton->Position.x + Pluginsbutton->Size.x / 2, Pluginsbutton->Position.y + 2 }, 32, { 192, 192, 122, 0.3 }, L"Ironclad Bold", L"PLUGINS");
        if (Pluginsbutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Pluginsbutton), { 100, 100, 0, 0.2 });
        Rendering::Solid::Outlinerectangle(Elementbox(Pluginsbutton), { 100, 100, 0, 0.6 });
    };
    Pluginsbutton->onStatechange = [](Elementstate_t) {Invalidatewindow(); };
    Composition::Registerelement("Leftbar.Plugins", Pluginsbutton);

    static auto Statsbutton = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Statsbutton->Margins = { 0.02, 0.95, 0.5, 0.113 };
    Statsbutton->onRender = []() -> void
    {
        Rendering::Drawtextcentered({ Statsbutton->Position.x + Statsbutton->Size.x / 2, Statsbutton->Position.y + 2 }, 32, { 192, 192, 122, 0.2 }, L"Ironclad Bold", L"STATISTICS");
        if (Statsbutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Statsbutton), { 100, 100, 0, 0.2 });
        Rendering::Solid::Outlinerectangle(Elementbox(Statsbutton), { 100, 100, 0, 0.6 });
    };
    Statsbutton->onStatechange = [](Elementstate_t) {Invalidatewindow(); };
    Composition::Registerelement("Leftbar.Stats", Statsbutton);

    Rendering::Registerfont({ Assets::Ironclad_Bold.data(), Assets::Ironclad_Bold.size() });
}
static void Createcontentview()
{
    static auto Background = Composition::Getelement("Leftbar");
    Background->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Background), { 30, 30, 30, 1 });
    };

    Createnavigationbuttons();

    /* TODO(tcn): Add the main content and such. */
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createcontentview); } }; static Startup Loader{}; }
