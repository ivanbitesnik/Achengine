#include "Achenginepch.h"
#include "ActorComponent.h"

#include "Achengine/Actor/Actor.h"
#include "Achengine/Core/Core.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Achengine
{
    glm::mat4 UActorComponent::GetComponentTransform() const
    {
        glm::vec3 scale = GetComponentScale();
        glm::vec3 axis = GetRelativeRotation().RotationAxis;
        if (glm::length(axis) < 0.0001f)
        {
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), GetRelativeLocation());
        transform = glm::rotate(transform, glm::radians(GetRelativeRotation().Angle), glm::normalize(axis));
        transform = glm::scale(transform, { scale.x, scale.y, scale.z });

        if (GetOwner())
        {
            return GetOwner()->GetActorTransform() * transform;
        }

        return transform;
    }

    glm::vec3 UActorComponent::GetWorldLocation() const
    {
        const glm::vec3 relativeLocation = GetRelativeLocation();

        if (GetOwner())
        {
            const glm::vec4 world = GetOwner()->GetActorTransform() * glm::vec4(relativeLocation, 1.0f);
            return glm::vec3(world);
        }

        return relativeLocation;
    }

    glm::quat UActorComponent::GetWorldRotation() const
    {
        glm::vec3 relativeAxis = GetRelativeRotation().RotationAxis;
        if (glm::length(relativeAxis) < 0.0001f)
        {
            relativeAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        const glm::quat relativeRotation = glm::angleAxis(glm::radians(GetRelativeRotation().Angle), glm::normalize(relativeAxis));

        if (GetOwner())
        {
            FRotation ownerRotation = GetOwner()->GetActorRotation();
            glm::vec3 axis = ownerRotation.RotationAxis;
            if (glm::length(axis) < 0.0001f)
            {
                axis = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            return glm::angleAxis(glm::radians(ownerRotation.Angle), glm::normalize(axis)) * relativeRotation;
        }

        return relativeRotation;
    }
}