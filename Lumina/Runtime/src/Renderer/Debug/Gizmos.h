#pragma once
#include "Core/Base.h"

namespace Lumina
{
    class GizmoRenderer
    {
    public:
        enum class GizmoType { TRANSLATE, ROTATE, SCALE };

        static void Init();
        static void Shutdown();

        static void SetGizmoType(GizmoType type);
        static void DrawTransformGizmo(glm::mat4& transform);
        static bool IsInteracting();

        static void Update(const Camera& camera, const glm::vec2& mousePos);
        static void Render(nri::CommandBuffer& cmd);
    };
}