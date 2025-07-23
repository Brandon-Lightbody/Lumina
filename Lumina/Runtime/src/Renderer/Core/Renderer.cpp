#include "Renderer.h"
#include "Scene/Scene.h"
#include "Rendering/RenderQueue.h"
#include "RayTracing/RayTracingCore.h"
#include "Text/TextRenderer.h"

namespace Lumina
{
    void Renderer::Init()
    {
        // Initialize subsystems
        RendererAPI::Init();
        ResourceManager::Init();
        ShaderSystem::Init();
        MaterialSystem::Init();
        MeshSystem::Init();
        ClusteredShading::Init();
        RayTracingCore::Init();
        TextRenderer::Init();
        DebugRenderer::Init();
        RenderQueue::Init();
        InstancedRenderer::Init();

        // Create global resources
        CreateGlobalUniformBuffer();
        CreateBRDFLUT();
        CreateDefaultTextures();

        // Compile essential shaders
        CompileCoreShaders();

        // Setup frame graph
        BuildFrameGraph();
    }

    void Renderer::SubmitScene(const SceneData& scene)
    {
        // Process all scene objects
        for (auto& object : scene.objects) {
            if (object.instanced) {
                InstancedRenderer::SubmitInstance(object.transform, object.material);
            }
            else {
                RenderQueue::AddDraw(object.mesh, object.material, object.transform);
            }
        }

        // Process lights
        ClusteredShading::SubmitLights(scene.lights);
    }

    void Renderer::EndFrame()
    {
        auto* cmd = RendererAPI::GetCommandBuffer();

        // Execute frame graph
        FrameGraph::Execute(cmd);

        // Execute instanced rendering
        InstancedRenderer::FlushAll(cmd);

        // Execute GPU-driven rendering
        RenderQueue::BuildCommandBuffer(cmd);

        // Render debug/UI
        DebugRenderer::Flush(cmd);
        TextRenderer::EndFrame(cmd);

        // Present
        RendererAPI::Present();
    }
}