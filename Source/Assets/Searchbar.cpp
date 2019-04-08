/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-04-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

static void Createsearchbar()
{
    static auto Searchbar = Composition::Getelement("Toolbar")->Children.emplace_back(std::make_shared<Element_t>());
    Searchbar->Margins = { 0.7, 0.0, 0.285, 0.0 };
    Searchbar->onRender = []() -> void
    {
        static auto Texture{ Rendering::Creategradient({220, 223, 116, 1 }, { 114, 99, 56, 1 }, 256) };
        const size_t Remainder = (int)Searchbar->Size.x % (int)Texture.Size.x;
        const size_t Segmentcount = Searchbar->Size.x / Texture.Size.x;

        for(size_t i = 0; i < Segmentcount; ++i)
            Rendering::Drawimage(Searchbar->Position, Texture.Size, Texture.Data, { Texture.Size.x * i, Searchbar->Size.y - 3 });
        Rendering::Drawimage(Searchbar->Position, { (float)Remainder, 1 }, Texture.Data, { Searchbar->Size.x - Remainder, Searchbar->Size.y - 3 });

        /*
            TODO(tcn): Draw some text here.
        */
    };
    Composition::Registerelement("Toolbar.Searchbar", Searchbar);
}

// Create a callback for initialization on startup.
namespace { struct Startup { Startup() { Subscribetostack(Events::Enginestack, Events::Engineevent::STARTUP, Createsearchbar); } }; static Startup Loader{}; }
