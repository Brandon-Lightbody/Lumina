#pragma once
#include "ScriptAPI.h"
#include <unordered_map>
#include <string>
#include <functional>
#include <filesystem>

namespace Lumina {
    class SCRIPTAPI_EXPORT ScriptEngine {
    public:
        using StringCallback = void(*)(const char*);

        static bool Initialize();
        static void Shutdown();
        static void RegisterNativeMethod(const std::string& name, StringCallback callback);
        static void ExecuteManagedFunction(const std::string& methodName);
        static void CreateString(StringCallback callback, const char* message);

    private:
        static bool InitializeCoreClr();
        static std::filesystem::path GetCurrentModuleDirectory();

        static std::unordered_map<std::string, StringCallback> native_methods;
    };
}