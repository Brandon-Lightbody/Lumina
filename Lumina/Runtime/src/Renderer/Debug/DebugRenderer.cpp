#include "DebugRenderer.h"
#include "ShaderSystem.h"
#include "RendererAPI.h"

namespace Lumina
{
    BufferHandle DebugRenderer::m_VertexBuffer = nullptr;
    std::vector<DebugVertex> DebugRenderer::m_LineVertices;
    std::vector<DebugVertex> DebugRenderer::m_TriangleVertices;
    nri::Pipeline* DebugRenderer::m_LinePipeline = nullptr;
    nri::Pipeline* DebugRenderer::m_TrianglePipeline = nullptr;

    void DebugRenderer::Init()
    {
        // Create vertex buffer
        BufferDesc desc;
        desc.size = 1024 * sizeof(DebugVertex);
        desc.usage = BUFFER_USAGE_VERTEX;
        m_VertexBuffer = ResourceManager::CreateBuffer(desc);

        // Create pipelines
        ShaderHandle vs = ShaderSystem::Load("Shaders/DebugLine.hlsl", nri::ShaderStage::VERTEX);
        ShaderHandle ps = ShaderSystem::Load("Shaders/DebugLine.hlsl", nri::ShaderStage::FRAGMENT);

        nri::GraphicsPipelineDesc pipelineDesc = {};
        pipelineDesc.vertexShader = ShaderSystem::GetShaderDesc(vs);
        pipelineDesc.fragmentShader = ShaderSystem::GetShaderDesc(ps);
        // ... setup other pipeline state

        m_LinePipeline = RendererAPI::Get()->CreatePipeline(pipelineDesc);

        // Similar for triangle pipeline
    }

    void DebugRenderer::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
    {
        m_LineVertices.push_back({ start, color });
        m_LineVertices.push_back({ end, color });
    }

    void DebugRenderer::EndFrame(nri::CommandBuffer& cmd)
    {
        if (!m_LineVertices.empty()) FlushLines(cmd);
        if (!m_TriangleVertices.empty()) FlushTriangles(cmd);
    }

    void DebugRenderer::FlushLines(nri::CommandBuffer& cmd)
    {
        // Update vertex buffer
        void* data = ResourceManager::MapBuffer(m_VertexBuffer);
        memcpy(data, m_LineVertices.data(), m_LineVertices.size() * sizeof(DebugVertex));
        ResourceManager::UnmapBuffer(m_VertexBuffer);

        // Draw
        nri::Buffer* vtxBuffer = ResourceManager::GetBuffer(m_VertexBuffer);
        nri::BufferBinding binding = { vtxBuffer, 0 };
        cmd.SetVertexBuffers(0, 1, &binding);
        cmd.SetPipeline(*m_LinePipeline);
        cmd.Draw(m_LineVertices.size(), 1, 0, 0);

        m_LineVertices.clear();
    }
}