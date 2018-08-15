/*
    Initial author: Convery (tcn@ayria.se)
    Started: 15-07-2018
    License: MIT

    Provides a generic interface for menu-management.
*/

#include "../Stdinclude.hpp"

// Global engine-variables.
namespace Engine { Element_t *gRootelement{}; }

// Manage the scenes and compositing.
namespace Engine::Compositing
{
    // Local engine-variables.
    std::unordered_map<std::string, std::function<void(Element_t *)>> *Composers{};

    // Callbacks for composition creation.
    void Registercomposer(std::string &&Name, std::function<void(Element_t *)> Callback)
    {
        if (!Composers) Composers = new std::unordered_map<std::string, std::function<void(Element_t *)>>();
        (*Composers)[Name] = Callback;
    }

    // Recreate the root element with another focus.
    void Switchcomposition(std::string &&Name)
    {
        auto newRootelement = new Element_t("Rootelement");
        if (!Composers) Composers = new std::unordered_map<std::string, std::function<void(Element_t *)>>();

        // Add the content.
        if (const auto Result = Composers->find(Name); Result != Composers->end())
        {
            Result->second(newRootelement);
        }

        // Add the default topmost elements.
        Composers->find("toolbar")->second(newRootelement);
        Composers->find("sidebar")->second(newRootelement);

        if(gRootelement) delete gRootelement;
        gRootelement = newRootelement;
    }

    // Recalculate the elements dimensions.
    void Recalculate()
    {
        assert(gRootelement);

        auto newRootelement = new Element_t("Rootelement");
        newRootelement->Dimensions = { 0, 0, gWindowsize.x, gWindowsize.y - 1 };
        for (const auto &Child : gRootelement->Childelements) newRootelement->addChild(Child);

        delete gRootelement;
        gRootelement = newRootelement;
    }
}
