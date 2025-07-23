#include "Lighting.h"
#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    std::vector<Light> LightingSystem::s_Lights;
    glm::vec4 LightingSystem::s_AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };
    BufferHandle LightingSystem::s_LightBuffer = nullptr;

    void LightingSystem::Init()
    {
        BufferDesc desc;
        desc.size = MAX_LIGHTS * sizeof(Light);
        desc.usage = BUFFER_USAGE_UNIFORM;
        s_LightBuffer = ResourceManager::CreateBuffer(desc);
    }

    void LightingSystem::SubmitLight(const Light& light)
    {
        if (s_Lights.size() < MAX_LIGHTS) {
            s_Lights.push_back(light);
        }
    }

    void LightingSystem::Update()
    {
        if (s_Lights.empty()) return;

        void* data = ResourceManager::MapBuffer(s_LightBuffer);
        memcpy(data, s_Lights.data(), s_Lights.size() * sizeof(Light));
        ResourceManager::UnmapBuffer(s_LightBuffer);
    }
}