/*
    Initial author: Convery (tcn@ayria.se)
    Started: 27-06-2018
    License: MIT

    Provides a unified interface for all elements.
    TODO(Convery): Rewrite this entire module later.
*/

#include "../Stdinclude.hpp"
#include <nlohmann/json.hpp>

namespace Engine
{
    Element_t *gRootelement{};

    // Manage the compositions and assets.
    namespace Compositions
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

        // Track all our assets by name.
        std::unordered_map<std::string, Asset_t> Assetmap;
        std::unordered_map<std::string, std::function<void(Element_t *Caller, double Deltatime)>> Framemap;
        std::unordered_map<std::string, std::function<bool(Element_t *Caller, bool Released)>> Hoovermap;
        std::unordered_map<std::string, std::function<bool(Element_t *Caller, bool Released)>> Clickmap;

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
                    gWindowsize = { Object["width"].get<int16_t>(), Object["height"].get<int16_t>() };
                    gWindowposition =
                    {
                        gDisplayrectangle.x0 + (gDisplayrectangle.x1 - gDisplayrectangle.x0 - gWindowsize.x) / 2,
                        gDisplayrectangle.y0 + (gDisplayrectangle.y1 - gDisplayrectangle.y0 - gWindowsize.y) / 2
                    };
                    Window::Move(gWindowposition);
                    Window::Resize(gWindowsize);

                    auto Newroot = new Element_t(Name);
                    std::swap(gRootelement, Newroot);
                    if (Newroot)
                    {
                        std::thread([](Element_t *Newroot) { std::this_thread::sleep_for(std::chrono::seconds(5)); delete Newroot; }, Newroot).detach();
                    }

                    for (const auto &Item : Object["areas"])
                    {
                        std::function<void(Element_t *, std::string *)> Lambda = [&Lambda](Element_t *This, std::string *Content)
                        {
                            if (!Content)
                            {
                                #if !defined(NDEBUG)
                                Debugprint(va("Missing asset: %s", This->Identity.c_str()));
                                #endif
                                return;
                            }

                            try
                            {
                                auto Object = nlohmann::json::parse(Content->c_str());
                                std::vector<float> Margin = Object["margin"];

                                This->Margin = { Margin[0], Margin[1], Margin[2], Margin[3] };
                                if (auto Entry = Framemap.find(Object["onframe"]); Entry != Framemap.end())
                                    This->onFrame = Entry->second;
                                if (auto Entry = Hoovermap.find(Object["onhoover"]); Entry != Hoovermap.end())
                                    This->onHoover = Entry->second;
                                if (auto Entry = Clickmap.find(Object["onclick"]); Entry != Clickmap.end())
                                    This->onClick = Entry->second;

                                for (const auto &Item : Object["subareas"])
                                {
                                    auto Child = new Element_t(Item.get<std::string>());
                                    Child->Worldbox = This->Worldbox;
                                    Child->Localbox = This->Localbox;

                                    Lambda(Child, Findasset(Item));
                                    This->Children.push_back(Child);
                                }
                            }
                            catch (std::exception &e)
                            {
                                Infoprint(e.what());
                            }
                        };

                        auto Child = new Element_t(Item.get<std::string>());
                        Lambda(Child, Findasset(Item));
                        gRootelement->Children.push_back(Child);
                    }

                    onWindowchange();
                }
                catch (std::exception &e)
                {
                    Infoprint(e.what());
                }
            }
        }

        // Dump an asset if available.
        std::string *Findasset(const std::string &&Name)
        {
            if (auto Entry = Assetmap.find(Name); Entry != Assetmap.end())
                return &Entry->second.Content;
            else
                return nullptr;
        }

        // Register callbacks.
        void addFrame(const std::string &&Name, std::function<void(Element_t *Caller, double Deltatime)> Callback)
        {
            Framemap[Name] = Callback;
        }
        void addHoover(const std::string &&Name, std::function<bool(Element_t *Caller, bool Released)> Callback)
        {
            Hoovermap[Name] = Callback;
        }
        void addClick(const std::string &&Name, std::function<bool(Element_t *Caller, bool Released)> Callback)
        {
            Clickmap[Name] = Callback;
        }

        // We need to update all coordinates when the window changes.
        void onWindowchange()
        {
            auto Createbox = [](point4_t Boundingbox, vec4_t Margin) -> point4_t
            {
                point4_t Dimensions;

                // Calculate the dimensions.
                double Width = std::abs(Boundingbox.x1 - Boundingbox.x0) / 2;
                Dimensions.x0 = std::round(Boundingbox.x0 + Width * Margin.x0);
                Dimensions.x1 = std::round(Boundingbox.x1 - Width * Margin.x1);
                double Height = std::abs(Boundingbox.y1 - Boundingbox.y0) / 2;
                Dimensions.y0 = std::round(Boundingbox.y0 + Height * Margin.y0);
                Dimensions.y1 = std::round(Boundingbox.y1 - Height * Margin.y1);

                return Dimensions;
            };
            std::function<void(Element_t *, Element_t *)> Lambda = [&Lambda, &Createbox](Element_t *Parent, Element_t *This)
            {
                This->Worldbox = Createbox(Parent->Worldbox, This->Margin);
                This->Localbox = Createbox(Parent->Localbox, This->Margin);

                for (const auto &Child : This->Children) Lambda(This, Child);
            };

            gRootelement->Worldbox = { gWindowposition.x, gWindowposition.y, gWindowposition.x + gWindowsize.x, gWindowposition.y + gWindowsize.y };
            gRootelement->Localbox = { 0, 0, gRenderingresolution.x, gRenderingresolution.y };
            Lambda(gRootelement, gRootelement);
        }
    }
}
