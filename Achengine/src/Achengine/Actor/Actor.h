#pragma once

#include "Achengine/Core/Core.h"
#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Actor/Mesh.h"

namespace Achengine
{
    struct FActorBounds
    {
        glm::vec3 Center = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Extents = glm::vec3(0.0f, 0.0f, 0.0f);
        float SphereRadius = 0.0f;
        bool IsValid = false;
    };
    
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
            virtual ~AActor();

            virtual void Tick(float DeltaTime);
            
            void AddActorComponent(UActorComponent* NewComponent);

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

            void SetMesh(UMesh* NewMesh);
            UMesh* GetMesh() const { return GetComponentByClass<UMesh>(); }

            void SetActorName(const std::string& NewName) { ActorName = NewName; }
            const std::string& GetActorName() const { return ActorName; }
            
            void SetActorLocation(glm::vec3 NewLocation) { ActorLocation = NewLocation; }
            glm::vec3 GetActorLocation() const { return ActorLocation; }

            void SetActorRotation(glm::vec3 RotationAxis, float NewAngle) { ActorRotation = FActorRotation(RotationAxis, NewAngle); }
            FActorRotation GetActorRotation() const { return ActorRotation; }

            void SetActorScale(glm::vec3 NewScale) { ActorScale = NewScale; }
            glm::vec3 GetActorScale() const { return ActorScale; }

            virtual FActorBounds GetBounds() const;

            glm::mat4 GetActorTransform() const;
            
            void Draw();
        private:
            std::vector<UActorComponent*> ActorComponents;
            std::string ActorName;
            glm::vec3 ActorLocation = {0.0f, 0.0f, 0.0f};
            FActorRotation ActorRotation;
            glm::vec3 ActorScale = {1.0f, 1.0f, 1.0f};

    };
}