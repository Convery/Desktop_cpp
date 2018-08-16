/*
    Initial author: Convery (tcn@ayria.se)
    Started: 16-08-2018
    License: MIT

    Forward the userinput to the elements.
*/

#include "../Stdinclude.hpp"

// Forward user input to the elements.
namespace Engine::Input
{
    // Callbacks on user-events.
    void onMousemove(point2_t Position)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&Lambda, Position](Element_t *Target, bool Missed)
        {
            // If we're out of bounds, return early.
            if (Missed)
            {
                // Notify the children that they are not interesting.
                for (const auto &Item : Target->Childelements)
                    Lambda(Item, true);

                // Notify the element if the state changed.
                if (Target->Properties.Hoover)
                {
                    Target->Properties.Hoover = false;

                    if (Target->onHoover)
                        Target->onHoover(Target, true);
                }

                return false;
            }

            // Is the position within the elements boundries?
            if (Position.x >= Target->Dimensions.x0 && Position.x <= Target->Dimensions.x1 &&
                Position.y >= Target->Dimensions.y0 && Position.y <= Target->Dimensions.y1)
            {
                // Let the children sink the event first.
                for (const auto &Item : Target->Childelements)                
                    if (Lambda(Item, false)) 
                        return true;                

                // Notify the element if the state changed.
                if (!Target->Properties.Hoover)
                {
                    Target->Properties.Hoover = true;

                    if (Target->onHoover)
                        return Target->onHoover(Target, false);
                }

                return false;
            }

            // Else we notify the children about being out of bounds.
            return Lambda(Target, true);
        };

        Lambda(gRootelement, false);
    }
    void onMouseclick(point2_t Position, bool Released)
    {
        std::function<bool(Element_t *, bool)> Lambda = [&Lambda, Position, Released](Element_t *Target, bool Missed)
        {
            // If we're out of bounds, return early.
            if (Missed)
            {
                // Notify the children that they are not interesting.
                for (const auto &Item : Target->Childelements)
                    Lambda(Item, true);

                // Notify the element if the state changed.
                if (Target->Properties.Clicked)
                {
                    Target->Properties.Clicked = false;

                    if (Target->onClicked)
                        Target->onClicked(Target, true);
                }

                return false;
            }

            // Is the position within the elements boundries?
            if (Position.x >= Target->Dimensions.x0 && Position.x <= Target->Dimensions.x1 &&
                Position.y >= Target->Dimensions.y0 && Position.y <= Target->Dimensions.y1)
            {
                // Let the children sink the event first.
                for (const auto &Item : Target->Childelements)
                    if (Lambda(Item, false))
                        return true;

                // Notify the element if the state changed.
                if (Target->Properties.Clicked != !Released)
                {
                    Target->Properties.Clicked = !Released;

                    if (Target->onClicked)
                        return Target->onClicked(Target, Released);
                }

                return false;
            }

            // Else we notify the children about being out of bounds.
            return Lambda(Target, true);
        };

        Lambda(gRootelement, false);
    }
}
