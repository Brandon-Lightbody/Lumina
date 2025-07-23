#include "InstancedRenderer.h"
#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    std::vector<InstancedRenderer::InstanceBatch> InstancedRenderer::s_Batches;
    BufferHandle InstancedRenderer::s_InstanceBuffer = nullptr;
    bool InstancedRenderer::s_Batching = false;

    void InstancedRenderer::Init()
    {
        BufferDesc desc;
        desc.size = MAX_INSTANCES * sizeof(InstanceData);
        desc.usage = BUFFER_USAGE_VERTEX;
        s_InstanceBuffer = ResourceManager::CreateBuffer(desc);
    }

    void InstancedRenderer::SubmitInstance(const glm::mat4& transform, MaterialHandle material)
    {
        if (!s_Batching) return;

        for (auto& batch : s_Batches) {
            if (batch.material == material) {
                InstanceData data;
                data.transform = transform;
                batch.instances.push_back(data);
                return;
            }
        }

        InstanceBatch newBatch;
        newBatch.material = material;
        newBatch.instances.push_back({ transform });
        s_Batches.push_back(newBatch);
    }

    void InstancedRenderer::EndBatch(nri::CommandBuffer& cmd, MeshHandle mesh)
    {
        if (s_Batches.empty()) return;

        for (auto& batch : s_Batches) {
            if (batch.instances.empty()) continue;

            // Update instance buffer
            void* data = ResourceManager::MapBuffer(s_InstanceBuffer);
            memcpy(data, batch.instances.data(), batch.instances.size() * sizeof(InstanceData));
            ResourceManager::UnmapBuffer(s_InstanceBuffer);

            // Draw
            Mesh* meshPtr = MeshSystem::GetMesh(mesh);
            meshPtr->Draw(cmd, batch.instances.size());
        }

        s_Batches.clear();
    }
}