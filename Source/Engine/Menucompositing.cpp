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
    Element_t *oldRootelement{};
    std::unordered_map<std::string, std::function<void(Element_t *Target)>> *Composers{};

    // Recalculate all elements dimensions for when the window changes.
    void Recalculateroot()
    {
        if (oldRootelement) delete oldRootelement;
        Element_t *newRootelement = new Element_t("Root");
        newRootelement->Dimensions = { 0, 0, gWindowsize.x, gWindowsize.y };
        for (const auto &Child : gRootelement->Childelements) newRootelement->addChild(Child);

        oldRootelement = gRootelement;
        gRootelement = newRootelement;
    }

    // Remove the old root and recreate.
    void Switchscene(std::string &&Name)
    {
        if (oldRootelement) delete oldRootelement;
        auto newRootelement = new Element_t("Root");
        if (!Composers) Composers = new std::unordered_map<std::string, std::function<void(Element_t *)>>();

        // Return a new composition.
        if (const auto Result = Composers->find(Name); Result != Composers->end())
        {
            Result->second(newRootelement);

            /*
                TODO(Convery):
                Some recursive lookup to do more composing to the element.
            */
        }
        else
        {
            auto Errorelement = new Element_t("I_am_error");
            Errorelement->onRender = [](Element_t *Caller) { Draw::Quad({ 0xFF, 0, 0, 1 }, Caller->Dimensions); };
            newRootelement->addChild(Errorelement);
        }

        oldRootelement = gRootelement;
        gRootelement = newRootelement;
    }

    // Register callbacks for scene-creation.
    void Registercomposer(std::string &&Name, std::function<void(Element_t *Target)> Callback)
    {
        if (!Composers) Composers = new std::unordered_map<std::string, std::function<void(Element_t *)>>();
        (*Composers)[Name] = Callback;
    }
}
