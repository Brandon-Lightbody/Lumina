#include "Runtime/Application.h"
#include "Runtime/EntryPoint.h"
#include "Runtime/ScriptEngine.h"

#include <iostream>
#include <memory>

#ifdef _MSC_VER
#include <windows.h>  // For OutputDebugStringA
#include <cstdlib>    // For _putenv
#endif

void TriggerASanError() {
    // Heap buffer overflow
    int* array = new int[10];
    array[10] = 42;  // Out-of-bounds write
    delete[] array;

    // Use-after-free
    int* ptr = new int(123);
    delete ptr;
    *ptr = 456;  // Use after free

    // Stack buffer overflow
    int stack_array[5];
    stack_array[5] = 789;  // Out-of-bounds write

    // Memory leak (should be detected if enabled)
    int* leak = new int[100];
}

class Sandbox : public Lumina::Application {
public:
    void Initialize() override {
        std::cout << "[Sandbox] Initializing...\n";

        TriggerASanError();

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