/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createsocial()
{
    static auto Socialbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Socialbutton->Margins = { 0.965, 0.0, 0.866, 0.0 };
    Socialbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Socialbutton), { 0x00, 0x88, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Socialbutton", Socialbutton);

    static auto Socialdropdown = Composition::Getelement("Rightbar")->Children.emplace_back(std::make_shared<Element_t>());
    Socialdropdown->Margins = { 0.0, 0.0, 0.0, -4000.0 };
    Socialdropdown->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Socialdropdown), { 0x00, 0x88, 0x00, 1 });
    };
    Composition::Registerelement("Rightbar.Socialdropdown", Socialdropdown);

    // Animate the dropdown when the button is clicked.
    static double Remainingtime = 0, Speed = 0;
    Socialbutton->isExclusive = [](Elementstate_t State) -> bool
    {
        return State.isLeftclicked;
    };
    Socialbutton->onStatechange = [](Elementstate_t State) -> void
    {
        if (State.isLeftclicked && !State.isHoveredover && Socialbutton->State.isLeftclicked && Socialbutton->State.isHoveredover)
        {
            // Replace the placeholder margin with the real one on the first frame.
            if (Socialdropdown->Margins.y1 == -4000.0f) Socialdropdown->Margins.y1 = -Socialdropdown->Size.y;

            static bool Tick{ false };
            Remainingtime = 0.5;
            Tick ^= 1;

            if(Tick) Speed = (0 - Socialdropdown->Margins.y1) / Remainingtime;
            else Speed = (-Socialdropdown->Size.y - Socialdropdown->Margins.y1) / Remainingtime;
        }
    };
    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double Deltatime) -> void
    {
        if(Remainingtime != 0.0f)
        {
            if (Remainingtime <= Deltatime)
            {
                Socialdropdown->Margins.y1 += Speed * Remainingtime;
                Remainingtime = 0.0f;
                Window::Forceredraw();
            }
            else
            {
                Socialdropdown->Margins.y1 += Speed * Deltatime;;
                Remainingtime -= Deltatime;
                Window::Forceredraw();
            }
        }
    });
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createsocial); } }; static Startup Loader{}; }
