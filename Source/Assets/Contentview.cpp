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
    Homebutton->Margins = { 0.0, 0.95, 0.0, 0.0 };
    Homebutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Homebutton), { 0xFF, 0x66, 0x66, 1 });
    };
    Composition::Registerelement("Leftbar.Home", Homebutton);

    static auto Pluginsbutton = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Pluginsbutton->Margins = { 0.0, 0.95, 0.0, 0.055 };
    Pluginsbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Pluginsbutton), { 0xFF, 0x66, 0x66, 1 });
    };
    Composition::Registerelement("Leftbar.Plugins", Pluginsbutton);

    static auto Statsbutton = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Statsbutton->Margins = { 0.0, 0.95, 0.0, 0.108 };
    Statsbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Statsbutton), { 0xFF, 0x66, 0x66, 1 });
    };
    Composition::Registerelement("Leftbar.Stats", Statsbutton);
}
static void Createcontentview()
{
    Createnavigationbuttons();

    /* TODO(tcn): Add the main content and such. */
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createcontentview); } }; static Startup Loader{}; }
