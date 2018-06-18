/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides default callbacks for Element_t and menus.
*/

#include "../Stdinclude.hpp"

namespace Rendering
{
    // The root for all menus.
    Element_t *getRootelement()
    {
        static Element_t Root{ "ui" };
        return &Root;
    }

    // Menu management.
    namespace Menu
    {
        std::unordered_map<std::string /* Menuname */, std::function<void()> /* Builder */> *Menumap;

        void Register(std::string Name, std::function<void()> onChange)
        {
            if (!Menumap) Menumap = new std::unordered_map<std::string, std::function<void()>>();
            Menumap->emplace(Name, onChange);
        }
        void Switch(std::string Name)
        {
            if (!Menumap) Menumap = new std::unordered_map<std::string, std::function<void()>>();
            auto Root{ getRootelement() };

            // Add the default elements.
            Root->Children.clear();
            Menumap->find("toolbar")->second();

            // Build the rest of the menu.
            if (auto Entry = Menumap->find(Name); Entry != Menumap->end()) Entry->second();

            // Reinitialize the root.
            Root->onModifiedstate(Root);
        }
    }
}

// Default callbacks for the elements.
rect_t Createbox(rect_t Boundingbox, rect_t Margin)
{
    rect_t Dimensions;

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
    static auto Resolution{ Rendering::getResolution() };
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
    if(!Caller->State.Hidden) Rendering::Draw::Quad(Caller->Backgroundcolor, Caller->Renderdimensions);
}

// Elements require some sort of identifier.
Element_t::Element_t(std::string Identifier) : Identifier(Identifier)
{
    // Callbacks on user-interaction.
    onClicked = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };
    onHoover = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };

    // Recalculate the properties and render them.
    onModifiedstate = Recalculateboxes;
    onRender = Renderelement;
}
