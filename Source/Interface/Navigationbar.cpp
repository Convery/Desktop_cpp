/*
    Initial author: Convery (tcn@ayria.se)
    Started: 23-06-2018
    License: MIT

    Have some navigation and quick-launch icons.
*/

#include "../Stdinclude.hpp"

extern texture_t Goldgradient;
extern texture_t Brassgradient;
void Createnavbar()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };

    // Rotate the gradient.
    //std::swap(Brassgradient.Height, Brassgradient.Width);
    //Brassgradient.Alpha = 0.9f;

    // Bounding box.
    auto Navbar = new Element_t("ui.navbar");
    Rootelement->Children.push_back(Navbar);
    Navbar->Margin = { 0, 0, 1.9f, 0 };
    Navbar->State.Noinput = true;
    Navbar->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };
        Rendering::Textureddraw::Quad(Brassgradient, Caller->Renderdimensions);
        //Rendering::Textureddraw::Line(Brassgradient, { Box.x0, Box.y0 }, { Box.x0, Box.y1 });
        //Rendering::Effectdraw::Blur(Box);
    };
}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Scene::Register("navbar", Createnavbar);
        };
    };
    static Loader Load{};
}
