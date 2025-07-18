#pragma once

#include "Defines.h"

namespace Lumina {
    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize();
        void Shutdown();

        bool IsRunning() const { return m_Running; }
        void Quit() { m_Running = false; }

        static Engine& Get();

    private:
        static Engine* s_Instance;
        bool m_Running = true;
    };
} // namespace Lumina