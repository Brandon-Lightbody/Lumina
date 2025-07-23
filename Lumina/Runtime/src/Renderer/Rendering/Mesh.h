#pragma once
#include "Core/Base.h"
#include "ResourceManager.h"

namespace Lumina
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 tangent;
    };

    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~Mesh();

        void Draw(nri::CommandBuffer& cmd) const;

    private:
        BufferHandle m_VertexBuffer;
        BufferHandle m_IndexBuffer;
        uint32_t m_IndexCount;
    };

    using MeshHandle = uint32_t;

    class MeshSystem
    {
    public:
        static void Init();
        static MeshHandle CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        static Mesh* GetMesh(MeshHandle handle);
    };
}