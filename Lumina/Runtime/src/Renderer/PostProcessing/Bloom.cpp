#include "Bloom.h"
#include "ShaderSystem.h"
#include "RendererAPI.h"

namespace Lumina
{
    nri::Pipeline* BloomEffect::m_DownsamplePipeline = nullptr;
    nri::Pipeline* BloomEffect::m_UpsamplePipeline = nullptr;
    nri::Pipeline* BloomEffect::m_CombinePipeline = nullptr;
    std::vector<TextureHandle> BloomEffect::m_Pyramid;

    void BloomEffect::Init()
    {
        ShaderHandle downsampleCS = ShaderSystem::Load("Shaders/BloomDownsample.hlsl", nri::ShaderStage::COMPUTE);
        ShaderHandle upsampleCS = ShaderSystem::Load("Shaders/BloomUpsample.hlsl", nri::ShaderStage::COMPUTE);
        ShaderHandle combineVS = ShaderSystem::Load("Shaders/BloomCombine.hlsl", nri::ShaderStage::VERTEX);
        ShaderHandle combinePS = ShaderSystem::Load("Shaders/BloomCombine.hlsl", nri::ShaderStage::FRAGMENT);

        nri::ComputePipelineDesc downsampleDesc = {};
        downsampleDesc.computeShader = ShaderSystem::GetShaderDesc(downsampleCS);
        m_DownsamplePipeline = RendererAPI::Get()->CreateComputePipeline(downsampleDesc);

        // Similar for other pipelines
    }

    void BloomEffect::Apply(nri::CommandBuffer& cmd, TextureHandle input, TextureHandle output)
    {
        // Downsample pass
        cmd.SetPipeline(*m_DownsamplePipeline);
        cmd.SetDescriptorTexture(0, 0, ResourceManager::GetTexture(input));
        cmd.Dispatch(16, 16, 1); // Thread group size

        // Upsample and combine
        // ...
    }
}