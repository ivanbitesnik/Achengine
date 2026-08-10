#pragma once

#include "Achengine/Core/Core.h"
#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Actor/Mesh.h"

namespace Achengine
{
    class ACHENGINE_API AActor
    {
        public:
            AActor();
            virtual ~AActor();

            virtual void Tick(float DeltaTime);
            
            void AddActorComponent(UActorComponent* NewComponent);
            bool RemoveActorComponent(UActorComponent* ComponentToRemove);

            template<class T>
            T* GetComponentByClass() const
            {
                for (UActorComponent* Comp : ActorComponents)
                {
                    if (T* CastedComp = dynamic_cast<T*>(Comp))
                    {
                        return CastedComp;
                    }
                }

                return nullptr;
            };

            const std::vector<UActorComponent*>& GetActorComponents() const { return ActorComponents; }

            void SetActorName(const std::string& NewName) { ActorName = NewName; }
            const std::string& GetActorName() const { return ActorName; }

            void SetTemplateType(const std::string& NewTemplateType) { TemplateType = NewTemplateType; }
            const std::string& GetTemplateType() const { return TemplateType; }
            
            void SetActorLocation(glm::vec3 NewLocation) { ActorLocation = NewLocation; }
            glm::vec3 GetActorLocation() const { return ActorLocation; }

            void SetActorRotation(glm::vec3 RotationAxis, float NewAngle) { ActorRotation = FRotation(RotationAxis, NewAngle); }
            FRotation GetActorRotation() const { return ActorRotation; }

            void SetActorScale(glm::vec3 NewScale) { ActorScale = NewScale; }
            glm::vec3 GetActorScale() const { return ActorScale; }

            virtual FBounds GetBounds() const;

            glm::mat4 GetActorTransform() const;
            
            void Draw();
        private:
            std::vector<UActorComponent*> ActorComponents;
            std::string ActorName;
            std::string TemplateType;
            glm::vec3 ActorLocation = {0.0f, 0.0f, 0.0f};
            FRotation ActorRotation;
            glm::vec3 ActorScale = {1.0f, 1.0f, 1.0f};

    };
}