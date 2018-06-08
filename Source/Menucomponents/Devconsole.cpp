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
    Element_t *Tooltiparea;
    Element_t *Inputarea;
    Element_t *Logarea;
};

Element_t Components::Createdevconsole()
{
    auto State = new Consolestate();

    Element_t Boundingbox("ui.devcon");
    Boundingbox.Userpointer = State;
    Boundingbox.Margin = { -1, -1, 1, 1 };
    Boundingbox.Settexture(Graphics::Createtexture({ 0, 1, 0, 1 }));
    Boundingbox.onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) -> bool
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
                        State->Tooltiparea->Resize({ -1, 0.9, 1, 0.7 });
                        State->Inputarea->Resize({ -1, 1, 1, 0.9 });
                        State->Logarea->Resize({ -1, -1, -1, -1 });
                        State->Expanded = false;
                    }

                    // Expand.
                    else
                    {
                        // Use the expanded view.
                        State->Tooltiparea->Resize({ -1, -0.1, 1, -0.3 });
                        State->Inputarea->Resize({ -1, 0, 1, -0.1 });
                        State->Logarea->Resize({ -1, 1, 1, 0 });
                        State->Expanded = true;
                    }
                }

                // Close the console.
                else
                {
                    // Reset the console-state.
                    std::memset(State->Input, 0, 1024);
                    State->Tooltiparea->Resize({});
                    State->Inputarea->Resize({});
                    State->Logarea->Resize({});
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
                State->Tooltiparea->Resize({ -1, 0.9, 1, 0.7 });
                State->Inputarea->Resize({ -1, 1, 1, 0.9 });
                State->Logarea->Resize({});
                State->Expanded = false;
                State->Visible = true;
                State->Cursorpos = {};
            }

            // HACK(Convery): Trigger a global recalc.
            Application::onResize(nullptr, NULL, NULL);
            return true;
        }

        return false;
    };
    Boundingbox.onInit = [](Element_t *Caller) -> void
    {
        auto State = (Consolestate *)Caller->Userpointer;

        // Hide the console.
        Caller->Resize({});

        // Input area.
        auto Input = new Element_t("ui.devcon.input");
        Input->Settexture(Graphics::Createtexture({ 0, 1, 0, 0.2f }));
        Input->Userpointer = State;
        State->Inputarea = Input;

        // Tooltip area.
        auto Tool = new Element_t("ui.devcon.tooltip");
        Tool->Settexture(Graphics::Createtexture({ 1, 0, 0, 0.2f }));
        Tool->Userpointer = State;
        State->Tooltiparea = Tool;

        // Log area.
        auto Log = new Element_t("ui.devcon.log");
        Log->Settexture(Graphics::Createtexture({ 0, 0, 1, 0.2f }));
        Log->Userpointer = State;
        State->Logarea = Log;

        // Merge into the wrapper.
        Caller->Addchild(Tool);
        Caller->Addchild(Input);
        Caller->Addchild(Log);
    };



    return Boundingbox;
}
