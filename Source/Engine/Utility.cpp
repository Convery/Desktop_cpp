/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Provides a platform abstraction for the core utilities.
*/

#include "../Stdinclude.hpp"

// Core properties.
namespace Engine
{
    extern point2_t gWindowsize{};

    uint32_t Errno{};
    void setErrno(uint32_t Code)
    {
        Errno = Code;
    }
    uint32_t getErrno()
    {
        return Errno;
    }

    #if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
    point2_t getWindowposition() {}
    point2_t getMouseposition() {}
    }
    #else
    point2_t getWindowposition()
    {
        RECT Window;
        GetWindowRect((HWND)gWindowhandle, &Window);
        return { int16_t(Window.left), int16_t(Window.top) };
    }
    point2_t getMouseposition()
    {
        POINT Origin;
        GetCursorPos(&Origin);
        return { int16_t(Origin.x), int16_t(Origin.y) };
    }
    #endif
}
