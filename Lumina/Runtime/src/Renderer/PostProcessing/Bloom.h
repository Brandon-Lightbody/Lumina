#pragma once
#include "Core/Base.h"
#include "Resources/ResourceManager.h"

namespace Lumina
{
    class BloomEffect
    {
    public:
        static void Init();
        static void Apply(nri::CommandBuffer& cmd, TextureHandle input, TextureHandle output);

    private:
        static void CreatePipelines();
        static void CreateTextures(uint32_t width, uint32_t height);

        static nri::Pipeline* m_DownsamplePipeline;
        static nri::Pipeline* m_UpsamplePipeline;
        static nri::Pipeline* m_CombinePipeline;
        static std::vector<TextureHandle> m_Pyramid;
    };
}