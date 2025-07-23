#pragma once
#include "RendererAPI.h"
#include <NRI.h>
#include <Extensions/NRIHelper.h>
#include <Extensions/Streamer.h>

namespace Lumina
{
    class NRIRenderer : public RendererAPI
    {
    public:
        NRIRenderer();
        ~NRIRenderer();

        void Initialize(void* nativeDevice, void* nativeWindow) override;
        void Shutdown() override;
        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;
        void Resize(uint32_t width, uint32_t height) override;

        nri::CommandBuffer* GetCommandBuffer() override;
        nri::Device* GetDevice() override;

        nri::Texture* CreateTexture(const nri::TextureDesc& desc) override;
        nri::Buffer* CreateBuffer(const nri::BufferDesc& desc) override;
        nri::Pipeline* CreatePipeline(const nri::GraphicsPipelineDesc& desc) override;
        nri::Pipeline* CreateComputePipeline(const nri::ComputePipelineDesc& desc) override;
        nri::Descriptor* CreateDescriptor(const nri::DescriptorDesc& desc) override;

        FrameStats GetFrameStats() const override;

    private:
        void CreateSwapChain();
        void CreateFrameContexts();
        void DestroyFrameContexts();

        nri::Device* m_Device = nullptr;
        nri::SwapChain* m_SwapChain = nullptr;
        nri::CommandQueue* m_GraphicsQueue = nullptr;
        nri::Streamer* m_Streamer = nullptr;
        const nri::HelperInterface* m_Helper = nullptr;

        struct FrameContext {
            nri::CommandAllocator* commandAllocator;
            nri::CommandBuffer* commandBuffer;
            nri::Fence* fence;
            uint64_t fenceValue;
        };
        std::vector<FrameContext> m_Frames;
        uint32_t m_FrameIndex = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;

        FrameStats m_FrameStats;
    };
}