#include "ShaderCompiler.h"
#include "Core/Platform/Platform.h"
#include <fstream>
#include <shaderc/shaderc.hpp>

namespace Lumina
{
    ShaderCompiler::ShaderCompiler()
    {
        // Initialize compiler
    }

    std::future<ShaderCompiler::ShaderBlob> ShaderCompiler::CompileShaderAsync(
        const std::string& filePath,
        const std::string& entryPoint,
        nri::ShaderStage stage)
    {
        return std::async(std::launch::async, [=]() {
            // Read source code
            std::ifstream file(filePath);
            std::string source((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            // Compile based on API
            switch (Platform::GetGraphicsAPI()) {
            case Platform::GraphicsAPI::VULKAN:
                return CompileGLSL(source, stage, filePath);
            case Platform::GraphicsAPI::D3D12:
                return CompileHLSL(source, entryPoint, stage, filePath);
            default:
                throw std::runtime_error("Unsupported graphics API");
            }
            });
    }

    ShaderCompiler::ShaderBlob ShaderCompiler::CompileGLSL(
        const std::string& source,
        nri::ShaderStage stage,
        const std::string& sourceName)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc_shader_kind kind;
        switch (stage) {
        case nri::ShaderStage::VERTEX: kind = shaderc_vertex_shader; break;
        case nri::ShaderStage::FRAGMENT: kind = shaderc_fragment_shader; break;
        case nri::ShaderStage::COMPUTE: kind = shaderc_compute_shader; break;
        default: throw std::runtime_error("Unsupported shader stage");
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            source, kind, sourceName.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error(result.GetErrorMessage());
        }

        ShaderBlob blob;
        blob.bytecode.assign(result.cbegin(), result.cend());
        blob.stage = stage;
        return blob;
    }

    nri::ShaderDesc ShaderCompiler::CreateShaderDesc(const ShaderBlob& blob)
    {
        nri::ShaderDesc desc = {};
        desc.bytecode = blob.bytecode.data();
        desc.size = blob.bytecode.size() * sizeof(uint32_t);
        desc.stage = blob.stage;
        return desc;
    }
}