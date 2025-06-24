#include "Engine/Runtime/Application.h"

class SandboxApp : public Engine::Runtime::Application {
public:
    SandboxApp() {

    }

    ~SandboxApp() override {

    }

protected:
    void initialize() override {
        Engine::Runtime::Application::initialize();
    }

    void update() override {
        // Game logic updates
    }

    void render() override {
        // Rendering code
    }
};

int main() {
    try {
        SandboxApp app;
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        return -1;
    }
}