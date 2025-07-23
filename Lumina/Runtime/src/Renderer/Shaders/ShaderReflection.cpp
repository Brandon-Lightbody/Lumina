#include "ShaderReflection.h"
#include <spirv_cross/spirv_cross.hpp>

namespace Lumina
{
    ShaderReflection::ShaderReflection(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv)
    {
        ReflectShader();
    }

    void ShaderReflection::ReflectShader()
    {
        spirv_cross::Compiler compiler(m_SPIRV);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        // Extract uniform buffers
        for (auto& resource : resources.uniform_buffers) {
            BindingInfo binding;
            binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.type = nri::DescriptorType::CONSTANT_BUFFER;
            m_Bindings[resource.name] = binding;
        }

        // Extract textures
        for (auto& resource : resources.sampled_images) {
            BindingInfo binding;
            binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.type = nri::DescriptorType::TEXTURE;
            m_Bindings[resource.name] = binding;
        }

        // Extract vertex attributes
        if (compiler.get_execution_model() == spv::ExecutionModelVertex) {
            for (auto& resource : resources.stage_inputs) {
                nri::VertexAttributeDesc attr = {};
                attr.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                // ... other attributes
                m_VertexAttributes.push_back(attr);
            }
        }
    }
}