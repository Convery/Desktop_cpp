/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

#if defined(_WIN32)

int __stdcall WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
    Gdiplus::GdiplusStartupInput GDIInput{};
    ULONG_PTR GDIToken{};
    RECT Displaysize{};
    MSG Message{};

    // Clear the old log so we only have this session.
    Clearlog(); Infoprint("Initializing session..");

    // Get the monitor dimensions.
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);

    // Register the window.
    WNDCLASSEXA Windowclass{};
    Windowclass.cbSize = sizeof(WNDCLASSEXA);
    Windowclass.lpszClassName = "Desktop_cpp";
    Windowclass.hInstance = GetModuleHandleA(NULL);
    Windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    Windowclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    Windowclass.lpfnWndProc = [](HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        if (Message == WM_PAINT) return LRESULT(1);
        return DefWindowProcA(Handle, Message, wParam, lParam);
    };
    RegisterClassExA(&Windowclass);

    // Initialize GDI.
    GDIInput.SuppressBackgroundThread = TRUE;
    GdiplusStartup(&GDIToken, &GDIInput, NULL);
    while (true) Sleep(1); return 1;

    // Create the window.
    auto Handle = CreateWindowExA(WS_EX_APPWINDOW, "Desktop_cpp", "", WS_POPUP,
        (Displaysize.right - 1280) / 2, (Displaysize.bottom - 720) / 2, 1280, 720, NULL, NULL, Windowclass.hInstance, NULL);

    // Set up the input-handling.
    TRACKMOUSEEVENT Track{};
    Track.cbSize = sizeof(TRACKMOUSEEVENT);
    Track.dwFlags = TME_LEAVE;
    Track.hwndTrack = Handle;
    TrackMouseEvent(&Track);
    //Input::onInit(Handle);

    // Cache window properties.
    ShowWindow(Handle, SW_SHOWNORMAL);
    //Input::getWindowposition();
    //Input::getWindowsize();

    //// Create the initial scene and render the first frame.
    //Rendering::Invalidatearea({ 0, 0, Rendering::Resolution.x, Rendering::Resolution.y });
    //Rendering::Scene::getRootelement()->State.Noinput = true;
    //Rendering::Scene::getRootelement()->State.Hidden = true;
    //Rendering::Scene::Switch("homescreen");
    //Rendering::onRender();

    // Render in a separate thread.
    std::thread([&]() -> void
    {
        while (true)
        {
            static auto Lastframe{ std::chrono::high_resolution_clock::now() };
            constexpr std::chrono::microseconds Framedelay{ 1000000 / 60 };

            InvalidateRect(Handle, NULL, FALSE);
            std::this_thread::sleep_until(Lastframe + Framedelay / 4);

            //Rendering::onRender();
            Lastframe += Framedelay;
            std::this_thread::sleep_until(Lastframe);
        }
    }).detach();

    // Loop until we crash.
    while (GetMessageA(&Message, NULL, 0, 0) > 0)
    {
        static uint32_t Keymodifiers{};
        static PAINTSTRUCT State{};

        // Render the next frame.
        if (Message.message == WM_PAINT)
        {
            auto Devicecontext = BeginPaint(Handle, &State);
            //Rendering::onPresent(Devicecontext);
            EndPaint(Handle, &State);
            continue;
        }

        // Mouse input.
        if (Message.message == WM_MOUSEMOVE)
        {
            //Input::onMousemove(GET_X_LPARAM(Message.lParam), GET_Y_LPARAM(Message.lParam));
            TrackMouseEvent(&Track);
            continue;
        }
        if (Message.message == WM_LBUTTONDOWN || Message.message == WM_LBUTTONUP)
        {
            //Input::onMouseclick(GET_X_LPARAM(Message.lParam), GET_Y_LPARAM(Message.lParam), 0, Message.message == WM_LBUTTONUP);
            continue;
        }
        if (Message.message == WM_RBUTTONDOWN || Message.message == WM_RBUTTONUP)
        {
            //Input::onMouseclick(GET_X_LPARAM(Message.lParam), GET_Y_LPARAM(Message.lParam), 1, Message.message == WM_RBUTTONUP);
            continue;
        }
        if (Message.message == WM_MOUSEWHEEL)
        {
            //Input::onMousescroll(GET_WHEEL_DELTA_WPARAM(Message.wParam) < 0);
            continue;
        }
        if (Message.message == WM_MOUSELEAVE)
        {
            //Input::onMouseclick(90000, 90000, 0, true);
            //Input::onMousemove(90000, 90000);
            continue;
        }

        // Keyboard input.
        if (Message.message == WM_KEYDOWN || Message.message == WM_KEYUP)
        {
            switch (Message.wParam)
            {
                case VK_LSHIFT:
                case VK_RSHIFT:
                case VK_SHIFT:
                    Keymodifiers |= 0x01;
                    break;

                default:
                    //Input::onKeyclick(Message.wParam, Keymodifiers, Message.message == WM_KEYUP);
                    break;
            }
            continue;
        }

        // If we should quit, let Windows clean it up.
        if (Message.message == WM_QUIT || Message.message == WM_DESTROY || (Message.message == WM_SYSCOMMAND && Message.wParam == SC_CLOSE))
        {
            // Save a timestamp for tracing.
            Infoprint("Session terminated..");
            std::exit(0);
        }

        // Let windows handle the message if we haven't.
        DispatchMessageA(&Message);
    }

    return 0;
}

#else

int main(int argc, char **argv)
{
    return 0;
}

#endif
