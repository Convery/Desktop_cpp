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
    auto Homebutton = new Element_t("Navbar.Home");
    auto Pluginsbutton = new Element_t("Navbar.Plugins");
    auto Communitybutton = new Element_t("Navbar.Community");
    auto Leaderboardbutton = new Element_t("Navbar.Leaderboard");
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
    Accountbox->onRender = [&](Element_t *Caller)
    {
        // TODO(Convery): Replace with an avatar.
        Draw::Circle({ 50, 50, 50, 0xFF }, { Caller->Dimensions.x0 + 25, Caller->Dimensions.y0 + 30 }, 22);

        // TODO(Convery): Replace with he user-ID like "#455643".
        Draw::Quad<true>({ 0xAA, 0xAA, 0xAA, 0xFF }, { Caller->Dimensions.x0 + 5, Caller->Dimensions.y0 + 56, Caller->Dimensions.x1 - 5, Caller->Dimensions.y1 - 3 });
    };
    Boundingbox->addChild(Accountbox);

    // Next the navbar.
    Navigationbox->Properties.Fixedheight = true;
    Navigationbox->Margins = { 0, 70, 0, 270 };
    Boundingbox->addChild(Navigationbox);

    // Navigate to the frontpage / library.
    Homebutton->Properties.Fixedheight = true;
    Homebutton->Margins = { 0, 0, 0, 50 };
    Homebutton->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 50, 50, 50, 0xFF }, Caller->Dimensions);
        /* TODO(Convery): Draw the icon. */
    };
    Homebutton->onHoover = [](Element_t *Caller, bool Released)
    {
        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
        return false;
    };
    Navigationbox->addChild(Homebutton);

    // Navigate to the plugin 'store'.
    Pluginsbutton->Properties.Fixedheight = true;
    Pluginsbutton->Margins = { 0, 50, 0, 100 };
    Pluginsbutton->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 50, 50, 50, 0xFF }, Caller->Dimensions);
        /* TODO(Convery): Draw the icon. */
    };
    Navigationbox->addChild(Pluginsbutton);

    // Navigate to the social stuffs.
    Communitybutton->Properties.Fixedheight = true;
    Communitybutton->Margins = { 0, 100, 0, 150 };
    Communitybutton->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 50, 50, 50, 0xFF }, Caller->Dimensions);
        /* TODO(Convery): Draw the icon. */
    };
    Navigationbox->addChild(Communitybutton);

    // Navigate to the statistics.
    Leaderboardbutton->Properties.Fixedheight = true;
    Leaderboardbutton->Margins = { 0, 150, 0, 200 };
    Leaderboardbutton->onRender = [](Element_t *Caller)
    {
        if(Caller->Properties.Hoover) Draw::Quad({ 50, 50, 50, 0xFF }, Caller->Dimensions);
        /* TODO(Convery): Draw the icon. */
    };
    Navigationbox->addChild(Leaderboardbutton);
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
