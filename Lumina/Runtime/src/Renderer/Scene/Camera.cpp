#include "Camera.h"
#include "Math/Matrix.h"

namespace Lumina
{
    Camera::Camera() : m_Position(0.0f, 0.0f, 0.0f)
    {
        SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
        SetView({ 0,0,0 }, { 0,0,1 }, { 0,1,0 });
    }

    void Camera::SetPerspective(float fov, float aspect, float near, float far)
    {
        m_Projection = Matrix4::Perspective(fov, aspect, near, far);
    }

    void Camera::SetView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    {
        m_Position = position;
        m_View = Matrix4::LookAt(position, target, up);
    }

    void Camera::UpdateUniformBuffer(BufferHandle buffer)
    {
        struct CameraData {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 viewProjection;
            glm::vec3 position;
        } data;

        data.view = m_View;
        data.projection = m_Projection;
        data.viewProjection = m_Projection * m_View;
        data.position = m_Position;

        void* mapped = ResourceManager::MapBuffer(buffer);
        memcpy(mapped, &data, sizeof(data));
        ResourceManager::UnmapBuffer(buffer);
    }
}