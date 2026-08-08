#include "Achenginepch.h"
#include "CameraComponent.h"

#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/Player.h"
#include "Achengine/Actor/PlayerController.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Achengine
{
    UCameraComponent::UCameraComponent()
    {
        UpdateProjection();
    }

    UCameraComponent::UCameraComponent(float width, float height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        UpdateProjection();
    }

    void UCameraComponent::Tick(float DeltaTime)
    {
        UpdateView();
    }

    void UCameraComponent::SetClipPlanes(float NewNearClip, float NewFarClip)
    {
        m_NearClip = NewNearClip;
        m_FarClip = NewFarClip;
    }

    glm::vec3 UCameraComponent::GetWorldLocation() const
    {
        if (GetOwner())
        {
            glm::vec3 ownerAxis = GetOwner()->GetActorRotation().RotationAxis;
            if (glm::length(ownerAxis) < 0.0001f)
            {
                ownerAxis = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            const glm::quat ownerRotation = glm::angleAxis(glm::radians(GetOwner()->GetActorRotation().Angle), glm::normalize(ownerAxis));

            glm::vec3 relativeLocation = GetRelativeLocation();
            if (APlayer* player = dynamic_cast<APlayer*>(GetOwner()))
            {
                if (APlayerController* controller = player->GetPlayerController())
                {
                    // Player local right axis is +Z in this movement convention.
                    const glm::quat pitchRotation = glm::angleAxis(glm::radians(controller->GetPitchDegrees()), glm::vec3(0.0f, 0.0f, 1.0f));
                    relativeLocation = pitchRotation * relativeLocation;
                }
            }

            return GetOwner()->GetActorLocation() + (ownerRotation * relativeLocation);
        }

        return GetRelativeLocation();
    }

    glm::quat UCameraComponent::GetWorldRotation() const
    {
        if (GetOwner())
        {
            const glm::vec3 cameraLocation = GetWorldLocation();
            const glm::vec3 target = GetOwner()->GetActorLocation() + glm::vec3(0.0f, 2.0f, 0.0f);
            const glm::vec3 toTarget = target - cameraLocation;
            if (glm::length(toTarget) > 0.0001f)
            {
                const glm::vec3 forward = glm::normalize(toTarget);
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                if (glm::abs(glm::dot(forward, up)) > 0.999f)
                {
                    up = glm::vec3(0.0f, 0.0f, 1.0f);
                }

                const glm::mat4 view = glm::lookAt(cameraLocation, target, up);
                return glm::normalize(glm::quat_cast(glm::transpose(glm::mat3(view))));
            }
        }

        return UActorComponent::GetWorldRotation();
    }
    
    void UCameraComponent::UpdateView()
    {
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), GetWorldLocation()) * glm::mat4(GetWorldRotation());
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
        m_Position = GetWorldLocation();
    }

    void UCameraComponent::UpdateProjection()
    {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    }
}