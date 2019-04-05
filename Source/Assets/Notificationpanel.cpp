/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createnotifications()
{
    static auto Notificationbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Notificationbutton->Properties.push_back({ "Margins", "[ 0.965, 0.0, 0.9035, 0.0 ]" });
    Notificationbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Notificationbutton), { 0xFF, 0xFF, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Notificationbutton", Notificationbutton);

    /*
        TODO(tcn):
        Modify the right bar when clicked.
    */
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createnotifications); } }; static Startup Loader{}; }
