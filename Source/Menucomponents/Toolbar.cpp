/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides the toolbar so we can drag the window.
*/

#include "../Stdinclude.hpp"

void Createtoolbar()
{
    auto Rootelement{ Rendering::getRootelement() };

    auto Toolbar = new Element_t("ui.toolbar");
    Toolbar->Backgroundcolor = { 50, 58, 69, 1.0f };
    Toolbar->Margin = { 0, 0, 0, 1.9 };

    Rootelement->Children.push_back(Toolbar);
}

namespace
{
    struct Loader
    {
        Loader()
        {
            Rendering::Menu::Register("toolbar", Createtoolbar);
        };
    };
    static Loader Load{};
}
