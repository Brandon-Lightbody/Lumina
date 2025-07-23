#include "RayTracingCore.h"
#include "AccelerationStructure.h"
#include "ResourceManager.h"

namespace Lumina
{
    AccelerationStructure* RayTracingCore::s_TLAS = nullptr;
    AccelerationStructure* RayTracingCore::s_BLAS = nullptr;
    BufferHandle RayTracingCore::s_ShaderBindingTable = nullptr;

    void RayTracingCore::Init()
    {
        nri::Device* device = RendererAPI::Get()->GetDevice();
        s_TLAS = new AccelerationStructure(*device, nri::AccelerationStructureType::TOP_LEVEL);
        s_BLAS = new AccelerationStructure(*device, nri::AccelerationStructureType::BOTTOM_LEVEL);

        // Create SBT buffer
        BufferDesc sbtDesc;
        sbtDesc.size = 1024; // Actual size depends on pipelines
        sbtDesc.usage = BUFFER_USAGE_RAY_TRACING;
        s_ShaderBindingTable = ResourceManager::CreateBuffer(sbtDesc);
    }

    void RayTracingCore::DispatchRays(nri::CommandBuffer& cmd)
    {
        nri::DispatchRaysDesc dispatch = {};
        dispatch.rayGenShader = s_ShaderBindingTable;
        dispatch.width = Renderer::GetWidth();
        dispatch.height = Renderer::GetHeight();
        cmd.DispatchRays(dispatch);
    }
}