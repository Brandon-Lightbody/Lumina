#pragma once
#include "Core/Base.h"
#include "ResourceManager.h"

namespace Lumina
{
    class RenderQueue
    {
    public:
        struct GPUDrivenData {
            glm::mat4 model;
            uint32_t materialIndex;
            uint32_t meshIndex;
        };

        static void Init();
        static void AddDraw(MeshHandle mesh, MaterialHandle material, const glm::mat4& transform);
        static void BuildCommandBuffer(nri::CommandBuffer& cmd);

    private:
        static BufferHandle m_DrawDataBuffer;
        static BufferHandle m_IndirectBuffer;
        static bool m_Dirty;
    };
}