#pragma once

#include "Core/Engine.h"

#ifdef _WIN32
#define SCRIPTAPI_EXPORT __declspec(dllexport)
#else
#define SCRIPTAPI_EXPORT __attribute__((visibility("default")))
#endif

namespace Lumina {
    class ScriptEngine {
    public:
        SCRIPTAPI_EXPORT static void Initialize();
        SCRIPTAPI_EXPORT static void Shutdown();

        // Called from C#
        SCRIPTAPI_EXPORT static void NativeLog(const char* message);
    };
} // namespace Lumina