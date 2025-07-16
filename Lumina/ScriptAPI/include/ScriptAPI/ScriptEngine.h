#pragma once
#include "Core/Defines.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace Lumina {
    class ScriptEngine {
    public:
        using StringCallback = void(*)(const char*);

        LUMINA_EXPORT static bool Initialize();
        LUMINA_EXPORT static void Shutdown();

        LUMINA_EXPORT static void ExecuteManagedFunction(const std::string& methodName);

        LUMINA_EXPORT static void RegisterNativeMethod(const std::string& name, StringCallback callback);

        // Internal P/Invoke interface
        LUMINA_EXPORT static void CreateString(StringCallback callback, const char* message);

    private:
        static void InitializeCoreClr();
        static void LoadEmbeddedAssembly();

        static std::unordered_map<std::string, StringCallback> native_methods;
    };
} // namespace Lumina