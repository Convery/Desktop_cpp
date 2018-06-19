/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#pragma once
#include "../Stdinclude.hpp"

// A virtual element that's the core of all menus.
// The state is largely for the system, not user-code.
struct Element_t
{
    vec4_t Margin{};
    vec4_t Renderbox{};
    vec4_t Boundingbox{};
    vec4_t Worlddimensions{};
    vec4_t Renderdimensions{};
    std::string Identifier;
    struct
    {
        unsigned int Hidden : 1;
        unsigned int Hoover : 1;
        unsigned int Clicked : 1;
        unsigned int Noinput : 1;
        unsigned int Reserved : 4;
    } State{};
    rgba_t Backgroundcolor{};
    std::vector<Element_t *> Children{};

    // Callbacks on user-interaction, returns if the event is handled.
    std::function<bool (Element_t *Caller, bool Released)> onClicked;
    std::function<bool (Element_t *Caller, bool Released)> onHoover;
    std::function<void(Element_t *Caller, vec4_t Clip)> onRender;
    std::function<void(Element_t *Caller)> onModifiedstate;

    // Inlined modifiers.
    void Show() { State.Hidden = false; }
    void Hide() { State.Hidden = true; }

    // Elements require some sort of identifier.
    Element_t(std::string Identifier);
};

namespace Rendering
{
    // Internal rendering resolution.
    constexpr vec2_t Resolution{ 1280, 720 };

    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle);
    Element_t *getRootelement();
    void onRender();

    // User-code interaction.
    namespace Draw
    {
        // Basic drawing.
        void Quad(const rgba_t Color, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });
        void Line(const rgba_t Color, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });
        void Border(const rgba_t Color, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });

        // Textured drawing.
        void Texturedquad(const texture_t Texture, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });
        void Texturedline(const texture_t Texture, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });
        void Texturedborder(const texture_t Texture, const vec4_t Box, const vec4_t Clip = { 0, 0, Resolution.x, Resolution.y });
    }
    namespace Texture
    {
        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2);
        std::vector<pixel24_t> *getGradient(const texture_t ID);
    }
    void Invalidatearea(const vec4_t Box);

    // Scene-management.
    namespace Menu
    {
        void Register(std::string Name, std::function<void()> onChange);
        void Switch(std::string Name);
    }
}
