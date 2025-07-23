#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    std::vector<Texture*> ResourceManager::s_Textures;
    std::vector<Buffer*> ResourceManager::s_Buffers;
    std::vector<AccelerationStructure*> ResourceManager::s_AccelerationStructures;
    nri::Device* ResourceManager::s_Device = nullptr;

    void ResourceManager::Init()
    {
        s_Device = RendererAPI::Get()->GetDevice();
    }

    TextureHandle ResourceManager::CreateTexture(const TextureDesc& desc)
    {
        Texture* texture = new Texture(desc);
        s_Textures.push_back(texture);
        return static_cast<TextureHandle>(s_Textures.size() - 1);
    }

    BufferHandle ResourceManager::CreateBuffer(const BufferDesc& desc)
    {
        Buffer* buffer = new Buffer(desc);
        s_Buffers.push_back(buffer);
        return static_cast<BufferHandle>(s_Buffers.size() - 1);
    }

    void* ResourceManager::MapBuffer(BufferHandle handle)
    {
        return s_Buffers[handle]->Map();
    }

    void ResourceManager::UnmapBuffer(BufferHandle handle)
    {
        s_Buffers[handle]->Unmap();
    }

    nri::Texture* ResourceManager::GetTexture(TextureHandle handle)
    {
        return s_Textures[handle]->GetHandle();
    }

    nri::Buffer* ResourceManager::GetBuffer(BufferHandle handle)
    {
        return s_Buffers[handle]->GetHandle();
    }

    void ResourceManager::BeginFrame()
    {
        // Process pending deletions
        for (auto& [frame, texture] : m_PendingTextureDeletions) {
            if (frame <= m_CurrentFrame) {
                delete texture;
            }
        }

        // Similar for buffers

        m_CurrentFrame++;
    }

    void ResourceManager::Destroy(ResourceHandle handle)
    {
        // Schedule for deletion after N frames
        m_PendingTextureDeletions.push_back({ m_CurrentFrame + 3, s_Textures[handle] });
    }
}