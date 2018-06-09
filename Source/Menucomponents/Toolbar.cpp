/*
    Initial author: Convery (tcn@ayria.se)
    Started: 09-06-2018
    License: MIT

    Implement a toolbar for borderless windows.
*/

#include "../Stdinclude.hpp"

// A fancy toolbar.
bool Shouldmove;
GLFWwindow *Handle;
double StartX, StartY;
static bool Armed = false;
Element_t *Components::Createtoolbar()
{
    Handle = (GLFWwindow *)Application::Windowhandle();

    auto Boundingbox = new Element_t("ui.toolbar");
    Boundingbox->Texture = Graphics::Createtexture({50, 58, 69, 1 });
    Boundingbox->ZIndex = -0.2f;
    Boundingbox->Margin = { 0.0, 1.9, 0.0, 0.0 };
    Boundingbox->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        static double Lastclick = 0;
        if (!Released)
        {
            if (glfwGetTime() - Lastclick < 0.5)
            {
                if(glfwGetWindowAttrib(Handle, GLFW_MAXIMIZED)) glfwRestoreWindow(Handle);
                else glfwMaximizeWindow(Handle);
            }
            Lastclick = glfwGetTime();
        }

        Shouldmove = !Released;
        if (!Released) glfwGetCursorPos(Handle, &StartX, &StartY);

        return false;
    };
    Boundingbox->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        if (!Released && Shouldmove)
        {
            double X, Y;
            glfwGetCursorPos(Handle, &X, &Y);

            int PosX, PosY;
            glfwGetWindowPos(Handle, &PosX, &PosY);

            PosX += (X - StartX) * 1.5; PosY += (Y - StartY) * 1.5;
            glfwSetWindowPos(Handle, PosX, PosY);
        }
        else Shouldmove = false;

        return false;
    };

    auto Closebutton = new Element_t("ui.toolbar.close");
    Closebutton->Texture = Graphics::Createtexture({ 226, 35, 35, 0.8f });
    Closebutton->Margin = { 1.94, 0.0, 0.0, 0.0 };
    Closebutton->ZIndex = -0.3f;
    Closebutton->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (Released && Armed) glfwSetWindowShouldClose((GLFWwindow *)Application::Windowhandle(), 1);
        if (!Released)
        {
            Armed = true;
            Caller->Texture = Graphics::Createtexture({ 205, 197, 186, 0.8f });
        }
        return true;
    };
    Closebutton->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        if (Released && Armed)
        {
            Armed = false;
            Caller->Texture = Graphics::Createtexture({ 226, 35, 35, 0.8f });
        }
        return false;
    };

    auto Maxbutton = new Element_t("ui.toolbar.max");
    Maxbutton->Texture = Graphics::Createtexture({ 226, 90, 35, 0.8f });
    Maxbutton->Margin = { 1.88, 0.0, 0.06, 0.0 };
    Maxbutton->ZIndex = -0.3f;
    Maxbutton->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (!Released) Caller->Texture = Graphics::Createtexture({ 205, 197, 186, 0.8f });
        else
        {
            if(glfwGetWindowAttrib(Handle, GLFW_MAXIMIZED)) glfwRestoreWindow(Handle);
            else glfwMaximizeWindow(Handle);
        }
        return true;
    };
    Maxbutton->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        if (Released) Caller->Texture = Graphics::Createtexture({ 226, 90, 35, 0.8f });
        return false;
    };

    auto Minbutton = new Element_t("ui.toolbar.min");
    Minbutton->Texture = Graphics::Createtexture({ 226, 226, 35, 0.8f });
    Minbutton->Margin = { 1.82, 0.0, 0.12, 0.0 };
    Minbutton->ZIndex = -0.3f;
    Minbutton->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (!Released) Caller->Texture = Graphics::Createtexture({ 205, 197, 186, 0.8f });
        else
        {
            if(glfwGetWindowAttrib(Handle, GLFW_ICONIFIED)) glfwRestoreWindow(Handle);
            else glfwIconifyWindow(Handle);
        }
        return true;
    };
    Minbutton->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        if (Released) Caller->Texture = Graphics::Createtexture({ 226, 226, 35, 0.8f });
        return false;
    };

    Boundingbox->Children.push_back(Closebutton);
    Boundingbox->Children.push_back(Maxbutton);
    Boundingbox->Children.push_back(Minbutton);

    return Boundingbox;
};
