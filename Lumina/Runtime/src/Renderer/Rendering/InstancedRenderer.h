#pragma once
#include "Core/Base.h"
#include "ResourceManager.h"
#include "MaterialSystem.h"
#include "Mesh.h"

namespace Lumina
{
    struct InstanceData {
        glm::mat4 transform;
        glm::vec4 materialParams; // x: metallic, y: roughness, etc.
        uint32_t materialID;
        uint32_t instanceID;
    };

    class InstancedRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginBatch();
        static void SubmitInstance(const glm::mat4& transform, MaterialHandle material);
        static void EndBatch(nri::CommandBuffer& cmd, MeshHandle mesh);

        static void FlushAll(nri::CommandBuffer& cmd);

    private:
        static const uint32_t MAX_INSTANCES = 1024;
        static const uint32_t MAX_BATCHES = 128;

        struct InstanceBatch {
            MeshHandle mesh;
            MaterialHandle material;
            std::vector<InstanceData> instances;
        };

        static std::vector<InstanceBatch> s_Batches;
        static BufferHandle s_InstanceBuffer;
        static bool s_Batching;
    };
}