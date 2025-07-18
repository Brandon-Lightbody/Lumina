#pragma once

#ifdef _WIN32
#ifdef SCRIPTAPI_EXPORTS
#define SCRIPTAPI_EXPORT __declspec(dllexport)
#else
#define SCRIPTAPI_EXPORT __declspec(dllimport)
#endif
#else
#define SCRIPTAPI_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    SCRIPTAPI_EXPORT void LogToNative(const char* message);
}