/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Defines the applications callbacks.
*/

#pragma once
#include "../Stdinclude.hpp"
#include "../Graphicsbackend/Graphics.hpp"

// Representation of a virtual element.
struct Element_t
{
    Rect Dimensions{};
    Rect Boundingbox{};
    void *Userpointer{};
    Texture_t Texture{};
    std::string Identifier;
    struct
    {
        unsigned int Active : 1;
        unsigned int Focused : 1;
        unsigned int Fixedpos : 1;
        unsigned int Reserved : 5;
    } State{};
    std::vector<Element_t *> Children;

    // Sort of internal.
    float ZIndex{};
    Rect Margin{};

    // Callbacks on state-changes.
    std::function<void(Element_t *Caller, bool Released)> onFocus{};
    std::function<void(Element_t *Caller, bool Released)> onActive{};
    std::function<void(Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released)> onKey{};

    // Do work on the element.
    void onModifiedstate();
    void onRender();

    // Inlined modifiers.
    void Restore() { onModifiedstate(); }
    void Hide() { Rect Temp = Margin;  Margin = {}; onModifiedstate(); Margin = Temp; }

    // Elements require some sort of identifier.
    Element_t(std::string Identifier);

private:
    // Internal rendering-properties.
    uint32_t VBO, VAO, EBO;
    Shader_t *Shader;
};

// Holds the applications state.
struct State_t
{
    struct GLFWwindow *Handle;
    double MouseX, MouseY;
    int Width, Height;
    double Frametime;
    Element_t *Root;
};

namespace Application
{
    // Take a pointer to the global state.
    const State_t *getState();

    // Subscriptions to input-events.
    namespace Subscriptions
    {
        void addKeyclick(Element_t *Element);
        void addMousemove(Element_t *Element);
        void addMouseclick(Element_t *Element);
        void addMousescroll(Element_t *Element);
        void addMouseenter(std::function<void(int Entered)> Callback);
    }

    // User-input callbacks.
    void onKeyclick(struct GLFWwindow *Handle, int Key, int Scancode, int Action, int Modifier);
    void onMouseclick(struct GLFWwindow *Handle, int Button, int Action, int Modifiers);
    void onMousescroll(struct GLFWwindow *Handle, double OffsetX, double OffsetY);
    void onMousemove(struct GLFWwindow *Handle, double PosX, double PosY);
    void onMouseenter(struct GLFWwindow *Handle, int Entered);

    // Graphics callbacks.
    void onResize(struct GLFWwindow *Handle, int Width, int Height);
    void onDraw(struct GLFWwindow *Handle);

    // Rare events.
    void onError(int Error, const char *Description);
    void onSceneswitch(std::string_view Scenename);
    void onInit(struct GLFWwindow *Handle);
}
