/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Poke OpenGL with a stick.
*/

#include "../Stdinclude.hpp"

namespace Application
{
    extern int gWidth, gHeight;

    // Graphics.
    void onResize(struct GLFWwindow *Handle, int Width, int Height)
    {
        gWidth = Width;
        gHeight = Height;
        ((Element_t *)glfwGetWindowUserPointer(Handle))->Calculatedimentions({ 0, 0, (double)Width, (double)Height });
    }
    void onDraw(struct GLFWwindow *Handle)
    {
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the main element from the global viewport.
        glViewport(0, 0, gWidth, gHeight);

        // Reads on x86 are atomic so we just copy the value and set it between frames.
        ((Element_t *)glfwGetWindowUserPointer(Handle))->Renderelement();

        // Swap the buffers.
        glfwSwapBuffers(Handle);
    }
}
