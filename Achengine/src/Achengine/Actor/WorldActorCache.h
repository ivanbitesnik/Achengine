#pragma once

#include "Actor.h"

namespace Achengine
{
    class UMeshDrawable;

    class ACHENGINE_API WorldActorCache
    {
        friend class AActor;
        friend class UMeshDrawable;

        public:
            WorldActorCache() {}
            virtual ~WorldActorCache();

            inline static WorldActorCache* Get() { return s_Instance; }
            const std::unordered_set<AActor*>& GetActorCache() const { return ActorCache; }
            UMeshDrawable* GetMeshDrawable(const std::string& DrawableID) const;
            
            template<typename T>
            static AActor* SpawnActor()
            {
                T* NewActor = new T();
                return (AActor*)NewActor;
            }

            private:
            void AddActorToCache(AActor* NewActor) { ActorCache.insert(NewActor); }
            void AddDrawableToCache(const std::string& DrawableName, UMeshDrawable* Drawable) { MeshDrawableCache.insert({DrawableName, Drawable}); }

            std::unordered_set<AActor*> ActorCache;
            std::map<std::string, UMeshDrawable*> MeshDrawableCache;
            
            static WorldActorCache* s_Instance;
    };
}