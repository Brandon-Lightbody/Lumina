#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#define LUMINA_DLL_EXPORT __declspec(dllexport)
#define LUMINA_DLL_IMPORT __declspec(dllimport)

#ifdef LUMINA_BUILD_DLL
#define LUMINA_API LUMINA_DLL_EXPORT
#else
#define LUMINA_API LUMINA_DLL_IMPORT
#endif

#define BIT(x) (1 << x)

namespace Lumina
{
    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;
    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using float32 = float;
    using float64 = double;

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;
}