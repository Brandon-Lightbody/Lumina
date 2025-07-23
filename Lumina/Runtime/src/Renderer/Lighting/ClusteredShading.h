#pragma once
#include "LightingCommon.h"

namespace Lumina
{
    class ClusteredShading
    {
    public:
        static void Init();

        static void UpdateClusters(const CameraData& camera);
        static void CullLights(const LightData* lights, uint32_t lightCount);

        static BufferHandle GetLightIndexBuffer();
        static BufferHandle GetClusterDataBuffer();
        static TextureHandle GetLightGrid();

        struct ClusterData {
            glm::uvec3 gridSize;
            float zNear;
            float zFar;
            float scale;
            float bias;
        };
    };
}