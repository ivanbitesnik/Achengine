#pragma once

#include "Achengine/Actor/ActorComponent.h"

namespace Achengine
{
    class Texture;
    struct RendererStorage;

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

    class UStaticMesh : public UActorComponent
    {
        public:
            UStaticMesh();
            void Initialize();

            void SetMaterial(FMeshMaterial* NewMaterial) { m_Material = NewMaterial; }
            FMeshMaterial* GetMaterial() const { return m_Material; }

            void SetTexture(Texture* NewTexture) { m_Texture = NewTexture; }
            Texture* GetTexture() const { return m_Texture; }
            void SetSpecular(Texture* NewSpecular) { m_Specular = NewSpecular; }
            Texture* GetSpecular() const { return m_Specular; }

            virtual void DrawMesh(RendererStorage* RenderData);
        protected:
            std::string m_ShaderPath;
            std::string m_ShaderName;
        private:
            FMeshMaterial* m_Material = nullptr;
            Texture* m_Texture = nullptr;
            Texture* m_Specular = nullptr;
    };
}