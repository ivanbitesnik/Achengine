#include "Achenginepch.h"
#include "Actor.h"

#include "Achengine/Actor/Mesh.h"
#include "Achengine/Actor/WorldActorCache.h"
#include "Achengine/Core/Utilities.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Achengine
{
    static uint64_t s_ActorNameCounter = 1;

    AActor::AActor()
    {
        ActorName = "Actor_" + std::to_string(s_ActorNameCounter++);

        if (WorldActorCache* Cache = WorldActorCache::Get())
        {
            Cache->AddActorToCache(this);
        }
    }

    AActor::~AActor()
    {
        for (UActorComponent* component : ActorComponents)
        {
            delete component;
        }

        ActorComponents.clear();
    }

    void AActor::Tick(float DeltaTime)
    {
        for (UActorComponent* Comp : ActorComponents)
        {
            if (!Comp)
            {
                continue;
            }

            if (!UActorComponent::IsPointerAlive(Comp))
            {
                ACHENGINE_CORE_WARN("Skipping stale component pointer on actor '{0}'", GetActorName().c_str());
                continue;
            }

            Comp->Tick(DeltaTime);
        }
    }

    void AActor::AddActorComponent(UActorComponent* NewComponent)
    {
        if (!NewComponent)
        {
            return;
        }

        NewComponent->SetOwner(this);
        ActorComponents.push_back(NewComponent);
    }

    bool AActor::RemoveActorComponent(UActorComponent* ComponentToRemove)
    {
        if (!ComponentToRemove)
        {
            return false;
        }

        const auto it = std::find(ActorComponents.begin(), ActorComponents.end(), ComponentToRemove);
        if (it == ActorComponents.end())
        {
            return false;
        }

        (*it)->SetOwner(nullptr);
        delete *it;
        ActorComponents.erase(it);
        return true;
    }

    void AActor::Draw()
    {
        if (UMesh* Mesh = GetComponentByClass<UMesh>())
        {
            Mesh->DrawMesh();
        }
    }

    FBounds AActor::GetBounds() const
    {
        FBounds actorBounds;

        const UMesh* mesh = GetComponentByClass<UMesh>();
        if (!mesh)
        {
            return actorBounds;
        }

        const FBounds meshBounds = mesh->GetBounds();
        if (!meshBounds.IsValid)
        {
            return actorBounds;
        }

        const glm::vec3 scale = glm::abs(GetActorScale());
        glm::vec3 rotationAxis = GetActorRotation().RotationAxis;
        if (glm::length(rotationAxis) < 0.0001f)
        {
            rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        const glm::quat rotation = glm::angleAxis(glm::radians(GetActorRotation().Angle), glm::normalize(rotationAxis));
        const glm::vec3 scaledLocalCenter = meshBounds.Center * scale;
        const glm::vec3 worldCenterOffset = rotation * scaledLocalCenter;

        const glm::vec3 scaledExtents = meshBounds.Extents * scale;
        const glm::mat3 rotationMat = glm::mat3_cast(rotation);
        const glm::mat3 absRotationMat = glm::mat3(
            glm::abs(rotationMat[0]),
            glm::abs(rotationMat[1]),
            glm::abs(rotationMat[2])
        );

        actorBounds.Center = GetActorLocation() + worldCenterOffset;
        actorBounds.Extents = absRotationMat * scaledExtents;
        actorBounds.SphereRadius = meshBounds.SphereRadius * glm::max(scale.x, glm::max(scale.y, scale.z));
        actorBounds.IsValid = true;
        return actorBounds;
    }

    glm::mat4 AActor::GetActorTransform() const
    {
        glm::vec3 scale = GetActorScale();
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), GetActorLocation());
		transform = glm::rotate(transform, glm::radians(GetActorRotation().Angle), GetActorRotation().RotationAxis);
		transform = glm::scale(transform, { scale.x, scale.y, scale.z });

        return transform;
    }
}