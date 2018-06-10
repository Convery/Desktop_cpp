/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    A simple console to test input.
*/

#include "../Stdinclude.hpp"

struct Consolestate
{
    bool Visible;
    bool Expanded;
    char Input[1024];
    uint32_t Cursorpos;

    // Access to the blocks.
    Element_t *Inputarea;
    Element_t *Logarea;
};

Element_t *Components::Createdevconsole()
{
    auto State = new Consolestate();

    auto Boundingbox = new Element_t("ui.devcon");
    Boundingbox->Texture = Graphics::Createtexture({0, 1, 1, 0 });
    Boundingbox->Userpointer = State;
    Boundingbox->ZIndex = -0.1f;
    Boundingbox->onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) -> bool
    {
        if (Key == GLFW_KEY_GRAVE_ACCENT && !Released)
        {
            auto State = (Consolestate *)Caller->Userpointer;
            bool Expand = Modifier & GLFW_MOD_SHIFT;

            // If we are already visible, close or expand.
            if (State->Visible)
            {
                // Expand  or close the log-area.
                if (Expand)
                {
                    // Close.
                    if (State->Expanded)
                    {
                        // Use the non-expanded view.
                        State->Inputarea->Margin = { 0.01, 1.73, 0.01, 0.17 };
                        State->Logarea->Margin = { 1, 1, 1, 1 };
                        State->Expanded = false;
                    }

                    // Expand.
                    else
                    {
                        // Use the expanded view.
                        State->Inputarea->Margin = { 0.01, 0.9, 0.01, 1.0 };
                        State->Logarea->Margin = { 0.01, 1.0, 0.01, 0.17 };
                        State->Expanded = true;
                    }
                }

                // Close the console.
                else
                {
                    // Reset the console-state.
                    State->Inputarea->Margin = { 1, 1, 1, 1};
                    State->Logarea->Margin = { 1, 1, 1, 1};
                    State->Cursorpos = {};
                    State->Expanded = {};
                    State->Visible = {};
                }
            }

            // Else we create the windows.
            else
            {
                // Use the non-expanded view.
                std::memset(State->Input, 0, 1024);
                State->Inputarea->Margin = { 0.01, 1.73, 0.01, 0.17 };
                State->Logarea->Margin = { 1, 1, 1, 1 };
                State->Expanded = false;
                State->Visible = true;
                State->Cursorpos = {};
            }

            State->Inputarea->onModifiedstate();
            State->Logarea->onModifiedstate();

            return true;
        }

        return false;
    };

    // Input area.
    auto Input = new Element_t("ui.devcon.input");
    Input->Texture = Graphics::Createtexture({ 38, 35, 38, 1 });
    Input->Margin = {1, 1, 1, 1};
    Input->Userpointer = State;
    State->Inputarea = Input;
    Input->ZIndex = -0.2f;
    Input->onClick = [](Element_t *Caller, uint32_t Key, bool Released) -> bool
    {
        if (Released) Caller->Texture = Graphics::Createtexture({ 0, 1, 1, 0.2f });
        else Caller->Texture = Graphics::Createtexture({ 1, 1, 0, 0.2f });
        return true;
    };

    // Log area.
    auto Log = new Element_t("ui.devcon.log");
    Log->Texture = Graphics::Createtexture({ 38, 35, 38, 0.8f });
    Log->Margin = { 1, 1, 1, 1};
    Log->Userpointer = State;
    State->Logarea = Log;
    Log->ZIndex = -0.2f;

    // Add the items to the box.
    Boundingbox->Children.push_back(Input);
    Boundingbox->Children.push_back(Log);

    return Boundingbox;
}
