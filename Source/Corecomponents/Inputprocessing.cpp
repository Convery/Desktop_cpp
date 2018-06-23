/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides input processing.
*/

#include "../Stdinclude.hpp"

vec2_t Windowdimensions{}, Windowposition{};

#if defined(_WIN32)

namespace Input
{
    HWND Windowhandle{};

    // System-code interaction, assumes single-threaded sync.
    void onMouseclick(double PosX, double PosY, uint32_t Key, bool Released)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&](Element_t *Parent, bool Miss) -> bool
        {
            // Don't check for hits.
            if (Miss)
            {
                // Invalidate all children.
                for (const auto &Child : Parent->Children) Lambda(Child, true);

                // Notify the element.
                if (Parent->State.Clicked)
                {
                    Parent->State.Clicked = false;
                    if (Parent->State.Noinput) return false;
                    return Parent->onClicked(Parent, true);
                }

                return false;
            }

            // See if the mouse if inside the dimensions.
            bool HitX = (PosX >= Parent->Worlddimensions.x0 && PosX <= Parent->Worlddimensions.x1);
            bool HitY = (PosY >= Parent->Worlddimensions.y0 && PosY <= Parent->Worlddimensions.y1);

            // Missed.
            if (!(HitX && HitY)) return Lambda(Parent, true);

            // See if any children wants to intercept the event.
            for (const auto &Child : Parent->Children) if (Lambda(Child, false)) return true;

            // Did the state change?
            if (!!Parent->State.Clicked != !Released)
            {
                Parent->State.Clicked = !Released;
                if (Parent->State.Noinput) return false;
                return Parent->onClicked(Parent, Released);
            }

            return false;
        };

        Lambda(Rendering::Scene::getRootelement(), false);
    }
    void onKeyclick(uint32_t Key, uint32_t Modifier, bool Released) {}
    void onMousemove(double PosX, double PosY)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&](Element_t *Parent, bool Miss) -> bool
        {
            // Don't check for hits.
            if (Miss)
            {
                // Invalidate all children.
                for (const auto &Child : Parent->Children) Lambda(Child, true);

                // Notify the element.
                if (Parent->State.Hoover)
                {
                    Parent->State.Hoover = false;
                    if (Parent->State.Noinput) return false;
                    return Parent->onHoover(Parent, !Parent->State.Hoover);
                }

                return false;
            }

            // See if the mouse if inside the dimensions.
            bool HitX = (PosX >= Parent->Worlddimensions.x0 && PosX <= Parent->Worlddimensions.x1);
            bool HitY = (PosY >= Parent->Worlddimensions.y0 && PosY <= Parent->Worlddimensions.y1);

            // Missed.
            if (!(HitX && HitY)) return Lambda(Parent, true);

            // See if any children wants to intercept the event.
            for (const auto &Child : Parent->Children) if (Lambda(Child, !(HitX && HitY))) return true;

            // Did the state change?
            if (Parent->State.Hoover != HitX && HitY)
            {
                Parent->State.Hoover = HitX && HitY;
                if (Parent->State.Noinput) return false;
                return Parent->onHoover(Parent, !Parent->State.Hoover);
            }

            return false;
        };

        Lambda(Rendering::Scene::getRootelement(), false);
    }
    void onMousescroll(bool Down) {}

    // Caller-agnostic interactions.
    void onWindowresize(double Width, double Height)
    {
        Windowdimensions.x = Width; Windowdimensions.y = Height;
        SetWindowPos(Windowhandle, NULL, Windowposition.x, Windowposition.y, Windowdimensions.x, Windowdimensions.y, SWP_NOSENDCHANGING | SWP_NOMOVE);
    }
    void onWindowmove(double PosX, double PosY)
    {
        Windowposition.x = PosX; Windowposition.y = PosY;
        SetWindowPos(Windowhandle, NULL, Windowposition.x, Windowposition.y, Windowdimensions.x, Windowdimensions.y, SWP_NOSENDCHANGING | SWP_NOSIZE);
    }
    void onInit(const void *Handle)
    {
        Windowhandle = (HWND)Handle;
    }

    // User-code interaction.
    vec2_t getWindowposition()
    {
        RECT Window{};
        if (TRUE == GetWindowRect(Windowhandle, &Window))
        {
            Windowposition.x = Window.left;
            Windowposition.y = Window.top;
        }

        return Windowposition;
    }
    vec2_t getMouseposition()
    {
        POINT Mouse{};
        GetCursorPos(&Mouse);

        return
        {
            std::clamp(Mouse.x - Windowposition.x, 0.0f, Windowdimensions.x),
            std::clamp(Mouse.y - Windowposition.y, 0.0f, Windowdimensions.y)
        };
    }
    vec2_t getMonitorsize()
    {
        RECT Displaysize{};
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &Displaysize, 0);
        return { float(Displaysize.right - Displaysize.left), float(Displaysize.bottom - Displaysize.top) };
    }
    vec2_t getWindowsize()
    {
        RECT Window{};
        if (TRUE == GetWindowRect(Windowhandle, &Window))
        {
            Windowdimensions.x = Window.right - Window.left;
            Windowdimensions.y = Window.bottom - Window.top;
        }

        return Windowdimensions;
    }
    void Minimize()
    {
        ShowWindow(Windowhandle, SW_MINIMIZE);
    }
}

#else

namespace Input
{
    // System-code interaction, assumes single-threaded sync.
    void onKeyclick(uint32_t Key, uint32_t Modifier, bool Released) {}
    void onMouseclick(uint32_t Key, bool Released) {}
    void onMousemove(double PosX, double PosY) {}
    void onMousescroll(bool Down) {}

    // Caller-agnostic interactions.
    void onWindowresize(double Width, double Height) {}
    void onWindowmove(double PosX, double PosY) {}
    void onInit(const void *Handle) {}

    // User-code interaction.
    vec2_t getWindowposition() { return {}; }
    vec2_t getMouseposition() { return {}; }
    vec2_t getMonitorsize() { return {}; }
    vec2_t getWindowsize() { return {}; }
    void Minimize() {}
}

#endif
