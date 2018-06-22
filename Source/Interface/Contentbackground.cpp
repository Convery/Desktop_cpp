/*
    Initial author: Convery (tcn@ayria.se)
    Started: 22-06-2018
    License: MIT

    Create a random background on startup.
*/

#include "../Stdinclude.hpp"

static auto Goldgradient{ Rendering::Texture::Creategradient(64, { 255, 255, 168, 1.0f }, { 246, 201, 76, 1.0f }) };
void Createbackground()
{
    auto Rootelement{ Rendering::Scene::getRootelement() };
    static std::vector<vec2_t> Points;
    Goldgradient.Alpha = 0.1f;
    Points.reserve(64);

    // Create some nice bubbles for the background.
    srand(time(NULL));
    for (int i = 0; i < 64; i++)
    {
        Points.push_back({ float(rand() % size_t(Rendering::Resolution.x)), float(rand() % size_t(Rendering::Resolution.y)) });
    }

    // Bounding box.
    auto Background = new Element_t("ui.background");
    Background->State.Noinput = true;
    Background->onRender = [&](Element_t *Caller) -> void
    {
        auto Box{ Caller->Renderdimensions };

        // Background
        Rendering::Soliddraw::Quad({ 0x0C, 0x0C, 0x0C, 1.0f }, Box);

        // Bubbles.
        for (const auto &Item : Points)
        {
            Rendering::Textureddraw::Circle(Goldgradient, Item, float(*(uint64_t *)Item.Raw % 31));
        }
    };
    Rootelement->Children.push_back(Background);
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
