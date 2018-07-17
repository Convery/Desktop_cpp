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
    Engine::Rendering::Invalidatearea({ 0, 0, 520, 720 });
    Engine::Compositing::Switchscene("loginmenu");
    Engine::Window::Centerwindow(false);            // Replace with Engine::Composition::Load("Blueprint.json");
    Engine::Window::Togglevisibility();

    // Main application-loop.
    while (!Engine::gShouldquit)
    {
        // Non-blocking call.
        Engine::Window::onFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    // Log the termination for tracing.
    Debugprint(va("Application terminated with code %u.", Engine::gErrno));

    return 0;
}
