#include "MaterialSystem.h"
#include "ResourceManager.h"
#include "ShaderSystem.h"

namespace Lumina
{
    Material::Material(ShaderHandle shader) : m_Shader(shader)
    {
        // Create uniform buffer
        BufferDesc desc;
        desc.size = 256; // Initial size
        desc.usage = BUFFER_USAGE_UNIFORM;
        desc.memory = MEMORY_LOCATION_HOST_VISIBLE;
        m_UniformBuffer = ResourceManager::CreateBuffer(desc);
    }

    void Material::SetTexture(const std::string& name, TextureHandle texture)
    {
        auto reflection = ShaderSystem::Reflect(m_Shader);
        if (reflection.bindings.find(name) != reflection.bindings.end()) {
            m_Textures[name] = { texture, reflection.bindings[name].binding };
        }
    }

    void Material::SetParameter(const std::string& name, const glm::vec4& value)
    {
        m_Parameters[name] = value;
        // TODO: Update uniform buffer
    }

    void Material::Bind(nri::CommandBuffer& cmd)
    {
        // Bind shader
        ShaderSystem::Bind(m_Shader, cmd);

        // Bind uniform buffer
        auto* buffer = ResourceManager::GetBuffer(m_UniformBuffer);
        nri::DescriptorRange range;
        range.descriptors = &buffer;
        range.descriptorNum = 1;
        cmd.SetDescriptorSet(0, 0, &range);

        // Bind textures
        for (auto& [name, tex] : m_Textures) {
            auto* texture = ResourceManager::GetTexture(tex.handle);
            cmd.SetDescriptorTexture(0, tex.binding, texture);
        }
    }

    // MaterialSystem implementation
    std::vector<Material*> MaterialSystem::s_Materials;

    void MaterialSystem::Init()
    {
        // Initialize material system
    }

    MaterialHandle MaterialSystem::CreateMaterial(ShaderHandle shader)
    {
        Material* material = new Material(shader);
        s_Materials.push_back(material);
        return static_cast<MaterialHandle>(s_Materials.size() - 1);
    }

    Material* MaterialSystem::GetMaterial(MaterialHandle handle)
    {
        return s_Materials[handle];
    }
}