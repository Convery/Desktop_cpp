/*
    Initial author: Convery (tcn@ayria.se)
    Started: 10-06-2018
    License: MIT

    We only implement the bottom and right borders.
    This is because expanding in the other directions
    is a pain in the current system. Maybe in the future.
*/

#include "../Stdinclude.hpp"

Element_t *Components::Createborders()
{
    static auto VResize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    static auto HResize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    static auto Arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    static auto IBeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    static auto State{ Application::getState() };
    static double StartX, StartY;
    static bool Shouldresize{};

    // Stop resizing if the mouse exits the clientarea.
    Application::Subscriptions::addMouseenter([&](int Entered) { if (!Entered) Shouldresize = false; });

    auto Boundingbox = new Element_t("ui.border");
    Boundingbox->Texture = Graphics::Createtexture({});
    Boundingbox->onActive = [&](Element_t *Caller, bool Released) -> void
    {
        if (Released) Shouldresize = false;
    };
    Application::Subscriptions::addMouseclick(Boundingbox);

    auto Borderbottom = new Element_t("ui.border.bottom");
    Borderbottom->ZIndex = -0.15f;
    Borderbottom->Margin = { 0.0, 0.0, 0.0, 1.985 };
    Borderbottom->Texture = Graphics::Createtexture({});
    Borderbottom->onFocus = [&](Element_t *Caller, bool Released) -> void
    {
        static bool Active = false;

        if (Released && Active)
        {
            glfwSetCursor(State->Handle, Arrow);
            Active = false;
        }

        if (!Released)
        {
            glfwSetCursor(State->Handle, VResize);
            Active = true;
        }
    };
    Borderbottom->onActive = [&](Element_t *Caller, bool Released) -> void
    {
        if (Released) Shouldresize = false;
        else
        {
            glfwGetCursorPos(State->Handle, &StartX, &StartY);
            Shouldresize = true;

            auto Lambda = [&]()
            {
                while (Shouldresize)
                {
                    double X, Y;
                    glfwGetCursorPos(State->Handle, &X, &Y);

                    int SizeX, SizeY;
                    glfwGetWindowSize(State->Handle, &SizeX, &SizeY);

                    SizeY = std::clamp(int(SizeY + Y - StartY), 540, 4320);
                    SizeX = SizeY * 1.78;
                    glfwSetWindowSize(State->Handle, SizeX, SizeY);
                    StartX = X; StartY = Y;
                }
            };
            std::thread(Lambda).detach();
        }
    };
    Application::Subscriptions::addMouseclick(Borderbottom);
    Application::Subscriptions::addMousemove(Borderbottom);
    Boundingbox->Children.push_back(Borderbottom);

    auto Borderright = new Element_t("ui.border.right");
    Borderright->ZIndex = -0.15f;
    Borderright->Margin = { 1.995, 0.0, 0.0, 0.0 };
    Borderright->Texture = Graphics::Createtexture({});
    Borderright->onFocus = [&](Element_t *Caller, bool Released) -> void
    {
        static bool Active = false;

        if (Released && Active)
        {
            glfwSetCursor(State->Handle, Arrow);
            Active = false;
        }

        if (!Released)
        {
            glfwSetCursor(State->Handle, HResize);
            Active = true;
        }
    };
    Borderright->onActive = [&](Element_t *Caller, bool Released) -> void
    {
        if (Released) Shouldresize = false;
        else
        {
            glfwGetCursorPos(State->Handle, &StartX, &StartY);
            Shouldresize = true;

            auto Lambda = [&]()
            {
                while (Shouldresize)
                {
                    double X, Y;
                    glfwGetCursorPos(State->Handle, &X, &Y);

                    int SizeX, SizeY;
                    glfwGetWindowSize(State->Handle, &SizeX, &SizeY);

                    SizeX = std::clamp(int(SizeX + X - StartX), 960, 7680);
                    SizeY = SizeX / 1.78;
                    glfwSetWindowSize(State->Handle, SizeX, SizeY);
                    StartX = X; StartY = Y;
                }
            };
            std::thread(Lambda).detach();
        }
    };
    Application::Subscriptions::addMouseclick(Borderright);
    Application::Subscriptions::addMousemove(Borderright);
    Boundingbox->Children.push_back(Borderright);

    return Boundingbox;
}
