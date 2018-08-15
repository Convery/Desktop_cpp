/*
    Initial author: Convery (tcn@ayria.se)
    Started: 15-08-2018
    License: MIT

    Provides quick-launch options for the games,
    and some navigation.. I guess..
*/

#include "../Stdinclude.hpp"

void Composesidebar(Element_t *Target)
{
    auto Boundingbox = new Element_t("Sidebar");
    Boundingbox->Properties.Fixedwidth = true;
    Boundingbox->Margins = { 0, 0, 50, 0 };
    Boundingbox->onRender = [](Element_t *Caller)
    {
        Draw::Quad({ 0x22, 0x22, 0x22, 0xFF }, Caller->Dimensions);
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
            Engine::Compositing::Registercomposer("sidebar", Composesidebar);
        }
    };
    Startup Loader{};
}
