#include "Achenginepch.h"
#include "Actor.h"

#include "Achengine/Actor/WorldActorCache.h"
#include "Achengine/Actor/StaticMesh.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Achengine
{
    AActor::AActor()
    {
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

    void AActor::SetStaticMesh(UStaticMesh* NewStaticMesh)
    {
        UStaticMesh* CurrentStaticMesh = GetComponentByClass<UStaticMesh>();
        if (!CurrentStaticMesh)
        {
            AddActorComponent(NewStaticMesh);
        }
        else
        {
            CurrentStaticMesh = NewStaticMesh;
        }
    }

    void AActor::Draw(RendererStorage* RenderData)
    {
        if (UStaticMesh* StaticMesh = GetStaticMesh())
        {
            StaticMesh->DrawMesh(RenderData);
        }
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