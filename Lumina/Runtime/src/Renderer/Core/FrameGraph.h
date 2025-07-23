#pragma once
#include "ResourceManager.h"

namespace Lumina
{
    class FrameGraph
    {
    public:
        struct Pass {
            std::string name;
            std::function<void(nri::CommandBuffer&, const ResourceHandle*)> execute;
            std::vector<ResourceHandle> inputs;
            std::vector<ResourceHandle> outputs;
        };

        ResourceHandle CreateTexture(const TextureDesc& desc);
        ResourceHandle CreateBuffer(const BufferDesc& desc);

        void AddPass(const Pass& pass);
        void Compile();
        void Execute(nri::CommandBuffer& cmd);

        void SetBackbuffer(ResourceHandle handle);

    private:
        void CullUnusedPasses();
        void CalculateResourceLifetimes();
        void AllocateTransientResources();
    };
}