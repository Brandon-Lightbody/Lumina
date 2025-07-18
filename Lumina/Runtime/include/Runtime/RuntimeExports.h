#pragma once

#ifdef _WIN32
#ifdef RUNTIME_EXPORTS
#define RUNTIME_API __declspec(dllexport)
#else
#define RUNTIME_API __declspec(dllimport)
#endif
#else
#define RUNTIME_API __attribute__((visibility("default")))
#endif