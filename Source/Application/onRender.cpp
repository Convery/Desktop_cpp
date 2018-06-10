/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Poke OpenGL with a stick.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    extern State_t Globalstate;

    // Graphics.
    void onResize(struct GLFWwindow *Handle, int Width, int Height)
    {
        // Update the state.
        Globalstate.Height = Height;
        Globalstate.Width = Width;

        // Resize to the current window and update.
        Globalstate.Root->Boundingbox = { 0, 0, (double)Width, (double)Height };
        Globalstate.Root->onModifiedstate();
    }
    void onDraw(struct GLFWwindow *Handle)
    {
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the main element from the global viewport.
        ((Element_t *)glfwGetWindowUserPointer(Handle))->onRender();

        // Swap the buffers.
        glfwSwapBuffers(Handle);
    }
}
