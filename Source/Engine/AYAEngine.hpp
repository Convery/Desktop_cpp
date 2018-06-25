/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides an engine system to be platform independent.
*/

#pragma once
#include "../Stdinclude.hpp"

// Will represent an object in the compositions.
struct Element_t;

namespace Engine
{
    // Global engine-variables.
    extern point2_t gWindowsize, gWindowposition;
    extern point4_t gDisplayrectangle;
    extern Element_t *gRootelement;
    extern void  *gWindowhandle;

    // Main-loop for the application, returns error.
    bool doFrame(double Deltatime);

    // Manage the window area.
    namespace Window
    {
        // Create a new window centered and switch focus.
        bool Create(point2_t Windowsize);

        // Modify the window.
        void Move(point2_t Position);
        void Resize(point2_t Size);
        void Togglevisibility();
    }

    // Manage the compositions and assets.
    namespace Compositions
    {
        // Read the layout from disk.
        void Parseblueprint();

        // Switch focus to another composition.
        void Switch(const std::string &&Name);
    }
}
