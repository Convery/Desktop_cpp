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
            struct Asset_t
            {
                enum
                {
                    eGradient,
                    eColor,
                    eArea,
                    eView,
                } Type;
                std::string Content;
            };
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

            // Parse the blueprint.
            try
            {
                auto Rootobject{ nlohmann::json::parse(Filebuffer.c_str()) };

                for (const auto &Object : Rootobject["gradients"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    Entry->Content = Object.dump();
                    Entry->Type = Entry->eGradient;
                }
                for (const auto &Object : Rootobject["colors"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    Entry->Content = Object.dump();
                    Entry->Type = Entry->eColor;
                }
                for (const auto &Object : Rootobject["areas"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    Entry->Content = Object.dump();
                    Entry->Type = Entry->eArea;
                }
                for (const auto &Object : Rootobject["views"])
                {
                    auto Entry = &Assetmap[Object["name"].get<std::string>()];
                    Entry->Content = Object.dump();
                    Entry->Type = Entry->eView;
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
                try
                {
                    auto Object = nlohmann::json::parse(Entry->second.Content.c_str());
                    Window::Resize({Object["width"].get<int16_t>(), Object["height"].get<int16_t>()});
                }
                catch (std::exception &e)
                {
                    Infoprint(e.what());
                }
            }
        }
    }
}
