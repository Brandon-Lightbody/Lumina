#pragma once
#include "Core/Base.h"
#include "ResourceManager.h"

namespace Lumina
{
    class ShadowMapper
    {
    public:
        static void Init();
        static void Update(const Camera& camera);

        static TextureHandle GetShadowAtlas();
        static glm::mat4 GetLightMatrix(uint32_t lightIndex);

        static void RenderShadows(nri::CommandBuffer& cmd);

    private:
        static void CreateShadowAtlas();
        static void UpdateCascades(const Camera& camera);
    };
}