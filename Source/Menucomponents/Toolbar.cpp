/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides the toolbar so we can drag the window.
*/

#include "../Stdinclude.hpp"

static auto Goldgradient{ Rendering::Creategradient(512, { 175, 133, 23, 1.0f }, { 201, 157, 26, 1.0f }) };
void Renderbutton(Element_t *Caller)
{
    auto Box{ Caller->Dimensions }; Box.y0 -= 1;
    if(!Caller->State.Hoover) Rendering::Draw::Bordergradient(Goldgradient, Box);
    else Rendering::Draw::Quadgradient(Goldgradient, Caller->Dimensions);
}
void Createtoolbar()
{
    auto Rootelement{ Rendering::getRootelement() };

    auto Toolbar = new Element_t("ui.toolbar");
    Toolbar->Margin = { 0, 0, 0, 1.9 };
    Toolbar->onRender = [](Element_t *Caller) -> void
    {
        auto Box{ Caller->Dimensions }; Box.y0 = Box.y1;
        Rendering::Draw::Linegradient(Goldgradient, Box);
    };

    auto Closebutton = new Element_t("ui.toolbar.close");
    Closebutton->Margin = { 1.95, 0.0, 0.005, 0.9f };
    Closebutton->onRender = Renderbutton;
    Closebutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed; Armed = !Released;
        if(Armed && Caller->State.Hoover) std::terminate();
        return Armed;
    };
    Toolbar->Children.push_back(Closebutton);

    auto Maxbutton = new Element_t("ui.toolbar.max");
    Maxbutton->Margin = { 1.89, 0.0, 0.06, 0.9f };
    Maxbutton->onRender = Renderbutton;
    Toolbar->Children.push_back(Maxbutton);

    auto Minbutton = new Element_t("ui.toolbar.min");
    Minbutton->Margin = { 1.83, 0.0, 0.12, 0.9f };
    Minbutton->onRender = Renderbutton;
    Toolbar->Children.push_back(Minbutton);

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
