#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <filesystem>

// Forward declaration to avoid including ScriptAPI.h here
#ifdef _WIN32
#ifdef RUNTIME_EXPORTS
#define RUNTIME_ENGINE_API __declspec(dllexport)
#else
#define RUNTIME_ENGINE_API __declspec(dllimport)
#endif
#else
#define RUNTIME_ENGINE_API
#endif

namespace Lumina {
    class RUNTIME_ENGINE_API ScriptEngine {
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
    };
}