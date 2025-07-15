#include "Runtime/Application.h"

namespace Lumina {
    Application::Application() : m_Engine(&Engine::Get()) {}

    Application::~Application() {}

    void Application::Run() {
        if (m_Engine->Initialize()) {
            Initialize();

            while (true) {
                Update();
                Render();
            }

            Shutdown();
        }
        m_Engine->Shutdown();
    }
} // namespace Lumina