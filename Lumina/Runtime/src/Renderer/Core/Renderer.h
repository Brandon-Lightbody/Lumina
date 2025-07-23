#pragma once
#include "Core/Base.h"
#include "RendererAPI.h"
#include "FrameGraph.h"
#include "ResourceManager.h"

namespace Lumina
{
    class LUMINA_API Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
        static void SubmitScene(const SceneData& scene);

        // Configuration
        static void SetResolution(uint32_t width, uint32_t height);
        static void SetRayTracingEnabled(bool enabled);

        // Resource Access
        static TextureHandle GetOutputTexture();
        static BufferHandle GetGlobalUniformBuffer();

        // Stats
        struct RenderStats {
            uint32_t drawCalls;
            uint32_t triangles;
            float gpuTime;
            float cpuTime;
        };
        static RenderStats GetStats();

    private:
        static void BuildFrameGraph();
        static void CreateGlobalUniformBuffer();
        static void CreateBRDFLUT();
        static void CreateDefaultTextures();
        static void CompileCoreShaders();
        static void UpdateGlobalUniforms();
        static void UploadDynamicData();
        static void UpdateRenderStats();
    };
}