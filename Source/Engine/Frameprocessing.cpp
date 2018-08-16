/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Processes the input and events for the current frame.
*/

#include "../Stdinclude.hpp"

#if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
#else
namespace Engine::Window
{
    #define FRAMETIMER_ID 3242
    uint32_t Keymodifiers{};
    TRACKMOUSEEVENT Track{};
    PAINTSTRUCT State{};
    MSG Event{};

    // Process the window-events.
    void onFrame()
    {
        static bool Initialized{ false };
        if (unlikely(!Initialized))
        {
            // Track mouse movement.
            Track.dwFlags = TME_LEAVE;
            Track.hwndTrack = HWND(gWindowhandle);
            Track.cbSize = sizeof(TRACKMOUSEEVENT);
            TrackMouseEvent(&Track);

            // Post a message every 16ms for frame redrawing.
            SetTimer(HWND(gWindowhandle), FRAMETIMER_ID, (1000 / 60), NULL);

            Initialized = true;
        }

        while (PeekMessageA(&Event, HWND(gWindowhandle), NULL, NULL, PM_REMOVE) > 0)
        {
            // Render the next frame.
            if (Event.message == WM_PAINT)
            {
                auto Devicecontext = BeginPaint(HWND(gWindowhandle), &State);
                Rendering::onRender(Devicecontext);
                EndPaint(HWND(gWindowhandle), &State);
                continue;
            }

            // When the timer hits 0, repaint.
            if (Event.message == WM_TIMER && Event.wParam == FRAMETIMER_ID)
            {
                InvalidateRect(HWND(gWindowhandle), NULL, FALSE);
                continue;
            }

            // On mouse movement.
            if (Event.message == WM_MOUSEMOVE)
            {
                Input::onMousemove({ GET_X_LPARAM(Event.lParam), GET_Y_LPARAM(Event.lParam) });
                TrackMouseEvent(&Track);
                continue;
            }

            // When the mouse exits the client area.
            if (Event.message == WM_MOUSELEAVE)
            {
                Input::onMousemove({ (int16_t)0xFFFF, (int16_t)0xFFFF });
                continue;
            }

            // If we should quit, break the loop early.
            if (Event.message == WM_QUIT || Event.message == WM_DESTROY || (Event.message == WM_SYSCOMMAND && Event.wParam == SC_CLOSE))
            {
                gErrno = Hash::FNV1a_32("VM_QUIT");
                return;
            }

            // Let windows handle the event if we haven't.
            DispatchMessageA(&Event);
        }
    }
}
#endif
