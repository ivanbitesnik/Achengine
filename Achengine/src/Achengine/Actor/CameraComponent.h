#pragma once

#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Actor/SpringArmComponent.h"
#include "Achengine/Renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Achengine
{
    class ACHENGINE_API UCameraComponent : public UActorComponent, public Camera
    {
    public:
        UCameraComponent() = default;
        UCameraComponent(float width, float height)
        {
            m_ViewportWidth = width;
            m_ViewportHeight = height;
            UpdateProjection();
        }

        virtual void Tick(float DeltaTime)
        {
		   UpdateView();
	    }

        void SetFieldOfView(float NewFovDegrees)
        {
            m_FOV = NewFovDegrees;
        }

        float GetFieldOfView() const { return m_FOV; }

        void SetClipPlanes(float NewNearClip, float NewFarClip)
        {
            m_NearClip = NewNearClip;
            m_FarClip = NewFarClip;
        }

        void SetRelativeLocation(const glm::vec3& NewLocation)
        {
            m_RelativeLocation = NewLocation;
        }

        const glm::vec3& GetRelativeLocation() const { return m_RelativeLocation; }

        void AttachToSpringArm(USpringArmComponent* SpringArm)
        {
            m_SpringArm = SpringArm;
        }

        USpringArmComponent* GetSpringArm() const { return m_SpringArm; }

        glm::vec3 GetWorldLocation() const
        {
            if (m_SpringArm)
            {
                return m_SpringArm->GetSocketWorldLocation() + m_RelativeLocation;
            }

            if (GetOwner())
            {
                return GetOwner()->GetActorLocation() + m_RelativeLocation;
            }

            return m_RelativeLocation;
        }

        glm::quat GetWorldRotation() const
        {
            if (m_SpringArm)
            {
                return m_SpringArm->GetWorldRotation();
            }

            if (GetOwner())
            {
                FActorRotation ownerRotation = GetOwner()->GetActorRotation();
                glm::vec3 axis = ownerRotation.RotationAxis;
                if (glm::length(axis) < 0.0001f)
                {
                    axis = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                return glm::angleAxis(glm::radians(ownerRotation.Angle), glm::normalize(axis));
            }

            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        
        void UpdateView()
        {
		    m_ViewMatrix = glm::translate(glm::mat4(1.0f), GetWorldLocation()) * glm::mat4(GetWorldRotation());
		    m_ViewMatrix = glm::inverse(m_ViewMatrix);
        }

        void UpdateProjection()
        {
            m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		    m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        }

    private:
        USpringArmComponent* m_SpringArm = nullptr;
        glm::vec3 m_RelativeLocation = glm::vec3(0.0f, 0.0f, 0.0f);
        float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;
    };
}
