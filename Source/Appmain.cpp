/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-08-2018
    License: MIT

    Provides the entrypoint for the application.
*/

#include "Stdinclude.hpp"

int main(int argc, char **argv)
{
    // Have the window rendering in the highest allowed state.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Create the initial scene.
    Engine::Compositing::Switchcomposition("loginmenu");
    Engine::Window::Resize({ 720, 720 }, true);
    Engine::Window::Centerwindow(false);
    Engine::Window::Togglevisibility();

    while (true)
    {
        // Track the frame-time, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Currentframe{ std::chrono::high_resolution_clock::now() };

        // Notify all components about the new frame.
        Engine::Input::onFrame();
        Engine::Compositing::onFrame(std::chrono::duration<double>(Currentframe - Lastframe).count());
        Engine::Rendering::onFrame();

        // If we got an error, terminate.
        if (unlikely(Engine::getErrno())) break;

        // Sleep until the next frame.
        std::this_thread::sleep_until(Lastframe + std::chrono::microseconds(1000000 / 60));
        Lastframe = Currentframe;
    }

    // Log the termination for tracing.
    Debugprint(va("Application terminated with code %u.", Engine::getErrno()));

    return 0;
}
