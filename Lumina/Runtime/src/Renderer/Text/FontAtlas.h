#pragma once
#include "Core/Base.h"
#include "ResourceManager.h"

namespace Lumina
{
    struct GlyphInfo {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 bearing;
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        float advance;
    };

    class FontAtlas
    {
    public:
        FontAtlas(const std::string& path, uint32_t size);
        ~FontAtlas();

        const GlyphInfo& GetGlyph(uint32_t codepoint) const;
        TextureHandle GetTexture() const { return m_Texture; }
        float GetLineHeight() const { return m_LineHeight; }

    private:
        void LoadFont(const std::string& path, uint32_t size);
        void CreateAtlasTexture();

        TextureHandle m_Texture;
        std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;
        float m_LineHeight = 0.0f;
    };
}