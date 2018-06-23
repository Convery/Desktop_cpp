/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides the toolbar so we can drag the window.
*/

#include "../Stdinclude.hpp"

auto Goldgradient{ Rendering::Texture::Creategradient(512, { 255, 255, 168, 1.0f }, { 246, 201, 76, 1.0f }) };
static void Renderbutton(Element_t *Caller, vec4_t Clip)
{
    auto Box{ Caller->Renderdimensions }; Box.y0 -= 1;
    if(Caller->State.Hoover) Rendering::Draw::Texturedquad(Goldgradient, Caller->Renderdimensions, Clip);
    else Rendering::Draw::Quad({ 205, 197, 186, 0.1f }, Caller->Renderdimensions, Clip);
    Rendering::Draw::Texturedborder(Goldgradient, Box, Clip);
}
static void Renderbox(Element_t *Caller, vec4_t Clip)
{
    auto Box{ Caller->Renderdimensions }; Box.y0 -= 1;
    Rendering::Draw::Border(Caller->Backgroundcolor, Box, Clip);
}
static void Rendercross(Element_t *Caller, vec4_t Clip)
{
    auto Box{ Caller->Renderdimensions };

    Rendering::Draw::Line(Caller->Backgroundcolor, Box, Clip);
    Box.x0 += 1; Box.x1 += 1;
    Rendering::Draw::Line(Caller->Backgroundcolor, Box, Clip);
}

void Createtoolbar()
{
    static double lWidth{}, lHeight{}, lPosX{}, lPosY{};
    auto Rootelement{ Rendering::getRootelement() };
    static bool Shouldmove{ false };

    // Bounding box.
    auto Toolbar = new Element_t("ui.toolbar");
    Toolbar->Margin = { 0.0f, 0.0f, 0.0f, 1.95f };
    Toolbar->onRender = [](Element_t *Caller, vec4_t Clip) -> void
    {
        auto Box{ Caller->Renderdimensions }; Box.y0 = Box.y1;
        Rendering::Draw::Texturedline(Goldgradient, Box, Clip);
    };
    Toolbar->onClicked = [&](Element_t *Caller, bool Released) -> bool
    {
        // Stop moving the window if needed.
        Shouldmove = !Released;

        // Toggle fullscreen on double-click.
        if (!Released)
        {
            static auto Lastclick{ std::chrono::high_resolution_clock::time_point() };
            if (std::chrono::high_resolution_clock::now() - Lastclick < std::chrono::milliseconds(500))
            {
                static auto Monitorsize{ Input::getMonitorsize() };
                auto Windowposition = Input::getWindowposition();
                auto Windowsize = Input::getWindowsize();

                // Restore the window.
                if (Windowsize.x == Monitorsize.x && Windowsize.y == Monitorsize.y)
                {
                    Input::onWindowresize(lWidth, lHeight);
                    Input::onWindowmove(lPosX, lPosY);
                }

                // Fullscreen the window.
                else
                {
                    lPosX = Windowposition.x; lPosY = Windowposition.y;
                    lWidth = Windowsize.x; lHeight = Windowsize.y;

                    Input::onWindowresize(Monitorsize.x, Monitorsize.y);

                    /*
                    TODO(Convery):
                    Refactor the work-area into a portable version.
                */

                    RECT Displaysize{};
                    SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
                    Input::onWindowmove(Displaysize.left, Displaysize.top);
                }

                // Hackery.
                Shouldmove = false;
            }

            Lastclick = std::chrono::high_resolution_clock::now();
        }

        if (Shouldmove)
        {
            std::thread([&]() -> void
            {
                auto Origin{ Input::getMouseposition() };
                auto Window{ Input::getWindowposition() };

                while (Shouldmove)
                {
                    auto Current = Input::getMouseposition();
                    Window.x += (Current.x - Origin.x) * 1.0;
                    Window.y += (Current.y - Origin.y) * 1.0;

                    Input::onWindowmove(Window.x, Window.y);
                }
            }).detach();
        }

        return Shouldmove;
    };
    Rootelement->Children.push_back(Toolbar);

    // Top-right buttons.
    auto Closebutton = new Element_t("ui.toolbar.close");
    Closebutton->Margin = { 1.945f, 0.0f, 0.005f, 0.6f };
    Closebutton->onRender = Renderbutton;
    Closebutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed{ false };
        if(Armed && Caller->State.Hoover) std::exit(0);
        Armed = !Released;
        return Armed;
    };
    Toolbar->Children.push_back(Closebutton);

    auto Maxbutton = new Element_t("ui.toolbar.max");
    Maxbutton->Margin = { 1.891f, 0.0f, 0.059f, 0.6f };
    Maxbutton->onRender = Renderbutton;
    Maxbutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed{ false };
        if (Armed && Caller->State.Hoover)
        {
            static auto Monitorsize{ Input::getMonitorsize() };
            auto Windowposition = Input::getWindowposition();
            auto Windowsize = Input::getWindowsize();

            // Restore the window.
            if (Windowsize.x == Monitorsize.x && Windowsize.y == Monitorsize.y)
            {
                Input::onWindowresize(lWidth, lHeight);
                Input::onWindowmove(lPosX, lPosY);
            }

            // Fullscreen the window.
            else
            {
                lPosX = Windowposition.x; lPosY = Windowposition.y;
                lWidth = Windowsize.x; lHeight = Windowsize.y;

                Input::onWindowresize(Monitorsize.x, Monitorsize.y);

                /*
                    TODO(Convery):
                    Refactor the work-area into a portable version.
                */

                RECT Displaysize{};
                SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
                Input::onWindowmove(Displaysize.left, Displaysize.top);
            }
        }
        Armed = !Released;
        return Armed;
    };
    Toolbar->Children.push_back(Maxbutton);

    auto Minbutton = new Element_t("ui.toolbar.min");
    Minbutton->Margin = { 1.837f, 0.0f, 0.114f, 0.6f };
    Minbutton->onRender = Renderbutton;
    Minbutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed{ false };
        if (Armed && Caller->State.Hoover)
        {
            Input::Minimize();
            return true;
        }
         Armed = !Released;
        return Armed;
    };
    Toolbar->Children.push_back(Minbutton);

    // The icons drawn over the buttons.
    auto Closeicon = new Element_t("ui.toolbar.close.icon");
    Closeicon->Backgroundcolor = { 0.4f, 0.4f, 0.4f, 1.0f };
    Closeicon->Margin = { 0.4f, 0.4f, 0.4f, 0.4f };
    Closeicon->onRender = Rendercross;
    Closeicon->State.Noinput = true;
    Closebutton->Children.push_back(Closeicon);

    auto Maxicon = new Element_t("ui.toolbar.max.icon");
    Maxicon->Backgroundcolor = { 0.4f, 0.4f, 0.4f, 1.0f };
    Maxicon->Margin = { 0.6f, 0.6f, 0.6f, 0.6f };
    Maxicon->onRender = Renderbox;
    Maxicon->State.Noinput = true;
    Maxbutton->Children.push_back(Maxicon);

    auto Minicon = new Element_t("ui.toolbar.min.icon");
    Minicon->Backgroundcolor = { 0.4f, 0.4f, 0.4f, 1.0f };
    Minicon->Margin = { 0.6f, 1.4f, 0.6f, 0.6f };
    Minicon->onRender = Renderbox;
    Minicon->State.Noinput = true;
    Minbutton->Children.push_back(Minicon);
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
