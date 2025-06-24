#pragma once
#include <Engine/Core/Window.h>
#include <memory>

namespace Engine::Runtime {

    class Application {
    public:
        Application();
        virtual ~Application() = default;

        void run();

    protected:
        virtual void initialize();
        virtual void update();
        virtual void render();
        virtual void shutdown();

        std::unique_ptr<Core::Window> window;
        bool running = true;
    };

} // namespace engine::runtime