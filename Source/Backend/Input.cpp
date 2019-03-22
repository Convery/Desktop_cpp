/*
    Initial author: Convery (tcn@ayria.se)
    Started: 18-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

namespace Window
{
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
            // Mouse-movement events.
            switch (Event.message)
            {
                case WM_MOUSEMOVE:
                {
                    // Check if we need to track the mouse again.
                    if (!isTracking)
                    {
                        isTracking = true;
                        TrackMouseEvent(&Tracker);
                    }

                    // Update the global state.
                    Global.Mouse.Position = { float(LOWORD(Event.lParam)), float(HIWORD(Event.lParam)) };

                    // Invalidate all children if the parent is bad.
                    std::function<void(Element_t *)> Invalidate = [&](Element_t * Node) -> void
                    {
                        // Log the state-modification if needed, else just handle the next node.
                        if (Node->State.isHoveredover) Modifiedstates[Node].isHoveredover = true;
                        for (auto &Child : Node->Children) Invalidate(Child.get());
                    };

                    // Find if elements are hovered over.
                    std::function<bool(Element_t *)> Lambda = [&](Element_t * Node) -> bool
                    {
                        // Check if the entity is hit.
                        if (Global.Mouse.Position.x >= std::max(Node->Position.x, {}) &&
                            Global.Mouse.Position.y >= std::max(Node->Position.y, {}) &&
                            Global.Mouse.Position.x <= std::max(Node->Position.x + Node->Size.x, {}) &&
                            Global.Mouse.Position.y <= std::max(Node->Position.y + Node->Size.y, {}))
                        {
                            // Check if a child wants to sink this event first.
                            for (const auto &Child : Node->Children) if (Lambda(Child.get())) return true;

                            // Log the state-modification if needed and return.
                            Modifiedstates[Node].isHoveredover = Node->State.isHoveredover != true;
                            return Node->isExclusive && Node->isExclusive(Modifiedstates[Node]);
                        }

                        // Else we invalidate it and its children.
                        Invalidate(Node);
                        return false;
                    };

                    // Check all elements from the root down.
                    Lambda(Global.Rootelement.get());
                    continue;
                }
                case WM_MOUSELEAVE:
                {
                    // Invalidate all children.
                    std::function<void(Element_t *)> Invalidate = [&](Element_t *Node) -> void
                    {
                        // Log the state-modification if needed, else just handle the next node.
                        if (Node->State.isHoveredover) Modifiedstates[Node].isHoveredover = true;
                        else if (Modifiedstates[Node].isHoveredover) Modifiedstates[Node].isHoveredover = false;
                        for (auto &Child : Node->Children) Invalidate(Child.get());
                    };

                    Invalidate(Global.Rootelement.get());
                    isTracking = false;
                    continue;
                }
            }

            // Mouse-clicking events.
            switch (Event.message)
            {
                case WM_RBUTTONDOWN:
                {
                    Global.Mouse.Flags.isRightdown = true;
                    for (auto &[Node, State] : Modifiedstates)
                    if (!Node->State.isRightclicked) State.isRightclicked = true;
                    continue;
                }
                case WM_LBUTTONDOWN:
                {
                    Global.Mouse.Flags.isLeftdown = true;
                    for (auto &[Node, State] : Modifiedstates)
                    if (!Node->State.isLeftclicked) State.isLeftclicked = true;
                    continue;
                }
                case WM_MBUTTONDOWN:
                {
                    Global.Mouse.Flags.isMiddledown = true;
                    for (auto &[Node, State] : Modifiedstates)
                    if (!Node->State.isMiddleclicked) State.isMiddleclicked = true;
                    continue;
                }
                case WM_RBUTTONUP:
                {
                    Global.Mouse.Flags.isRightdown = false;
                    for (auto &[Node, State] : Modifiedstates)
                    if (Node->State.isRightclicked) State.isRightclicked = true;
                    continue;
                }
                case WM_LBUTTONUP:
                {
                    Global.Mouse.Flags.isLeftdown = false;
                    for (auto &[Node, State] : Modifiedstates)
                    if (Node->State.isLeftclicked) State.isLeftclicked = true;
                    continue;
                }
                case WM_MBUTTONUP:
                {
                    Global.Mouse.Flags.isMiddledown = false;
                    for (auto &[Node, State] : Modifiedstates)
                    if (Node->State.isMiddleclicked) State.isMiddleclicked = true;
                    continue;
                }
            }

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
