#pragma once

#include "Achengine/Actor/Player.h"
#include "Achengine/Core/KeyCodes.h"
#include "Achengine/Core/MouseButtonCodes.h"
#include "Achengine/Events/Event.h"
#include "Achengine/Events/KeyEvent.h"
#include "Achengine/Events/MouseEvent.h"

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>
#include <unordered_set>

namespace Achengine
{
    class ACHENGINE_API APlayerController
    {
    public:
        virtual ~APlayerController() = default;

        virtual void Possess(APlayer* InPlayer);
        virtual void UnPossess();

        APlayer* GetPossessedPlayer() const { return m_PossessedPlayer; }
        bool IsPossessingPlayer() const { return m_PossessedPlayer != nullptr; }

        bool IsKeyDown(int keyCode) const { return m_KeysDown.count(keyCode) > 0; }
        bool IsMouseButtonDown(int button) const { return m_MouseButtonsDown.count(button) > 0; }
        glm::vec2 GetMousePosition() const { return m_MousePosition; }
        glm::vec2 GetMouseDelta() const { return m_MouseDelta; }
        float GetPitchDegrees() const { return m_PitchDegrees; }
        float GetYawDegrees() const { return m_YawDegrees; }

        virtual void Tick(float DeltaTime);

        virtual bool OnEvent(Event& InEvent);

    protected:
        virtual void OnPossess(APlayer* InPlayer)
        {
            (void)InPlayer;
            InPlayer->SetPlayerController(this);
        }

        virtual void OnUnPossess(APlayer* InPlayer)
        {
            (void)InPlayer;
            InPlayer->SetPlayerController(nullptr);
        }

        virtual bool OnKeyPressed(const KeyPressedEvent& InEvent);

        virtual bool OnKeyReleased(const KeyReleasedEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

        virtual bool OnMouseButtonPressed(const MouseButtonPressedEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

        virtual bool OnMouseButtonReleased(const MouseButtonReleasedEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

        virtual bool OnMouseMoved(const MouseMovedEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

        virtual bool OnMouseScrolled(const MouseScrolledEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

    private:
        bool HandleKeyPressed(KeyPressedEvent& InEvent)
        {
            m_KeysDown.insert(InEvent.GetKeyCode());
            return OnKeyPressed(InEvent);
        }

        bool HandleKeyReleased(KeyReleasedEvent& InEvent)
        {
            m_KeysDown.erase(InEvent.GetKeyCode());
            return OnKeyReleased(InEvent);
        }

        bool HandleMouseButtonPressed(MouseButtonPressedEvent& InEvent)
        {
            m_MouseButtonsDown.insert(InEvent.GetMouseButton());
            return OnMouseButtonPressed(InEvent);
        }

        bool HandleMouseButtonReleased(MouseButtonReleasedEvent& InEvent)
        {
            m_MouseButtonsDown.erase(InEvent.GetMouseButton());
            return OnMouseButtonReleased(InEvent);
        }

        bool HandleMouseMoved(MouseMovedEvent& InEvent)
        {
            const glm::vec2 newPosition(InEvent.GetX(), InEvent.GetY());
            if (!m_HasMousePosition)
            {
                m_MousePosition = newPosition;
                m_HasMousePosition = true;
                return OnMouseMoved(InEvent);
            }

            m_MouseDelta += (newPosition - m_MousePosition);
            m_MousePosition = newPosition;
            return OnMouseMoved(InEvent);
        }

        bool HandleMouseScrolled(MouseScrolledEvent& InEvent)
        {
            return OnMouseScrolled(InEvent);
        }

    private:
        APlayer* m_PossessedPlayer = nullptr;
        std::unordered_set<int> m_KeysDown;
        std::unordered_set<int> m_MouseButtonsDown;
        bool m_HasMousePosition = false;
        glm::vec2 m_MousePosition = glm::vec2(0.0f, 0.0f);
        glm::vec2 m_MouseDelta = glm::vec2(0.0f, 0.0f);
        float m_MoveSpeed = 10.0f;
        float m_MouseLookSensitivity = 0.10f;
        float m_YawDegrees = 0.0f;
        float m_PitchDegrees = -20.0f;
    };
}
