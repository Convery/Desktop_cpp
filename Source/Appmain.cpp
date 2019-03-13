/*
    Initial author: Convery (tcn@ayria.se)
    Started: 14-03-2019
    License: MIT
*/

#include "Stdinclude.hpp"

int __cdecl main(int argc, char **argv)
{
    // Only keep a log for this session.
    Logging::Clearlog();

    // As we are single-threaded (in release), boost our priority.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    /*
        1. Subscribe to termination events.
        2. Fire startup events.
    */

    /*
        1. Load the blueprint from file.
        2. Create a thread to poll the file for changes.
        3. Reload as needed.
    */

    // Main-loop, quit on error.
    while (true)
    {
        // Track the frame-time, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Thisframe{ std::chrono::high_resolution_clock::now() };

        /*
            1.  Check input.
            2.  Fire a tick event.
            3a. Get dirty-region.
            3b. Render region.
            3c. Bitblt
        */

        // If we got an error, terminate.
        if (Global::State.Errorno) break;

        // Sleep until the next frame.
        std::this_thread::sleep_until(Lastframe + std::chrono::milliseconds(16));
        Lastframe = Thisframe;
    }

    // Notify all subscribers about terminating.
    // TODO^

    return 0;
}
