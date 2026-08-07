#pragma once

#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/CameraComponent.h"
#include "Achengine/Actor/SpringArmComponent.h"

namespace Achengine
{
    class ACHENGINE_API APlayer : public AActor
    {
    public:
        APlayer()
        {
            SetActorName("Player");

            m_SpringArm = new USpringArmComponent();
            m_SpringArm->SetTargetArmLength(350.0f);
            m_SpringArm->SetTargetOffset(glm::vec3(0.0f, 90.0f, 0.0f));
            AddActorComponent(m_SpringArm);

            m_CameraComponent = new UCameraComponent();
            m_CameraComponent->AttachToSpringArm(m_SpringArm);
            AddActorComponent(m_CameraComponent);
        }

        USpringArmComponent* GetSpringArm() const { return m_SpringArm; }
        UCameraComponent* GetCameraComponent() const { return m_CameraComponent; }

    private:
        USpringArmComponent* m_SpringArm = nullptr;
        UCameraComponent* m_CameraComponent = nullptr;
    };
}
