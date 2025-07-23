#pragma once
#include <NRI.h>
#include <variant>

namespace Lumina
{
    enum class ResourceType { Texture, Buffer, AccelerationStructure };

    struct TextureDesc {
        nri::Format format;
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arraySize = 1;
        nri::TextureUsageBits usage;
    };

    struct BufferDesc {
        uint64_t size;
        nri::BufferUsageBits usage;
        nri::MemoryLocation memory;
    };

    using ResourceHandle = uint32_t;

    class ResourceManager
    {
    public:
        static void Init(nri::Device& device);

        static ResourceHandle CreateTexture(const TextureDesc& desc);
        static ResourceHandle CreateBuffer(const BufferDesc& desc);
        static ResourceHandle CreateAccelerationStructure(const nri::AccelerationStructureDesc& desc);

        static void* MapBuffer(ResourceHandle handle);
        static void UnmapBuffer(ResourceHandle handle);

        static nri::Texture* GetTexture(ResourceHandle handle);
        static nri::Buffer* GetBuffer(ResourceHandle handle);
        static nri::AccelerationStructure* GetAccelerationStructure(ResourceHandle handle);

        static void BeginFrame();
        static void EndFrame();
        static void Destroy(ResourceHandle handle);
    };
}