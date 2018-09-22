/*
    Initial author: Convery (tcn@ayria.se)
    Started: 16-09-2018
    License: MIT

    Provides the main area.
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

static void Composetoolbar(Element_t *Target)
{
    auto Boundingbox = new Element_t("Toolbar");
    auto Closebutton = new Element_t("Toolbar.Close");

    // The full toolbar, click to drag.
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->Margins = { 0, 0, 0, 20 };
    Boundingbox->onRender = [](const Element_t *Caller)
    {
        Draw::Quad({ 12, 12, 12, 0xFF }, Caller->Dimensions);
    };
    Boundingbox->onFrame = [](const Element_t *Caller, double Deltatime)
    {
        static point2_t Previous{};

        if (Caller->Properties.Clicked)
        {
            if (Previous.x + Previous.y == 0) Previous = Engine::getMouseposition();
            const auto Current{ Engine::getMouseposition() };
            auto Window{ Engine::getWindowposition() };

            Window.x += (Current.x - Previous.x);
            Window.y += (Current.y - Previous.y);
            Engine::Window::Move(Window);
            Previous = Current;
        }
        else
        {
            Previous = {};
        }
    };
    Target->addChild(Boundingbox);

    // Terminate the client.
    Closebutton->Properties.ExclusiveIO = true;
    Closebutton->Properties.Fixedheight = true;
    Closebutton->Margins = { 1.955f, -1, 0.005, 15 };
    Closebutton->onRender = [](const Element_t *Caller)
    {
        if (Caller->Properties.Focused) Draw::Quad({ 0x33, 0x33, 0x33, 1 }, Caller->Dimensions);
        if (Caller->Properties.Clicked) Draw::Quad({ 0, 0xFF, 0xFF, 1 }, Caller->Dimensions);
        Draw::Quad<true>({ 0, 0xFF, 0xFF, 1 }, Caller->Dimensions);
    };
    Closebutton->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        return Newstate.Clicked;
    };
    Closebutton->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        if (Newstate.Clicked && !Newstate.Focused && Caller->Properties.Clicked && Caller->Properties.Focused)
        {
            Engine::setErrno(Hash::FNV1a_32("Toolbar.Close"));
        }

        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
    };
    Boundingbox->addChild(Closebutton);
}
void Composemainmenu(Element_t *Target)
{
    auto Boundingbox = new Element_t("Mainmenu");

    // The content is window-height - 20 for the toolbar.
    Boundingbox->onRender = [](const Element_t *Caller)
    {
        auto Local{ Caller->Dimensions };
        Draw::Quad({ 0xFE, 0x00, 0xFE, 1 }, Local);
    };
    Target->addChild(Boundingbox);

    Engine::gCurrentmenuID = Hash::FNV1a_32("mainmenu");
    Engine::Window::Resize({ 1280, 720 }, false);
    Composetoolbar(Target);
}

// Register the composer for later.
namespace
{
    struct Startup
    {
        Startup()
        {
            Engine::Compositing::Registercomposer("mainmenu", Composemainmenu);
        }
    };
    Startup Loader{};
}
