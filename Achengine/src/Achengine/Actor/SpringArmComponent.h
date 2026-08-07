#pragma once

#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/ActorComponent.h"

#include <glm/gtc/quaternion.hpp>

namespace Achengine
{
    class ACHENGINE_API USpringArmComponent : public UActorComponent
    {
    public:
        void SetTargetArmLength(float NewLength) { m_TargetArmLength = NewLength < 0.0f ? 0.0f : NewLength; }
        float GetTargetArmLength() const { return m_TargetArmLength; }

        void SetTargetOffset(const glm::vec3& NewOffset) { m_TargetOffset = NewOffset; }
        const glm::vec3& GetTargetOffset() const { return m_TargetOffset; }

        void SetSocketOffset(const glm::vec3& NewOffset) { m_SocketOffset = NewOffset; }
        const glm::vec3& GetSocketOffset() const { return m_SocketOffset; }

        void SetInheritActorRotation(bool bInherit) { m_InheritActorRotation = bInherit; }
        bool DoesInheritActorRotation() const { return m_InheritActorRotation; }

        void SetRelativeRotation(const glm::vec3& Axis, float AngleDegrees)
        {
            if (glm::length(Axis) < 0.0001f)
            {
                m_RelativeRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                m_RelativeRotationAxis = glm::normalize(Axis);
            }
            m_RelativeRotationAngle = AngleDegrees;
        }

        glm::quat GetWorldRotation() const
        {
            glm::quat actorRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            if (m_InheritActorRotation && GetOwner())
            {
                FActorRotation ownerRotation = GetOwner()->GetActorRotation();
                glm::vec3 axis = ownerRotation.RotationAxis;
                if (glm::length(axis) < 0.0001f)
                {
                    axis = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                actorRotation = glm::angleAxis(glm::radians(ownerRotation.Angle), glm::normalize(axis));
            }

            const glm::quat relativeRotation = glm::angleAxis(glm::radians(m_RelativeRotationAngle), m_RelativeRotationAxis);
            return actorRotation * relativeRotation;
        }

        glm::vec3 GetSocketWorldLocation() const
        {
            glm::vec3 origin = m_TargetOffset;
            if (GetOwner())
            {
                origin += GetOwner()->GetActorLocation();
            }

            const glm::quat worldRotation = GetWorldRotation();
            const glm::vec3 socketLocal = glm::vec3(0.0f, 0.0f, m_TargetArmLength) + m_SocketOffset;
            return origin + (worldRotation * socketLocal);
        }

    private:
        float m_TargetArmLength = 300.0f;
        glm::vec3 m_TargetOffset = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_SocketOffset = glm::vec3(0.0f, 0.0f, 0.0f);
        bool m_InheritActorRotation = true;
        glm::vec3 m_RelativeRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        float m_RelativeRotationAngle = 0.0f;
    };
}
