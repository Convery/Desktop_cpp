/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Provides the entrypoint for the application.
*/

#include "Stdinclude.hpp"
namespace Engine { bool gShouldquit{ false }; uint32_t gErrno{}; }

int main(int argc, char **argv)
{
    // Have the window rendering in the highest allowed state.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Create the scene.
    Engine::Compositing::Switchcomposition("loginmenu");
    Engine::Window::Centerwindow(false);
    Engine::Window::Togglevisibility();

    // Main application-loop.
    while (Engine::gErrno == 0)
    {
        Engine::Window::onFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    // Log the termination for tracing.
    Debugprint(va("Application terminated with code %u.", Engine::gErrno));

    return 0;
}
