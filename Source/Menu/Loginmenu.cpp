/*
    Initial author: Convery (tcn@ayria.se)
    Started: 26-08-2018
    License: MIT

    Provides the login-menu, which is fixed size at 500x720
*/

#include "../Stdinclude.hpp"
#include "Assets.hpp"

void Composeloginmenu(Element_t *Target)
{
    auto Boundingbox = new Element_t("Loginmenu");

    // The content is window-height - 20 for the toolbar.
    Boundingbox->onRender = [](const Element_t *Caller)
    {
        auto Local{ Caller->Dimensions };
        Local.x0 += 1; Local.x1 -= 1; Local.y0 += 20;
        Draw::Quad(Assets::Loginbackground, Local);
    };
    Target->addChild(Boundingbox);

    Engine::gCurrentmenuID = Hash::FNV1a_32("loginmenu");
}

// Register the composer for later.
namespace
{
    struct Startup
    {
        Startup()
        {
            Engine::Compositing::Registercomposer("loginmenu", Composeloginmenu);
        }
    };
    Startup Loader{};
}
