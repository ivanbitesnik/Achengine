#pragma once

#include "Actor.h"

namespace Achengine
{
    class ACHENGINE_API WorldActorCache
    {
        friend class AActor;

        public:
            WorldActorCache() {}
            virtual ~WorldActorCache();

            inline static WorldActorCache* Get() { return s_Instance; }
            const std::unordered_set<AActor*>& GetActorCache() const { return ActorCache; }
            void ClearActorCache();
            static void DestroyActor(AActor* ActorToDestroy);
            
            template<typename T>
            static AActor* SpawnActor()
            {
                T* NewActor = new T();
                return (AActor*)NewActor;
            }

            private:
            void AddActorToCache(AActor* NewActor) { ActorCache.insert(NewActor); }

            std::unordered_set<AActor*> ActorCache;
            
            static WorldActorCache* s_Instance;
    };
}