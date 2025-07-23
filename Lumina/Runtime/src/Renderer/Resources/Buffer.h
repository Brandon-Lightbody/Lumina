#pragma once
#include "Core/Base.h"
#include <NRI.h>

namespace Lumina
{
    enum class BufferUsage {
        VERTEX,
        INDEX,
        UNIFORM,
        STORAGE,
        INDIRECT,
        RAY_TRACING
    };

    enum class MemoryLocation {
        DEVICE,
        HOST_VISIBLE
    };

    struct BufferDesc {
        uint64_t size;
        BufferUsage usage;
        MemoryLocation memory;
    };

    class Buffer
    {
    public:
        Buffer(const BufferDesc& desc);
        ~Buffer();

        nri::Buffer* GetHandle() const { return m_Handle; }
        void* Map();
        void Unmap();

    private:
        nri::Buffer* m_Handle = nullptr;
        nri::Memory* m_Memory = nullptr;
        BufferDesc m_Desc;
    };
}