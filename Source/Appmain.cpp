/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-08-2018
    License: MIT

    Provides the entrypoint for the application.
*/

#include "Stdinclude.hpp"
namespace Engine { bool gShouldquit{ false }; uint32_t gErrno{}; }

int main(int argc, char **argv)
{
    // Have the window rendering in the highest allowed state.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    return 0;
}
