/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"
using namespace std::string_literals;

// Keep this around.
vec4_t Desktoparea;

// Initialization on startup.
namespace
{
    struct Startup
    {
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
            Gdiplus::GdiplusStartupInput Input; ULONG_PTR Token;
            Gdiplus::GdiplusStartup(&Token, &Input, NULL);

            // Register the window.
            WNDCLASSEXA Windowclass{};
            Windowclass.lpfnWndProc = DefWindowProc;
            Windowclass.cbSize = sizeof(WNDCLASSEXA);
            Windowclass.lpszClassName = "Desktop_cpp";
            Windowclass.hInstance = GetModuleHandleA(NULL);
            Windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
            Windowclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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
    };
    static Startup Loader{};
}

// Basic windowing operations.
namespace Window
{
    // Argument can be in pixels or percentage (<= 1.0).
    void Move(vec2_t Newposition, bool Deferredraw)
    {
        const auto Height{ std::abs(Desktoparea.y1 - Desktoparea.y0) };
        const auto Width{ std::abs(Desktoparea.x1 - Desktoparea.x0) };

        // Position given in percentage.
        if (std::abs(Newposition.x) <= 1.0) Newposition.x = Desktoparea.x + (Width - Global.Windowsize.x) * Newposition.x;
        if (std::abs(Newposition.y) <= 1.0) Newposition.y = Desktoparea.y + (Height - Global.Windowsize.y) * Newposition.y;

        // Move the window.
        Global.Windowposition = Newposition;
        SetWindowPos((HWND)Global.Windowhandle, NULL, (int)Global.Windowposition.x, (int)Global.Windowposition.y,
                     (int)Global.Windowsize.x, (int)Global.Windowsize.y, SWP_NOSENDCHANGING | SWP_NOSIZE);

        if (!Deferredraw) Forceredraw();
    }
    void Resize(vec2_t Newsize, bool Deferredraw)
    {
        const auto Height{ std::abs(Desktoparea.y1 - Desktoparea.y0) };
        const auto Width{ std::abs(Desktoparea.x1 - Desktoparea.x0) };

        // Size given in percentage.
        if (std::abs(Newsize.x) <= 1.0) Newsize.x = Width * Newsize.x;
        if (std::abs(Newsize.y) <= 1.0) Newsize.y = Height * Newsize.y;

        // Resize the window.
        Global.Windowsize = Newsize;
        SetWindowPos((HWND)Global.Windowhandle, NULL, (int)Global.Windowposition.x, (int)Global.Windowposition.y,
                     (int)Global.Windowsize.x, (int)Global.Windowsize.y, SWP_NOSENDCHANGING | SWP_NOMOVE);

        if (!Deferredraw) Forceredraw();
    }
    void Togglevisibility()
    {
        static bool isVisible{ IsWindowVisible((HWND)Global.Windowhandle) == TRUE };

        isVisible ^= true;
        ShowWindow((HWND)Global.Windowhandle, isVisible);

        if (isVisible) Forceredraw();
    }
    void Forceredraw()
    {
        // Maybe the blueprint failed to load?
        assert(Global.Rootelement);

        // The root is the same size as the window, no margins.
        Global.Rootelement->Position = Global.Windowposition;
        Global.Rootelement->Size = Global.Windowsize;

        // Save my fingers and just recurse over the elements.
        std::function<void(Element_t *)> Lambda = [&](Element_t *Node) -> void
        {
            for (const auto &Child : Node->Children)
            {
                vec4_t Margins{};

                // If a margin is provided.
                for (const auto &Item : Child->Properties)
                {
                    if (Item.first == "Margins"s)
                    {
                        try
                        {
                            std::vector<float> Parsed = nlohmann::json::parse(Item.second.c_str());
                            Parsed.resize(4); Margins = { Parsed[0], Parsed[1], Parsed[2], Parsed[3] };
                            break;
                        }
                        catch (std::exception & e) { (void)e; Errorprint(va("JSON parsing failed with: %s", e.what())); };
                    }
                }

                // Position and size given in pixels or percentage.
                if (std::abs(Margins.x0) >  1.0) Child->Size.x = Margins.x0;
                if (std::abs(Margins.y0) >  1.0) Child->Size.y = Margins.y0;
                if (std::abs(Margins.x0) <= 1.0) Child->Size.x = Node->Size.x - (Node->Size.x * Margins.x0);
                if (std::abs(Margins.y0) <= 1.0) Child->Size.y = Node->Size.y - (Node->Size.y * Margins.y0);
                if (std::abs(Margins.x1) >  1.0) Child->Position.x = Node->Position.x + Margins.x1;
                if (std::abs(Margins.y1) >  1.0) Child->Position.y = Node->Position.y + Margins.y1;
                if (std::abs(Margins.x1) <= 1.0) Child->Position.x = Node->Position.x + (Node->Size.x - Child->Size.x) * Margins.x1;
                if (std::abs(Margins.y1) <= 1.0) Child->Position.y = Node->Position.y + (Node->Size.y - Child->Size.y) * Margins.y1;

                // Update recursively.
                Lambda(Child.get());
            }
        };
        Lambda(Global.Rootelement.get());

        // We need to repaint =(
        Global.Dirtyframe = true;

        // Rebuild the callgraph.
        Rendering::Clearcallgraph();
        Rendering::Buildcallgraph(Global.Rootelement.get());
    }
}
