#pragma once
#include "Core/Base.h"
#include "Math/Matrix.h"

namespace Lumina
{
    class Camera
    {
    public:
        Camera();

        void SetPerspective(float fov, float aspect, float near, float far);
        void SetView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

        const glm::mat4& GetView() const { return m_View; }
        const glm::mat4& GetProjection() const { return m_Projection; }
        const glm::mat4 GetViewProjection() const { return m_Projection * m_View; }

        void UpdateUniformBuffer(BufferHandle buffer);

    private:
        glm::mat4 m_View;
        glm::mat4 m_Projection;
        glm::vec3 m_Position;
    };
}