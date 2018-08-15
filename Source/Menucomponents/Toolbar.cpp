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
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->Margins = { 0, 0, 0, 20 };
    Boundingbox->onRender = [](Element_t *Caller)
    {
        Draw::Quad({ 97, 72, 47, 0xFF }, Caller->Dimensions);
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
