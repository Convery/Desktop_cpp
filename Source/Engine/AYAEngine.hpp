/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides an engine system to be platform independent.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Engine
{
    // Global engine-variables.
    extern vec2_t gWindowsize, gWindowposition;
    extern vec4_t gDisplayrectangle;
    extern void  *gWindowhandle;

    // Main-loop for the application, returns error.
    bool doFrame(double Deltatime);

    // Manage the window area.
    namespace Window
    {
        // Create a new window centered and switch focus.
        bool Create(vec2_t Windowsize);

        // Modify the window.
        void Move(vec2_t Position);
        void Resize(vec2_t Size);
        void Togglevisibility();
    }

    // Handle the compositions and bitblt.
    namespace Rendering
    {



    }
}
