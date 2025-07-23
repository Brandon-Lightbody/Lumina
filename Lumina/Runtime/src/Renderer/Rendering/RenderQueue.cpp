#include "RenderQueue.h"
#include "ResourceManager.h"

namespace Lumina
{
    BufferHandle RenderQueue::m_DrawDataBuffer = nullptr;
    BufferHandle RenderQueue::m_IndirectBuffer = nullptr;
    bool RenderQueue::m_Dirty = true;
    std::vector<RenderQueue::GPUDrivenData> RenderQueue::s_DrawData;

    void RenderQueue::Init()
    {
        BufferDesc desc;
        desc.size = MAX_DRAWS * sizeof(GPUDrivenData);
        desc.usage = BUFFER_USAGE_STORAGE;
        m_DrawDataBuffer = ResourceManager::CreateBuffer(desc);

        desc.usage = BUFFER_USAGE_INDIRECT;
        m_IndirectBuffer = ResourceManager::CreateBuffer(desc);
    }

    void RenderQueue::AddDraw(MeshHandle mesh, MaterialHandle material, const glm::mat4& transform)
    {
        GPUDrivenData data;
        data.model = transform;
        data.materialIndex = material;
        data.meshIndex = mesh;
        s_DrawData.push_back(data);
        m_Dirty = true;
    }

    void RenderQueue::BuildCommandBuffer(nri::CommandBuffer& cmd)
    {
        if (!m_Dirty) return;

        // Update GPU buffers
        void* data = ResourceManager::MapBuffer(m_DrawDataBuffer);
        memcpy(data, s_DrawData.data(), s_DrawData.size() * sizeof(GPUDrivenData));
        ResourceManager::UnmapBuffer(m_DrawDataBuffer);

        // Execute GPU culling
        // ...

        m_Dirty = false;
    }
}