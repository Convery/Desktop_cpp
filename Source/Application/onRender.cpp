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
    extern GLFWwindow *Fallbackhandle;

    // Graphics.
    void onResize(struct GLFWwindow *Handle, int Width, int Height)
    {
        Element_t *Root;

        if (Handle)
        {
            gWidth = Width;
            gHeight = Height;
            Root = (Element_t *)glfwGetWindowUserPointer(Handle);
        }
        else
        {
            Root = (Element_t *)glfwGetWindowUserPointer(Fallbackhandle);
        }

        // Resize to the current window and update.
        Root->Boundingbox = { 0, 0, (double)Width, (double)Height };
        Root->onModifiedstate();
    }
    void onDraw(struct GLFWwindow *Handle)
    {
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the main element from the global viewport.
        glViewport(0, 0, (double)gWidth, (double)gHeight);
        ((Element_t *)glfwGetWindowUserPointer(Handle))->onRender();
        glViewport(0, 0, (double)gWidth, (double)gHeight);

        // Swap the buffers.
        glfwSwapBuffers(Handle);
    }
}
