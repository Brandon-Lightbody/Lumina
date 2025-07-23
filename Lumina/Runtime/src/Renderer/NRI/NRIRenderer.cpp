#include "NRIRenderer.h"
#include "Core/Platform/Platform.h"

namespace Lumina
{
    NRIRenderer::NRIRenderer() = default;

    NRIRenderer::~NRIRenderer()
    {
        Shutdown();
    }

    void NRIRenderer::Initialize(void* nativeDevice, void* nativeWindow)
    {
        nri::DeviceCreationDesc deviceDesc = {};
        deviceDesc.graphicsAPI = Platform::GetGraphicsAPI() == Platform::GraphicsAPI::VULKAN ?
            nri::GraphicsAPI::VULKAN : nri::GraphicsAPI::D3D12;
        deviceDesc.d3d12Device = nativeDevice;
        deviceDesc.vulkanDevice = nativeDevice;
        nriCreateDeviceFromD3D12Device(deviceDesc, &m_Device);

        m_Helper = nri::GetHelperInterface();
        m_Streamer = m_Helper->CreateStreamer(m_Device, 1024 * 1024);

        nri::CommandQueue* queues = nullptr;
        uint32_t queueNum = 1;
        m_Device->GetCommandQueues(&queues, queueNum);
        m_GraphicsQueue = queues[0];

        CreateSwapChain();
        CreateFrameContexts();
    }

    void NRIRenderer::Shutdown()
    {
        if (m_Device) {
            m_Device->WaitForIdle();
            DestroyFrameContexts();
            nriDestroySwapChain(m_SwapChain);
            nriDestroyDevice(*m_Device);
            m_Device = nullptr;
        }
    }

    void NRIRenderer::BeginFrame()
    {
        m_FrameIndex = m_SwapChain->AcquireNextFrame();
        FrameContext& frame = m_Frames[m_FrameIndex];

        m_Device->Wait(*frame.fence, frame.fenceValue);
        m_Device->ResetCommandAllocator(*frame.commandAllocator);
    }

    void NRIRenderer::EndFrame()
    {
        FrameContext& frame = m_Frames[m_FrameIndex];
        nri::CommandBuffer* cmd = frame.commandBuffer;

        nri::QueueSubmitDesc submitDesc = {};
        submitDesc.commandBuffers = &cmd;
        submitDesc.commandBufferNum = 1;
        m_GraphicsQueue->Submit(submitDesc);

        nri::PresentDesc presentDesc = {};
        presentDesc.swapChain = m_SwapChain;
        m_GraphicsQueue->Present(presentDesc);

        frame.fenceValue = m_Device->QueueSignal(*m_GraphicsQueue, *frame.fence);
    }

    void NRIRenderer::Present()
    {
        // Handled in EndFrame
    }

    void NRIRenderer::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width == width && m_Height == height) return;

        m_Device->WaitForIdle();
        nriDestroySwapChain(m_SwapChain);

        m_Width = width;
        m_Height = height;
        CreateSwapChain();
    }

    nri::CommandBuffer* NRIRenderer::GetCommandBuffer()
    {
        return m_Frames[m_FrameIndex].commandBuffer;
    }

    nri::Device* NRIRenderer::GetDevice()
    {
        return m_Device;
    }

    void NRIRenderer::CreateSwapChain()
    {
        nri::SwapChainDesc swapChainDesc = {};
        swapChainDesc.window = m_NativeWindow;
        swapChainDesc.format = nri::Format::RGBA8_SRGB;
        swapChainDesc.verticalSyncInterval = 1;
        swapChainDesc.width = m_Width;
        swapChainDesc.height = m_Height;
        swapChainDesc.textureNum = 3;
        nriCreateSwapChain(m_Device, swapChainDesc, &m_SwapChain);
    }

    void NRIRenderer::CreateFrameContexts()
    {
        m_Frames.resize(3);

        for (auto& frame : m_Frames) {
            m_Device->CreateCommandAllocator(*m_GraphicsQueue, nri::CommandAllocatorType::DIRECT, &frame.commandAllocator);
            m_Device->CreateCommandBuffer(*frame.commandAllocator, &frame.commandBuffer);
            m_Device->CreateFence(0, &frame.fence);
            frame.fenceValue = 0;
        }
    }

    void NRIRenderer::DestroyFrameContexts()
    {
        for (auto& frame : m_Frames) {
            nriDestroyFence(*frame.fence);
            nriDestroyCommandBuffer(*frame.commandBuffer);
            nriDestroyCommandAllocator(*frame.commandAllocator);
        }
        m_Frames.clear();
    }

    // ... Resource creation methods ...
}