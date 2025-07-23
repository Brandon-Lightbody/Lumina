#include "Shadows.h"
#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    TextureHandle ShadowMapper::s_ShadowAtlas = nullptr;
    std::vector<glm::mat4> ShadowMapper::s_LightMatrices;

    void ShadowMapper::Init()
    {
        TextureDesc desc;
        desc.width = SHADOW_MAP_SIZE * 4; // 4 cascades
        desc.height = SHADOW_MAP_SIZE;
        desc.format = FORMAT_D32_SFLOAT;
        desc.usage = TEXTURE_USAGE_DEPTH_STENCIL;
        s_ShadowAtlas = ResourceManager::CreateTexture(desc);
    }

    void ShadowMapper::RenderShadows(nri::CommandBuffer& cmd)
    {
        // Set viewport for shadow map
        nri::Viewport viewport = { 0, 0, (float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE, 0, 1 };
        cmd.SetViewports(&viewport, 1);

        // Render each cascade
        for (uint32_t i = 0; i < s_LightMatrices.size(); i++) {
            // Set render target
            nri::Texture* depthTexture = ResourceManager::GetTexture(s_ShadowAtlas);
            nri::ClearDesc clear = { 0, 0, 1.0f };
            cmd.ClearDepthStencil(depthTexture, nri::ClearBits::DEPTH_STENCIL, clear);

            // Set light matrix
            // cmd.SetConstants(0, &s_LightMatrices[i], sizeof(glm::mat4));

            // Render shadow casters
            // ...
        }
    }
}