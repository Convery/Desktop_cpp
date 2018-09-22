/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Provides a generic interface for menu-management.
*/

#include "../Stdinclude.hpp"

namespace Engine
{
    Element_t *gRootelement{ new Element_t("Rootelement") };
    uint32_t gCurrentmenuID{};
}

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
        //Composers->find("toolbar")->second(newRootelement);
        // Devconsole

        // TODO(Convery): Perform better cleanup!!one!
        if (gRootelement)
        {
            std::function<void(Element_t *)> Lambda = [&](Element_t *Target)
            {
                for (const auto &Item : Target->Childelements) Lambda(Item);
                for (auto Iterator = Target->Childelements.rbegin(); Iterator != Target->Childelements.rend(); ++Iterator)
                    delete *Iterator;
                Target->Childelements.clear();
            };
            Lambda(gRootelement);
            delete gRootelement;
        }
        gRootelement = newRootelement;
    }

    // Recalculate the elements dimensions.
    void Recalculate()
    {
        assert(gRootelement);
        gRootelement->setDimensions({ 0, 0, gWindowsize.x, gWindowsize.y });
    }

    // Notify the elements about a tick.
    void onFrame(const double Deltatime)
    {
        // Helper to save my fingers.
        std::function<void(Element_t *)> Tick = [&](Element_t *This) -> void
        {
            if (This->onFrame) This->onFrame(This, Deltatime);
            for (const auto &Item : This->Childelements) Tick(Item);
        };

        // Update all elements.
        assert(gRootelement);
        Tick(gRootelement);
    }
}
