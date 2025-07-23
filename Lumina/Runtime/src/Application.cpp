#include "Runtime/Application.h"
#include "Core/Engine.h"

namespace Lumina {
    Application::Application() : m_Engine(&Engine::Get()) {}

    // Define destructor in implementation file
    Application::~Application() {
        // Engine is a singleton, should not be deleted here
        m_Engine = nullptr;
    }
} // namespace Lumina