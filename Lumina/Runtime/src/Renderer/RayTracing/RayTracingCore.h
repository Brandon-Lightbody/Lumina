#pragma once
#include "AccelerationStructure.h"

namespace Lumina
{
    class RayTracingCore
    {
    public:
        static void Init();

        static void BuildSceneAccelerationStructure(
            const MeshInstance* instances,
            uint32_t count);

        static void UpdateTLAS(const Transform* transforms, uint32_t count);

        static void DispatchRays(
            nri::CommandBuffer& cmd,
            ShaderHandle rayGenShader,
            uint32_t width,
            uint32_t height);

        struct RayTracingConfig {
            bool hybridRendering = true;
            uint32_t maxRecursion = 2;
            uint32_t samplesPerFrame = 1;
        };
        static void Configure(const RayTracingConfig& config);

    private:
        static void CreatePipeline(ShaderHandle shader);
        static void CreateShaderBindingTable();
    };
}