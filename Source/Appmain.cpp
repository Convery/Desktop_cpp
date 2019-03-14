/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#include "Stdinclude.hpp"

// TODO(tcn): Move to its own module.
void doStartup()
{
    // Register the window.
    WNDCLASSEXA Windowclass{};
    Windowclass.cbSize = sizeof(Windowclass);
    Windowclass.lpszClassName = "Desktop_cpp";
    Windowclass.hInstance = GetModuleHandleA(NULL);
    Windowclass.hbrBackground = CreateSolidBrush(RGB(0x33, 0x33, 0x33));
    Windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    Windowclass.style = CS_HREDRAW || CS_VREDRAW || CS_OWNDC;
    Windowclass.lpfnWndProc = [](HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        if (Message == WM_PAINT) return LRESULT(1);
        return DefWindowProcA(Handle, Message, wParam, lParam);
    };

    RegisterClassExA(&Windowclass);
    Global::State.Windowhandle = CreateWindowExA(WS_EX_APPWINDOW,
                                                 "Desktop_cpp", "Testing", WS_POPUP, 200, 200, 1280, 720, NULL, NULL, Windowclass.hInstance, NULL);
    ShowWindow((HWND)Global::State.Windowhandle, SW_SHOWNORMAL);
    Global::State.Dirtyregion = { 0, 0, 1280, 720 };
    UpdateWindow((HWND)Global::State.Windowhandle);
    Global::State.Windowposition = { 200, 200 };

    Gdiplus::GdiplusStartupInput Input; ULONG_PTR Token;
    Gdiplus::GdiplusStartup(&Token, &Input, NULL);
}

// Keep the global-state close.
namespace Global { Globalstate_t State{}; };

// Entrypoint for the application.
int __cdecl main(int argc, char **argv)
{
    // Only keep a log for this session.
    Logging::Clearlog();

    // As we are single-threaded (in release), boost our priority.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    /*
        1. Subscribe to termination events.
        2. Fire startup events.
    */

    /*
        1. Load the blueprint from file.
        2. Create a thread to poll the file for changes.
        3. Reload as needed.
    */

    doStartup();

    // Main-loop, quit on error.
    while (true)
    {
        // Track the frame-time, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Thisframe{ std::chrono::high_resolution_clock::now() };

        /*
            1.  Check input.
            2.  Fire a tick event.
        */

        // Check if we need to redraw the window-area.
        auto &Region{ Global::State.Dirtyregion };
        if (*(uint64_t *)&Region.Raw[0] || *(uint64_t *)&Region.Raw[2])
        {
            // Notify Windows, the window needs repainting.
            InvalidateRect((HWND)Global::State.Windowhandle, NULL, FALSE);

            // Grab a handle to the window.
            PAINTSTRUCT Updateinformation{};
            auto Devicecontext = BeginPaint((HWND)Global::State.Windowhandle, &Updateinformation);
            auto Memorycontext = CreateCompatibleDC(Devicecontext);

            // Create a surface that we can draw to.
            auto Surface = CreateCompatibleBitmap(Devicecontext, (int)(Region.x1 - Region.x0), (int)(Region.y1 - Region.y0));
            auto Backup = (HBITMAP)SelectObject(Memorycontext, Surface);

            // Create a graphics object and set the screen transformation.
            Global::State.Drawingcontext = std::make_unique<Gdiplus::Graphics>(Memorycontext);
            Global::State.Drawingcontext->TranslateTransform(-Region.x0 - Global::State.Windowposition.x, -Region.y0 - Global::State.Windowposition.y);

            // Clear the surface to white (chroma-key for transparent).
            Global::State.Drawingcontext->Clear(Gdiplus::Color::White);

            /*
                doRendering.
            */

            // Present to the window.
            BitBlt(Devicecontext, (int)Region.x0, (int)Region.y0, (int)(Region.x1 - Region.x0), (int)(Region.y1 - Region.y0), Memorycontext, 0, 0, SRCCOPY);

            // Restore the surface and do cleanup.
            SelectObject(Memorycontext, Backup);
            DeleteDC(Memorycontext);
            DeleteObject(Surface);

            // Notify Windows, we are done painting.
            EndPaint((HWND)Global::State.Windowhandle, &Updateinformation);
        }

        // If we got an error, terminate.
        if (Global::State.Errorno) break;

        // Sleep until the next frame.
        std::this_thread::sleep_until(Lastframe + std::chrono::milliseconds(16));
        Lastframe = Thisframe;
    }

    // Notify all subscribers about terminating.
    // TODO^

    return 0;
}
