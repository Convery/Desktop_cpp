/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Process userinput.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    extern int gWidth, gHeight;
    std::vector<std::function<void(int Entered)>> Subscribers{};

    // User-input.
    void onKeyclick(struct GLFWwindow *Handle, int Key, int Scancode, int Action, int Modifier)
    {
        std::function<bool(Element_t *)> Lambda = [&](Element_t *Element) -> bool
        {
            for (auto &Item : Element->Children) if (Lambda(Item)) return true;
            return Element->onKeyboard(Element, Key, Modifier, Action == GLFW_RELEASE);
        };
        Lambda((Element_t *)glfwGetWindowUserPointer(Handle));
    }
    void onMouseclick(struct GLFWwindow *Handle, int Button, int Action, int Modifiers)
    {
        double X, Y;
        glfwGetCursorPos(Handle, &X, &Y);
        Y = gHeight - Y;

        std::function<bool(Element_t *)> Lambda = [&](Element_t *Element) -> bool
        {
            if (Element->Dimensions.y0 <= Y && Element->Dimensions.y1 >= Y && Element->Dimensions.x0 <= X && Element->Dimensions.x1 >= X)
            {
                for (auto &Item : Element->Children) if (Lambda(Item)) return true;
                return Element->onClick(Element, Button == GLFW_MOUSE_BUTTON_LEFT, Action == GLFW_RELEASE);
            }

            return false;
        };
        Lambda((Element_t *)glfwGetWindowUserPointer(Handle));
    }
    void onMousescroll(struct GLFWwindow *Handle, double OffsetX, double OffsetY) {}
    void onMousemove(struct GLFWwindow *Handle, double PosX, double PosY)
    {
        PosY = gHeight - PosY;

        std::function<bool(Element_t *)> Lambda = [&](Element_t *Element) -> bool
        {
            for (auto &Item : Element->Children) Lambda(Item);
            bool Focused = (Element->Dimensions.y0 <= PosY && Element->Dimensions.y1 >= PosY && Element->Dimensions.x0 <= PosX && Element->Dimensions.x1 >= PosX);
            return Element->onFocus(Element, !Focused);
        };
        Lambda((Element_t *)glfwGetWindowUserPointer(Handle));
    }
    void onMouseenter(struct GLFWwindow *Handle, int Entered)
    {
        if(!Entered) onMousemove(Handle, -40000, -40000);
        for (const auto &Item : Subscribers) Item(Entered);
    }

    // Subscribe to the events.
    void addMouseentersub(std::function<void(int Entered)> Callback)
    {
        Subscribers.push_back(Callback);
    }
}
