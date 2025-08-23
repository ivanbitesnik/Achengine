#pragma once

#include "Achengine/Core/Core.h"


namespace Achengine
{
    class AActor;

    class ACHENGINE_API UActorComponent
    {
        friend class AActor;
        public:
            UActorComponent() {}
            
            AActor* GetOwner() const { return Owner; }
        protected:
            void SetOwner(AActor* NewOwner) { Owner = NewOwner; }
        private:
            AActor* Owner = nullptr;
    };
}