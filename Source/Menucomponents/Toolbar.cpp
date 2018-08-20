/*
    Initial author: Convery (tcn@ayria.se)
    Started: 15-08-2018
    License: MIT

    Provides movement and termination.
*/

#include "../Stdinclude.hpp"

void Composetoolbar(Element_t *Target)
{
    auto Boundingbox = new Element_t("Toolbar");


    // The full toolbar, click to drag.
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->Margins = { 0, 0, 0, 20 };
    Boundingbox->onRender = [](Element_t *Caller)
    {
        Draw::Quad({ 97, 72, 47, 0xFF }, Caller->Dimensions);
    };
    Boundingbox->onClicked = [](Element_t *Caller, bool Released)
    {
        // Move the window while pressed.
        static bool isMoving;
        isMoving = !Released;

        // In a separate thread so we wont block.
        std::thread([&]() -> void
        {
            auto Window{ Engine::gWindowposition };
            POINT Origin; GetCursorPos(&Origin);
            POINT Currentposition;

            // While moving.
            while (isMoving)
            {
                GetCursorPos(&Currentposition);
                Window.x += (Currentposition.x - Origin.x);
                Window.y += (Currentposition.y - Origin.y);
                Engine::Window::Move(Window, true);
                Origin = Currentposition;
            }

            // Recalculate on exit.
            Engine::Window::Move(Window);
        }).detach();

        return true;
    };
    Target->addChild(Boundingbox);


}

// Register the composer for later.
namespace
{
    struct Startup
    {
        Startup()
        {
            Engine::Compositing::Registercomposer("toolbar", Composetoolbar);
        }
    };
    Startup Loader{};
}
