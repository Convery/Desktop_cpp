/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Defines the applications callbacks.
*/

#pragma once
#include "../Stdinclude.hpp"
#include "../Graphicsbackend/Graphics.hpp"

namespace Application
{
    // User-input.
    void onKeyclick(struct GLFWwindow *Handle, int Key, int Scancode, int Action, int Modifier);
    void onMouseclick(struct GLFWwindow *Handle, int Button, int Action, int Modifiers);
    void onMousescroll(struct GLFWwindow *Handle, double OffsetX, double OffsetY);
    void onMousemove(struct GLFWwindow *Handle, double PosX, double PosY);
    void onMouseenter(struct GLFWwindow *Handle, int Entered);

    // Subscribe to the events.
    void addMouseentersub(std::function<void(int Entered)> Callback);

    // Graphics.
    void onResize(struct GLFWwindow *Handle, int Width, int Height);
    void onDraw(struct GLFWwindow *Handle);

    // Pushed events.
    void onError(int Error, const char *Description);
    void onInit(struct GLFWwindow *Handle);

    // Helpers.
    void *Windowhandle();
}

// Representation of a virtual element.
class Element_t
{
    // Internal rendering-properties.
    uint32_t VBO, VAO, EBO;
    Shader_t *Shader;

    public:
    std::vector<Element_t *> Children;
    std::string Identifier;
    Texture_t Texture{};
    void *Userpointer{};
    Rect Boundingbox{};
    Rect Dimensions{};
    float ZIndex{};
    Rect Margin{};

    // Callbacks on events, returns if they handled it.
    std::function<bool(Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released)> onKeyboard;
    std::function<bool(Element_t *Caller, uint32_t Key, bool Released)> onClick;
    std::function<bool(Element_t *Caller, bool Released)> onFocus;

    // Manage the state.
    void onModifiedstate();
    void onRender();

    // Always require an identity.
    Element_t(std::string Identity);
};
