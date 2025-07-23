#include "Runtime/EntryPoint.h"
#include "Runtime/Application.h"

#include <iostream>
#include <chrono>
#include <thread>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

namespace Lumina {

    void EntryPoint::Run(Application* app) {
        app->Initialize();

        size_t frameCount = 0;
        bool running = true;
        const double TARGET_FRAME_TIME = 16.67; // 60 FPS in milliseconds

        auto lastTime = std::chrono::high_resolution_clock::now();

        while (running) {
            auto frameStart = std::chrono::high_resolution_clock::now();

            frameCount++;
            app->Update();
            app->Render();

            // Calculate frame time and sleep to maintain 60 FPS
            auto frameEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = frameEnd - frameStart;
            double sleepTime = TARGET_FRAME_TIME - elapsed.count();

            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepTime));
            }

            // Print memory stats every 100 frames
            if (frameCount % 100 == 0) {
#ifdef _MSC_VER
                _CrtMemState state;
                _CrtMemCheckpoint(&state);
                std::cout << "Memory stats:\n";
                std::cout << "  Total: " << state.lTotalCount << " bytes\n";
                std::cout << "  Blocks: " << state.lCounts[_NORMAL_BLOCK] << "\n";
                _CrtMemDumpStatistics(&state);
#endif
            }
        }

        app->Shutdown();
        delete app;
    }

} // namespace Lumina