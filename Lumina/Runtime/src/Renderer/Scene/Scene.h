#pragma once
#include "Core/Base.h"
#include "Rendering/Mesh.h"
#include "Shading/MaterialSystem.h"

namespace Lumina
{
    struct GameObject {
        glm::mat4 transform;
        MeshHandle mesh;
        MaterialHandle material;
    };

    class Scene
    {
    public:
        Scene();

        GameObject* CreateObject(MeshHandle mesh, MaterialHandle material);
        void DestroyObject(GameObject* object);

        void Update();
        void SubmitToRenderer();

    private:
        std::vector<GameObject> m_Objects;
        Camera m_MainCamera;
    };
}