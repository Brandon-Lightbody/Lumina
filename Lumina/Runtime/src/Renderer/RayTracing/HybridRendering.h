#pragma once
#include "Core/Base.h"
#include "FrameGraph.h"

namespace Lumina
{
    class HybridRenderer
    {
    public:
        static void Init();
        static void SetupFrameGraph(FrameGraph& frameGraph);

        static void SetRayBounceCount(uint32_t count);
        static void SetDenoiserEnabled(bool enabled);

    private:
        static void CreatePipelines();
        static void CreateDenoiser();

        static nri::Pipeline* m_RayGenPipeline;
        static nri::Pipeline* m_ShadowPipeline;
        static nri::Pipeline* m_ReflectionPipeline;
    };
}