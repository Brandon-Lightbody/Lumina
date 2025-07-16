#pragma once
#include "Core/Engine.h"

namespace Lumina {
    class Application {
    public:
        Application();
        virtual ~Application() = default;

        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void Shutdown() = 0;

    protected:
        Engine* m_Engine;
    };
} // namespace Lumina