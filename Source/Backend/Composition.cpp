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
        phmap::flat_hash_map<std::string, std::basic_string<uint8_t>> Fonts{};
        phmap::flat_hash_map<std::string, std::basic_string<uint8_t>> Images{};
    }

    // Keep a map for by-string access to the elements.
    phmap::flat_hash_map<std::string, std::shared_ptr<Element_t>> Elements;

    // Add a name for an element (mostly for debugging).
    void Registerelement(std::string Name, std::shared_ptr<Element_t> Element)
    {
        Elements[Name] = Element;
    }
    std::shared_ptr<Element_t> Getelement(std::string Name)
    {
        if (auto Result = Elements.find(Name); Result != Elements.end())
            return Result->second;
        else return nullptr;
    }

    // Parse the JSON and update the elements.
    void Parseelement(nlohmann::json::value_type Object)
    {
        // Ensure that the element has a name to lookup.
        if (Object["Name"].is_null()) return;

        // Check if we have an element with the name.
        auto Element = Getelement(Object["Name"]);
        if (!Element) return;

        // Just update the margins for now.
        if (!Object["Margins"].is_null())
        {
            std::vector<float> Parsed = Object["Margins"];
            Parsed.resize(4); Element->Margins = { Parsed[0], Parsed[1], Parsed[2], Parsed[3] };
        }
    }

    // DEV(tcn): Updates all elements properties.
    bool ParseJSON(const std::string_view JSON)
    {
        try
        {
            const auto Parsed = nlohmann::json::parse(JSON.data());
            if (!Global.Rootelement) Global.Rootelement = std::make_unique<Element_t>();

            // Iterate over all elements and modify them.
            for (const auto &Item : Parsed["Elements"]) Parseelement(Item);

            // Update the window-size if needed.
            Global.Windowsize = { (float)atof(Parsed.value("Width", "1280").c_str()), (float)atof(Parsed.value("Height", "720").c_str()) };
            Window::Resize(Global.Windowsize);

            return true;
        }
        catch (std::exception &e)
        {
            (void)e;
            Errorprint(va("JSON parsing error: %s", e.what()));
            return false;
        }
    }
}

