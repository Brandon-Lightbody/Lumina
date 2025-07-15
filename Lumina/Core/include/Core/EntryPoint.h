#pragma once

#ifdef LUMINA_PLATFORM_WINDOWS

extern Lumina::Application* CreateApplication();

int main(int argc, char** argv) {
    auto app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}

#endif