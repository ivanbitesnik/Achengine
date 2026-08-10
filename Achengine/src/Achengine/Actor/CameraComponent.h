#pragma once

#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Renderer/Camera.h"

namespace Achengine
{
    class ACHENGINE_API UCameraComponent : public UActorComponent, public Camera
    {
    public:
        UCameraComponent();
        UCameraComponent(float width, float height);

        virtual void Tick(float DeltaTime);

        void SetFieldOfView(float NewFovDegrees) { m_FOV = NewFovDegrees; }
        void SetViewportSize(float width, float height);

        float GetFieldOfView() const { return m_FOV; }

        void SetClipPlanes(float NewNearClip, float NewFarClip);
        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }

        virtual glm::vec3 GetWorldLocation() const override;
        virtual glm::quat GetWorldRotation() const override;
        
        void UpdateView();
        void UpdateProjection();

    private:
        float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;
    };
}
