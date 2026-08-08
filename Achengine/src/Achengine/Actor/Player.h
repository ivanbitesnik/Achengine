#pragma once

#include "Achengine/Actor/Actor.h"

namespace Achengine
{
    class APlayerController;
    class UCameraComponent;

    class ACHENGINE_API APlayer : public AActor
    {
    public:
        APlayer();

        UCameraComponent* GetCameraComponent() const { return m_CameraComponent; }

        void SetPlayerController(APlayerController* NewController) { m_PlayerController = NewController; }
        APlayerController* GetPlayerController() const { return m_PlayerController; }
    private:
        UCameraComponent* m_CameraComponent = nullptr;
        UMesh* m_PlayerMesh = nullptr;
        APlayerController* m_PlayerController = nullptr;
    };
}
