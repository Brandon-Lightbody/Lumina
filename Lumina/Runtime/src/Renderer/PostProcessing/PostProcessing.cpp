#include "PostProcessing.h"
#include "Bloom.h"
#include "ToneMapping.h"

namespace Lumina
{
    void PostProcessing::SetupFrameGraph(FrameGraph& frameGraph)
    {
        FrameGraph::Pass pass;
        pass.name = "Bloom";
        pass.execute = [](nri::CommandBuffer& cmd, const ResourceHandle* resources) {
            BloomEffect::Apply(cmd, resources[0], resources[1]);
            };
        frameGraph.AddPass(pass);

        pass.name = "Tonemapping";
        pass.execute = [](nri::CommandBuffer& cmd, const ResourceHandle* resources) {
            TonemappingEffect::Apply(cmd, resources[0], resources[1]);
            };
        frameGraph.AddPass(pass);
    }
}