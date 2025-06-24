#include "Engine/Platform/GLFW/GLFWWindow.h"
#include <stdexcept>

namespace Engine::Platform {
    GLFWWindow::GLFWWindow() {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW initialization failed");
        }
    }

    GLFWWindow::~GLFWWindow() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    bool GLFWWindow::create(const char* title, uint32_t width, uint32_t height) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(
            static_cast<int>(width),
            static_cast<int>(height),
            title, nullptr, nullptr);
        return m_Window != nullptr;
    }

    void GLFWWindow::close() {
        if (m_Window) {
            glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
        }
    }

    bool GLFWWindow::should_close() const {
        return m_Window ? glfwWindowShouldClose(m_Window) : true;
    }

    void GLFWWindow::poll_events() {
        glfwPollEvents();
    }
}