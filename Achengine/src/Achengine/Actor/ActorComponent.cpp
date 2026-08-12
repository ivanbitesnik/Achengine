#include "Achenginepch.h"
#include "ActorComponent.h"

#include "Achengine/Actor/Actor.h"
#include "Achengine/Core/Core.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_map>

namespace Achengine
{
    namespace
    {
        struct FRegisteredComponentRegistry
        {
            std::vector<UActorComponent::FRegisteredComponentClass> Classes;
            std::unordered_map<std::string, size_t> TypeTagToIndex;
            std::unordered_map<std::type_index, size_t> TypeToIndex;
        };

        FRegisteredComponentRegistry& GetComponentRegistry()
        {
            static FRegisteredComponentRegistry registry;
            return registry;
        }
    }

    bool UActorComponent::RegisterComponentClass(
        const std::type_index& typeIndex,
        const char* className,
        const char* typeTag,
        bool exposeInTemplatePicker,
        std::function<UActorComponent*()> factory)
    {
        if (!className || className[0] == '\0' || !typeTag || typeTag[0] == '\0' || !factory)
        {
            return false;
        }

        FRegisteredComponentRegistry& registry = GetComponentRegistry();
        const std::string typeTagKey = typeTag;
        auto existingTag = registry.TypeTagToIndex.find(typeTagKey);
        if (existingTag != registry.TypeTagToIndex.end())
        {
            UActorComponent::FRegisteredComponentClass& entry = registry.Classes[existingTag->second];
            entry.ClassName = className;
            entry.TypeTag = typeTag;
            entry.ExposeInTemplatePicker = exposeInTemplatePicker;
            entry.Factory = factory;
            registry.TypeToIndex[typeIndex] = existingTag->second;
            return true;
        }

        auto existingType = registry.TypeToIndex.find(typeIndex);
        if (existingType != registry.TypeToIndex.end())
        {
            UActorComponent::FRegisteredComponentClass& entry = registry.Classes[existingType->second];
            entry.ClassName = className;
            entry.TypeTag = typeTag;
            entry.ExposeInTemplatePicker = exposeInTemplatePicker;
            entry.Factory = factory;
            registry.TypeTagToIndex[typeTagKey] = existingType->second;
            return true;
        }

        UActorComponent::FRegisteredComponentClass entry;
        entry.ClassName = className;
        entry.TypeTag = typeTag;
        entry.ExposeInTemplatePicker = exposeInTemplatePicker;
        entry.Factory = factory;

        const size_t index = registry.Classes.size();
        registry.Classes.push_back(entry);
        registry.TypeTagToIndex[typeTagKey] = index;
        registry.TypeToIndex[typeIndex] = index;
        return true;
    }

    const std::vector<UActorComponent::FRegisteredComponentClass>& UActorComponent::GetRegisteredComponentClasses()
    {
        return GetComponentRegistry().Classes;
    }

    const UActorComponent::FRegisteredComponentClass* UActorComponent::FindRegisteredComponentClassByTypeTag(const std::string& typeTag)
    {
        const FRegisteredComponentRegistry& registry = GetComponentRegistry();
        auto it = registry.TypeTagToIndex.find(typeTag);
        if (it == registry.TypeTagToIndex.end() || it->second >= registry.Classes.size())
        {
            return nullptr;
        }

        return &registry.Classes[it->second];
    }

    const UActorComponent::FRegisteredComponentClass* UActorComponent::FindRegisteredComponentClassByInstance(const UActorComponent* component)
    {
        if (!component)
        {
            return nullptr;
        }

        const std::type_index typeIndex(typeid(*component));
        const FRegisteredComponentRegistry& registry = GetComponentRegistry();
        auto it = registry.TypeToIndex.find(typeIndex);
        if (it == registry.TypeToIndex.end() || it->second >= registry.Classes.size())
        {
            return nullptr;
        }

        return &registry.Classes[it->second];
    }

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