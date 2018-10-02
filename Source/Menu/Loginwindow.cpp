/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-09-2018
    License: MIT

    Provides the login-menu, which is fixed size at 500x720
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

namespace Loginmenu
{
    Element_t *Createtoolbar()
    {
        auto Element = new Element_t("Toolbar");
        Element->Margins = { 17, 0, 17, 700 };
        Element->onRender = [](const Element_t *Caller)
        {
            Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
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

        Element->addChild([]()
        {
            auto Element = new Element_t("Close");
            Element->Properties.ExclusiveIO = true;
            Element->Margins = { 0.95f, 0, 0, 0 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Closeicon, Centertexture(Caller->Dimensions, Assets::Closeicon));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Closeicon, Centertexture(Caller->Dimensions, Assets::Closeicon));
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

        return Element;
    }
    Element_t *Createmailarea()
    {
        auto Element = new Element_t("Emaillogin");
        Element->Margins = { 0.2, 0.35, 0.2, 0.5 };

        Element->addChild([]()
        {
            auto Element = new Element_t("Emailtext");
            Element->Margins = { 0, 0, 0.7, 0.85 };
            Element->onRender = [](const Element_t *Caller)
            {

                Draw::Quad(Assets::Emailprompt, Centertexture(Caller->Dimensions, Assets::Emailprompt));
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Emailtop");
            Element->Margins = { 0, 0.16, 0, 0.82 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Emailbottom");
            Element->Margins = { 0, 0.46, 0, 0.53 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Signin");
            Element->Margins = { 0.1, 0.55, 0.5, 0.2 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Signintext, Centertexture(Caller->Dimensions, Assets::Signintext));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Signintext, Centertexture(Caller->Dimensions, Assets::Signintext));
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Register");
            Element->Margins = { 0.5, 0.55, 0.1, 0.2 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Registertext, Centertexture(Caller->Dimensions, Assets::Registertext));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Registertext, Centertexture(Caller->Dimensions, Assets::Registertext));
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());

        return Element;
    }
    Element_t *Createbackground()
    {
        auto Element = new Element_t("Background");
        Element->Margins = { 7, 20, 7, 0 };
        Element->onRender = [](const Element_t *Caller)
        {
            Draw::Quad(Assets::Loginbackground, Caller->Dimensions);
        };

        return Element;
    }
    Element_t *CreateSSOarea()
    {
        auto Element = new Element_t("SSOLogin");
        Element->Margins = { 0.3, 0.55, 0.3, 0.35 };

        Element->addChild([]()
        {
            auto Element = new Element_t("Github");
            Element->Margins = { 0, 0, 0.66, 0.6 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Github, Centertexture(Caller->Dimensions, Assets::Github));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Github, Centertexture(Caller->Dimensions, Assets::Github));
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Twitter");
            Element->Margins = { 0.33, 0, 0.33, 0.6 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Twitter, Centertexture(Caller->Dimensions, Assets::Twitter));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Twitter, Centertexture(Caller->Dimensions, Assets::Twitter));
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Google");
            Element->Margins = { 0.66, 0, 0, 0.6 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Google, Centertexture(Caller->Dimensions, Assets::Google));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Google, Centertexture(Caller->Dimensions, Assets::Google));
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("SSOline");
            Element->Margins = { 0, 0.4, 0, 0.58 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
            };

            return Element;
        }());

        return Element;
    }
    Element_t *Createoffline()
    {
        auto Element = new Element_t("Offlinemode");
        Element->Margins = { 0.3, 0.9, 0.3, 0 };

        Element->addChild([]()
        {
            auto Element = new Element_t("Offlineline");
            Element->Margins = { 0, 0, 0, 0.98 };
            Element->onRender = [](const Element_t *Caller)
            {
                Draw::Quad({ 49, 48, 47, 0xFF }, Caller->Dimensions);
            };

            return Element;
        }());
        Element->addChild([]()
        {
            auto Element = new Element_t("Offlinetext");
            Element->Properties.ExclusiveIO = true;
            Element->Margins = { 0.2, 0.2, 0.2, 0.3 };
            Element->onRender = [](const Element_t *Caller)
            {
                // NOTE(Convery): This is not a typo.
                Draw::Quad(Assets::Offlinetext, Centertexture(Caller->Dimensions, Assets::Offlinetext));
                if (Caller->Properties.Focused) Draw::Quad(Assets::Offlinetext, Centertexture(Caller->Dimensions, Assets::Offlinetext));
            };
            Element->isExclusive = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                return Newstate.Clicked;
            };
            Element->onStatechange = [](const Element_t *Caller, const elementstate_t Newstate)
            {
                if (Newstate.Clicked && !Newstate.Focused && Caller->Properties.Clicked && Caller->Properties.Focused)
                {
                    Engine::Compositing::Switchcomposition("mainwindow");
                    Engine::Window::Centerwindow();
                }
                else Engine::Rendering::Invalidateregion(Caller->Dimensions);
            };

            return Element;
        }());

        return Element;
    }

    void Compose(Element_t *Target)
    {
        auto Element = new Element_t("Loginmenu");
        Element->addChild(Createbackground());
        Element->addChild(Createmailarea());
        Element->addChild(Createtoolbar());
        Element->addChild(CreateSSOarea());
        Element->addChild(Createoffline());
        Target->addChild(Element);

        Engine::gCurrentmenuID = Hash::FNV1a_32("loginmenu");
        Engine::Window::Resize({ 512, 720 }, true);
    }
}

// Register the composer for later.
namespace
{
    struct Startup
    {
        Startup()
        {
            Engine::Compositing::Registercomposer("loginwindow", Loginmenu::Compose);
        }
    };
    Startup Loader{};
}
