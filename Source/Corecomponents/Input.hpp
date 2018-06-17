/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides input processing.
*/

#pragma once
#include <cstdint>

// Some common types.
using vec2_t = struct { double x, y; };

namespace Input
{
    // System-code interaction, assumes single-threaded sync.
    void onKeyclick(uint32_t Key, uint32_t Modifier, bool Released);
    void onMouseclick(uint32_t Key, bool Released);
    void onMousescroll(double PosX, double PosY);
    void onMousemove(double PosX, double PosY);

    // Caller-agnostic interactions.
    void onWindowresize(double Width, double Height);
    void onWindowmove(double PosX, double PosY);

    // User-code interaction.
    vec2_t getWindowposition();
    vec2_t getMouseposition();
    vec2_t getWindowsize();
}
