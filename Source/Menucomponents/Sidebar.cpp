/*
    Initial author: Convery (tcn@ayria.se)
    Started: 19-06-2018
    License: MIT

    Provides a sidebar-area on the side.
*/

#include "../Stdinclude.hpp"

extern texture_t Goldgradient;

static void Renderbutton(Element_t *Caller, vec4_t Clip)
{
    auto Box{ Caller->Renderdimensions }; Box.y0 += 2; Box.x0 -= 1;
    if (Caller->State.Hoover) Rendering::Draw::Quad({ 205, 197, 186, 0.2f }, Box, Clip);
    else Rendering::Draw::Quad(Caller->Backgroundcolor, Box, Clip);
    Rendering::Draw::Texturedborder(Goldgradient, Box, Clip);
}

void Createsidebar()
{
    auto Rootelement{ Rendering::getRootelement() };

    // Bounding box.
    auto Sidebar = new Element_t("ui.sidebar");
    Sidebar->Margin = { 0.0f, 0.05f, 1.70f, 0.05f };
    Sidebar->State.Hidden = true;
    Sidebar->State.Noinput = true;
    Rootelement->Children.push_back(Sidebar);

    // Developmentstatus..
    auto Devstatus = new Element_t("ui.sidebar.devstatus");
    Devstatus->Margin = { 0.0f, 0.03f, 0.0f, 1.85f };
    Devstatus->Backgroundcolor = { 1, 1, 1, 1 };
    Devstatus->onRender = [](Element_t *Caller, vec4_t Clip) -> void
    {
        auto Box{ Caller->Renderdimensions }; Box.y0 += 2; Box.x0 -= 1;
        Rendering::Draw::Texturedquad(Goldgradient, Box, Clip);
        Rendering::Draw::Texturedborder(Goldgradient, Box, Clip);
    };
    Sidebar->Children.push_back(Devstatus);

    // Homescreen.
    auto Button1 = new Element_t("ui.sidebar.button1");
    Button1->Margin = { 0.0f, 0.152f, 0.0f, 1.70f };
    Button1->Backgroundcolor = { 50, 58, 69, 1.0f };
    Button1->onRender = Renderbutton;
    Sidebar->Children.push_back(Button1);

    // Library.
    auto Button2 = new Element_t("ui.sidebar.button2");
    Button2->Margin = { 0.0f, 0.305f, 0.0f, 1.55f };
    Button2->Backgroundcolor = { 50, 58, 69, 1.0f };
    Button2->onRender = Renderbutton;
    Sidebar->Children.push_back(Button2);

    // Plugins.
    auto Button3 = new Element_t("ui.sidebar.button3");
    Button3->Margin = { 0.0f, 0.455f, 0.0f, 1.40f };
    Button3->Backgroundcolor = { 50, 58, 69, 1.0f };
    Button3->onRender = Renderbutton;
    Sidebar->Children.push_back(Button3);

    // Leaderboards or something..
    auto Button4 = new Element_t("ui.sidebar.button4");
    Button4->Margin = { 0.0f, 0.605f, 0.0f, 1.25f };
    Button4->Backgroundcolor = { 50, 58, 69, 1.0f };
    Button4->onRender = Renderbutton;
    Sidebar->Children.push_back(Button4);

    auto Area1 = new Element_t("ui.sidebar.area1");
    Area1->Margin = { 0.0f, 0.76f, 0.0f, 0.21f };
    Area1->Backgroundcolor = { 205, 197, 186, 0.2f };
    Area1->onRender = Renderbutton;
    Area1->State.Noinput = true;
    Sidebar->Children.push_back(Area1);

    auto Area2 = new Element_t("ui.sidebar.area2");
    Area2->Margin = { 0.0f, 1.8f, 0.0f, 0.0f };
    Area2->Backgroundcolor = { 205, 197, 186, 0.2f };
    Area2->onRender = Renderbutton;
    Area2->State.Noinput = true;
    Sidebar->Children.push_back(Area2);
}


namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Menu::Register("sidebar", Createsidebar);
        };
    };
    static Loader Load{};
}
