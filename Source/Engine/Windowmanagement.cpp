/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Provides an abstraction for the platforms windowing.
*/

#include "../Stdinclude.hpp"

// Global engine-variables.
namespace Engine
{
    point2_t gWindowsize, gWindowposition;
    point4_t gDisplayrectangle;
    void    *gWindowhandle;
}

#if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
#else
namespace Engine::Window
{
    // Internal variables.
    bool Visible{ false };

    // Modify the windows visible state and notify the composition-manager.
    void Move(point2_t Position, bool Deferupdate)
    {
        gWindowposition = Position;
        SetWindowPos((HWND)gWindowhandle, NULL, gWindowposition.x, gWindowposition.y, gWindowsize.x, gWindowsize.y, SWP_NOSENDCHANGING | SWP_NOACTIVATE | SWP_NOSIZE);
        if (!Deferupdate) Compositing::Recalculateroot();
    }
    void Resize(point2_t Size, bool Deferupdate)
    {
        gWindowsize = Size;
        SetWindowPos((HWND)gWindowhandle, NULL, gWindowposition.x, gWindowposition.y, gWindowsize.x, gWindowsize.y, SWP_NOSENDCHANGING | SWP_NOACTIVATE | SWP_NOMOVE);
        if (!Deferupdate) Compositing::Recalculateroot();

        // Recreate the framebuffer.
        Rendering::Createframebuffer(gWindowsize);
    }
    void Togglevisibility()
    {
        Visible = !Visible;
        ShowWindow(HWND(gWindowhandle), BOOL(Visible));
    }
    void Centerwindow(bool Deferupdate)
    {
        Move({ int16_t(std::abs((gDisplayrectangle.x1 - gWindowsize.x) / 2)), int16_t(std::abs((gDisplayrectangle.y1 - gWindowsize.y) / 2)) }, Deferupdate);
    }

    // Initialize the manager on startup.
    struct Initializer
    {
        Initializer()
        {
            // Get the monitor dimensions.
            RECT Displaysize{};
            SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
            gDisplayrectangle = { int16_t(Displaysize.left), int16_t(Displaysize.top), int16_t(Displaysize.right), int16_t(Displaysize.bottom) };

            // Register the window.
            WNDCLASSEXA Windowclass{};
            Windowclass.cbSize = sizeof(WNDCLASSEXA);
            Windowclass.lpszClassName = "Desktop_cpp";
            Windowclass.hInstance = GetModuleHandleA(NULL);
            Windowclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
            Windowclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            Windowclass.lpfnWndProc = [](HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam) -> LRESULT
            {
                if (Message == WM_PAINT) return LRESULT(1);
                return DefWindowProcA(Handle, Message, wParam, lParam);
            };
            if (NULL == RegisterClassExA(&Windowclass))
            {
                gErrno = Hash::FNV1a_32("Registerclass");
                gShouldquit = true;
                return;
            }

            // Create a hidden window.
            gWindowhandle = CreateWindowExA(WS_EX_LAYERED, "Desktop_cpp", "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, Windowclass.hInstance, NULL);
            if (!gWindowhandle)
            {
                gErrno = Hash::FNV1a_32("Createwindow");
                gShouldquit = true;
                return;
            }

            // Use a pixel-value of {0xFF, 0xFF, 0xFF} to mean transparent, because we should not use pure white anyways.
            SetLayeredWindowAttributes((HWND)gWindowhandle, 0x00FFFFFF, 0, LWA_COLORKEY);
        }
    };
    static Initializer Loader{};
}
#endif
