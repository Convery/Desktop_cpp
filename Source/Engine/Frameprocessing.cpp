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
    bool isTrackingmouse{ true };
    TRACKMOUSEEVENT Tracker{};
    uint32_t Keymodifiers{};
    PAINTSTRUCT State{};
    MSG Event{};

    // Process the window-events.
    void onFrame()
    {
        static bool Initialized{ false };
        if (unlikely(!Initialized))
        {
            // Track mouse movement.
            Tracker.dwFlags = TME_LEAVE;
            Tracker.hwndTrack = HWND(gWindowhandle);
            Tracker.cbSize = sizeof(TRACKMOUSEEVENT);
            TrackMouseEvent(&Tracker);

            // Post a message every 16ms for frame redrawing.
            SetTimer(HWND(gWindowhandle), FRAMETIMER_ID, (1000 / 60), NULL);

            Initialized = true;
        }

        while (PeekMessageA(&Event, HWND(gWindowhandle), NULL, NULL, PM_REMOVE) > 0)
        {
            /*
                NOTE(Convery):
                We'll process the two most common events
                outside of the switch. Because performance.
            */
            if (likely(Event.message == WM_PAINT))
            {
                auto Devicecontext = BeginPaint(HWND(gWindowhandle), &State);
                Rendering::onRender(Devicecontext);
                EndPaint(HWND(gWindowhandle), &State);
                continue;
            }

            /*
                NOTE(Convery):
                The system supports out-of-order frame rendering that can
                be triggered by window-movement or resizing. As such we
                just invalidate the area to trigger a redraw periodically.
            */
            if (likely(Event.message == WM_TIMER && Event.wParam == FRAMETIMER_ID))
            {
                InvalidateRect(HWND(gWindowhandle), NULL, FALSE);
                continue;
            }

            // The rest of the messages in no particular order.
            switch (Event.message)
            {
                // Mouse interaction.
                case WM_MOUSEMOVE:
                {
                    Input::onMousemove({ GET_X_LPARAM(Event.lParam), GET_Y_LPARAM(Event.lParam) });
                    if (unlikely(!isTrackingmouse)) TrackMouseEvent(&Tracker);
                    isTrackingmouse = true;
                    continue;
                }
                case WM_MOUSELEAVE:
                {
                    Input::onMousemove({ (int16_t)0x7FFF, (int16_t)0x7FFF });
                    isTrackingmouse = false;
                    continue;
                }
                case WM_RBUTTONDOWN:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_LBUTTONUP:
                {
                    Input::onMouseclick({ GET_X_LPARAM(Event.lParam), GET_Y_LPARAM(Event.lParam) }, 
                        Event.message == WM_RBUTTONUP || Event.message == WM_LBUTTONUP);
                    continue;
                }

                default:
                {
                    // Let windows handle the event if we haven't.
                    DispatchMessageA(&Event);
                }
            }

            // If we should quit, break the loop without processing the rest of the queue.
            if (unlikely(Event.message == WM_QUIT || Event.message == WM_DESTROY || (Event.message == WM_SYSCOMMAND && Event.wParam == SC_CLOSE)))
            {
                gErrno = Hash::FNV1a_32("WM_QUIT");
                return;
            }
        }
    }
}
#endif
