/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides the main loop.
*/

#include "../Stdinclude.hpp"

#if defined(_WIN32)
#define REDRAW_EVENT 42
namespace Engine
{
    TRACKMOUSEEVENT Track{};

    // Main-loop for the application, returns error.
    bool doFrame(double Deltatime)
    {
        static MSG Event;
        static bool Initialized = false;
        if (!Initialized)
        {
            // Track mouse movement.
            Track.dwFlags = TME_LEAVE;
            Track.hwndTrack = HWND(gWindowhandle);
            Track.cbSize = sizeof(TRACKMOUSEEVENT);
            TrackMouseEvent(&Track);

            // Post a message every 30ms for frame redrawing.
            SetTimer(HWND(gWindowhandle), REDRAW_EVENT, 1000 / 30, NULL);

            Initialized = true;
        }

        // Process the window-events.
        while (PeekMessageA(&Event, HWND(gWindowhandle), NULL, NULL, PM_REMOVE) > 0)
        {
            static uint32_t Keymodifiers{};
            static PAINTSTRUCT State{};

            // Present the next frame.
            if (Event.message == WM_PAINT)
            {
                auto Devicecontext = BeginPaint(HWND(gWindowhandle), &State);
                Rendering::onPresent(Devicecontext);
                EndPaint(HWND(gWindowhandle), &State);
                continue;
            }

            // When the timer hits 0, repaint.
            if (Event.message == WM_TIMER && Event.wParam == REDRAW_EVENT)
            {
                InvalidateRect(HWND(gWindowhandle), NULL, FALSE);
                continue;
            }

            // If we should quit, break the loop early.
            if (Event.message == WM_QUIT || Event.message == WM_DESTROY || (Event.message == WM_SYSCOMMAND && Event.wParam == SC_CLOSE))
            {
                return true;
            }

            // Let windows handle the event if we haven't.
            DispatchMessageA(&Event);
        }

        // Start rendering the next frame.
        Rendering::onRender();

        // Notify the elements about the new frame.
        std::function<void(Element_t *)> Lambda = [&](Element_t *This)
        {
            if (This->onFrame) This->onFrame(This, Deltatime);
            for (const auto &Item : This->Children) Lambda(Item);
        };
        Lambda(gRootelement);

        return false;
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
