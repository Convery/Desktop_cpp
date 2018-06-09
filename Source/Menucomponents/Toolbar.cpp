/*
    Initial author: Convery (tcn@ayria.se)
    Started: 09-06-2018
    License: MIT

    Implement a toolbar for borderless windows.
*/

#include "../Stdinclude.hpp"

// A fancy toolbar.
Element_t *Components::Createtoolbar()
{
    auto Boundingbox = new Element_t("ui.toolbar");
    Boundingbox->Texture = Graphics::Createtexture({0.1f, 0.1f, 0.1f, 1 });
    Boundingbox->ZIndex = -0.2f;
    Boundingbox->Margin = { 0, 1.9, 0, 0 };

    return Boundingbox;
};
