/*
    Initial author: Convery (tcn@ayria.se)
    Started: 22-06-2018
    License: MIT

    Create a random background on startup.
*/

#include "../Stdinclude.hpp"

auto Goldgradient{ Rendering::Texture::Creategradient(32, { 255, 255, 168, 1.0f }, { 246, 201, 76, 1.0f }) };
auto Brassgradient{ Rendering::Texture::Creategradient(512, {166, 145, 112, 1.0f}, {202, 178, 147, 1.0f}) };
void Createtoolbar()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };
    static double lWidth{}, lHeight{}, lPosX{}, lPosY{};
    static bool Shouldmove{ false };

    // Rotate the gradient.
    std::swap(Goldgradient.Height, Goldgradient.Width);

    // Bounding box.
    auto Toolbar = new Element_t("ui.toolbar");
    Rootelement->Children.push_back(Toolbar);
    Toolbar->Margin = { 0, 0, 0, 1.97f };
    Toolbar->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        Rendering::Textureddraw::Quad(Brassgradient, Box);
    };
    Toolbar->onHoover = [](Element_t *Caller, bool Released) { return false; };
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

        // Drag the window.
        if (Shouldmove)
        {
            std::thread([&]() -> void
            {
                auto Origin{ Input::getMouseposition() };
                auto Window{ Input::getWindowposition() };

                while (Shouldmove)
                {
                    auto Current = Input::getMouseposition();
                    Window.x += (Current.x - Origin.x);
                    Window.y += (Current.y - Origin.y);

                    Input::onWindowmove(Window.x, Window.y);
                }
            }).detach();
        }

        return Shouldmove;
    };

    // Closing-button, top right.
    auto Closebutton = new Element_t("ui.toolbar.closebutton");
    Toolbar->Children.push_back(Closebutton);
    Closebutton->Margin = { 1.945f, 0, 0.005f, 0 };
    Closebutton->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions }; Box.y0 -= 1;
        if (Caller->State.Hoover) Rendering::Textureddraw::Quad(Goldgradient, Box);
        Rendering::Soliddraw::Polygon({ 12, 12, 12, 0.2f }, { {Box.x0 - 2, Box.y0 - 1}, {Box.x0 + 3, Box.y0 - 2}, {Box.x1 + 3, Box.y1 + 1}, {Box.x1 - 2, Box.y1 + 1} });
        Rendering::Textureddraw::Quad<false>(Goldgradient, Box);
    };
    Closebutton->onClicked = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Armed{ false };
        if (Armed && Caller->State.Hoover) std::exit(0);
        Armed = !Released;
        return Armed;
    };

    // Maximizing-button, top middle.
    auto Maxbutton = new Element_t("ui.toolbar.maxbutton");
    Toolbar->Children.push_back(Maxbutton);
    Maxbutton->Margin = { 1.891f, 0, 0.059f, 0 };
    Maxbutton->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions }; Box.y0 -= 1;
        if (Caller->State.Hoover) Rendering::Textureddraw::Quad(Goldgradient, Box);
        Rendering::Textureddraw::Quad<false>(Goldgradient, Box);

        Rendering::Soliddraw::Quad<false>({ 12, 12, 12, 0.2f }, { Box.x0 + 10, Box.y0 + 3, Box.x1 - 10, Box.y1 - 3 });
    };
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

    // Minimizing-button, top left.
    auto Minbutton = new Element_t("ui.toolbar.minbutton");
    Toolbar->Children.push_back(Minbutton);
    Minbutton->Margin = { 1.837f, 0, 0.114f, 0 };
    Minbutton->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions }; Box.y0 -= 1;
        if (Caller->State.Hoover) Rendering::Textureddraw::Quad(Goldgradient, Box);
        Rendering::Textureddraw::Quad<false>(Goldgradient, Box);

        Rendering::Soliddraw::Quad<false>({ 12, 12, 12, 0.2f }, { Box.x0 + 10, Box.y0 + 7, Box.x1 - 10, Box.y1 - 3 });
    };
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
}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Scene::Register("toolbar", Createtoolbar);
        };
    };
    static Loader Load{};
}
