/*
    Initial author: Convery (tcn@ayria.se)
    Started: 09-06-2018
    License: MIT

    Implement a toolbar for borderless windows.
*/

#include "../Stdinclude.hpp"

struct Toolbarstate
{
    bool Shouldmove;
    GLFWwindow *Handle;
    double StartX, StartY;
};

// A fancy toolbar.
Element_t *Components::Createtoolbar()
{
    auto Boundingbox = new Element_t("ui.toolbar");
    Boundingbox->Userpointer = new Toolbarstate();
    Boundingbox->Texture = Graphics::Createtexture({0.1f, 0.1f, 0.1f, 1 });
    ((Toolbarstate *)Boundingbox->Userpointer)->Handle = (GLFWwindow *)Application::Windowhandle();
    Boundingbox->ZIndex = -0.2f;
    Boundingbox->Margin = { 0, 1.9, 0, 0 };
    Boundingbox->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        auto State = (Toolbarstate *)Caller->Userpointer;
        State->Shouldmove = !Released;

        if (!Released) glfwGetCursorPos(State->Handle, &State->StartX, &State->StartY);

        return false;
    };
    Boundingbox->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        auto State = (Toolbarstate *)Caller->Userpointer;
        if (!Released && State->Shouldmove)
        {
            double X, Y;
            glfwGetCursorPos(State->Handle, &X, &Y);

            int PosX, PosY;
            glfwGetWindowPos(State->Handle, &PosX, &PosY);

            PosX += X - State->StartX; PosY += Y - State->StartY;
            glfwSetWindowPos(State->Handle, PosX, PosY);
        }
        else State->Shouldmove = false;

        return false;
    };

    return Boundingbox;
};
