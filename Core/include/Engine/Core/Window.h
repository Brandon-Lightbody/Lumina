#pragma once
#include <memory>
#include <functional>
#include <stdexcept>

namespace Engine::Core {
    class Window {
    public:
        using FactoryFunc = std::function<std::unique_ptr<Window>()>;

        virtual ~Window() = default;

        // Factory management (declaration only)
        static void registerFactory(FactoryFunc factory);
        static std::unique_ptr<Window> create();

        // Window interface
        virtual bool create(const char* title, uint32_t width, uint32_t height) = 0;
        virtual void close() = 0;
        virtual bool should_close() const = 0;
        virtual void poll_events() = 0;

    protected:
        // Declaration only - defined in Window.cpp
        static FactoryFunc s_factory;
        static bool s_factoryRegistered;
    };
}