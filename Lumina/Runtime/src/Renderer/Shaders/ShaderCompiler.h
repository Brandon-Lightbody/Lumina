#pragma once
#include "Core/Base.h"
#include <NRI.h>
#include <vector>
#include <future>

namespace Lumina
{
    class ShaderCompiler
    {
    public:
        struct ShaderBlob {
            std::vector<uint32_t> bytecode;
            nri::ShaderDesc desc;
        };

        ShaderCompiler();

        std::future<ShaderBlob> CompileShaderAsync(
            const std::string& filePath,
            const std::string& entryPoint,
            nri::ShaderStage stage);

        static nri::ShaderDesc CreateShaderDesc(const ShaderBlob& blob);

    private:
        ShaderBlob CompileGLSL(const std::string& source, nri::ShaderStage stage, const std::string& sourceName);
        ShaderBlob CompileHLSL(const std::string& source, const std::string& entryPoint, nri::ShaderStage stage, const std::string& sourceName);
    };
}