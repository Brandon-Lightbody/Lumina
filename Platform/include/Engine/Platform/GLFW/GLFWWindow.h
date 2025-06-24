#pragma once
#include "Engine/Core/Window.h"
#include <GLFW/glfw3.h>

namespace Engine::Platform {
    class GLFWWindow final : public Core::Window {
    public:
        GLFWWindow();
        ~GLFWWindow();

        bool create(const char* title, uint32_t width, uint32_t height) override;
        void close() override;
        bool should_close() const override;
        void poll_events() override;

    private:
        GLFWwindow* m_Window = nullptr;
    };
}