#pragma once
#include <unordered_map>
#include <NRI.h>

namespace Lumina
{
    struct ShaderReflectionData {
        struct Binding {
            uint32_t set;
            uint32_t binding;
            nri::DescriptorType type;
            nri::ShaderStage stage;
        };

        std::unordered_map<std::string, Binding> bindings;
        std::vector<nri::PushConstant> pushConstants;
        nri::VertexAttributeDesc* vertexAttributes;
        uint32_t vertexAttributeCount;
    };

    class ShaderSystem
    {
    public:
        static void Init();

        static ShaderHandle Load(const std::string& path);
        static ShaderReflectionData Reflect(ShaderHandle handle);

        static void HotReload();

    private:
        static void CompileShader(ShaderHandle handle);
        static void ExtractReflection(ShaderHandle handle);
    };
}