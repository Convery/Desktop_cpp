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
    auto Accountbox = new Element_t("Sidebar.Account");
    auto Navigationbox = new Element_t("Sidebar.Navbar");
    auto Launchbox = new Element_t("Sidebar.Quicklaunch");

    // Limit the size to 50px.
    Boundingbox->Properties.Fixedwidth = true;
    Boundingbox->Margins = { 0, 0, 50, 0 };
    Boundingbox->onRender = [](Element_t *Caller)
    {
        Draw::Quad({ 0x22, 0x22, 0x22, 0xFF }, Caller->Dimensions);
    };
    Target->addChild(Boundingbox);

    // Have the account info up top.
    Accountbox->Properties.Fixedheight = true;
    Accountbox->Margins = { 0, 0, 0, 70 };
    Accountbox->onRender = [](Element_t *Caller)
    {
        Draw::Circle<>({ 50, 50, 50, 0xFF }, { Caller->Dimensions.x0 + 25, Caller->Dimensions.y0 + 30 }, 22);
        Draw::Quad<true>({ 50, 50, 50, 0xFF }, { Caller->Dimensions.x0 + 5, Caller->Dimensions.y1, Caller->Dimensions.x1 - 5, Caller->Dimensions.y1 });
        Draw::Quad<true>({ 0xAA, 0xAA, 0xAA, 0xFF }, { Caller->Dimensions.x0 + 5, Caller->Dimensions.y0 + 56, Caller->Dimensions.x1 - 5, Caller->Dimensions.y1 - 3 });
    };
    Boundingbox->addChild(Accountbox);
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
