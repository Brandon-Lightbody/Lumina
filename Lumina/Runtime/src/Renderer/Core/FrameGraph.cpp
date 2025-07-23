#include "FrameGraph.h"
#include "ResourceManager.h"
#include "RendererAPI.h"

namespace Lumina
{
    void FrameGraph::AddPass(const Pass& pass)
    {
        m_Passes.push_back(pass);
        m_Dirty = true;
    }

    void FrameGraph::Compile()
    {
        if (!m_Dirty) return;

        // 1. Cull unused passes
        CullUnusedPasses();

        // 2. Calculate resource lifetimes
        CalculateResourceLifetimes();

        // 3. Allocate transient resources
        AllocateTransientResources();

        // 4. Build execution order
        BuildExecutionOrder();

        m_Dirty = false;
    }

    void FrameGraph::Execute(nri::CommandBuffer& cmd)
    {
        Compile();

        for (auto& pass : m_ExecutionOrder) {
            // Set up resources
            for (auto& res : pass.inputs) {
                // Transition to read state
            }
            for (auto& res : pass.outputs) {
                // Transition to write state
            }

            // Execute pass
            pass.execute(cmd, pass.resources.data());
        }
    }
}