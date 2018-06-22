/*
    Initial author: Convery (tcn@ayria.se)
    Started: 22-06-2018
    License: MIT

    Create a random background on startup.
*/

#include "../Stdinclude.hpp"

void Createbackground()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };
    static std::vector<vec2_t> Points;
    Points.reserve(64);

    // Create some nice bubbles for the background.
    srand(time(NULL));
    for (int i = 0; i < 64; i++)
    {
        Points.push_back({ float(rand() % size_t(Rendering::Resolution.x)), float(rand() % size_t(Rendering::Resolution.y)) });
    }

    // Bounding box.
    auto Background = new Element_t("ui.background");
    Rootelement->Children.push_back(Background);
    Background->State.Noinput = true;
    Background->onRender = [&](Element_t *Caller) -> void
    {
        // Background
        Rendering::Soliddraw::Quad({ 0x0C, 0x0C, 0x0C, 1.0f }, Caller->Renderdimensions);

        // Bubbles.
        for (const auto &Item : Points)
        {
            Rendering::Soliddraw::Circle({0xca, 0x95, 0x5e, float((*(uint64_t *)Item.Raw % 25 + 1)) / 100}, Item, float(*(uint64_t *)Item.Raw % 35));
        }

        // Why anti-alias when you can just blur everything?
        Rendering::Effectdraw::Blur(Caller->Renderdimensions);
    };

}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Scene::Register("contentbackground", Createbackground);
        };
    };
    static Loader Load{};
}
