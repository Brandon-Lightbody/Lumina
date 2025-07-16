#include "Runtime/Application.h"
#include "Core/Engine.h"

namespace Lumina {
	Application::Application() : m_Engine(&Engine::Get()) {}
} // namespace Lumina