/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-09-2018
    License: MIT

    Provides the main area and core components.
*/

#include "../Stdinclude.hpp"

/*
    div Contentwrapper
    {
        div Background
        div Toolbar
        div Sidebar
        div Navbar
        div View
    }
*/

namespace Contentwrapper
{
    Element_t *Createbackground()
    {
        auto Element = new Element_t("Background");
        Element->onRender = [](const Element_t *Caller)
        {
            Draw::Quad({ 0xCC, 0xCC, 0xCC, 0xFF }, Caller->Dimensions);
        };

        return Element;
    }
    Element_t *Createsidebar()
    {
        auto Element = new Element_t("Sidebar");
        Element->Margins = { 0, 0, 0.95, 0};
        Element->onRender = [](const Element_t *Caller)
        {
            Draw::Quad({ 0x00, 0xCC, 0xCC, 0xFF }, Caller->Dimensions);
        };

        return Element;
    }
    Element_t *Createtoolbar()
    {
        auto Element = new Element_t("Toolbar");
        Element->Margins = { 0.05, 0, 0, 0.95 };
        Element->onRender = [](const Element_t *Caller)
        {
            Draw::Quad({ 0xCC, 0x00, 0xCC, 0xFF }, Caller->Dimensions);
        };
        Element->onFrame = [](const Element_t *Caller, double Deltatime)
        {
            static point2_t Previous{};

            if (Caller->Properties.Clicked)
            {
                if (Previous.x + Previous.y == 0) Previous = Engine::getMouseposition();
                const auto Current{ Engine::getMouseposition() };
                auto Window{ Engine::getWindowposition() };

                Window.x += (Current.x - Previous.x);
                Window.y += (Current.y - Previous.y);
                Engine::Window::Move(Window, true);
                Previous = Current;
            }
            else
            {
                Previous = {};
            }
        };

        return Element;
    }

    void Compose(Element_t *Target)
    {
        auto Element = new Element_t("Contentwrapper");
        Element->addChild(Createbackground());
        Element->addChild(Createsidebar());
        Element->addChild(Createtoolbar());
        Target->addChild(Element);

        Engine::gCurrentmenuID = Hash::FNV1a_32("contentwrapper");
        Engine::Window::Resize({ 1280, 720 }, true);
    }
}

// Register the composer for later.
namespace
{
    struct Startup
    {
        Startup()
        {
            Engine::Compositing::Registercomposer("mainwindow", Contentwrapper::Compose);
        }
    };
    Startup Loader{};
}
