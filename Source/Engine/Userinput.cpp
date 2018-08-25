/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Forwards user input to the elements.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Engine::Input
{
    std::unordered_map<Element_t *, elementstate_t> Dirtystates;
    bool isTrackingmouse{ true };
    TRACKMOUSEEVENT Tracker{};
    uint32_t Keymodifiers{};
    MSG Event{};

    // Callbacks on user-events.
    void onMouseclick(point2_t Position, bool Released)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&](Element_t *Target, bool Missed)
        {
            // If we're already out of bounds, return early.
            if (Missed)
            {
                // Notify the children that they are not interesting.
                for (const auto &Item : Target->Childelements)
                    Lambda(Item, true);

                // Mark the element as dirty.
                if (Target->State.Clicked)
                {
                    Dirtystates[Target].Clicked = false;
                }

                return false;
            }

            // Is the position within the elements boundaries?
            if (Position.x >= Target->Dimensions.x0 && Position.x <= Target->Dimensions.x1 &&
                Position.y >= Target->Dimensions.y0 && Position.y <= Target->Dimensions.y1)
            {
                // Let the children sink the event first.
                for (const auto &Item : Target->Childelements)
                    if (Lambda(Item, false))
                        return true;

                // Mark the element as dirty.
                if (Target->State.Clicked != !Released)
                {
                    Dirtystates[Target].Clicked = !Released;

                    if (Target->State.ExclusiveIO && Target->isExclusive)
                        if (Target->isExclusive(Target, { 2 }))
                            return true;
                }

                return false;
            }

            // Else we notify the children about being out of bounds.
            return Lambda(Target, true);
        };

        Lambda(getRootelement(), false);
    }
    void onMousemove(point2_t Position)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&](Element_t *Target, bool Missed)
        {
            // If we're already out of bounds, return early.
            if (Missed)
            {
                // Notify the children that they are not interesting.
                for (const auto &Item : Target->Childelements)
                    Lambda(Item, true);

                // Mark the element as dirty.
                if (Target->State.Focused)
                {
                    Dirtystates[Target].Focused = false;
                }

                return false;
            }

            // Is the position within the elements boundaries?
            if (Position.x >= Target->Dimensions.x0 && Position.x <= Target->Dimensions.x1 &&
                Position.y >= Target->Dimensions.y0 && Position.y <= Target->Dimensions.y1)
            {
                // Let the children sink the event first.
                for (const auto &Item : Target->Childelements)
                    if (Lambda(Item, false))
                        return true;

                // Mark the element as dirty.
                if (!Target->State.Focused)
                {
                    Dirtystates[Target].Focused = true;

                    if (Target->State.ExclusiveIO && Target->isExclusive)
                        if (Target->isExclusive(Target, { 1 }))
                            return true;
                }

                return false;
            }

            // Else we notify the children about being out of bounds.
            return Lambda(Target, true);
        };

        Lambda(getRootelement(), false);
    }

    // Process any and all window-events.
    void onFrame()
    {
        static bool Initialized{ false };
        if (unlikely(!Initialized))
        {
            // Track mouse movement.
            Tracker.dwFlags = TME_LEAVE;
            Tracker.cbSize = sizeof(TRACKMOUSEEVENT);
            Tracker.hwndTrack = HWND(getWindowhandle());
            TrackMouseEvent(&Tracker);

            Initialized = true;
        }

        // Poll for any new events.
        while (PeekMessageA(&Event, HWND(getWindowhandle()), NULL, NULL, PM_REMOVE) > 0)
        {
            switch (Event.message)
            {
                // Mouse interaction.
                case WM_MOUSEMOVE:
                {
                    onMousemove({ GET_X_LPARAM(Event.lParam), GET_Y_LPARAM(Event.lParam) });
                    if (unlikely(!isTrackingmouse)) TrackMouseEvent(&Tracker);
                    isTrackingmouse = true;
                    continue;
                }
                case WM_MOUSELEAVE:
                {
                    onMousemove({ (int16_t)0x7FFF, (int16_t)0x7FFF });
                    isTrackingmouse = false;
                    continue;
                }
                case WM_RBUTTONDOWN:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_LBUTTONUP:
                {
                    onMouseclick({ GET_X_LPARAM(Event.lParam), GET_Y_LPARAM(Event.lParam) },
                        Event.message == WM_RBUTTONUP || Event.message == WM_LBUTTONUP);
                    continue;
                }

                default:
                {
                    // Let windows handle the event if we haven't.
                    DispatchMessageA(&Event);
                }
            }

            // If we should quit, break the loop without processing the rest of the queue.
            if (unlikely(Event.message == WM_QUIT || Event.message == WM_DESTROY || (Event.message == WM_SYSCOMMAND && Event.wParam == SC_CLOSE)))
            {
                setErrno(Hash::FNV1a_32("WM_QUIT"));
                return;
            }
        }

        // Notify the elements about any updates.
        for (const auto &Item : Dirtystates)
        {
            Item.first->onStatechange(Item.first, Item.second);
            Item.first->State = Item.second;
        }
        Dirtystates.clear();
    }
}
