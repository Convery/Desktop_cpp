/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-09-2018
    License: MIT

    Provides the main area and core components.
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

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
        Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
        {
            if (Newstate.Clicked && !Caller->Properties.Clicked)
            {
                static auto Lastclick{ std::chrono::high_resolution_clock::time_point() };
                if (std::chrono::high_resolution_clock::now() - Lastclick < std::chrono::milliseconds(300))
                {
                    Engine::Window::Maximize();
                }

                Lastclick = std::chrono::high_resolution_clock::now();
            }
        };

        // Toolbar buttons.
        Element->addChild([]()
        {
            auto Element = new Element_t("Close");
            Element->Properties.ExclusiveIO = true;
            Element->Margins = { 0.978, -1.001, 0.002, 0.5 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad<true>({ 0,0,0,1 }, Caller->Dimensions);
                if (Caller->Properties.Focused) Draw::Quad({ 0,0,0,1 }, Caller->Dimensions);
            };
            Element->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                return Newstate.Clicked;
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                if (Newstate.Clicked && !Newstate.Focused && Caller->Properties.Clicked && Caller->Properties.Focused)
                {
                    Engine::setErrno(Hash::FNV1a_32("Toolbar.Close"));
                }

                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Max");
            Element->Properties.ExclusiveIO = true;
            Element->Margins = { 0.956, -1.001, 0.024, 0.5 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad<true>({ 0,0,0,1 }, Caller->Dimensions);
                if (Caller->Properties.Focused) Draw::Quad({ 0,0,0,1 }, Caller->Dimensions);
            };
            Element->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                return Newstate.Clicked;
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                if (Newstate.Clicked && !Newstate.Focused && Caller->Properties.Clicked && Caller->Properties.Focused)
                {
                    Engine::Window::Maximize();
                }

                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Min");
            Element->Properties.ExclusiveIO = true;
            Element->Margins = { 0.934, -1.001, 0.046, 0.5 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad<true>({ 0,0,0,1 }, Caller->Dimensions);
                if (Caller->Properties.Focused) Draw::Quad({ 0,0,0,1 }, Caller->Dimensions);
            };
            Element->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                return Newstate.Clicked;
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                if (Newstate.Clicked && !Newstate.Focused && Caller->Properties.Clicked && Caller->Properties.Focused)
                {
                    Engine::Window::Minimize();
                }

                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());

        // Version info.
        Element->addChild([]()
        {
            auto Element = new Element_t("Version");
            Element->Margins = { 0.934, 0.5, 0.002, 0 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad({ 0,0,0,0.1 }, Caller->Dimensions);
            };

            return Element;
        }());

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
