#include "Engine/Core/Window.h"
#include "Engine/Core/Log.h"

namespace Engine::Core {
    // Definitions of static members
    Window::FactoryFunc Window::s_factory = nullptr;
    bool Window::s_factoryRegistered = false;

    void Window::registerFactory(FactoryFunc factory) {
        if (s_factoryRegistered) {
            LOG_WARN("Window factory already registered - replacing previous registration");
        }
        s_factory = factory;
        s_factoryRegistered = true;
        LOG_DEBUG("Window factory registered successfully (Registered: {})", s_factoryRegistered);
    }

    std::unique_ptr<Window> Window::create() {
        LOG_TRACE("Attempting to create window instance...");

        if (!s_factoryRegistered) {
            LOG_CRITICAL("Window factory not registered! Call RegisterFactory() first");
            throw std::runtime_error("No window factory registered!");
        }

        try {
            auto window = s_factory();
            if (!window) {
                LOG_ERROR("Factory returned null window instance");
                throw std::runtime_error("Factory returned null window");
            }
            LOG_DEBUG("Window instance created successfully");
            return window;
        }
        catch (const std::exception& e) {
            LOG_CRITICAL("Window creation failed: {}", e.what());
            throw;
        }
    }
}