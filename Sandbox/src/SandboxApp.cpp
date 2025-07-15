#include "Core/EntryPoint.h"
#include "Runtime/Application.h"
#include "ScriptAPI/ScriptEngine.h"

#include <iostream>

class SandboxApp : public Lumina::Application {
public:
    void Initialize() override {
        std::cout << "[Sandbox] Initializing...\n";
        Lumina::ScriptEngine::Initialize();
    }

    void Update() override {
        // Game logic update
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

int main(int argc, char** argv) {
    Lumina::Application* app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}