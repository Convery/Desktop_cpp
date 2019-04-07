/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createsearchbar()
{
    static auto Searchbar = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Searchbar->Margins = { 0.7, 0.0, 0.285, 0.0 };
    Searchbar->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Searchbar), { 0xFF, 0xFF, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Searchbar", Searchbar);
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createsearchbar); } }; static Startup Loader{}; }
