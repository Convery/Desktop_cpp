/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createstatusbox()
{
    static auto Status = Composition::Getelement("Leftbar")->Children.emplace_back(std::make_shared<Element_t>());
    Status->Margins = { 0.0, 0.95, 0.0, 1.0 };
    Status->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Status), { 0xDD, 0xFF, 0x12, 1 });
    };
    Composition::Registerelement("Leftbar.Status", Status);
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createstatusbox); } }; static Startup Loader{}; }
