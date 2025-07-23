#pragma once
#include "Core/Base.h"
#include <NRI.h>
#include <spirv_cross.hpp>

namespace Lumina
{
    class ShaderReflection
    {
    public:
        struct BindingInfo {
            uint32_t set;
            uint32_t binding;
            nri::DescriptorType type;
            nri::ShaderStage stage;
        };

        ShaderReflection(const std::vector<uint32_t>& spirv);

        const std::unordered_map<std::string, BindingInfo>& GetBindings() const;
        const std::vector<nri::VertexAttributeDesc>& GetVertexAttributes() const;

    private:
        void ReflectShader();

        std::vector<uint32_t> m_SPIRV;
        std::unordered_map<std::string, BindingInfo> m_Bindings;
        std::vector<nri::VertexAttributeDesc> m_VertexAttributes;
    };
}