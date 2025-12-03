#pragma once

#include "Achengine/Actor/StaticMesh.h"

namespace Achengine
{
    struct FLightSource
    {
        public:
            FLightSource() {};
            FLightSource(const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) :
                color(color), ambient(ambient), diffuse(diffuse), specular(specular) {}
            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 ambient = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    };

    class ULightMesh : public UStaticMesh
    {
        public:
            ULightMesh();

            void SetLightSource(FLightSource* NewLightSource) { m_LightSource = NewLightSource; }
            FLightSource* GetLightSource() const { return m_LightSource; }

            virtual void DrawMesh() override;
        private:
            FLightSource* m_LightSource = nullptr;
    };
}