/*
    Initial author: Convery (tcn@ayria.se)
    Started: 22-06-2018
    License: MIT

    Create a random background on startup.
*/

#include "../Stdinclude.hpp"

auto Brassgradient{ Rendering::Texture::Creategradient(32, {202, 178, 147, 1.0f}, {166, 145, 112, 1.0f}) };
void Createtoolbar()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };
    static double lWidth{}, lHeight{}, lPosX{}, lPosY{};
    static bool Shouldmove{ false };

    // Rotate the gradient.
    std::swap(Brassgradient.Height, Brassgradient.Width);

    // Bounding box.
    auto Toolbar = new Element_t("ui.toolbar");
    Rootelement->Children.push_back(Toolbar);
    Toolbar->Margin = { 0, 0, 0, 1.95f };
    Toolbar->onRender = [&](Element_t *Caller) -> void
    {
        Rendering::Textureddraw::Quad(Brassgradient, Caller->Renderdimensions);
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
