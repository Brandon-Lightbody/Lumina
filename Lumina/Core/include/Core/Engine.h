#pragma once

namespace Lumina {
    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize();
        void Shutdown();
        void Run();

        static Engine& Get();

    private:
        static Engine* s_Instance;
    };
} // namespace Lumina