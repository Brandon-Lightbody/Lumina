#include "Buffer.h"
#include "RendererAPI.h"

namespace Lumina
{
    Buffer::Buffer(const BufferDesc& desc) : m_Desc(desc)
    {
        nri::BufferDesc nriDesc = {};
        nriDesc.size = desc.size;
        nriDesc.usageMask = ConvertUsage(desc.usage); // Custom conversion function

        nri::Device* device = RendererAPI::Get()->GetDevice();
        device->CreateBuffer(nriDesc, &m_Handle);

        nri::MemoryAllocationDesc memDesc = {};
        memDesc.size = desc.size;
        memDesc.type = (desc.memory == MemoryLocation::HOST_VISIBLE) ?
            nri::MemoryLocation::HOST_VISIBLE : nri::MemoryLocation::DEVICE;
        device->AllocateMemory(memDesc, &m_Memory);

        device->BindBufferMemory(*m_Handle, *m_Memory);
    }

    void* Buffer::Map()
    {
        if (m_Desc.memory != MemoryLocation::HOST_VISIBLE) return nullptr;
        return m_Device->MapBuffer(*m_Handle, 0, m_Desc.size);
    }

    void Buffer::Unmap()
    {
        if (m_Desc.memory != MemoryLocation::HOST_VISIBLE) return;
        m_Device->UnmapBuffer(*m_Handle);
    }
}