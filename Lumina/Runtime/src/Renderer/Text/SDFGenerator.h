#pragma once
#include "Core/Base.h"

namespace Lumina
{
    class SDFGenerator
    {
    public:
        static void GenerateSDF(
            const uint8_t* source,
            uint32_t width,
            uint32_t height,
            uint8_t* output,
            uint32_t spread,
            float scale = 1.0f);
    };
}