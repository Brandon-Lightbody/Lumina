#include "ScriptAPI/ScriptEngine.h"
#include <iostream>

namespace Lumina {
    void ScriptEngine::Initialize() {
        std::cout << "[ScriptEngine] Initialized\n";
    }

    void ScriptEngine::Shutdown() {
        std::cout << "[ScriptEngine] Shutdown\n";
    }

    void ScriptEngine::NativeLog(const char* message) {
        std::cout << "[ScriptEngine] Received: " << message << "\n";
    }
} // namespace Lumina