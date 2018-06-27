/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

int main(int argc, char **argv)
{
    Engine::Rendering::Createframebuffer({ 1280, 720 });
    Engine::Window::Create({ 1280, 720 });
    Engine::Compositions::Parseblueprint();
    Engine::Compositions::Switch("login");
    Engine::Window::Togglevisibility();

    auto Lastframe{ std::chrono::high_resolution_clock::now() };
    constexpr std::chrono::microseconds Framedelay{ 1000000 / 60 };

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

    while (!Engine::doFrame(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - Lastframe).count()))
    {
        // Let the core rest a little.
        Lastframe = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
