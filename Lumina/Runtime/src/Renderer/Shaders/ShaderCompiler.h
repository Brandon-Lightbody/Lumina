#pragma once
#include "Core/Base.h"
#include <NRI.h>
#include <vector>
#include <future>

namespace Lumina
{
    enum class ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE,
        GEOMETRY,
        TESS_CONTROL,
        TESS_EVALUATION,
        RAYGEN,
        MISS,
        CLOSEST_HIT,
        ANY_HIT,
        INTERSECTION,
        MESH_CONTROL,
        MESH_EVALUATION,
        TASK,
        MAX_NUM
    };

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
            ShaderStage stage);

        static nri::ShaderDesc CreateShaderDesc(const ShaderBlob& blob);

    private:
        ShaderBlob CompileGLSL(const std::string& source, ShaderStage stage, const std::string& sourceName);
        ShaderBlob CompileHLSL(const std::string& source, const std::string& entryPoint, ShaderStage stage, const std::string& sourceName);
    };
}