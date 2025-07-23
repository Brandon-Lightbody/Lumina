#include "Gizmos.h"
#include "DebugRenderer.h"

namespace Lumina
{
    void GizmoRenderer::DrawTransformGizmo(glm::mat4& transform)
    {
        const glm::vec3 origin = transform[3];
        const float axisLength = 1.0f;

        DebugRenderer::DrawLine(origin, origin + transform[0] * axisLength, { 1,0,0,1 });
        DebugRenderer::DrawLine(origin, origin + transform[1] * axisLength, { 0,1,0,1 });
        DebugRenderer::DrawLine(origin, origin + transform[2] * axisLength, { 0,0,1,1 });
    }

    void GizmoRenderer::Render(nri::CommandBuffer& cmd)
    {
        // Gizmo rendering handled by DebugRenderer
    }
}