/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

int main(int argc, char **argv)
{
    // Clear the previous log.
    Clearlog();

    // Initialize glfw.
    glfwInit();
    //glfwWindowHint(GLFW_DECORATED, GL_FALSE);

    // Create the main window.
    auto Handle = glfwCreateWindow(1280, 720, "Main", nullptr, nullptr);
    if (Handle == NULL) return 3;

    // Initialize GLEW.
    glfwMakeContextCurrent(Handle);
    glewInit();

    // Rendering options.
    glEnable(GL_BLEND);
    glfwSwapInterval(1);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glewExperimental = GL_TRUE;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Set the callbacks for the application.
    glfwSetErrorCallback(Application::onError);
    glfwSetKeyCallback(Handle, Application::onKeyclick);
    glfwSetWindowSizeCallback(Handle, Application::onResize);
    glfwSetWindowRefreshCallback(Handle, Application::onDraw);
    glfwSetScrollCallback(Handle, Application::onMousescroll);
    glfwSetCursorPosCallback(Handle, Application::onMousemove);
    glfwSetMouseButtonCallback(Handle, Application::onMouseclick);

    // Create a default element and bind.
    Element_t Mainwindow("The root application element");
    Mainwindow.Calculatedimentions({ 0, 0, 1280, 720 });
    glfwSetWindowUserPointer(Handle, &Mainwindow);
    Mainwindow.Margin = { -1, -1, 1, 1 };
    Mainwindow.Reinitializebuffers(0);

    // Trigger the initialization event.
    Application::onInit(Handle);

       // Loop until we crash.
    while (!glfwWindowShouldClose(Handle))
    {
        // Process any new events.
        glfwPollEvents();

        // Draw the next frame.
        Application::onDraw(Handle);
    }

    // Cleanup.
    glfwTerminate();
    return 0;
}
