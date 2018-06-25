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
    vec2_t gWindowsize, gWindowposition;
    vec4_t gDisplayrectangle;
    void  *gWindowhandle;

    // Manage the window area.
    namespace Window
    {
        // Module variables.
        bool Visible{ false };

        // Create a new window centered and switch focus.
        bool Create(vec2_t Windowsize)
        {
            do
            {
                // Get the monitor dimensions.
                RECT Displaysize{};
                SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
                gDisplayrectangle = { float(Displaysize.left), float(Displaysize.top), float(Displaysize.right), float(Displaysize.bottom) };
                gWindowposition = { float(Displaysize.right - Windowsize.x) / 2, float(Displaysize.bottom - Windowsize.y) / 2 };
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
                    int(gWindowposition.x), int(gWindowposition.y), int(gWindowsize.x), int(gWindowsize.y), NULL, NULL, Windowclass.hInstance, NULL);
                if (!gWindowhandle) break;

                // Initialize GDI.
                ULONG_PTR GDIToken{};
                Gdiplus::GdiplusStartupInput GDIInput{};
                GdiplusStartup(&GDIToken, &GDIInput, NULL);

                return true;
            }
            while(false);

            return false;
        }

        // Modify the window.
        void Move(vec2_t Position)
        {
            gWindowposition = Position;
            SetWindowPos((HWND)gWindowhandle, NULL, int(gWindowposition.x), int(gWindowposition.y), int(gWindowsize.x), int(gWindowsize.y), SWP_NOSENDCHANGING | SWP_NOSIZE);
        }
        void Resize(vec2_t Size)
        {
            gWindowsize = Size;
            SetWindowPos((HWND)gWindowhandle, NULL, int(gWindowposition.x), int(gWindowposition.y), int(gWindowsize.x), int(gWindowsize.y), SWP_NOSENDCHANGING | SWP_NOMOVE);
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
