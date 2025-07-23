#include "HybridRendering.h"
#include "RayTracingCore.h"
#include "ShaderSystem.h"
#include "RendererAPI.h"

namespace Lumina
{
    nri::Pipeline* HybridRenderer::m_RayGenPipeline = nullptr;

    void HybridRenderer::Init()
    {
        ShaderHandle rgen = ShaderSystem::Load("Shaders/RayGen.hlsl", nri::ShaderStage::RAY_GEN);
        ShaderHandle rchit = ShaderSystem::Load("Shaders/RayCHit.hlsl", nri::ShaderStage::CLOSEST_HIT);
        ShaderHandle rmiss = ShaderSystem::Load("Shaders/RayMiss.hlsl", nri::ShaderStage::MISS);

        nri::RayTracingPipelineDesc desc = {};
        desc.shaders = {
            {nri::ShaderStage::RAY_GEN, ShaderSystem::GetShaderDesc(rgen)},
            {nri::ShaderStage::CLOSEST_HIT, ShaderSystem::GetShaderDesc(rchit)},
            {nri::ShaderStage::MISS, ShaderSystem::GetShaderDesc(rmiss)}
        };
        m_RayGenPipeline = RendererAPI::Get()->CreateRayTracingPipeline(desc);
    }

    void HybridRenderer::SetupFrameGraph(FrameGraph& frameGraph)
    {
        FrameGraph::Pass pass;
        pass.name = "RayTracing";
        pass.execute = [](nri::CommandBuffer& cmd, const ResourceHandle* resources) {
            RayTracingCore::DispatchRays(cmd);
            };
        frameGraph.AddPass(pass);
    }
}