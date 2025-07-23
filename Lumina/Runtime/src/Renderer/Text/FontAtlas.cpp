#include "FontAtlas.h"
#include "ResourceManager.h"
#include "SDFGenerator.h"
#include <freetype/freetype.h>

namespace Lumina
{
    FontAtlas::FontAtlas(const std::string& path, uint32_t size) : m_Size(size)
    {
        FT_Library ft;
        FT_Init_FreeType(&ft);

        FT_Face face;
        FT_New_Face(ft, path.c_str(), 0, &face);
        FT_Set_Pixel_Sizes(face, 0, size);

        // Calculate atlas size
        uint32_t width = 0, height = 0;
        for (uint32_t c = 0; c < 128; c++) {
            FT_Load_Char(face, c, FT_LOAD_RENDER);
            width += face->glyph->bitmap.width;
            height = std::max(height, face->glyph->bitmap.rows);
        }

        // Create texture
        TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = FORMAT_R8_UNORM;
        m_Texture = ResourceManager::CreateTexture(desc);

        // Generate glyphs
        uint32_t x = 0;
        for (uint32_t c = 0; c < 128; c++) {
            FT_Load_Char(face, c, FT_LOAD_RENDER);

            GlyphInfo glyph;
            glyph.size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
            glyph.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
            glyph.advance = face->glyph->advance.x >> 6;
            glyph.uvMin = { x / (float)width, 0.0f };
            glyph.uvMax = { (x + glyph.size.x) / (float)width, glyph.size.y / (float)height };

            // Generate SDF
            std::vector<uint8_t> sdf;
            SDFGenerator::Generate(face->glyph->bitmap.buffer, glyph.size.x, glyph.size.y, sdf);

            // Upload to texture
            void* data = ResourceManager::MapTexture(m_Texture, x, 0);
            memcpy(data, sdf.data(), sdf.size());
            ResourceManager::UnmapTexture(m_Texture);

            x += glyph.size.x;
            m_Glyphs[c] = glyph;
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
}