#include "Achenginepch.h"
#include "Actor.h"

#include "Achengine/Actor/WorldActorCache.h"
#include "Achengine/Actor/StaticMesh.h"

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
}