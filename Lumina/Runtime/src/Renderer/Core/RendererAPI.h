#pragma once
#include "Core/Base.h"
#include <NRI.h>

namespace Lumina
{
    class LUMINA_API RendererAPI
    {
    public:
        virtual ~RendererAPI() = default;

        static RendererAPI* Create();

        virtual void Initialize(void* nativeDevice, void* nativeWindow) = 0;
        virtual void Shutdown() = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Present() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual nri::CommandBuffer* GetCommandBuffer() = 0;
        virtual nri::Device* GetDevice() = 0;

        // Resource creation
        virtual nri::Texture* CreateTexture(const nri::TextureDesc& desc) = 0;
        virtual nri::Buffer* CreateBuffer(const nri::BufferDesc& desc) = 0;
        virtual nri::Pipeline* CreatePipeline(const nri::GraphicsPipelineDesc& desc) = 0;
        virtual nri::Pipeline* CreateComputePipeline(const nri::ComputePipelineDesc& desc) = 0;
        virtual nri::Pipeline* CreateRayTracingPipeline(const nri::RayTracingPipelineDesc& desc) = 0;
        virtual nri::Descriptor* CreateDescriptor(const nri::DescriptorDesc& desc) = 0;

        // Stats
        struct FrameStats {
            uint32_t drawCalls;
            uint32_t triCount;
            float gpuTime;
        };
        virtual FrameStats GetFrameStats() const = 0;
    };
}