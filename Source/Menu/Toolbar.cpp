/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Provides movement and termination.
*/

#include "../Stdinclude.hpp"

void Composetoolbar(Element_t *Target)
{
    auto Boundingbox = new Element_t("Toolbar");
    auto Closebutton = new Element_t("Toolbar.Close");

    // The full toolbar, click to drag.
    Boundingbox->State.Fixedheight = true;
    Boundingbox->Margins = { 0, 0, 0, 20 };
    Boundingbox->onRender = [](const Element_t *Caller)
    {
        Draw::Quad({ 97, 72, 47, 0xFF }, Caller->Dimensions);
    };
    Boundingbox->onFrame = [](const Element_t *Caller, double Deltatime)
    {
        static point2_t Previous{};

        if (Caller->State.Clicked)
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

    // Let the user exit in a natural way.
    Closebutton->State.ExclusiveIO = true;
    Closebutton->Margins = { 1.9f, 0, 0.01f, 0 };
    Closebutton->onRender = [](const Element_t *Caller)
    {
        if(Caller->State.Clicked) Draw::Quad({ 0, 0xFF, 0, 1 }, Caller->Dimensions);
        else Draw::Quad({ 0xFF, 0, 0, 1 }, Caller->Dimensions);
    };
    Closebutton->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        return Newstate.Clicked;
    };
    Closebutton->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        if (Caller->State.Clicked && Newstate.Clicked && Caller->State.Focused)
        {
            Engine::setErrno(Hash::FNV1a_32("Toolbar.Close"));
        }

        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
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
