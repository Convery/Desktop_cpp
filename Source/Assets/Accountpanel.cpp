/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createaccountview()
{
    static auto Accountbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Accountbutton->Margins = { 0.965, 0.0, 0.829, 0.0 };
    Accountbutton->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Accountbutton), { 0x886, 0x00, 0x00, 1 });
    };
    Composition::Registerelement("Toolbar.Accountbutton", Accountbutton);

    static auto Accountdropdown = Composition::Getelement("Rightbar")->Children.emplace_back(std::make_shared<Element_t>());
    Accountdropdown->Margins = { 0.0, 0.0, 0.0, -4000.0 };
    Accountdropdown->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Accountdropdown), { 0x88, 0x00, 0x00, 0.3 });
    };
    Composition::Registerelement("Rightbar.Accountdropdown", Accountdropdown);

    // Animate the dropdown when the button is clicked.
    static double Remainingtime = 0, Speed = 0;
    Accountbutton->isExclusive = [](Elementstate_t State) -> bool
    {
        return State.isLeftclicked;
    };
    Accountbutton->onStatechange = [](Elementstate_t State) -> void
    {
        if (State.isLeftclicked && !State.isHoveredover && Accountbutton->State.isLeftclicked && Accountbutton->State.isHoveredover)
        {
            // Replace the placeholder margin with the real one on the first frame.
            if (Accountdropdown->Margins.y1 == -4000.0f) Accountdropdown->Margins.y1 = -Accountdropdown->Size.y;

            static bool Tick{ false };
            Remainingtime = 0.5;
            Tick ^= 1;

            if(Tick) Speed = (0 - Accountdropdown->Margins.y1) / Remainingtime;
            else Speed = (-Accountdropdown->Size.y - Accountdropdown->Margins.y1) / Remainingtime;
        }
    };
    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double Deltatime) -> void
    {
        if(Remainingtime != 0.0f)
        {
            if (Remainingtime <= Deltatime)
            {
                Accountdropdown->Margins.y1 += Speed * Remainingtime;
                Remainingtime = 0.0f;
                Window::Forceredraw();
            }
            else
            {
                Accountdropdown->Margins.y1 += Speed * Deltatime;;
                Remainingtime -= Deltatime;
                Window::Forceredraw();
            }
        }
    });
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createaccountview); } }; static Startup Loader{}; }
