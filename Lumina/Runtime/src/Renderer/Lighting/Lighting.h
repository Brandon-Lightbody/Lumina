#pragma once
#include "Core/Base.h"
#include "Shading/MaterialSystem.h"

namespace Lumina
{
    struct Light {
        enum Type { Directional, Point, Spot };

        Type type;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float range;
        float innerConeAngle;
        float outerConeAngle;
        bool castShadows;
    };

    class LightingSystem
    {
    public:
        static void Init();
        static void SubmitLight(const Light& light);
        static void Update();

        static uint32_t GetLightCount();
        static const Light* GetLights();

        static void SetAmbientLight(const glm::vec3& color, float intensity);
    };
}