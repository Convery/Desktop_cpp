/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Defines the applications callbacks.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Application
{
    // User-input.
    void onKeyclick(struct GLFWwindow *Handle, int Key, int Scancode, int Action, int Modifier);
    void onMouseclick(struct GLFWwindow *Handle, int Button, int Action, int Modifiers);
    void onMousescroll(struct GLFWwindow *Handle, double OffsetX, double OffsetY);
    void onMousemove(struct GLFWwindow *Handle, double PosX, double PosY);

    // Graphics.
    void onResize(struct GLFWwindow *Handle, int Width, int Height);
    void onDraw(struct GLFWwindow *Handle);

    // Pushed events.
    void onError(int Error, const char *Description);
    void onInit(struct GLFWwindow *Handle);
}

// Representation of a virtual element.
class Element_t
{
    // Internal rendering-properties.
    uint32_t VBO, VAO, EBO;
    Texture_t Texture;
    Shader_t *Shader;
    float ZIndex;

    public:
    std::vector<Element_t> Children;
    std::string Identifier;
    void *Userpointer;
    Rect Dimensions;
    Rect Margin;

    // Callbacks on events, returns if they handled it.
    std::function<bool(Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released)> onKeyboard;
    std::function<bool(Element_t *Caller, uint32_t Key, bool Released)> onClick;
    std::function<bool(Element_t *Caller, bool Released)> onFocus;

    // Create the element state.
    void Calculatedimentions(Rect Boundingbox);
    Texture_t Settexture(Texture_t Newtexture);
    void Reinitializebuffers(float ZIndex);
    void Addchild(Element_t &&Child);
    void Renderelement();

    // Initialize with default values.
    Element_t(std::string Identity, Shader_t *Shader, Texture_t Texture);
    Element_t(std::string Identity, Shader_t *Shader);
    Element_t(std::string Identity);
    Element_t();
};
