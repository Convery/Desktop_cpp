/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-06-2018
    License: MIT

    Provides a unified interface for all elements.
*/

#include "../Stdinclude.hpp"
#include <nlohmann/json.hpp>

namespace Engine
{
    // Manage the compositions and assets.
    namespace Compositions
    {
        namespace Internal
        {
            // Assets that a blueprint can contain.
            #pragma pack(push, 1)
            struct Gradient_t
            {
                pixel24_t Start{};
                pixel24_t Stop{};
            };
            struct Color_t
            {
                pixel24_t Color{};
            };
            struct Area_t
            {
                std::vector<std::string> Subareas{};
                std::string onHoover{};
                std::string onClick{};
                std::string onFrame{};
                vec4_t Margin{};
            };
            struct View_t
            {
                std::vector<std::string> Areas{};
                point2_t Size;
            };

            struct Asset_t
            {
                enum
                {
                    eGradient,
                    eColor,
                    eArea,
                    eView,
                } Type;
                union
                {
                    Gradient_t *Gradient;
                    Color_t *Color;
                    Area_t *Area;
                    View_t *View;
                };
            };
            #pragma pack(pop)
        }

        // Track all our assets by name.
        std::unordered_map<std::string, Internal::Asset_t> Assetmap;

        // Read the layout from disk.
        void Parseblueprint()
        {
            std::string Filebuffer;

            // Read the blueprint.
            if (auto Filehandle = std::fopen("Blueprint.json", "rb"))
            {
                std::fseek(Filehandle, 0, SEEK_END);
                auto Length = std::ftell(Filehandle);
                std::fseek(Filehandle, 0, SEEK_SET);

                auto Buffer = std::make_unique<char[]>(Length);
                std::fread(Buffer.get(), Length, 1, Filehandle);
                std::fclose(Filehandle);

                Filebuffer = { std::move(Buffer.get()), size_t(Length) };
            }

            // Parse it, just crash on errors.
            try
            {
                auto Rootobject{ nlohmann::json::parse(Filebuffer.c_str()) };

                // Parse all the gradients.
                for (const auto &Object : Rootobject["gradients"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    std::vector<uint8_t> Start = Object["start"];
                    std::vector<uint8_t> Stop = Object["stop"];

                    if(!Entry->Gradient) Entry->Gradient = new Internal::Gradient_t();
                    Entry->Type = Entry->eGradient;

                    Entry->Gradient->Start = { Start[0], Start[1], Start[2] };
                    Entry->Gradient->Stop = { Stop[0], Stop[1], Stop[2] };
                }

                // Parse all areas
                for (const auto &Object : Rootobject["areas"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    std::vector<float> Margin = Object["margin"];

                    if(!Entry->Area) Entry->Area = new Internal::Area_t();
                    Entry->Type = Entry->eArea;

                    for (const auto &Item : Object["subareas"]) Entry->Area->Subareas.push_back(Item.get<std::string>());
                    Entry->Area->Margin = { Margin[0], Margin[1], Margin[2], Margin[3] };
                    Entry->Area->onHoover = Object["onhoover"].get<std::string>();
                    Entry->Area->onClick = Object["onclick"].get<std::string>();
                    Entry->Area->onFrame = Object["onframe"].get<std::string>();
                }

                // Parse all views.
                for (const auto &Object : Rootobject["views"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];

                    if(!Entry->View) Entry->View = new Internal::View_t();
                    Entry->Type = Entry->eView;

                    Entry->View->Size.x = Object["width"];
                    Entry->View->Size.y = Object["height"];
                    for (const auto &Item : Object["areas"]) Entry->View->Areas.push_back(Item.get<std::string>());
                }
            }
            catch (std::exception &e)
            {
                Infoprint(e.what());
            }
        }

        // Switch focus to another composition.
        void Switch(const std::string &&Name)
        {
            if (auto Entry = Assetmap.find(Name); Entry != Assetmap.end() && Entry->second.Type == Entry->second.eView)
            {
                Window::Resize(Entry->second.View->Size);
            }
        }
    }
}
