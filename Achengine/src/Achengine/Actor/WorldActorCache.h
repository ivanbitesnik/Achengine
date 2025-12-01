#pragma once

#include "Actor.h"

namespace Achengine
{
    class ACHENGINE_API WorldActorCache
    {
        public:
            WorldActorCache() {}
            virtual ~WorldActorCache();

            void AddActorToCache(AActor* NewActor) { ActorCache.push_back(NewActor); }
            inline static WorldActorCache* Get() { return s_Instance; }
            const std::vector<AActor*>& GetActorCache() const { return ActorCache; }
        private:
            std::vector<AActor*> ActorCache;

            static WorldActorCache* s_Instance;
    };
}