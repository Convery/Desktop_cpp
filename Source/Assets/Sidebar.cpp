/*
    Initial author: Convery (tcn@ayria.se)
    Started: 01-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createstatusbox()
{
    static auto Status = Composition::Getelement("Sidebar")->Children.emplace_back(std::make_shared<Element_t>());
    Status->Properties.push_back({ "Margins", "[ 0.0, 0.95, 0.0, 0.055 ]" });
    Status->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Status), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Sidebar.Status", Status);
}
static void Createnavigation()
{
    static auto Homebutton = Composition::Getelement("Sidebar")->Children.emplace_back(std::make_shared<Element_t>());
    Homebutton->Properties.push_back({ "Margins", "[ 0.0, 0.95, 0.0, 0.0 ]" });
    Homebutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Homebutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Sidebar.Home", Homebutton);

    static auto Pluginsbutton = Composition::Getelement("Sidebar")->Children.emplace_back(std::make_shared<Element_t>());
    Pluginsbutton->Properties.push_back({ "Margins", "[ 0.0, 0.95, 0.0, 0.055 ]" });
    Pluginsbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Pluginsbutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Sidebar.Plugins", Pluginsbutton);

    static auto Statsbutton = Composition::Getelement("Sidebar")->Children.emplace_back(std::make_shared<Element_t>());
    Statsbutton->Properties.push_back({ "Margins", "[ 0.0, 0.95, 0.0, 0.108 ]" });
    Statsbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Statsbutton), { 0xFF, 0xED, 0x00, 1 });
    };
    Composition::Registerelement("Sidebar.Stats", Statsbutton);
}
static void Createsidebar()
{
    // Root-level component.
    static auto Sidebar = Global.Rootelement->Children.emplace_back(std::make_shared<Element_t>());
    Sidebar->Properties.push_back({ "Margins", "[ 0.8, 0.056, 0.0, 1.0 ]" });
    Composition::Registerelement("Sidebar", Sidebar);
    Sidebar->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Sidebar), { 0x11, 0x0f, 0x0c, 1 });
    };

    Createnavigation();
    Createstatusbox();
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createsidebar); } }; static Startup Loader{}; }
