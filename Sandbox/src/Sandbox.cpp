#include "Runtime/Application.h"
#include "Runtime/EntryPoint.h"
#include "Runtime/ScriptEngine.h"

#include <iostream>
#include <memory>

#ifdef _MSC_VER
#include <windows.h>  // For OutputDebugStringA
#include <cstdlib>    // For _putenv
#endif

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
#if defined(_MSC_VER) && defined(_DEBUG)
    // MSVC-specific ASan configuration
    ::_set_abort_behavior(0, _WRITE_ABORT_MSG);

    // Set ASan options
    ::_putenv("ASAN_OPTIONS=detect_leaks=0:halt_on_error=0:report_path=asan_report.txt");
    ::_putenv("UBSAN_OPTIONS=halt_on_error=0:report_path=ubsan_report.txt");
#endif

    auto app = std::make_unique<Sandbox>();
    Lumina::EntryPoint::Run(app.get());
    return 0;
}