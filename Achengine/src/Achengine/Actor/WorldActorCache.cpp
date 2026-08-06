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

    WorldActorCache::~WorldActorCache() 
    { 
        ClearActorCache();

        delete s_Instance; 
    }
}