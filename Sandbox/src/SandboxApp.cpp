#include "Runtime/Application.h"
#include "ScriptAPI/ScriptEngine.h"

#include <iostream>

class SandboxApp : public Lumina::Application {
public:
    void Initialize() override {
        std::cout << "[Sandbox] Initializing...\n";

        // Register native method callbacks
        Lumina::ScriptEngine::RegisterNativeMethod("LogMessage",
            [](const char* message) {
                std::cout << "[Sandbox] " << message << "\n";
            }
        );

        // Initialize scripting
        if (Lumina::ScriptEngine::Initialize()) {
            Lumina::ScriptEngine::ExecuteManagedFunction("InitializeManaged");
        }
    }

    void Update() override {
        Lumina::ScriptEngine::ExecuteManagedFunction("UpdateManaged");
    }

    void Render() override {
        // Rendering code
    }

    void Shutdown() override {
        Lumina::ScriptEngine::Shutdown();
        std::cout << "[Sandbox] Shutting down...\n";
    }
};

Lumina::Application* CreateApplication() {
    return new SandboxApp();
}

int main() {
    auto app = CreateApplication();
    app->Initialize();
    delete app;
    std::cin.get();
    return 0;
}