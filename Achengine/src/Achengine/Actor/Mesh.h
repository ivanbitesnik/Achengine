#pragma once

#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Renderer/Buffer.h"

namespace Achengine
{
    struct FMeshBounds
    {
        glm::vec3 LocalCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 LocalExtents = glm::vec3(1.0f, 1.0f, 1.0f);
        float LocalSphereRadius = 1.7320508f;
        bool IsValid = true;
    };

    struct FMeshMaterial
    {
        public:
            FMeshMaterial(const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess) :
                color(color), ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}
            glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 ambient = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
            float shininess = 32.0f;
    };

    class Texture;

    class UMesh : public UActorComponent
    {
        public:
            UMesh() {}

            std::string GetShaderName() const;
            const std::string& GetVertexArrayName() const { return m_VertexArrayName; }

            const FMeshMaterial* GetMaterial() const { return m_Material; }
            const Texture* GetTexture() const { return m_Texture; }
            const Texture* GetSpecular() const { return m_Specular; }
            virtual FMeshBounds GetBounds() const { return m_Bounds; }

            virtual void SubmitLighting() {}
            virtual void DrawMesh();
            virtual void DrawGeometry();
        protected:
            void Initialize();

            virtual void SetUniforms() {}
            virtual void GenerateVertexArray() {}

            std::string m_ShaderPath;
            std::string m_VertexArrayName;

            FMeshMaterial* m_Material = nullptr;
            Texture* m_Texture = nullptr;
            Texture* m_Specular = nullptr;
            FMeshBounds m_Bounds;
    };
}