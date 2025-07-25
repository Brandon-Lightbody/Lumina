#pragma once
#include "Core/Base.h"

namespace Lumina
{
    class TonemappingEffect
    {
    public:
        enum Operator { ACES, Filmic, Reinhard, Uncharted };

        static void Init();
        static void Apply(
            nri::CommandBuffer& cmd,
            TextureHandle input,
            TextureHandle output,
            Operator op = ACES,
            float exposure = 1.0f);

        static void SetOperator(Operator op);
        static void SetExposure(float exposure);
    };
}