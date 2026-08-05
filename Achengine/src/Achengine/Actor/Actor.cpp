#include "Achenginepch.h"
#include "Actor.h"

#include "Achengine/Actor/Mesh.h"
#include "Achengine/Actor/WorldActorCache.h"
#include "Achengine/Actor/StaticMesh.h"

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

    void AActor::AddActorComponent(UActorComponent* NewComponent)
    {
        NewComponent->SetOwner(this);
        ActorComponents.push_back(NewComponent);
    }

    void AActor::SetMesh(UMesh* NewMesh)
    {
        UMesh* CurrentMesh = GetComponentByClass<UMesh>();
        if (!CurrentMesh)
        {
            AddActorComponent(NewMesh);
        }
        else
        {
            CurrentMesh = NewMesh;
        }
    }

    void AActor::Draw()
    {
        if (UMesh* Mesh = GetMesh())
        {
            Mesh->DrawMesh();
        }
    }

    FActorBounds AActor::GetBounds() const
    {
        FActorBounds actorBounds;

        const UMesh* mesh = GetMesh();
        if (!mesh)
        {
            return actorBounds;
        }

        const FMeshBounds meshBounds = mesh->GetBounds();
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
        const glm::vec3 scaledLocalCenter = meshBounds.LocalCenter * scale;
        const glm::vec3 worldCenterOffset = rotation * scaledLocalCenter;

        const glm::vec3 scaledExtents = meshBounds.LocalExtents * scale;
        const glm::mat3 rotationMat = glm::mat3_cast(rotation);
        const glm::mat3 absRotationMat = glm::mat3(
            glm::abs(rotationMat[0]),
            glm::abs(rotationMat[1]),
            glm::abs(rotationMat[2])
        );

        actorBounds.Center = GetActorLocation() + worldCenterOffset;
        actorBounds.Extents = absRotationMat * scaledExtents;
        actorBounds.SphereRadius = meshBounds.LocalSphereRadius * glm::max(scale.x, glm::max(scale.y, scale.z));
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