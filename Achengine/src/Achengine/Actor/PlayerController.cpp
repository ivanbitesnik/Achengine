#include "Achenginepch.h"
#include "PlayerController.h"

#include "Achengine/Actor/MovementComponent.h"

namespace Achengine
{
    bool APlayerController::OnKeyPressed(const KeyPressedEvent& InEvent)
    {
        if (!m_PossessedPlayer)
        {
            return false;
        }

        if (InEvent.GetKeyCode() == ACHENGINE_KEY_SPACE && InEvent.GetRepeatCount() == 0)
        {
            if (UMovementComponent* movementComponent = m_PossessedPlayer->GetMovementComponent())
            {
                movementComponent->RequestJump();
                return true;
            }
        }

        return false;
    }

    void APlayerController::Possess(APlayer* InPlayer)
    {
        if (m_PossessedPlayer == InPlayer)
        {
            return;
        }

        if (m_PossessedPlayer)
        {
            OnUnPossess(m_PossessedPlayer);
        }

        m_PossessedPlayer = InPlayer;
        m_KeysDown.clear();
        m_MouseButtonsDown.clear();
        m_HasMousePosition = false;
        m_MouseDelta = glm::vec2(0.0f, 0.0f);

        if (m_PossessedPlayer)
        {
            const FRotation rotation = m_PossessedPlayer->GetActorRotation();
            m_YawDegrees = rotation.Angle;
        }

        if (m_PossessedPlayer)
        {
            OnPossess(m_PossessedPlayer);
        }
    }

    void APlayerController::UnPossess()
    {
        if (!m_PossessedPlayer)
        {
            return;
        }

        APlayer* PreviousPlayer = m_PossessedPlayer;
        m_PossessedPlayer = nullptr;
        m_KeysDown.clear();
        m_MouseButtonsDown.clear();
        m_HasMousePosition = false;
        m_MouseDelta = glm::vec2(0.0f, 0.0f);
        OnUnPossess(PreviousPlayer);
    }

    void APlayerController::Tick(float DeltaTime)
    {
        if (m_PossessedPlayer)
        {
            m_YawDegrees -= GetMouseDelta().x * m_MouseLookSensitivity;
            m_PitchDegrees -= GetMouseDelta().y * m_MouseLookSensitivity;
            m_PitchDegrees = glm::clamp(m_PitchDegrees, -80.0f, 80.0f);

            m_PossessedPlayer->SetActorRotation(glm::vec3(0.0f, 1.0f, 0.0f), m_YawDegrees);

            glm::vec3 moveDirection(0.0f);
            glm::vec3 actorAxis = m_PossessedPlayer->GetActorRotation().RotationAxis;
            if (glm::length(actorAxis) < 0.0001f)
            {
                actorAxis = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            const glm::quat actorRotation = glm::angleAxis(glm::radians(m_PossessedPlayer->GetActorRotation().Angle), glm::normalize(actorAxis));
            const glm::vec3 forward = glm::normalize(actorRotation * glm::vec3(1.0f, 0.0f, 0.0f));
            const glm::vec3 right = glm::normalize(actorRotation * glm::vec3(0.0f, 0.0f, 1.0f));

            if (IsKeyDown(ACHENGINE_KEY_W)) { moveDirection += forward; }
            if (IsKeyDown(ACHENGINE_KEY_S)) { moveDirection -= forward; }
            if (IsKeyDown(ACHENGINE_KEY_D)) { moveDirection += right; }
            if (IsKeyDown(ACHENGINE_KEY_A)) { moveDirection -= right; }

            if (UMovementComponent* movementComponent = m_PossessedPlayer->GetMovementComponent())
            {
                if (glm::length(moveDirection) > 0.0001f)
                {
                    moveDirection = glm::normalize(moveDirection);
                }

                movementComponent->AddInputVector(moveDirection);
            }
            else if (glm::length(moveDirection) > 0.0001f)
            {
                moveDirection = glm::normalize(moveDirection);
                m_PossessedPlayer->SetActorLocation(m_PossessedPlayer->GetActorLocation() + moveDirection * m_MoveSpeed * DeltaTime);
            }
        }

        m_MouseDelta = glm::vec2(0.0f, 0.0f);
    }

    bool APlayerController::OnEvent(Event& InEvent)
    {
        if (!m_PossessedPlayer)
        {
            return false;
        }

        EventDispatcher dispatcher(InEvent);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return this->HandleKeyPressed(e); });
        dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& e) { return this->HandleKeyReleased(e); });
        dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) { return this->HandleMouseButtonPressed(e); });
        dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) { return this->HandleMouseButtonReleased(e); });
        dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) { return this->HandleMouseMoved(e); });
        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return this->HandleMouseScrolled(e); });
        return InEvent.IsHandled();
    }
}