/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Process userinput.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    extern State_t Globalstate;

    // Subscriptions to eventtypes.
    namespace Subscriptions
    {
        std::vector<Element_t *> Keyclicklisteners;
        std::vector<Element_t *> Mousemovelisteners;
        std::vector<Element_t *> Mouseclicklisteners;
        std::vector<Element_t *> Mousescrolllisteners;
        std::vector<std::function<void(int Entered)>> Mouseenterlisteners;

        void addKeyclick(Element_t *Element) { Keyclicklisteners.push_back(Element); }
        void addMousemove(Element_t *Element) { Mousemovelisteners.push_back(Element); }
        void addMouseclick(Element_t *Element) { Mouseclicklisteners.push_back(Element); }
        void addMousescroll(Element_t *Element) { Mousescrolllisteners.push_back(Element); }
        void addMouseenter(std::function<void(int Entered)> Callback) { Mouseenterlisteners.push_back(Callback); }
    }

    // User-input.
    void onKeyclick(struct GLFWwindow *Handle, int Key, int Scancode, int Action, int Modifier)
    {
        for (const auto &Item : Subscriptions::Keyclicklisteners) Item->onKey(Item, Key, Modifier, Action == GLFW_RELEASE);
    }
    void onMouseclick(struct GLFWwindow *Handle, int Button, int Action, int Modifiers)
    {
        double X, Y;
        static const State_t *State{ Application::getState() };
        glfwGetCursorPos(Handle, &X, &Y);
        Y = State->Height - Y;

        for (const auto &Item : Subscriptions::Mouseclicklisteners)
        {
            if (Item->Dimensions.y0 <= Y && Item->Dimensions.y1 >= Y && Item->Dimensions.x0 <= X && Item->Dimensions.x1 >= X)
            {
                Item->onActive(Item, Action == GLFW_RELEASE);
                Item->State.Active = Action != GLFW_RELEASE;
            }
            else
            {
                if (Item->State.Active)
                {
                    Item->onActive(Item, true);
                    Item->State.Active = 0;
                }
            }
        }
    }
    void onMousescroll(struct GLFWwindow *Handle, double OffsetX, double OffsetY)
    {
    }
    void onMousemove(struct GLFWwindow *Handle, double PosX, double PosY)
    {
        // Flip the Y coordinate.
        PosY = Globalstate.Height - PosY;
        Globalstate.MouseX = PosX;
        Globalstate.MouseY = PosY;

        for (const auto &Item : Subscriptions::Mousemovelisteners)
        {
            bool Focused = Item->Dimensions.y0 <= PosY && Item->Dimensions.y1 >= PosY && Item->Dimensions.x0 <= PosX && Item->Dimensions.x1 >= PosX;
            Item->onFocus(Item, !Focused);
            Item->State.Focused = Focused;

            if (Item->Dimensions.y0 <= PosY && Item->Dimensions.y1 >= PosY && Item->Dimensions.x0 <= PosX && Item->Dimensions.x1 >= PosX)
            {
                Item->onFocus(Item, false);
                Item->State.Focused = 1;
            }
            else
            {
                if (Item->State.Focused)
                {
                    Item->onFocus(Item, true);
                    Item->State.Focused = 0;
                }
            }
        }
    }
    void onMouseenter(struct GLFWwindow *Handle, int Entered)
    {
        if(!Entered) onMousemove(Handle, -40000, -40000);
        for (const auto &Item : Subscriptions::Mouseenterlisteners) Item(Entered);
    }
}
