#include "Core/Engine.h"
#include <iostream>

namespace Lumina {
    // Initialize static member
    Engine* Engine::s_Instance = nullptr;

    Engine::Engine() {
        if (s_Instance) {
            std::cerr << "Engine instance already exists!\n";
            return;
        }
        s_Instance = this;
    }

    Engine::~Engine() {
        s_Instance = nullptr;
    }

    bool Engine::Initialize() {
        std::cout << "Engine initialized\n";
        return true;
    }

    void Engine::Shutdown() {
        std::cout << "Engine shutdown\n";
    }

    // Implement Get() method
    Engine& Engine::Get() {
        if (!s_Instance) {
            // Create a static instance if none exists
            static Engine defaultInstance;
            s_Instance = &defaultInstance;
        }
        return *s_Instance;
    }
} // namespace Lumina