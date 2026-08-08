#include "Achenginepch.h"
#include "Player.h"

#include "Achengine/Actor/CameraComponent.h"

namespace Achengine
{
    APlayer::APlayer()
    {
        SetActorName("Player");

        m_CameraComponent = new UCameraComponent();
        m_CameraComponent->SetRelativeLocation(glm::vec3(-15.0f, 0.0f, 0.0f));
        AddActorComponent(m_CameraComponent);

        m_PlayerMesh = new UMesh("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/models/Sphere.fbx");
        m_PlayerMesh->SetComponentScale(glm::vec3(6.0f, 18.0f, 6.0f));
        AddActorComponent(m_PlayerMesh);
    }
}