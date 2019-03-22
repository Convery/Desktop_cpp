/*
    Initial author: Convery (tcn@ayria.se)
    Started: 18-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"
using namespace std::string_literals;

namespace Composition
{
    namespace Assets
    {
        // TODO(tcn): Replace with absl::flat_hash_map later.
        std::unordered_map<std::string, std::basic_string<uint8_t>> Fonts{};
        std::unordered_map<std::string, std::basic_string<uint8_t>> Images{};
    }

    // Recursively parse the elements as needed.
    std::unique_ptr<Element_t> Parseelement(nlohmann::json::value_type Object)
    {
        auto Element = std::make_unique<Element_t>();

        // Sanity-checking incase we move to a null-based loop later.
        if (Object.empty()) return Element;

        // Load all POD properties.
        for (const auto &Iterator : Object.items())
            if (Iterator.key() != "Elements"s)
                Element->Properties.push_back({ Iterator.key(), Iterator.value() });

        // Parse all child-elements.
        for (const auto &Item : Object["Elements"])
            Element->Children.push_back(Parseelement(Item));

        return Element;
    }

    // TODO(tcn): This should probably be constexpr in the future.
    bool ParseJSON(const std::string_view JSON)
    {
        try
        {
            const auto Parsed = nlohmann::json::parse(JSON);

            // The root can be parsed as any other element and the window-size can also be specified.
            Global.Windowsize = { Parsed.value("Width", 1280.0f), Parsed.value("Height", 720.0f) };
            Global.Rootelement = Parseelement(Parsed);
            Window::Resize(Global.Windowsize);
            return true;
        }
        catch (std::exception &e)
        {
            (void)e;
            Errorprint(va("JSON parsing error: %s", e.what()));
            return false;
        }

        return true;
    }







    #if 0
    bool Loadfromfile(const std::string &Filename)
    {
        if (auto Filehandle = std::fopen(Filename.c_str(), "rb"))
        {
            std::fseek(Filehandle, 0, SEEK_END);
            const auto Filesize = std::ftell(Filehandle);
            std::fseek(Filehandle, 0, SEEK_SET);

            auto Buffer = std::make_unique<char[]>(Filesize + 1);
            std::fread(Buffer.get(), Filesize, 1, Filehandle);
            std::fclose(Filehandle);

            try
            {
                const auto Parsed = nlohmann::json::parse(Buffer.get());
                for (const auto &Item : Parsed["Fonts"]) Fonts[Item["Name"]] = Item["Data"].get<std::string>();
                for (const auto &Item : Parsed["Images"]) Images[Item["Name"]] = Item["Data"].get<std::string>();
                for (const auto &Item : Parsed["Colors"]) Colors[Item["Name"]] = Item["Data"].get<std::string>();

                auto Rootelement = new Element_t();
                const auto Rootobject = Parsed["Rootelement"];
                for (const auto &Item : Rootobject)
                {

                }


            } catch(std::exception &e)
            {
                (void)e;
                Errorprint(va("JSON parsing error: %s", e.what()));
                return false;
            }

            return true;
        }

        return false;
    }
    #endif


}

