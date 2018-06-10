/*
    Initial author: Convery (tcn@ayria.se)
    Started: 10-06-2018
    License: MIT

    We only implement the bottom and right borders.
    This is because expanding in the other directions
    is a pain in the current system. Maybe in the future.
*/

#include "../Stdinclude.hpp"

GLFWcursor *IBeam;
GLFWcursor *Arrow;
GLFWcursor *HResize;
GLFWcursor *VResize;
static GLFWwindow *Handle;
static bool Shouldresize{};
static double StartX, StartY;
Element_t *Components::Creeateborders()
{
    Handle = (GLFWwindow *)Application::Windowhandle();
    IBeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    Arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    HResize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    VResize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

    Application::addMouseendersub([&](int Entered) { if (!Entered) Shouldresize = false; });

    auto Boundingbox = new Element_t("ui.border");
    Boundingbox->Texture = Graphics::Createtexture({});

    auto Borderbottom = new Element_t("ui.border.bottom");
    Borderbottom->Texture = Graphics::Createtexture({});
    Borderbottom->Margin = { 0.0, 0.0, 0.0, 1.985 };
    Borderbottom->ZIndex = -0.15f;
    Borderbottom->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Active = false;

        if (!Released)
        {
            glfwSetCursor(Handle, VResize);
            Active = true;
            return true;
        }

        if (Active)
        {
            glfwSetCursor(Handle, Arrow);
            Active = false;
        }

        return false;
    };
    Borderbottom->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (Released) Shouldresize = false;
        else
        {
            glfwGetCursorPos(Handle, &StartX, &StartY);
            Shouldresize = true;

            auto Lambda = [&]()
            {
                while (Shouldresize)
                {
                    double X, Y;
                    glfwGetCursorPos(Handle, &X, &Y);

                    int SizeX, SizeY;
                    glfwGetWindowSize(Handle, &SizeX, &SizeY);

                    SizeY = std::clamp(int(SizeY + Y - StartY), 540, 4320);
                    SizeX = SizeY * 1.78;
                    glfwSetWindowSize(Handle, SizeX, SizeY);
                    StartX = X; StartY = Y;
                }
            };
            std::thread(Lambda).detach();
        }

        return true;
    };

    auto Borderright = new Element_t("ui.border.right");
    Borderright->Texture = Graphics::Createtexture({});
    Borderright->Margin = { 1.995, 0.0, 0.0, 0.0 };
    Borderright->ZIndex = -0.15f;
    Borderright->onFocus = [](Element_t *Caller, bool Released) -> bool
    {
        static bool Active = false;

        if (!Released)
        {
            glfwSetCursor(Handle, HResize);
            Active = true;
            return true;
        }

        if (Active)
        {
            glfwSetCursor(Handle, Arrow);
            Active = false;
        }

        return false;
    };
    Borderright->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (Released) Shouldresize = false;
        else
        {
            glfwGetCursorPos(Handle, &StartX, &StartY);
            Shouldresize = true;

            auto Lambda = [&]()
            {
                while (Shouldresize)
                {
                    double X, Y;
                    glfwGetCursorPos(Handle, &X, &Y);

                    int SizeX, SizeY;
                    glfwGetWindowSize(Handle, &SizeX, &SizeY);

                    SizeX = std::clamp(int(SizeX + X - StartX), 960, 7680);
                    SizeY = SizeX / 1.78;
                    glfwSetWindowSize(Handle, SizeX, SizeY);
                    StartX = X; StartY = Y;
                }
            };
            std::thread(Lambda).detach();
        }

        return true;
    };

    Boundingbox->Children.push_back(Borderbottom);
    Boundingbox->Children.push_back(Borderright);

    return Boundingbox;
}