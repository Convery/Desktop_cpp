/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Creates the initial scene to render.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    int gWidth{}, gHeight{};

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

        /*
            TODO(Convery):
            Build the scene here.
        */
    }
}
