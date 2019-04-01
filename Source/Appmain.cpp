/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#include "Stdinclude.hpp"

// Keep the global-state close.
Globalstate_t Global{};

// TODO(tcn): Move to a more appropriate module.
namespace Events
{
    Eventstack_t<Engineevent, void(void), void(void), void(double), void(void), void(void)> *Enginestack;
}

// Entrypoint for the application.
int __cdecl main(int argc, char **argv)
{
    // Only keep a log for this session.
    Logging::Clearlog();

    // As we are single-threaded (in release), boost our priority.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Touch the global state to ensure it's aligned in cache.
    if (Global.Errorno == 0) Global.Rootelement = std::make_unique<Element_t>();

    // Log the termination for tracing.
    Subscribetostack(Events::Enginestack, Events::Engineevent::TERMINATION, []()
                    { Infoprint(va("Application terminated with code %u.", Global.Errorno)); });

    // Notify all listeners that we are starting up.
    Executeevent(Events::Enginestack, Events::Engineevent::STARTUP);

    // Developers load the configuration from disk.
    #if !defined(NDEBUG)

    // Reload the file if it changes.
    std::thread([]()
    {
        FILETIME Lastresult{};

        while (true)
        {
            if (auto Filehandle = CreateFileA("../Blueprint.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))
            {
                FILETIME Localresult{};
                GetFileTime(Filehandle, NULL, NULL, &Localresult);

                if (*(uint64_t *)&Localresult != *(uint64_t *)&Lastresult)
                {
                    DWORD Bytesread{};
                    Lastresult = Localresult;
                    const auto Buffer = std::make_unique<char[]>(16 * 1024);
                    ReadFile(Filehandle, Buffer.get(), 16 * 1024, &Bytesread, NULL);
                    Composition::ParseJSON(Buffer.get());
                }

                CloseHandle(Filehandle);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

    }).detach();
    #endif

    // Main-loop, quit on error.
    while (true)
    {
        // Track the frame-time, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Thisframe{ std::chrono::high_resolution_clock::now() };

        // Poll for all messages to the main window.
        Window::Processmessages();

        // Notify the components about there being a new frame.
        Executeevent(Events::Enginestack, Events::Engineevent::TICK, std::chrono::duration<double>(Thisframe - Lastframe).count());

        // Check if we need to redraw the window-area.
        if (Global.Dirtyframe)
        {
            #if !defined(NDEBUG)
            // DEBUG: Notify components that we are going to repaint.
            Executeevent(Events::Enginestack, Events::Engineevent::PAINT);
            #endif

            // Notify Windows, the window needs repainting.
            InvalidateRect((HWND)Global.Windowhandle, NULL, FALSE);

            // Grab a handle to the window.
            PAINTSTRUCT Updateinformation{};
            auto Devicecontext = BeginPaint((HWND)Global.Windowhandle, &Updateinformation);
            auto Memorycontext = CreateCompatibleDC(Devicecontext);

            // Create a surface that we can draw to.
            auto Surface = CreateCompatibleBitmap(Devicecontext, (int)Global.Windowsize.x, (int)Global.Windowsize.y);
            auto Backup = (HBITMAP)SelectObject(Memorycontext, Surface);

            // Create a graphics object and set the screen transformation.
            Global.Drawingcontext = std::make_unique<Gdiplus::Graphics>(Memorycontext);
            Global.Drawingcontext->TranslateTransform(-Global.Windowposition.x, -Global.Windowposition.y);

            // Clear the surface to white (chroma-key for transparent).
            Global.Drawingcontext->Clear(Gdiplus::Color::White);

            // Render the dirty area.
            Rendering::Renderframe();

            #if !defined(NDEBUG)
            // DEBUG: Notify components about presenting.
            Executeevent(Events::Enginestack, Events::Engineevent::PRESENT);
            #endif

            // Present to the window.
            BitBlt(Devicecontext, 0, 0, (int)Global.Windowsize.x, (int)Global.Windowsize.y, Memorycontext, 0, 0, SRCCOPY);

            // Restore the surface and do cleanup.
            SelectObject(Memorycontext, Backup);
            DeleteDC(Memorycontext);
            DeleteObject(Surface);

            // Notify Windows, we are done painting.
            EndPaint((HWND)Global.Windowhandle, &Updateinformation);

            // This frame is cleeeean.
            Global.Dirtyframe = false;
        }

        // If we got an error, terminate.
        if (Global.Errorno) break;

        // Sleep until the next frame.
        std::this_thread::sleep_until(Lastframe + std::chrono::milliseconds(16));
        Lastframe = Thisframe;
    }

    // Notify all subscribers about terminating.
    Executeevent(Events::Enginestack, Events::Engineevent::TERMINATION);

    return 0;
}
