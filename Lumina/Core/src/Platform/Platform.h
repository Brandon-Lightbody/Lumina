#pragma once
#include "Core/Base.h"

namespace Lumina::Platform
{
    enum class WindowAPI { GLFW, WIN32, COCOA };
    enum class GraphicsAPI { VULKAN, D3D12, METAL };

    struct WindowConfig {
        const char* title = "Lumina Engine";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool fullscreen = false;
        bool vsync = true;
    };

    void Initialize();
    void Shutdown();
    void* CreateWindow(const WindowConfig& config);
    void DestroyWindow(void* window);
    void PollEvents();
    bool WindowShouldClose(void* window);
    void* GetNativeWindowHandle(void* window);
    void* GetNativeDeviceContext(void* window);

    void SetGraphicsAPI(GraphicsAPI api);
    GraphicsAPI GetGraphicsAPI();
}