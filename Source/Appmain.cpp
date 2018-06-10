/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

// Stolen from https://github.com/glfw/glfw/issues/310
void glfwSetWindowCenter(GLFWwindow* window) {
    // Get window position and size
    int window_x, window_y;
    glfwGetWindowPos(window, &window_x, &window_y);

    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    // Halve the window size and use it to adjust the window position to the center of the window
    window_width *= 0.5;
    window_height *= 0.5;

    window_x += window_width;
    window_y += window_height;

    // Get the list of monitors
    int monitors_length;
    GLFWmonitor **monitors = glfwGetMonitors(&monitors_length);

    if(monitors == NULL) {
        // Got no monitors back
        return;
    }

    // Figure out which monitor the window is in
    GLFWmonitor *owner = NULL;
    int owner_x, owner_y, owner_width, owner_height;

    for(int i = 0; i < monitors_length; i++) {
        // Get the monitor position
        int monitor_x, monitor_y;
        glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

        // Get the monitor size from its video mode
        int monitor_width, monitor_height;
        GLFWvidmode *monitor_vidmode = (GLFWvidmode*) glfwGetVideoMode(monitors[i]);

        if(monitor_vidmode == NULL) {
            // Video mode is required for width and height, so skip this monitor
            continue;

        } else {
            monitor_width = monitor_vidmode->width;
            monitor_height = monitor_vidmode->height;
        }

        // Set the owner to this monitor if the center of the window is within its bounding box
        if((window_x > monitor_x && window_x < (monitor_x + monitor_width)) && (window_y > monitor_y && window_y < (monitor_y + monitor_height))) {
            owner = monitors[i];

            owner_x = monitor_x;
            owner_y = monitor_y;

            owner_width = monitor_width;
            owner_height = monitor_height;
        }
    }

    if(owner != NULL) {
        // Set the window position to the center of the owner monitor
        glfwSetWindowPos(window, owner_x + (owner_width * 0.5) - window_width, owner_y + (owner_height * 0.5) - window_height);
    }
}

int main(int argc, char **argv)
{
    // Clear the previous log.
    Clearlog();

    // Initialize glfw.
    glfwInit();
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

    // Create the main window.
    auto Handle = glfwCreateWindow(1280, 720, "Main", nullptr, nullptr);
    if (Handle == NULL) return 3;
    glfwSetWindowCenter(Handle);

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

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Set the callbacks for the application.
    glfwSetErrorCallback(Application::onError);
    glfwSetKeyCallback(Handle, Application::onKeyclick);
    glfwSetWindowSizeCallback(Handle, Application::onResize);
    glfwSetWindowRefreshCallback(Handle, Application::onDraw);
    glfwSetScrollCallback(Handle, Application::onMousescroll);
    glfwSetCursorPosCallback(Handle, Application::onMousemove);
    glfwSetMouseButtonCallback(Handle, Application::onMouseclick);
    glfwSetCursorEnterCallback(Handle, Application::onMouseenter);

    // Create a default element and bind.
    Element_t Mainwindow("ui");
    Mainwindow.Texture = Graphics::Createtexture({});
    glfwSetWindowUserPointer(Handle, &Mainwindow);
    Mainwindow.Boundingbox = { 0, 0, 1280, 720 };
    Mainwindow.onModifiedstate();

    // Trigger the initialization event.
    Application::onInit(Handle);
    Application::onSceneswitch("library");

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
