#include "Engine/Runtime/Application.h"
#include "Engine/Core/Log.h"

namespace Engine::Runtime {
    Application::Application() {
        LOG_DEBUG("Application constructor started");
        initialize();
        LOG_INFO("Application initialized successfully");
	}
    
    void Application::run() {
        LOG_INFO("Starting main application loop");
        size_t frameCount = 0;

        while (running && !window->should_close()) {
            frameCount++;
            LOG_TRACE("Frame {} started", frameCount);

            window->poll_events();
            update();
            render();

            if (frameCount % 100 == 0) {
                LOG_DEBUG("Running frame {}", frameCount);
            }
        }

        LOG_INFO("Application loop ended after {} frames", frameCount);
    }

    void Application::initialize() {
        LOG_TRACE("Initializing application subsystems...");

        try {
			Core::Log::Initialize();

            // Create window through factory
            LOG_DEBUG("Creating main window...");
            window = Core::Window::create();

            if (!window->create("Lumina Engine", 1280, 720)) {
                LOG_ERROR("Window creation failed");
                running = false;
            }

            LOG_INFO("Window created (1280x720)");
        }
        catch (const std::exception& e) {
            LOG_CRITICAL("Initialization failed: {}", e.what());
            running = false;
        }
    }

    void Application::update() {
        LOG_TRACE("Updating game state...");
    }

    void Application::render() {
        LOG_TRACE("Rendering frame...");
    }

    void Application::shutdown() {
        LOG_TRACE("Shutting down...");
    }
}