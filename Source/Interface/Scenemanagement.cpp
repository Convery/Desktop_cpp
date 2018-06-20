/*
    Initial author: Convery (tcn@ayria.se)
    Started: 20-06-2018
    License: MIT

    Hold the root element and switch between scenes.
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

            // Build the rest of the menu.
            if (auto Entry = Menumap->find(Name); Entry != Menumap->end()) Entry->second();

            // Reinitialize the root.
            Root->onModifiedstate(Root);
        }
    }
}
