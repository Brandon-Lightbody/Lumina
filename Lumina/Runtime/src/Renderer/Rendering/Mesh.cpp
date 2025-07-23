#include "Mesh.h"
#include "ResourceManager.h"

namespace Lumina
{
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        // Create vertex buffer
        BufferDesc vtxDesc;
        vtxDesc.size = vertices.size() * sizeof(Vertex);
        vtxDesc.usage = BUFFER_USAGE_VERTEX;
        vtxDesc.memory = MEMORY_LOCATION_DEVICE;
        m_VertexBuffer = ResourceManager::CreateBuffer(vtxDesc);

        // Upload data
        void* data = ResourceManager::MapBuffer(m_VertexBuffer);
        memcpy(data, vertices.data(), vtxDesc.size);
        ResourceManager::UnmapBuffer(m_VertexBuffer);

        // Create index buffer
        BufferDesc idxDesc;
        idxDesc.size = indices.size() * sizeof(uint32_t);
        idxDesc.usage = BUFFER_USAGE_INDEX;
        idxDesc.memory = MEMORY_LOCATION_DEVICE;
        m_IndexBuffer = ResourceManager::CreateBuffer(idxDesc);

        // Upload data
        data = ResourceManager::MapBuffer(m_IndexBuffer);
        memcpy(data, indices.data(), idxDesc.size);
        ResourceManager::UnmapBuffer(m_IndexBuffer);

        m_IndexCount = static_cast<uint32_t>(indices.size());
    }

    void Mesh::Draw(nri::CommandBuffer& cmd) const
    {
        auto* vtxBuffer = ResourceManager::GetBuffer(m_VertexBuffer);
        auto* idxBuffer = ResourceManager::GetBuffer(m_IndexBuffer);

        nri::BufferBinding bufferBinding;
        bufferBinding.buffer = vtxBuffer;
        bufferBinding.offset = 0;

        cmd.SetVertexBuffers(0, 1, &bufferBinding);
        cmd.SetIndexBuffer(idxBuffer, 0);
        cmd.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
    }

    // MeshSystem implementation
    std::vector<Mesh*> MeshSystem::s_Meshes;

    void MeshSystem::Init()
    {
        // Initialize mesh system
    }

    MeshHandle MeshSystem::CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Mesh* mesh = new Mesh(vertices, indices);
        s_Meshes.push_back(mesh);
        return static_cast<MeshHandle>(s_Meshes.size() - 1);
    }

    Mesh* MeshSystem::GetMesh(MeshHandle handle)
    {
        return s_Meshes[handle];
    }
}