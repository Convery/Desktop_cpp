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
    auto Closebutton = new Element_t("Toolbar.Close");

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

        return !Released;
    };
    Target->addChild(Boundingbox);

    // Let the user exit in a natural way.
    Closebutton->Margins = { 1.9, 0, 0.01, 0 };
    Closebutton->onRender = [](Element_t *Caller)
    {
        Draw::Quad({ 0, 0x33, 0, 1 }, Caller->Dimensions);
    };
    Closebutton->onClicked = [](Element_t *Caller, bool Released)
    {
        static bool Armed = false;
        if (Released && Armed && Caller->Properties.Hoover)
        {
            Engine::gErrno = Hash::FNV1a_32("Toolbar.Close");
            return true;
        }

        Armed = !Released;
        return Armed;
    };
    Boundingbox->addChild(Closebutton);
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
