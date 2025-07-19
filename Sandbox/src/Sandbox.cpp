#include "Runtime/Application.h"
#include "Runtime/EntryPoint.h"
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

int main() {
	Sandbox* app = new Sandbox();
	Lumina::EntryPoint::Run(app);
    std::cin.get();
    delete app;
    return 0;
}