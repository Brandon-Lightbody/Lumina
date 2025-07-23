#include "TextRenderer.h"
#include "ResourceManager.h"
#include "ShaderSystem.h"
#include "RendererAPI.h"

namespace Lumina
{
    std::vector<TextRenderer::GlyphVertex> TextRenderer::s_Vertices;
    BufferHandle TextRenderer::m_VertexBuffer = nullptr;
    nri::Pipeline* TextRenderer::m_Pipeline = nullptr;
    FontHandle TextRenderer::m_CurrentFont = nullptr;

    void TextRenderer::Init()
    {
        // Create vertex buffer
        BufferDesc desc;
        desc.size = MAX_CHARS * 6 * sizeof(GlyphVertex); // 6 verts per quad
        desc.usage = BUFFER_USAGE_VERTEX;
        m_VertexBuffer = ResourceManager::CreateBuffer(desc);

        // Create pipeline
        ShaderHandle vs = ShaderSystem::Load("Shaders/Text.hlsl", nri::ShaderStage::VERTEX);
        ShaderHandle ps = ShaderSystem::Load("Shaders/Text.hlsl", nri::ShaderStage::FRAGMENT);

        nri::GraphicsPipelineDesc pipelineDesc = {};
        pipelineDesc.vertexShader = ShaderSystem::GetShaderDesc(vs);
        pipelineDesc.fragmentShader = ShaderSystem::GetShaderDesc(ps);
        m_Pipeline = RendererAPI::Get()->CreatePipeline(pipelineDesc);
    }

    void TextRenderer::DrawString(const std::string& text, const glm::vec2& position, const glm::vec4& color, FontHandle font, float size)
    {
        if (font != m_CurrentFont) Flush();
        m_CurrentFont = font;

        FontAtlas* atlas = FontSystem::GetFont(font);
        float scale = size / atlas->GetSize();
        glm::vec2 cursor = position;

        for (char c : text) {
            const GlyphInfo& glyph = atlas->GetGlyph(c);

            // Calculate quad vertices
            glm::vec2 pos = cursor + glm::vec2(glyph.bearing.x, -glyph.bearing.y) * scale;
            glm::vec2 size = glyph.size * scale;

            // Add quad vertices
            AddQuad(pos, size, glyph.uvMin, glyph.uvMax, color);

            // Advance cursor
            cursor.x += glyph.advance * scale;
        }
    }

    void TextRenderer::Flush(nri::CommandBuffer& cmd)
    {
        if (s_Vertices.empty()) return;

        // Update vertex buffer
        void* data = ResourceManager::MapBuffer(m_VertexBuffer);
        memcpy(data, s_Vertices.data(), s_Vertices.size() * sizeof(GlyphVertex));
        ResourceManager::UnmapBuffer(m_VertexBuffer);

        // Set pipeline and resources
        cmd.SetPipeline(*m_Pipeline);
        nri::Buffer* vtxBuffer = ResourceManager::GetBuffer(m_VertexBuffer);
        nri::BufferBinding binding = { vtxBuffer, 0 };
        cmd.SetVertexBuffers(0, 1, &binding);

        // Set font texture
        FontAtlas* atlas = FontSystem::GetFont(m_CurrentFont);
        nri::Texture* texture = ResourceManager::GetTexture(atlas->GetTexture());
        cmd.SetDescriptorTexture(0, 0, texture);

        // Draw
        cmd.Draw(s_Vertices.size(), 1, 0, 0);

        s_Vertices.clear();
    }
}