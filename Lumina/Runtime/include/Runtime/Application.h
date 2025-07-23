#pragma once
#include "Core/Engine.h"
#include "RuntimeExports.h"

namespace Lumina {
    class RUNTIME_API Application {
    public:
        Application();
        virtual ~Application();  // Declaration only

        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void Shutdown() = 0;

    protected:
        Engine* m_Engine;
    };
} // namespace Lumina