#include "Runtime/EntryPoint.h"
#include "Runtime/Application.h"
#include "Core/Engine.h"

namespace Lumina {
    int EntryPoint::Run(Application* application) {
        Engine& engine = Engine::Get();
        if (!engine.Initialize()) {
            return 1;
        }

        application->Initialize();

        while (engine.IsRunning()) {
            application->Update();
            application->Render();
        }

        application->Shutdown();
        engine.Shutdown();
        return 0;
    }
} // namespace Lumina