#pragma once

#include "RuntimeExports.h"

namespace Lumina {
    class RUNTIME_API Application;

    class EntryPoint {
    public:
        static int Run(Application* application);
    };
} // namespace Lumina