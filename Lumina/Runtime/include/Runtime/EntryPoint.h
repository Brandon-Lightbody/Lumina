#pragma once

#include "RuntimeExports.h"

namespace Lumina {
    class RUNTIME_API Application;

    class RUNTIME_API EntryPoint {
    public:
        static int Run(Application* application);
    };
} // namespace Lumina