/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

int main(int argc, char **argv)
{
    Engine::Window::Create({ 1920, 1080 });
    Engine::Compositions::Parseblueprint();
    Engine::Compositions::Switch("login");
    Engine::Window::Togglevisibility();

    // If debugging, reload the blueprint all the time.
    #if !defined(NDEBUG)
    std::thread([]()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            Engine::Compositions::Parseblueprint();
            Engine::Compositions::Switch("login");
        }
    }).detach();
    #endif

    // Process any and all events.
    while (!Engine::doFrame()) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    return 0;
}
