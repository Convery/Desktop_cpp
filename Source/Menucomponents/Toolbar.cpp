/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides the toolbar so we can drag the window.
*/

#include "../Stdinclude.hpp"

static auto Goldgradient{ Rendering::Creategradient(512, { 255, 255, 168, 1.0f }, { 246, 201, 76, 1.0f }) };
static void Renderbutton(Element_t *Caller)
{
    auto Box{ Caller->Dimensions }; Box.y0 -= 1;
    if(Caller->State.Hoover) Rendering::Draw::Quadgradient(Goldgradient, Caller->Dimensions);
    Rendering::Draw::Bordergradient(Goldgradient, Box);
}
static void Renderbox(Element_t *Caller)
{
    auto Box{ Caller->Dimensions }; Box.y0 -= 1;
    Rendering::Draw::Border(Caller->Backgroundcolor, Box);
}
static void Rendercross(Element_t *Caller)
{
    auto Box{ Caller->Dimensions };

    Rendering::Draw::Line(Caller->Backgroundcolor, Box);
    Box.x0 += 1; Box.x1 += 1;
    Rendering::Draw::Line(Caller->Backgroundcolor, Box);
    Box.x0 -= 2; Box.x1 -= 2;
    Rendering::Draw::Line(Caller->Backgroundcolor, Box);
}

void Createtoolbar()
{
    auto Rootelement{ Rendering::getRootelement() };

    // Bounding box.
    auto Toolbar = new Element_t("ui.toolbar");
    Toolbar->Margin = { 0, 0, 0, 1.9 };
    Toolbar->onRender = [](Element_t *Caller) -> void
    {
        auto Box{ Caller->Dimensions }; Box.y0 = Box.y1;
        Rendering::Draw::Linegradient(Goldgradient, Box);
    };

    // Top-right buttons.
    auto Closebutton = new Element_t("ui.toolbar.close");
    Closebutton->Margin = { 1.945, 0.0, 0.005, 0.9 };
    Closebutton->onRender = Renderbutton;
    Closebutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed; Armed = !Released;
        if(Armed && Caller->State.Hoover) std::terminate();
        return Armed;
    };
    Toolbar->Children.push_back(Closebutton);

    auto Maxbutton = new Element_t("ui.toolbar.max");
    Maxbutton->Margin = { 1.891, 0.0, 0.059, 0.9 };
    Maxbutton->onRender = Renderbutton;
    Toolbar->Children.push_back(Maxbutton);

    auto Minbutton = new Element_t("ui.toolbar.min");
    Minbutton->Margin = { 1.837, 0.0, 0.1125, 0.9 };
    Minbutton->onRender = Renderbutton;
    Minbutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed; Armed = !Released;
        if (Armed && Caller->State.Hoover)
        {
            Input::Minimize();
            return true;
        }
        return Armed;
    };
    Toolbar->Children.push_back(Minbutton);

    // The icons drawn over the buttons.
    auto Closeicon = new Element_t("ui.toolbar.close.icon");
    Closeicon->Backgroundcolor = { 0.4, 0.4, 0.4, 0.5 };
    Closeicon->Margin = { 0.4, 0.4, 0.4, 0.4 };
    Closeicon->onRender = Rendercross;
    Closebutton->Children.push_back(Closeicon);

    auto Maxicon = new Element_t("ui.toolbar.max.icon");
    Maxicon->Backgroundcolor = { 0.4, 0.4, 0.4, 0.5 };
    Maxicon->Margin = { 0.6, 0.6, 0.6, 0.6 };
    Maxicon->onRender = Renderbox;
    Maxbutton->Children.push_back(Maxicon);

    auto Minicon = new Element_t("ui.toolbar.min.icon");
    Minicon->Backgroundcolor = { 0.4, 0.4, 0.4, 0.5 };
    Minicon->Margin = { 0.6, 1.4, 0.6, 0.6 };
    Minicon->onRender = Renderbox;
    Minbutton->Children.push_back(Minicon);

    // Return the toolbar.
    Rootelement->Children.push_back(Toolbar);
}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Menu::Register("toolbar", Createtoolbar);
        };
    };
    static Loader Load{};
}
