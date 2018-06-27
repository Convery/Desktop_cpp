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
        #pragma pack(push, 1)
        // Assets that a blueprint can contain.
        struct Gradient_t { pixel24_t Start{}, Stop{}; };
        struct Color_t { pixel24_t Color{}; };
        struct Element_t
        {
            // The dimensions of the element.
            point4_t Localbox{};
            point4_t Worldbox{};
            vec4_t Margin{};

            // State so we only update when needed.
            struct
            {
                unsigned int Hoover : 1;
                unsigned int Clicked : 1;
                unsigned int Reserved : 6;
            } State{};

            // Children inherit the parents worldbox.
            std::vector<Element_t *> Children{};

            // Callbacks on user-interaction, returns if the event is handled.
            std::string onHoover{}, onClick{}, onFrame{};
        };
        struct Scene_t
        {
            std::vector<std::string> Children{};
            point2_t Windowsize{};
        };
        struct Asset_t
        {
            enum
            {
                eGradient,
                eElement,
                eColor,
                eScene,
            } Type;
            union
            {
                Gradient_t Gradient;
                Element_t Element;
                Color_t Color;
                Scene_t Scene;
            };

            ~Asset_t() {}
            Asset_t() {}
        };
        #pragma pack(pop)

        // Track all our assets by name.
        std::unordered_map<std::string, Asset_t> Assetmap;

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

                    Entry->Gradient.Start = { Start[0], Start[1], Start[2] };
                    Entry->Gradient.Stop = { Stop[0], Stop[1], Stop[2] };
                    Entry->Type = Entry->eGradient;
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

        }
    }
}
