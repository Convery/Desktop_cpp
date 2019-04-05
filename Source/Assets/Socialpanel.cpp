/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createsocial()
{
    static auto Socialbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Socialbutton->Properties.push_back({ "Margins", "[ 0.965, 0.0, 0.866, 0.0 ]" });
    Socialbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Socialbutton), { 0xFF, 0xFF, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Socialbutton", Socialbutton);

    /*
        TODO(tcn):
        Modify the right bar when clicked.
    */
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createsocial); } }; static Startup Loader{}; }
