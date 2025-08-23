#pragma once

#include "Achengine/Core/Core.h"
#include "Achengine/Actor/ActorComponent.h"

namespace Achengine
{
    class UStaticMesh;
    struct RendererStorage;

    struct FActorRotation
    {
        public:
            FActorRotation() {}
            FActorRotation(const glm::vec3& RotationAxis, float Angle) : RotationAxis(RotationAxis), Angle(Angle) {}
            glm::vec3 RotationAxis = {1.0f, 1.0f, 1.0f};
            float Angle = 0.0f;
    };

    class ACHENGINE_API AActor
    {
        public:
            AActor();

            void AddActorComponent(UActorComponent* NewComponent);

            template<class T>
            T* GetComponentByClass() const
            {
                for (UActorComponent* Comp : ActorComponents)
                {
                    if (T* CastedComp = (T*)Comp)
                    {
                        return CastedComp;
                    }
                }

                return nullptr;
            };

            void SetStaticMesh(UStaticMesh* NewStaticMesh);
            UStaticMesh* GetStaticMesh() const { return GetComponentByClass<UStaticMesh>(); }
            
            void SetActorLocation(glm::vec3 NewLocation) { ActorLocation = NewLocation; }
            glm::vec3 GetActorLocation() const { return ActorLocation; }

            void SetActorRotation(glm::vec3 RotationAxis, float NewAngle) { ActorRotation = FActorRotation(RotationAxis, NewAngle); }
            FActorRotation GetActorRotation() const { return ActorRotation; }

            void SetActorScale(glm::vec3 NewScale) { ActorScale = NewScale; }
            glm::vec3 GetActorScale() const { return ActorScale; }
            
            void Draw(RendererStorage* RenderData);
        private:
            std::vector<UActorComponent*> ActorComponents;
            glm::vec3 ActorLocation = {0.0f, 0.0f, 0.0f};
            FActorRotation ActorRotation;
            glm::vec3 ActorScale = {1.0f, 1.0f, 1.0f};

    };
}