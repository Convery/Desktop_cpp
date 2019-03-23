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

    // Keep a map for by-string access to the elements.
    std::unordered_map<std::string, std::shared_ptr<Element_t>> Elements;

    // Add a name for an element (mostly for debugging).
    void Registerelement(std::string Name, std::shared_ptr<Element_t> Element)
    {
        Elements[Name] = Element;
    }
    std::shared_ptr<Element_t> Getelement(std::string Name)
    {
        if (auto Result = Elements.find(Name); Result != Elements.end())
            return Result->second;
        else return {};
    }

    // Recursively parse the elements as needed.
    std::shared_ptr<Element_t> Parseelement(nlohmann::json::value_type Object)
    {
        auto Element = std::make_shared<Element_t>();

        // Sanity-checking incase we move to a null-based loop later.
        if (Object.empty()) return Element;

        // Load all POD properties.
        for (const auto &Iterator : Object.items())
            if (Iterator.key() != "Elements"s)
                Element->Properties.push_back({ Iterator.key(), Iterator.value() });

        // Register the element if named.
        if (!Object["Name"].is_null())
            Registerelement(Object["Name"].get<std::string>(), Element);

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
            const auto Parsed = nlohmann::json::parse(JSON.data());
            Elements.clear();

            // The root can be parsed as any other element, but it has to be moved because specs, and the window-size can also be specified.
            Global.Windowsize = { (float)atof(Parsed.value("Width", "1280").c_str()), (float)atof(Parsed.value("Height", "720").c_str()) };
            Global.Rootelement = std::make_unique<Element_t>(*Parseelement(Parsed));
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

