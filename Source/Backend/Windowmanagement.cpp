/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

// Keep this around.
vec4_t Desktoparea;

// Initialization on startup.
namespace
{
    struct Startup
    {
        ULONG_PTR Token;

        Startup()
        {
            RECT Area{};

            // Fetch information about our environment.
            SystemParametersInfoA(SPI_GETWORKAREA, 0, &Area, 0);

            // Convert to engine-format.
            Desktoparea = { float(Area.left), float(Area.top), float(Area.right), float(Area.bottom) };
            const auto Height{ std::abs(Desktoparea.y1 - Desktoparea.y0) };
            const auto Width{ std::abs(Desktoparea.x1 - Desktoparea.x0) };

            // Initialize GDI.
            Gdiplus::GdiplusStartupInput Input;
            Gdiplus::GdiplusStartup(&Token, &Input, NULL);

            // Register the window.
            WNDCLASSEXA Windowclass{};
            Windowclass.cbSize = sizeof(WNDCLASSEXA);
            Windowclass.lpszClassName = "Desktop_cpp";
            Windowclass.hInstance = GetModuleHandleA(NULL);
            Windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
            Windowclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            #if !defined(NDEBUG) // If debugging, make transparent areas pink.
            Windowclass.hbrBackground = CreateSolidBrush(RGB(0xFF, 0x00, 0xFF));
            #endif
            Windowclass.lpfnWndProc = [](HWND Handle, UINT Message, WPARAM wParam, LPARAM lParam) -> LRESULT
            {
                if (Message == WM_PAINT) return LRESULT(1);
                return DefWindowProcA(Handle, Message, wParam, lParam);
            };
            if (NULL == RegisterClassExA(&Windowclass)) Global.Errorno = Hash::FNV1a_32("RegisterClass");

            // Create the window as 'hidden', i.e. size of 0.
            // NOTE(tcn): This prevents windows from flashing a black frame over the desktop on startup.
            auto Windowhandle = CreateWindowExA(WS_EX_LAYERED | WS_EX_APPWINDOW, "Desktop_cpp", NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, Windowclass.hInstance, NULL);
            if (NULL == Windowhandle) { Global.Errorno = Hash::FNV1a_32("CreateWindow"); return; }
            Global.Windowhandle = Windowhandle;

            // Use a pixel-value of {0xFF, 0xFF, 0xFF} to mean transparent, because we should not use pure white anyways.
            if (FALSE == SetLayeredWindowAttributes(Windowhandle, 0xFFFFFF, 0, LWA_COLORKEY))
            { Global.Errorno = Hash::FNV1a_32("Windowattributes"); return; }

            // Resize the window to the default size (1280*720).
            // TODO(tcn): Save the window size and load from a config.
            Global.Windowposition = { Desktoparea.x + (Width - 1280) * 0.5f, Desktoparea.y + (Height - 720) * 0.5f };
            SetWindowPos(Windowhandle, NULL, (int)Global.Windowposition.x, (int)Global.Windowposition.y, 1280, 720, SWP_NOSENDCHANGING);
            ShowWindow(Windowhandle, SW_SHOWNORMAL);
        }
        ~Startup()
        {
            // Windows 10 gets upset if not called directly when treated as a UWP app.
            Gdiplus::GdiplusShutdown(Token);
        }
    };
    static Startup Loader{};
}

