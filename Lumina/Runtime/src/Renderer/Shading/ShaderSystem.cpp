#include "ShaderSystem.h"
#include "ShaderCompiler.h"
#include "RendererAPI.h"

namespace Lumina
{
    std::vector<Shader> ShaderSystem::s_Shaders;
    ShaderCompiler ShaderSystem::s_Compiler;

    ShaderHandle ShaderSystem::Load(const std::string& path, nri::ShaderStage stage)
    {
        // Check if already loaded
        for (uint32_t i = 0; i < s_Shaders.size(); i++) {
            if (s_Shaders[i].path == path && s_Shaders[i].stage == stage) {
                return i;
            }
        }

        // Compile new shader
        Shader shader;
        shader.path = path;
        shader.stage = stage;
        shader.blob = s_Compiler.CompileShaderAsync(path, "main", stage).get();
        s_Shaders.push_back(shader);

        return static_cast<ShaderHandle>(s_Shaders.size() - 1);
    }

    void ShaderSystem::Bind(ShaderHandle handle, nri::CommandBuffer& cmd)
    {
        auto& shader = s_Shaders[handle];
        switch (shader.stage) {
        case nri::ShaderStage::VERTEX:
            cmd.SetPipelineShaderStage(nri::ShaderStage::VERTEX, shader.blob.desc);
            break;
        case nri::ShaderStage::FRAGMENT:
            cmd.SetPipelineShaderStage(nri::ShaderStage::FRAGMENT, shader.blob.desc);
            break;
            // ... other stages ...
        }
    }

    ShaderReflectionData ShaderSystem::Reflect(ShaderHandle handle)
    {
        return ShaderReflection(s_Shaders[handle].blob.bytecode);
    }
}