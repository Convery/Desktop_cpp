/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides the main loop.
*/

#include "../Stdinclude.hpp"

#if defined(_WIN32)
#define FRAME_EVENT 42
namespace Engine
{
    TRACKMOUSEEVENT Track{};

    // Main-loop for the application, returns error.
    bool doFrame()
    {
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        static auto Lastpaint{ std::chrono::high_resolution_clock::now() };
        static bool Initialized = false;
        static MSG Event;        
        
        // Set up the system on the first pass.
        if (unlikely(!Initialized))
        {
            // Track mouse movement.
            Track.dwFlags = TME_LEAVE;
            Track.hwndTrack = HWND(gWindowhandle);
            Track.cbSize = sizeof(TRACKMOUSEEVENT);
            TrackMouseEvent(&Track);

            // Post a message every 33ms for frame redrawing.
            SetTimer(HWND(gWindowhandle), FRAME_EVENT, (1000 / 30), NULL);

            Initialized = true;
        }

        // Process the window-events.
        while (GetMessageA(&Event, HWND(gWindowhandle), NULL, NULL) > 0)
        {
            static uint32_t Keymodifiers{};

            // Present the next frame.
            if (Event.message == WM_PAINT)
            {
                PAINTSTRUCT State;
                auto Devicecontext = BeginPaint(HWND(gWindowhandle), &State);
                
                /*
                    onRender(Devicecontext);
                */

                EndPaint(HWND(gWindowhandle), &State);
                continue;
            }

            // When the timer hits 0, repaint.
            if (Event.message == WM_TIMER && Event.wParam == FRAME_EVENT)
            {
                // Trigger a redraw.
                if (std::chrono::high_resolution_clock::now() - Lastpaint > std::chrono::milliseconds(1000 / 30))
                    InvalidateRect(HWND(gWindowhandle), NULL, FALSE);

                // Time between frames, ~33ms.
                const auto Deltatime{ std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - Lastframe).count() };

                // Notify the elements about the new frame.
                std::function<void(Element_t *)> Lambda = [&](Element_t *This)
                {
                    if (This->onFrame) This->onFrame(This, Deltatime);
                    for (const auto &Item : This->Children) Lambda(Item);
                };
                assert(gRootelement);
                Lambda(gRootelement);

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
        
        /*
            TODO(Convery):
            Add some error-checking here.
        */

        return false;
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
