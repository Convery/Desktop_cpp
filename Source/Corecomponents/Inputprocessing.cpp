/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides input processing.
*/

#include "../Stdinclude.hpp"

double gWidth{}, gHeight{}, gPosX{}, gPosY{};

#if defined(_WIN32)

namespace Input
{
    // System-code interaction, assumes single-threaded sync.
    void onMouseclick(double PosX, double PosY, uint32_t Key, bool Released)
    {
        PosY += gPosY; PosX += gPosX;

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
                    if (!Parent->State.Noinput) return false;
                    return Parent->onClicked(Parent, Parent->State.Clicked);
                }

                return false;
            }

            // See if the mouse if inside the dimensions.
            bool HitX = (PosX >= Parent->Dimensions.x0 && PosX <= Parent->Dimensions.x1);
            bool HitY = (PosY >= Parent->Dimensions.y0 && PosY <= Parent->Dimensions.y1);

            // See if any children wants to intercept the event.
            for (const auto &Child : Parent->Children) if (Lambda(Child, !(HitX && HitY))) return true;

            // Missed.
            if (!(HitX && HitY))
            {
                Parent->State.Clicked = false;
                if (Parent->State.Noinput) return false;
                return Parent->onClicked(Parent, Parent->State.Clicked);
            }

            // Did the state change?
            if (!!Parent->State.Clicked != !Released)
            {
                Parent->State.Clicked = !Released;
                if (Parent->State.Noinput) return false;
                return Parent->onClicked(Parent, Parent->State.Clicked);
            }


            return false;
        };

        Lambda(Rendering::getRootelement(), false);
    }
    void onKeyclick(uint32_t Key, uint32_t Modifier, bool Released) {}
    void onMousescroll(double PosX, double PosY) {}
    void onMousemove(double PosX, double PosY)
    {
        PosY += gPosY; PosX += gPosX;

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
                    if (!Parent->State.Noinput) return false;
                    return Parent->onHoover(Parent, Parent->State.Hoover);
                }

                return false;
            }

            // See if the mouse if inside the dimensions.
            bool HitX = (PosX >= Parent->Dimensions.x0 && PosX <= Parent->Dimensions.x1);
            bool HitY = (PosY >= Parent->Dimensions.y0 && PosY <= Parent->Dimensions.y1);

            // See if any children wants to intercept the event.
            for (const auto &Child : Parent->Children) if (Lambda(Child, !(HitX && HitY))) return true;

            // Did the state change?
            if (Parent->State.Hoover != HitX && HitY)
            {
                Parent->State.Hoover = HitX && HitY;
                if (Parent->State.Noinput) return false;
                return Parent->onHoover(Parent, Parent->State.Hoover);
            }

            return false;
        };

        Lambda(Rendering::getRootelement(), false);
    }

    // Caller-agnostic interactions.
    void onWindowresize(double Width, double Height)
    {
        gWidth = Width; gHeight = Height;
        SetWindowPos(NULL, NULL, gPosX, gPosY, gWidth, gHeight, SWP_NOSENDCHANGING | SWP_NOMOVE);
    }
    void onWindowmove(double PosX, double PosY)
    {
        gPosX = PosX; gPosY = PosY;
        SetWindowPos(NULL, NULL, gPosX, gPosY, gWidth, gHeight, SWP_NOSENDCHANGING | SWP_NOSIZE);
    }

    // User-code interaction.
    vec2_t getWindowposition()
    {
        RECT Window{};
        GetWindowRect(GetActiveWindow(), &Window);
        gPosX = Window.left;
        gPosY = Window.top;
        return { double(Window.left), double(Window.top) };
    }
    vec2_t getMouseposition()
    {
        POINT Mouse{}; RECT Window{};
        GetWindowRect(NULL, &Window);
        GetCursorPos(&Mouse);

        return
        {
            double(std::clamp(Mouse.x + Window.left, 0L, Window.right - Window.left)),
            double(std::clamp(Mouse.y + Window.top, 0L, Window.bottom - Window.top))
        };
    }
    vec2_t getWindowsize()
    {
        RECT Window{};
        GetWindowRect(GetActiveWindow(), &Window);
        gWidth = Window.right - Window.left;
        gHeight = Window.bottom - Window.top;
        return { double(Window.right - Window.left), double(Window.bottom - Window.top) };
    }
}

#else

namespace Input
{
    // System-code interaction, assumes single-threaded sync.
    void onKeyclick(uint32_t Key, uint32_t Modifier, bool Released) {}
    void onMouseclick(uint32_t Key, bool Released) {}
    void onMousescroll(double PosX, double PosY) {}
    void onMousemove(double PosX, double PosY) {}
    void onInit(double PosX, double PosY) {}

    // Caller-agnostic interactions.
    void onWindowresize(double Width, double Height) {}
    void onWindowmove(double PosX, double PosY) {}

    // User-code interaction.
    vec2_t getWindowposition() { return {}; }
    vec2_t getMouseposition() { return {}; }
    vec2_t getWindowsize() { return {}; }
}

#endif
