/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-06-2018
    License: MIT

    Provides platform-independent windows.
*/

#include "../Stdinclude.hpp"

#if defined(_WIN32)
namespace Engine
{
    // Global engine-variables.
    point2_t gWindowsize, gWindowposition;
    point4_t gDisplayrectangle;
    void    *gWindowhandle;

    // Manage the window area.
    namespace Window
    {
        // Module variables.
        bool Visible{ false };

        // Create a new window centered and switch focus.
        bool Create(point2_t Windowsize)
        {
            do
            {
                // Get the monitor dimensions.
                RECT Displaysize{};
                SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
                gDisplayrectangle = { uint16_t(Displaysize.left), uint16_t(Displaysize.top), uint16_t(Displaysize.right), uint16_t(Displaysize.bottom) };
                gWindowposition = { uint16_t((Displaysize.right - Windowsize.x) / 2), uint16_t((Displaysize.bottom - Windowsize.y) / 2) };
                gWindowsize = Windowsize;

                // Register the window.
                WNDCLASSEXA Windowclass{};
                Windowclass.cbSize = sizeof(WNDCLASSEXA);
                Windowclass.lpszClassName = "Desktop_cpp";
                Windowclass.hInstance = GetModuleHandleA(NULL);
                Windowclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
                Windowclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS | CS_DROPSHADOW;
                Windowclass.lpfnWndProc = [](HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam) -> LRESULT
                {
                    if (Message == WM_PAINT) return LRESULT(1);
                    return DefWindowProcA(Handle, Message, wParam, lParam);
                };
                if (NULL == RegisterClassExA(&Windowclass)) break;

                // Create a hidden window.
                gWindowhandle = CreateWindowExA(WS_EX_APPWINDOW, "Desktop_cpp", "", WS_POPUP,
                    gWindowposition.x, gWindowposition.y, gWindowsize.x, gWindowsize.y, NULL, NULL, Windowclass.hInstance, NULL);
                if (!gWindowhandle) break;

                return true;
            }
            while(false);

            return false;
        }

        // Modify the window.
        void Move(point2_t Position)
        {
            gWindowposition = Position;
            SetWindowPos((HWND)gWindowhandle, NULL, gWindowposition.x, gWindowposition.y, gWindowsize.x, gWindowsize.y, SWP_NOSENDCHANGING | SWP_NOSIZE);
        }
        void Resize(point2_t Size)
        {
            gWindowsize = Size;
            SetWindowPos((HWND)gWindowhandle, NULL, gWindowposition.x, gWindowposition.y, gWindowsize.x, gWindowsize.y, SWP_NOSENDCHANGING | SWP_NOMOVE);
        }
        void Togglevisibility()
        {
            Visible = !Visible;
            ShowWindow(HWND(gWindowhandle), BOOL(Visible));
        }
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif