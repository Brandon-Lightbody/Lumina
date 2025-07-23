#pragma once
#include "Core/Base.h"
#include "ShaderSystem.h"
#include "ResourceManager.h"

namespace Lumina
{
    struct MaterialTexture {
        TextureHandle handle;
        uint32_t binding;
    };

    class Material
    {
    public:
        Material(ShaderHandle shader);

        void SetTexture(const std::string& name, TextureHandle texture);
        void SetParameter(const std::string& name, const glm::vec4& value);

        void Bind(nri::CommandBuffer& cmd);

    private:
        ShaderHandle m_Shader;
        std::unordered_map<std::string, MaterialTexture> m_Textures;
        std::unordered_map<std::string, glm::vec4> m_Parameters;
        BufferHandle m_UniformBuffer;
    };

    using MaterialHandle = uint32_t;

    class MaterialSystem
    {
    public:
        static void Init();
        static MaterialHandle CreateMaterial(ShaderHandle shader);
        static Material* GetMaterial(MaterialHandle handle);
    };
}