#pragma once

#include "Achengine/Core/Utilities.h"
#include "Achengine/Collision/CollisionTypes.h"

#include <functional>
#include <string>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace Achengine
{
    class AActor;

    class ACHENGINE_API UActorComponent
    {
        friend class AActor;
        public:
			struct FRegisteredComponentClass
			{
				const char* ClassName = "UActorComponent";
				const char* TypeTag = "component";
				bool ExposeInTemplatePicker = true;
				std::function<UActorComponent*()> Factory;
			};

            UActorComponent()
            {
                GetLiveComponentRegistry().insert(this);
            }

            virtual ~UActorComponent()
            {
                GetLiveComponentRegistry().erase(this);
            }

            virtual void Tick(float DeltaTime) {}

            static bool RegisterComponentClass(
                const std::type_index& typeIndex,
                const char* className,
                const char* typeTag,
                bool exposeInTemplatePicker,
                std::function<UActorComponent*()> factory);

            template<typename T>
            static bool RegisterComponentClass(
                const char* className,
                const char* typeTag,
                bool exposeInTemplatePicker = true)
            {
                return RegisterComponentClass(
                    std::type_index(typeid(T)),
                    className,
                    typeTag,
                    exposeInTemplatePicker,
                    []() -> UActorComponent* { return new T(); });
            }

            static const std::vector<FRegisteredComponentClass>& GetRegisteredComponentClasses();
            static const FRegisteredComponentClass* FindRegisteredComponentClassByTypeTag(const std::string& typeTag);
            static const FRegisteredComponentClass* FindRegisteredComponentClassByInstance(const UActorComponent* component);

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

            void SetCollisionChannel(ECollisionChannel NewChannel) { CollisionChannel = NewChannel; }
            ECollisionChannel GetCollisionChannel() const { return CollisionChannel; }

            void SetCollisionResponse(ECollisionResponse NewResponse) { CollisionResponse = NewResponse; }
            ECollisionResponse GetCollisionResponse() const { return CollisionResponse; }

            void SetCollisionObjectType(ECollisionObjectType NewObjectType) { CollisionObjectType = NewObjectType; }
            ECollisionObjectType GetCollisionObjectType() const { return CollisionObjectType; }
            
            void SetOwner(AActor* NewOwner) { Owner = NewOwner; }
            AActor* GetOwner() const { return Owner; }

        protected:
            AActor* Owner = nullptr;
    
            glm::vec3 ComponentLocation = {0.0f, 0.0f, 0.0f};
            FRotation ComponentRotation;
            glm::vec3 ComponentScale = {1.0f, 1.0f, 1.0f};

            ECollisionChannel CollisionChannel = ECollisionChannel::BlockAllDynamic;
            ECollisionResponse CollisionResponse = ECollisionResponse::ECR_Block;
            ECollisionObjectType CollisionObjectType = ECollisionObjectType::WorldDynamic;
    
            FBounds m_Bounds;
        private:
            static std::unordered_set<const UActorComponent*>& GetLiveComponentRegistry()
            {
                static std::unordered_set<const UActorComponent*> s_LiveComponents;
                return s_LiveComponents;
            }
    };
}