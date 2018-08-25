/*
    Initial author: Convery (tcn@ayria.se)
    Started: 25-08-2018
    License: MIT

    Provides a generic interface for menu-management.
*/

#include "../Stdinclude.hpp"

namespace Engine
{
    Element_t *Rootelement{ new Element_t("Rootelement") };
    Element_t *getRootelement()
    {
        return Rootelement;
    }
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
        Composers->find("toolbar")->second(newRootelement);
        //Composers->find("sidebar")->second(newRootelement);

        // TODO(Convery): Perform better cleanup!!one!
        if (Rootelement)
        {
            std::function<void(Element_t *)> Lambda = [&](Element_t *Target)
            {
                for (const auto &Item : Target->Childelements) Lambda(Item);
                for (auto Iterator = Target->Childelements.rbegin(); Iterator != Target->Childelements.rend(); ++Iterator)
                    delete *Iterator;
                Target->Childelements.clear();
            };
            Lambda(Rootelement);
            delete Rootelement;
        }
        Rootelement = newRootelement;
    }

    // Recalculate the elements dimensions.
    void Recalculate()
    {
        assert(Rootelement);
        Rootelement->setDimensions({ 0, 0, getWindowsize().x, getWindowsize().y });
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
        assert(Rootelement);
        Tick(Rootelement);
    }
}
