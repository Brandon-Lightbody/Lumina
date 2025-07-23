#include "Texture.h"
#include "RendererAPI.h"

namespace Lumina
{
    Texture::Texture(const TextureDesc& desc) : m_Desc(desc)
    {
        nri::TextureDesc nriDesc = {};
        nriDesc.width = desc.width;
        nriDesc.height = desc.height;
        nriDesc.depth = desc.depth;
        nriDesc.mipNum = desc.mipLevels;
        nriDesc.arraySize = desc.arraySize;
        nriDesc.format = ConvertFormat(desc.format); // Custom conversion
        nriDesc.usageMask = ConvertUsage(desc.usage); // Custom conversion

        nri::Device* device = RendererAPI::Get()->GetDevice();
        device->CreateTexture(nriDesc, &m_Handle);

        nri::MemoryAllocationDesc memDesc = {};
        memDesc.size = device->GetTextureMemorySize(*m_Handle);
        memDesc.type = nri::MemoryLocation::DEVICE;
        device->AllocateMemory(memDesc, &m_Memory);

        device->BindTextureMemory(*m_Handle, *m_Memory);
    }
}