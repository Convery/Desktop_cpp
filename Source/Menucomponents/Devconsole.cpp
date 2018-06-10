/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    A simple console to test input.
*/

#include "../Stdinclude.hpp"

Element_t *Components::Createdevconsole()
{
    static const State_t *State{ Application::getState() };
    static uint32_t Cursorpos;
    static char Input[1024];
    static bool Expanded;
    static bool Visible;

    auto Boundingbox = new Element_t("ui.devcon");
    Boundingbox->Texture = Graphics::Createtexture({});
    Application::Subscriptions::addKeyclick(Boundingbox);

    static auto Inputbox = new Element_t("ui.devcon.input");
    Inputbox->ZIndex = -0.8f;
    Inputbox->Margin = {1, 1, 1, 1};
    Inputbox->Texture = Graphics::Createtexture({ 38, 35, 38, 1.0f });
    Boundingbox->Children.push_back(Inputbox);

    static auto Outputbox = new Element_t("ui.devcon.output");
    Outputbox->ZIndex = -0.8f;
    Outputbox->Margin = {1, 1, 1, 1};
    Outputbox->Texture = Graphics::Createtexture({ 38, 35, 38, 0.8f });
    Boundingbox->Children.push_back(Outputbox);

    // Keyboard callback for all elements.
    Boundingbox->onKey = [&](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) -> void
    {
        if (Key == GLFW_KEY_GRAVE_ACCENT && !Released)
        {
            bool Advanced = Modifier & GLFW_MOD_SHIFT;

            // Show the standard input box.
            if (!Visible || (Visible && Advanced && Expanded))
            {
                Inputbox->Margin = { 0.005, 1.79, 0.005, 0.11 };
                Inputbox->onModifiedstate();
                Outputbox->Hide();
                Expanded = false;
                Visible = true;

                // Clear any old state-vars.
                std::memset(Input, 0, 1024);
                Cursorpos = 0;

                return;
            }

            // Hide the input box.
            if (Visible && !Advanced)
            {
                Outputbox->Hide();
                Inputbox->Hide();
                Expanded = false;
                Visible = false;
                return;
            }

            // Expand the output box.
            if (Visible && Advanced && !Expanded)
            {
                Outputbox->Margin = { 0.005, 1.0, 0.005, 0.11 };
                Inputbox->Margin = { 0.005, 0.9, 0.005, 1.0 };
                Outputbox->onModifiedstate();
                Inputbox->onModifiedstate();
                Expanded = true;
            }
        }


    };

    return Boundingbox;
}
