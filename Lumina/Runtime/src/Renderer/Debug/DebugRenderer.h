#pragma once
#include "Core/Base.h"
#include "Resources/ResourceManager.h"

namespace Lumina
{
    struct DebugVertex {
        glm::vec3 position;
        glm::vec4 color;
    };

    class DebugRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
        static void DrawAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color);
        static void DrawSphere(const glm::vec3& center, float radius, const glm::vec4& color);
        static void DrawFrustum(const glm::mat4& viewProj, const glm::vec4& color);

        static void EndFrame(nri::CommandBuffer& cmd);

    private:
        static void FlushLines(nri::CommandBuffer& cmd);
        static void FlushTriangles(nri::CommandBuffer& cmd);

        static BufferHandle m_VertexBuffer;
        static std::vector<DebugVertex> m_LineVertices;
        static std::vector<DebugVertex> m_TriangleVertices;
        static nri::Pipeline* m_LinePipeline;
        static nri::Pipeline* m_TrianglePipeline;
    };
}