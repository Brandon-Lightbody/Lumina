#include "Scene.h"
#include "Renderer.h"

namespace Lumina
{
    GameObject* Scene::CreateObject(MeshHandle mesh, MaterialHandle material)
    {
        GameObject obj;
        obj.mesh = mesh;
        obj.material = material;
        obj.transform = glm::mat4(1.0f);
        m_Objects.push_back(obj);
        return &m_Objects.back();
    }

    void Scene::SubmitToRenderer()
    {
        Renderer::SubmitScene(*this);
    }

    void Scene::Update()
    {
        // Update object transforms
        for (auto& obj : m_Objects) {
            // Update logic here
        }

        // Update camera
        m_MainCamera.Update();
    }
}