#include "Achenginepch.h"
#include "WorldActorCache.h"

namespace Achengine
{
    WorldActorCache* WorldActorCache::s_Instance = new WorldActorCache();

    WorldActorCache::~WorldActorCache() 
    { 
        for (AActor* actor : GetActorCache())
        {
            delete actor;
        }

        delete s_Instance; 
    }

}