#include "Engine/Platform/GLFW/GLFWWindow.h"
#include "Engine/Core/Window.h"

namespace {
    // Force factory registration during static initialization
    [[maybe_unused]] const bool registered = []() {
        Engine::Core::Window::registerFactory(
            []() { return std::make_unique<Engine::Platform::GLFWWindow>(); }
        );
        return true;
        }();
}