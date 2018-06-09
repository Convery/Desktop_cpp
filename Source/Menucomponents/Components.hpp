/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Simple elements for building menus.
*/

#pragma once
#include "../Stdinclude.hpp"
#include "../Application/Application.hpp"

namespace Components
{
    // Simple developer-console.
    Element_t *Createdevconsole();

    // A fancy toolbar.
    Element_t *Createtoolbar();

    // Borders for resizing.
    Element_t *Creeateborders();
}
