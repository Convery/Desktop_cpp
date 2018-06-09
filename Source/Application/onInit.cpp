/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Creates the initial scene to render.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    GLFWwindow *Fallbackhandle;
    int gWidth{}, gHeight{};

    // Helpers.
    void *Windowhandle()
    {
        return Fallbackhandle;
    }

    // Pushed events.
    void onError(int Error, const char *Description)
    {
        printf("Error: %s\n", Description);
    }
    void onInit(struct GLFWwindow *Handle)
    {
        // Initialize the global viewport.
        glfwGetFramebufferSize(Handle, &gWidth, &gHeight);
        glViewport(0, 0, gWidth, gHeight);

        // Set a fallback handle to the window.
        Fallbackhandle = Handle;

        /*
            TODO(Convery):
            Build the scene here.
        */
        auto Root = (Element_t *)glfwGetWindowUserPointer(Handle);
        Root->Children.push_back(Components::Createdevconsole());
        Root->Children.push_back(Components::Creeateborders());
        Root->Children.push_back(Components::Createtoolbar());
        Root->onModifiedstate();
    }
}
