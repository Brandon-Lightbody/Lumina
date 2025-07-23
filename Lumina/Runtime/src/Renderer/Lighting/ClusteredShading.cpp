#include "ClusteredShading.h"
#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    void ClusteredShading::Init()
    {
        // Create light grid texture
        TextureDesc gridDesc;
        gridDesc.width = CLUSTER_X * CLUSTER_Y * CLUSTER_Z;
        gridDesc.height = 1;
        gridDesc.format = FORMAT_RG32_UINT;
        gridDesc.usage = TEXTURE_USAGE_STORAGE;
        m_LightGrid = ResourceManager::CreateTexture(gridDesc);

        // Create light index buffer
        BufferDesc indexDesc;
        indexDesc.size = MAX_LIGHTS * sizeof(uint32_t);
        indexDesc.usage = BUFFER_USAGE_STORAGE;
        m_LightIndexBuffer = ResourceManager::CreateBuffer(indexDesc);
    }

    void ClusteredShading::UpdateClusters(const Camera& camera)
    {
        // Calculate cluster planes
        // ...

        // Cull lights against clusters
        // ...

        // Update GPU buffers
        // ...
    }
}