/*
    Initial author: Convery (tcn@ayria.se)
    Started: 18-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

namespace Window
{
    inline bool Hitcheck(Element_t *Node)
    {
        return { Global.Mouse.Position.x >= std::max(Node->Position.x, {}) &&
                 Global.Mouse.Position.y >= std::max(Node->Position.y, {}) &&
                 Global.Mouse.Position.x <= std::max(Node->Position.x + Node->Size.x, {}) &&
                 Global.Mouse.Position.y <= std::max(Node->Position.y + Node->Size.y, {}) };
    }
    void Processmessages()
    {
        // Track when the mouse leaves the window.
        MSG Event{};
        static bool isTracking{};
        static TRACKMOUSEEVENT Tracker
        { sizeof(TRACKMOUSEEVENT), TME_LEAVE, HWND(Global.Windowhandle) };

        // Used for delayed element-modification until all are notified.
        std::unordered_map<Element_t *, Elementstate_t> Modifiedstates;

        // Non-blocking polling for messages.
        while (PeekMessageA(&Event, (HWND)Global.Windowhandle, NULL, NULL, PM_REMOVE) > 0)
        {
            // Process the most common cases first.
            if (Event.message == WM_MOUSEMOVE)
            {
                Global.Mouse.Position =
                {
                    Global.Windowposition.x + float(LOWORD(Event.lParam)),
                    Global.Windowposition.y + float(HIWORD(Event.lParam))
                };

                // Check if we need to track the mouse again.
                if (!isTracking)
                {
                    isTracking = true;
                    TrackMouseEvent(&Tracker);
                }

                // Check all elements from the root down.
                std::function<bool(Element_t *)> Lambda = [&](Element_t *Node) -> bool
                {
                    const auto isHit{ Hitcheck(Node) };

                    // Check if a child wants to sink this event first.
                    for (const auto &Child : Node->Children) if (Lambda(Child.get())) return true;

                    // Else we may sink it.
                    if (Node->State.isHoveredover != isHit)
                    {
                        Modifiedstates[Node].isHoveredover = true;
                        return Node->isExclusive && Node->isExclusive(Modifiedstates[Node]);
                    }
                    return false;
                };
                Lambda(Global.Rootelement.get());
                continue;
            }
            if (Event.message == WM_MOUSELEAVE)
            {
                // Invalidate all states.
                std::function<void(Element_t *)> Lambda = [&](Element_t *Node)
                {
                    if (Node->State.Raw) Modifiedstates[Node].Raw = Node->State.Raw;
                    for (const auto &Item : Node->Children) Lambda(Item.get());
                };
                Lambda(Global.Rootelement.get());
                isTracking = false;
                continue;
            }

            // Mouse-clicking events.
            if (Event.message >= WM_LBUTTONDOWN && Event.message < WM_MOUSELAST)
            {
                // Update the global state.
                switch (Event.message)
                {
                    case WM_MBUTTONDOWN: Global.Mouse.Flags.isMiddledown = true; break;
                    case WM_RBUTTONDOWN: Global.Mouse.Flags.isRightdown = true; break;
                    case WM_LBUTTONDOWN: Global.Mouse.Flags.isLeftdown = true; break;
                    case WM_MBUTTONUP: Global.Mouse.Flags.isMiddledown = false; break;
                    case WM_RBUTTONUP: Global.Mouse.Flags.isRightdown = false; break;
                    case WM_LBUTTONUP: Global.Mouse.Flags.isLeftdown = false; break;
                }

                // Check all elements from the root down.
                std::function<bool(Element_t *)> Lambda = [&](Element_t *Node) -> bool
                {
                    if (Hitcheck(Node))
                    {
                        // Check if a child wants to sink this event first.
                        for (const auto &Child : Node->Children) if (Lambda(Child.get())) return true;

                        // Set the local state and check if this element wants to sink the event.
                        if (Node->State.isLeftclicked != Global.Mouse.Flags.isLeftdown) Modifiedstates[Node].isLeftclicked = true;
                        if (Node->State.isRightclicked != Global.Mouse.Flags.isRightdown) Modifiedstates[Node].isRightclicked = true;
                        if (Node->State.isMiddleclicked != Global.Mouse.Flags.isMiddledown) Modifiedstates[Node].isMiddleclicked = true;
                        if (Modifiedstates.find(Node) != Modifiedstates.end()) return Node->isExclusive && Node->isExclusive(Modifiedstates[Node]);
                    }

                    return false;
                };
                Lambda(Global.Rootelement.get());
                continue;
            }

            // If Windows wants us to redraw, we oblige.
            if (Event.message == WM_PAINT) Global.Dirtyframe = true;

            // If we should quit, break the loop without processing the rest of the queue.
            if (Event.message == WM_SYSCOMMAND && Event.wParam == SC_CLOSE)
            { Global.Errorno = Hash::FNV1a_32("WM_QUIT"); return; }

            // If we couldn't handle the event, let Windows do it.
            DispatchMessageA(&Event);
        }

        // Notify all modified elements about the update.
        for (auto &[Element, State] : Modifiedstates)
        {
            if (State.Raw != 0)
            {
                if (Element->onStatechange) Element->onStatechange(State);
                Element->State.Raw ^= State.Raw;
            }
        }
    }
}
