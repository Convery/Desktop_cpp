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
    point2_t getWindowsize() {}
    }
    #else
    point2_t getWindowposition()
    {
        RECT Client;
        GetClientRect((HWND)getWindowhandle(), &Client);
        return { int16_t(Client.left + getWindowsize().x / 2), int16_t(Client.top + getWindowsize().y / 2) };
    }
    point2_t getMouseposition()
    {
        POINT Origin;
        GetCursorPos(&Origin);
        return { int16_t(Origin.x), int16_t(Origin.y) };
    }
    point2_t getWindowsize()
    {
        RECT Client;
        GetClientRect((HWND)getWindowhandle(), &Client);
        return { int16_t(Client.right - Client.left), int16_t(Client.bottom - Client.top) };
    }
    #endif
}
