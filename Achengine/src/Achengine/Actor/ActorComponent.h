#pragma once

#include "Achengine/Core/Core.h"

#include <unordered_set>


namespace Achengine
{
    class AActor;

    class ACHENGINE_API UActorComponent
    {
        friend class AActor;
        public:
            UActorComponent()
            {
                GetLiveComponentRegistry().insert(this);
            }

            virtual ~UActorComponent()
            {
                GetLiveComponentRegistry().erase(this);
            }

            virtual void Tick(float DeltaTime) {}

            static bool IsPointerAlive(const UActorComponent* Component)
            {
                return Component != nullptr && GetLiveComponentRegistry().count(Component) > 0;
            }
            
            AActor* GetOwner() const { return Owner; }
        protected:
            void SetOwner(AActor* NewOwner) { Owner = NewOwner; }
        private:
            static std::unordered_set<const UActorComponent*>& GetLiveComponentRegistry()
            {
                static std::unordered_set<const UActorComponent*> s_LiveComponents;
                return s_LiveComponents;
            }

            AActor* Owner = nullptr;
    };
}