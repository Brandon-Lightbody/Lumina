#include "ToneMapping.h"
#include "ShaderSystem.h"
#include "RendererAPI.h"

namespace Lumina
{
    nri::Pipeline* TonemappingEffect::m_Pipeline = nullptr;

    void TonemappingEffect::Init()
    {
        ShaderHandle vs = ShaderSystem::Load("Shaders/Fullscreen.hlsl", nri::ShaderStage::VERTEX);
        ShaderHandle ps = ShaderSystem::Load("Shaders/Tonemapping.hlsl", nri::ShaderStage::FRAGMENT);

        nri::GraphicsPipelineDesc desc = {};
        desc.vertexShader = ShaderSystem::GetShaderDesc(vs);
        desc.fragmentShader = ShaderSystem::GetShaderDesc(ps);
        m_Pipeline = RendererAPI::Get()->CreatePipeline(desc);
    }

    void TonemappingEffect::Apply(nri::CommandBuffer& cmd, TextureHandle input, TextureHandle output)
    {
        cmd.SetPipeline(*m_Pipeline);
        cmd.SetDescriptorTexture(0, 0, ResourceManager::GetTexture(input));
        cmd.Draw(3, 1, 0, 0); // Fullscreen triangle
    }
}