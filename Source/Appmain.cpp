/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides an entrypoint for the application.
*/

#include "Stdinclude.hpp"

int main(int argc, char **argv)
{
    Engine::Window::Create({ 1280, 720 });
    Engine::Window::Togglevisibility();

    while (true) Sleep(1);
    return 0;
}

