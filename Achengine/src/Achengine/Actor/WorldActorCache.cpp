#include "Achenginepch.h"
#include "WorldActorCache.h"

namespace Achengine
{
    WorldActorCache* WorldActorCache::s_Instance = new WorldActorCache();

    void WorldActorCache::ClearActorCache()
    {
        std::vector<AActor*> actorsToDelete;
        actorsToDelete.reserve(ActorCache.size());
        for (AActor* actor : ActorCache)
        {
            actorsToDelete.push_back(actor);
        }

        ActorCache.clear();
        for (AActor* actor : actorsToDelete)
        {
            delete actor;
        }
    }

    void WorldActorCache::DestroyActor(AActor* ActorToDestroy)
    {
        if (!s_Instance || !ActorToDestroy)
        {
            return;
        }

        auto it = s_Instance->ActorCache.find(ActorToDestroy);
        if (it != s_Instance->ActorCache.end())
        {
            s_Instance->ActorCache.erase(it);
        }

        delete ActorToDestroy;
    }

    WorldActorCache::~WorldActorCache() 
    { 
        ClearActorCache();

        delete s_Instance; 
    }
}