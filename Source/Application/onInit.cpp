/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Creates the initial scene to render.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    State_t Globalstate{};

    // Take a pointer to the global state.
    const State_t *getState()
    {
        return &Globalstate;
    }

    // Rare events.
    void onError(int Error, const char *Description)
    {
        printf("Error: %s\n", Description);
    }
    void onSceneswitch(std::string_view Scenename)
    {
        if (0 == std::strcmp(Scenename.data(), "library"))
        {
            Globalstate.Root->Children.push_back(Components::Createtoolbar());

        }

        // Notify the root-element about the update.
        Globalstate.Root->onModifiedstate();
    }
    void onInit(struct GLFWwindow *Handle)
    {
        // Save the handle and element for easy access.
        Globalstate.Root = (Element_t *)glfwGetWindowUserPointer(Handle);
        Globalstate.Handle = Handle;

        // Initialize the global viewport.
        glfwGetFramebufferSize(Handle, &Globalstate.Width, &Globalstate.Height);
        glViewport(0, 0, Globalstate.Width, Globalstate.Height);
    }
}
