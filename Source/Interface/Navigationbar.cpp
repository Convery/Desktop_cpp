/*
    Initial author: Convery (tcn@ayria.se)
    Started: 23-06-2018
    License: MIT

    Have some navigation and quick-launch icons.
*/

#include "../Stdinclude.hpp"

static auto Dirtygoldgradient{ Rendering::Texture::Creategradient(64, { 235, 196, 127, 1.0f }, { 186, 158, 92, 1.0f }) };
extern texture_t Brassgradient;
extern texture_t Goldgradient;
void Createnavbar()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };
    static uint32_t Currentbutton{ 1 };

    // Bounding box.
    static auto Navbar = new Element_t("ui.navbar");
    Rootelement->Children.push_back(Navbar);
    Navbar->Margin = { 0, 0, 1.9f, 0 };
    Navbar->State.Noinput = true;
    Navbar->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        Rendering::Textureddraw::Quad(Brassgradient, Caller->Renderdimensions);
        Rendering::Soliddraw::Quad({12, 12, 12, 0.8f}, Caller->Renderdimensions);
    };

    // Logo-thingy?
    auto Button0 = new Element_t("ui.navbar.button0");
    Navbar->Children.push_back(Button0);
    Button0->Margin = { 0, 0, 0, 1.85f };
    Button0->State.Noinput = true;
    Button0->onRender = [&](Element_t *Caller) -> void
    {
        Rendering::Textureddraw::Quad(Dirtygoldgradient, Caller->Renderdimensions);
    };

    // Navigate to games.
    auto Button1 = new Element_t("ui.navbar.button1");
    Navbar->Children.push_back(Button1);
    Button1->Margin = { 0, 0.15f, 0, 1.7f };
    Button1->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if(Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });

        if (Currentbutton == 1)
        {
            Box.y0 += 1;
            Rendering::Textureddraw::Triangle(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 }, { Box.x0, Box.y0 }, { Box.x0 - 4, Box.y0 + (Box.y1 - Box.y0) / 2 });
        }
    };
    Button1->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        Rendering::Invalidatearea(Navbar->Renderdimensions);
        Currentbutton = 1;
        return !Released;
    };

    // Navigate to plugins.
    auto Button2 = new Element_t("ui.navbar.button2");
    Navbar->Children.push_back(Button2);
    Button2->Margin = { 0, 0.30f, 0, 1.55f };
    Button2->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });

        if (Currentbutton == 2)
        {
            Box.y0 += 1;
            Rendering::Textureddraw::Triangle(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 }, { Box.x0, Box.y0 }, { Box.x0 - 4, Box.y0 + (Box.y1 - Box.y0) / 2 });
        }
    };
    Button2->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        Rendering::Invalidatearea(Navbar->Renderdimensions);
        Currentbutton = 2;
        return !Released;
    };

    // Navigate to leaderboards.
    auto Button3 = new Element_t("ui.navbar.button3");
    Navbar->Children.push_back(Button3);
    Button3->Margin = { 0, 0.45f, 0, 1.4f };
    Button3->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if(Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });

        if (Currentbutton == 3)
        {
            Box.y0 += 1;
            Rendering::Textureddraw::Triangle(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 }, { Box.x0, Box.y0 }, { Box.x0 - 4, Box.y0 + (Box.y1 - Box.y0) / 2 });
        }
    };
    Button3->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        Rendering::Invalidatearea(Navbar->Renderdimensions);
        Currentbutton = 3;
        return !Released;
    };

    // Slots for quick-start games.
    auto Slot1 = new Element_t("ui.navbar.slot1");
    Navbar->Children.push_back(Slot1);
    Slot1->Margin = { 0, 0.60f, 0, 1.25f };
    Slot1->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover)
        {
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
        else
        {
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
    };
    auto Slot2 = new Element_t("ui.navbar.slot2");
    Navbar->Children.push_back(Slot2);
    Slot2->Margin = { 0, 0.75f, 0, 1.10f };
    Slot2->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover)
        {
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
        else
        {
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
    };
    auto Slot3 = new Element_t("ui.navbar.slot3");
    Navbar->Children.push_back(Slot3);
    Slot3->Margin = { 0, 0.90f, 0, 0.95f };
    Slot3->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover)
        {
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
        else
        {
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
    };
    auto Slot4 = new Element_t("ui.navbar.slot4");
    Navbar->Children.push_back(Slot4);
    Slot4->Margin = { 0, 1.05f, 0, 0.80f };
    Slot4->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover)
        {
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
        else
        {
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
    };
    auto Slot5 = new Element_t("ui.navbar.slot5");
    Navbar->Children.push_back(Slot5);
    Slot5->Margin = { 0, 1.20f, 0, 0.65f };
    Slot5->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if (Caller->State.Hoover)
        {
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
        else
        {
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 3);
            Rendering::Textureddraw::Circle<false>(Dirtygoldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 + (Box.y1 - Box.y0) / 2 }, (Box.y1 - Box.y0) / 2 - 4);
        }
    };

    // Navigate to settings.
    auto Button4 = new Element_t("ui.navbar.button4");
    Navbar->Children.push_back(Button4);
    Button4->Margin = { 0, 1.35f, 0, 0.50f };
    Button4->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if(Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y0 }, { Box.x1, Box.y0 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y0 }, { Box.x1, Box.y0 });

        if (Currentbutton == 4)
        {
            Box.y0 += 1;
            Rendering::Textureddraw::Triangle(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 }, { Box.x0, Box.y0 }, { Box.x0 - 4, Box.y0 + (Box.y1 - Box.y0) / 2 });
        }
    };
    Button4->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        Rendering::Invalidatearea(Navbar->Renderdimensions);
        Currentbutton = 4;
        return !Released;
    };

    // Navigate to cake?
    auto Button5 = new Element_t("ui.navbar.button5");
    Navbar->Children.push_back(Button5);
    Button5->Margin = { 0, 1.50f, 0, 0.35f };
    Button5->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        if(Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y0 }, { Box.x1, Box.y0 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y0 }, { Box.x1, Box.y0 });
        if(Caller->State.Hoover) Rendering::Textureddraw::Line(Goldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });
        else Rendering::Textureddraw::Line(Dirtygoldgradient, { Box.x0, Box.y1 }, { Box.x1, Box.y1 });

        if (Currentbutton == 5)
        {
            Box.y0 += 1;
            Rendering::Textureddraw::Triangle(Goldgradient, { Box.x0 + (Box.x1 - Box.x0) / 2, Box.y0 }, { Box.x0, Box.y0 }, { Box.x0 - 4, Box.y0 + (Box.y1 - Box.y0) / 2 });
        }
    };
    Button5->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        Rendering::Invalidatearea(Navbar->Renderdimensions);
        Currentbutton = 5;
        return !Released;
    };
}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Scene::Register("navbar", Createnavbar);
        };
    };
    static Loader Load{};
}
