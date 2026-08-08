#pragma once

#include "Achengine/Core/Utilities.h"

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

            void SetRelativeLocation(glm::vec3 NewLocation) { ComponentLocation = NewLocation; }
            glm::vec3 GetRelativeLocation() const { return ComponentLocation; }

            void SetRelativeRotation(glm::vec3 RotationAxis, float NewAngle) { ComponentRotation = FRotation(RotationAxis, NewAngle); }
            FRotation GetRelativeRotation() const { return ComponentRotation; }

            void SetComponentScale(glm::vec3 NewScale) { ComponentScale = NewScale; }
            glm::vec3 GetComponentScale() const { return ComponentScale; }

            virtual glm::mat4 GetComponentTransform() const;
            virtual glm::vec3 GetWorldLocation() const;
            virtual glm::quat GetWorldRotation() const;
            
            virtual FBounds GetBounds() const { return m_Bounds; }
            
            void SetOwner(AActor* NewOwner) { Owner = NewOwner; }
            AActor* GetOwner() const { return Owner; }

        protected:
            AActor* Owner = nullptr;
    
            glm::vec3 ComponentLocation = {0.0f, 0.0f, 0.0f};
            FRotation ComponentRotation;
            glm::vec3 ComponentScale = {1.0f, 1.0f, 1.0f};
    
            FBounds m_Bounds;
        private:
            static std::unordered_set<const UActorComponent*>& GetLiveComponentRegistry()
            {
                static std::unordered_set<const UActorComponent*> s_LiveComponents;
                return s_LiveComponents;
            }
    };
}