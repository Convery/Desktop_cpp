/*
    Initial author: Convery (tcn@ayria.se)
    Started: 16-07-2018
    License: MIT

    Initial
*/

#include "../Stdinclude.hpp"

void Composeloginmenu(Element_t *Target)
{
    auto Boundingbox = new Element_t("loginmenu");
    Boundingbox->onRender = [](Element_t *Caller)
    {
        auto Localbox{ Caller->Dimensions };
        Localbox.x0 += 4;
        Localbox.x1 -= 4;
        Draw::PNGFile("../Assets/Menubackground.png", Localbox);
    };
    Target->addChild(Boundingbox);

    auto Loginarea = new Element_t("loginarea");
    Loginarea->Margins = { 0.3f, 0.6f, 0.3f, 1.0f };
    Boundingbox->addChild(Loginarea);

    auto Message = new Element_t("loginmessage");
    Message->Margins = { 0, 0, 0, 1.0f };
    Message->onRender = [](Element_t *Caller)
    {
        //Draw::PNGFile("../Assets/Logintext.png", Caller->Dimensions);
    };
    Loginarea->addChild(Message);




    auto Child = new Element_t("Test");
    Child->Margins = { 0.1f, 0.0f, 0.1f, 0.05f };
    Child->onRender = [](Element_t *Caller) {

        //Draw::Quad({ 12, 12, 12, 1 }, Caller->Dimensions);
        //Draw::Circle<>({ 0xFF, 12, 0xFF, 1 }, { 200, 200 }, 50);
        //Draw::Pixelmask({ 0, 0xFF, 0, 1 }, { 511, 720 }, sizeof(Pixelmask::Data::LoginBG) / 3, (Pixelmask::segment_t *)Pixelmask::Data::LoginBG);
        //Draw::Line({ 0, 0xFF, 0, 1 }, { 0, 0 }, { 500, 720 });


    };
    //Boundingbox->addChild(Child);

    // A more manageable buffer-size.
    Engine::Window::Resize({ 520, 720 }, true);
}

// Register the composer for later.
struct Startup
{
    Startup()
    {
        Engine::Compositing::Registercomposer("loginmenu", Composeloginmenu);
    }
};
static Startup Loader{};