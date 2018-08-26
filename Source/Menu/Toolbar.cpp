/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Provides movement and termination.
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

void Composetoolbar(Element_t *Target)
{
    auto Boundingbox = new Element_t("Toolbar");
    auto Closebutton = new Element_t("Toolbar.Close");

    // The full toolbar, click to drag.
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->Margins = { 0.045f, 0, 0.045f, 20 };
    Boundingbox->onRender = [](const Element_t *Caller)
    {
        Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
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

    // Let the user exit in a natural way.
    Closebutton->Properties.ExclusiveIO = true;
    Closebutton->Properties.Fixedheight = true;
    Closebutton->Margins = { 1.9f, 1, 0.037f, 20 };
    Closebutton->onRender = [](const Element_t *Caller)
    {
        // NOTE(Convery): This is not a typo.
        if(Caller->Properties.Clicked) Draw::Quad(Assets::Closeicon, Caller->Dimensions);
        Draw::Quad(Assets::Closeicon, Caller->Dimensions);
    };
    Closebutton->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        return Newstate.Clicked;
    };
    Closebutton->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
    {
        if (Caller->Properties.Clicked && Newstate.Clicked && Caller->Properties.Focused)
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
