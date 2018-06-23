/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides default callbacks for Element_t and menus.
*/

#include "../Stdinclude.hpp"

// Default callbacks for the elements.
vec4_t Createbox(vec4_t Boundingbox, vec4_t Margin)
{
    vec4_t Dimensions;

    // Calculate the dimensions.
    double Width = std::abs(Boundingbox.x1 - Boundingbox.x0) / 2;
    Dimensions.x0 = std::round(Boundingbox.x0 + Width * Margin.x0);
    Dimensions.x1 = std::round(Boundingbox.x1 - Width * Margin.x1);
    double Height = std::abs(Boundingbox.y1 - Boundingbox.y0) / 2;
    Dimensions.y0 = std::round(Boundingbox.y0 + Height * Margin.y0);
    Dimensions.y1 = std::round(Boundingbox.y1 - Height * Margin.y1);

    return Dimensions;
}
void Recalculateboxes(Element_t *Caller)
{
    Caller->Renderdimensions = Createbox(Caller->Renderbox, Caller->Margin);
    Caller->Worlddimensions = Createbox(Caller->Boundingbox, Caller->Margin);

    // Recalculate all children as well.
    for (const auto &Item : Caller->Children)
    {
        Item->Boundingbox = Caller->Worlddimensions;
        Item->Renderbox = Caller->Renderdimensions;
        Item->onModifiedstate(Item);
    }
}
void Renderelement(Element_t *Caller)
{
    if(!Caller->State.Hidden) Rendering::Soliddraw::Quad(Caller->Backgroundcolor, Caller->Renderdimensions);
}

// Elements require some sort of identifier.
Element_t::Element_t(std::string Identifier) : Identifier(Identifier)
{
    // Callbacks on user-interaction.
    onClicked = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };
    onHoover = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released;  Rendering::Invalidatearea(Caller->Renderdimensions); return false; };

    // Recalculate the properties and render them.
    onModifiedstate = Recalculateboxes;
    onRender = Renderelement;
}
