#include "Runtime/Application.h"
#include "Runtime/ScriptEngine.h"

#include <iostream>

class Sandbox : public Lumina::Application {
public:
    void Initialize() override {
        std::cout << "[Sandbox] Initializing...\n";
        Lumina::ScriptEngine::Initialize();

        Lumina::ScriptEngine::ExecuteManagedFunction("InitializeManaged");
    }

    void Update() override {
        Lumina::ScriptEngine::ExecuteManagedFunction("UpdateManaged");
    }

    void Render() override {}

    void Shutdown() override {
        Lumina::ScriptEngine::Shutdown();
    }
};

Lumina::Application* CreateApplication() {
    return new Sandbox();
}

int main() {
    auto app = CreateApplication();
    app->Initialize();
    std::cin.get();
    delete app;
    return 0;
}