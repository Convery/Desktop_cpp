/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createnotifications()
{
    static auto Notificationbutton = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Notificationbutton->Margins = { 0.965, 0.0, 0.9035, 0.0 };
    Notificationbutton->onRender = []() -> void
    {
        Rendering::Solid::Outlinerectangle(Elementbox(Notificationbutton), { 100, 100, 0, 0.6 });
        if (Notificationbutton->State.isHoveredover) Rendering::Solid::Fillrectangle(Elementbox(Notificationbutton), { 100, 100, 0, 0.6 });
    };
    Composition::Registerelement("Toolbar.Notificationbutton", Notificationbutton);

    static auto Notificationdropdown = Composition::Getelement("Rightbar")->Children.emplace_back(std::make_shared<Element_t>());
    Notificationdropdown->Margins = { 0.0, 0.0, 0.0, -4000.0 };
    Notificationdropdown->onRender = []() -> void
    {
        Rendering::Solid::Fillrectangle(Elementbox(Notificationdropdown), { 0x0, 0x0, 0x88, 0.3 });
    };
    Composition::Registerelement("Rightbar.Notificationdropdown", Notificationdropdown);

    // Animate the dropdown when the button is clicked.
    static double Remainingtime = 0, Speed = 0;
    Notificationbutton->isExclusive = [](Elementstate_t State) -> bool
    {
        return State.isLeftclicked;
    };
    Notificationbutton->onStatechange = [](Elementstate_t State) -> void
    {
        if (State.isLeftclicked && !State.isHoveredover && Notificationbutton->State.isLeftclicked && Notificationbutton->State.isHoveredover)
        {
            // Replace the placeholder margin with the real one on the first frame.
            if (Notificationdropdown->Margins.y1 == -4000.0f) Notificationdropdown->Margins.y1 = -Notificationdropdown->Size.y;

            static bool Tick{ false };
            Remainingtime = 0.5;
            Tick ^= 1;

            if(Tick) Speed = (0 - Notificationdropdown->Margins.y1) / Remainingtime;
            else Speed = (-Notificationdropdown->Size.y - Notificationdropdown->Margins.y1) / Remainingtime;
        }
        Invalidatewindow();
    };
    Subscribetostack(Events::Enginestack, Events::Engineevent::TICK, [](const double Deltatime) -> void
    {
        if(Remainingtime != 0.0f)
        {
            if (Remainingtime <= Deltatime)
            {
                Notificationdropdown->Margins.y1 += Speed * Remainingtime;
                Remainingtime = 0.0f;
                Window::Forceredraw();
            }
            else
            {
                Notificationdropdown->Margins.y1 += Speed * Deltatime;;
                Remainingtime -= Deltatime;
                Window::Forceredraw();
            }
        }
    });
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createnotifications); } }; static Startup Loader{}; }
