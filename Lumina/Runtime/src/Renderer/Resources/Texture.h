#pragma once
#include "Core/Base.h"
#include <NRI.h>

namespace Lumina
{
    enum class TextureFormat {
        RGBA8_UNORM,
        RGBA16_FLOAT,
        D32_SFLOAT,
        // ... other formats
    };

    enum class TextureUsage {
        COLOR_ATTACHMENT,
        DEPTH_STENCIL,
        TEXTURE_SAMPLED,
        STORAGE
    };

    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arraySize = 1;
        TextureFormat format;
        TextureUsage usage;
    };

    class Texture
    {
    public:
        Texture(const TextureDesc& desc);
        ~Texture();

        nri::Texture* GetHandle() const { return m_Handle; }

    private:
        nri::Texture* m_Handle = nullptr;
        nri::Memory* m_Memory = nullptr;
        TextureDesc m_Desc;
    };
}