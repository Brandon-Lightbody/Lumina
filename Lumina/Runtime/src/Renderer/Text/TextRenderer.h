#pragma once
#include "FontAtlas.h"

namespace Lumina
{
    struct GlyphVertex {
        glm::vec2 position;
        glm::vec2 uv;
        glm::vec4 color;
    };

    class TextRenderer
    {
    public:
        static void Init();

        static void BeginFrame();
        static void DrawString(
            const std::string& text,
            const glm::vec2& position,
            const glm::vec4& color,
            FontHandle font,
            float size);

        static void EndFrame(nri::CommandBuffer& cmd);

        static FontHandle LoadFont(const std::string& path, uint32_t size);

    private:
        static void CreateFontAtlas(FontHandle handle);
        static void FlushBatch();
    };
}