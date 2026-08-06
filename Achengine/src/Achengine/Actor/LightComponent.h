#pragma once

#include "Achengine/Actor/ActorComponent.h"

namespace Achengine
{
    struct FLightSource
    {
        public:
            FLightSource() {};
            FLightSource(const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
                float constant = 1.0f, float linear = 0.014f, float quadratic = 0.0007f) :
                color(color), ambient(ambient), diffuse(diffuse), specular(specular),
                constant(constant), linear(linear), quadratic(quadratic) {}
            glm::vec3 color = glm::vec3{1.0f, 1.0f, 1.0f};
            glm::vec3 ambient = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
            float constant = 1.0f;
            float linear = 0.014f;
            float quadratic = 0.0007f;
    };

    class ULightComponent : public UActorComponent
    {
        public:
            ULightComponent();
            virtual ~ULightComponent();

            void SetLightSource(FLightSource* NewLightSource) { m_LightSource = NewLightSource; }
            FLightSource* GetLightSource() const { return m_LightSource; }

            void SubmitLighting() const;
        private:
            FLightSource* m_LightSource = nullptr;
    };
}