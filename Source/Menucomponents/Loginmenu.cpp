/*
    Initial author: Convery (tcn@ayria.se)
    Started: 16-07-2018
    License: MIT

    Initial
*/

#include "../Stdinclude.hpp"

void Composeloginmenu(Element_t *Target)
{
    auto Boundingbox = new Element_t("Content.Login");
    auto Loginbox = new Element_t("Login.Wrap");
    auto Titlebox = new Element_t("Wrap.Title");
    auto Emailbox = new Element_t("Wrap.Email");
    auto SSOBox = new Element_t("Wrap.SSO");
    auto Github = new Element_t("SSO.Github");
    auto Twitter = new Element_t("SSO.Twitter");
    auto Google = new Element_t("SSO.Googleplus");

    // Set up the background.
    Boundingbox->Margins = { 50, 20, 539, 720 };
    Boundingbox->Properties.Fixedwidth = true;
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->onRender = [](Element_t *Caller)
    {
        auto Localbox{ Caller->Dimensions };
        Localbox.x0 -= 11;
        Draw::Quad(Assets::Menubackground, Localbox);
    };
    Target->addChild(Boundingbox);

    // Limit the login-area.
    Loginbox->Margins = { 0.2f, 0.6f, 0.2f, 0.8f };
    Loginbox->onRender = [](Element_t *Caller)
    {
        //Draw::Quad<true>({ 0, 0xFF, 0, 0xFF }, Caller->Dimensions);
    };
    Boundingbox->addChild(Loginbox);

    // Notify the user what to do.
    Titlebox->Margins = { 22, 3, 368, 26 };
    Titlebox->Properties.Fixedheight = true;
    Titlebox->Properties.Fixedwidth = true;
    Titlebox->onRender = [](Element_t *Caller)
    {
        Draw::Quad(Assets::Logintext, Caller->Dimensions);
    };
    Loginbox->addChild(Titlebox);

    // Give the user the option to use an email.
    Emailbox->Margins = { 6, 36, 384, 75 };
    Emailbox->Properties.Fixedheight = true;
    Emailbox->Properties.Fixedwidth = true;
    Emailbox->onRender = [](Element_t *Caller)
    {
        Draw::Quad(Assets::Emailbox, Caller->Dimensions);
    };
    Loginbox->addChild(Emailbox);

    // Or to use another site.
    SSOBox->Margins = { 0, 81, 0, 120 };
    SSOBox->Properties.Fixedheight = true;
    Loginbox->addChild(SSOBox);

    // Like github.
    Github->Margins = { 6, 0, 130, 0 };
    Github->Properties.Fixedwidth = true;
    Github->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 0x22, 0x22, 0x22, 0xFF }, Caller->Dimensions);
        Draw::Quad(Assets::Github, Caller->Dimensions);
    };
    Github->onHoover = Github->onClicked = [](Element_t *Caller, bool Released)
    {
        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
        return false;
    };
    SSOBox->addChild(Github);

    // Or twitter.
    Twitter->Margins = { 136, 0, 256, 0 };
    Twitter->Properties.Fixedwidth = true;
    Twitter->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 0x22, 0x22, 0x22, 0xFF }, Caller->Dimensions);
        Draw::Quad(Assets::Twitter, Caller->Dimensions);
    };
    Twitter->onHoover = Twitter->onClicked = [](Element_t *Caller, bool Released)
    {
        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
        return false;
    };
    SSOBox->addChild(Twitter);

    // Or google.
    Google->Margins = { 262, 0, 385, 0 };
    Google->Properties.Fixedwidth = true;
    Google->onRender = [](Element_t *Caller)
    {
        if (Caller->Properties.Hoover) Draw::Quad({ 0x22, 0x22, 0x22, 0xFF }, Caller->Dimensions);
        Draw::Quad(Assets::Google, Caller->Dimensions);
    };
    Google->onHoover = Google->onClicked = [](Element_t *Caller, bool Released)
    {
        Engine::Rendering::Invalidatespan({ Caller->Dimensions.y0, Caller->Dimensions.y1 });
        return false;
    };
    SSOBox->addChild(Google);

#if 0

    auto Boundingbox = new Element_t("loginmenu");
    Boundingbox->Margins = { 50, 20, 539, 720 };
    Boundingbox->Properties.Fixedwidth = true;
    Boundingbox->Properties.Fixedheight = true;
    Boundingbox->onRender = [](Element_t *Caller)
    {
        auto Localbox{ Caller->Dimensions };
        Localbox.x0 -= 11;
        Draw::Quad(Assets::Menubackground, Localbox);
    };
    Target->addChild(Boundingbox);

    auto Loginarea = new Element_t("loginarea");
    Loginarea->Margins = { 0.2f, 0.6f, 0.2f, 1.0f };
    Loginarea->onRender = [](Element_t *Caller)
    {
        Draw::Quad<true>({ 0, 0xFF, 0, 0xFF }, Caller->Dimensions);
    };
    Boundingbox->addChild(Loginarea);

    auto Message = new Element_t("loginmessage");
    Message->Margins = { 0, 0, 0, 24 };
    Message->Properties.Fixedheight = true;
    Message->Properties.Fixedwidth = true;
    Message->onRender = [](Element_t *Caller)
    {
        auto Localbox{ Caller->Dimensions };
        Draw::Quad(Assets::Logintext, Localbox);
    };
    Loginarea->addChild(Message);

    auto Emailbox = new Element_t("emailbox");
    Emailbox->Margins = { 0.03f, 0.5f };
    Emailbox->onRender = [](Element_t *Caller)
    {
        auto Localbox{ Caller->Dimensions };
        Localbox.x1 = Localbox.x0 + Assets::Emailbox.Dimensions.x;
        Localbox.y1 = Localbox.y0 + Assets::Emailbox.Dimensions.y;
        Draw::Quad(Assets::Emailbox, Localbox);
    };
    Loginarea->addChild(Emailbox);

    auto Child = new Element_t("Test");
    Child->Margins = { 0.1f, 0.0f, 0.1f, 0.05f };
    Child->onRender = [](Element_t *Caller) {

        //Draw::Quad({ 12, 12, 12, 1 }, Caller->Dimensions);
        //Draw::Circle<>({ 0xFF, 12, 0xFF, 1 }, { 200, 200 }, 50);
        //Draw::Pixelmask({ 0, 0xFF, 0, 1 }, { 511, 720 }, sizeof(Pixelmask::Data::LoginBG) / 3, (Pixelmask::segment_t *)Pixelmask::Data::LoginBG);
        //Draw::Line({ 0, 0xFF, 0, 1 }, { 0, 0 }, { 500, 720 });


    };
    //Boundingbox->addChild(Child);

    #endif

    // A more manageable buffer-size.
    Engine::Window::Resize({ 539, 720 }, true);
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
