#pragma once

#include "Achengine/Actor/Player.h"
#include "Achengine/Events/Event.h"
#include "Achengine/Events/KeyEvent.h"
#include "Achengine/Events/MouseEvent.h"

#include <unordered_set>

namespace Achengine
{
    class ACHENGINE_API APlayerController
    {
    public:
        virtual ~APlayerController() = default;

        virtual void Possess(APlayer* InPlayer)
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
            m_MouseDelta = glm::vec2(0.0f, 0.0f);

            if (m_PossessedPlayer)
            {
                OnPossess(m_PossessedPlayer);
            }
        }

        virtual void UnPossess()
        {
            if (!m_PossessedPlayer)
            {
                return;
            }

            APlayer* PreviousPlayer = m_PossessedPlayer;
            m_PossessedPlayer = nullptr;
            m_KeysDown.clear();
            m_MouseButtonsDown.clear();
            m_MouseDelta = glm::vec2(0.0f, 0.0f);
            OnUnPossess(PreviousPlayer);
        }

        APlayer* GetPossessedPlayer() const { return m_PossessedPlayer; }
        bool IsPossessingPlayer() const { return m_PossessedPlayer != nullptr; }

        bool IsKeyDown(int keyCode) const { return m_KeysDown.count(keyCode) > 0; }
        bool IsMouseButtonDown(int button) const { return m_MouseButtonsDown.count(button) > 0; }
        glm::vec2 GetMousePosition() const { return m_MousePosition; }
        glm::vec2 GetMouseDelta() const { return m_MouseDelta; }

        virtual void Tick(float DeltaTime)
        {
            (void)DeltaTime;
            m_MouseDelta = glm::vec2(0.0f, 0.0f);
        }

        virtual bool OnEvent(Event& InEvent)
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

    protected:
        virtual void OnPossess(APlayer* InPlayer)
        {
            (void)InPlayer;
        }

        virtual void OnUnPossess(APlayer* InPlayer)
        {
            (void)InPlayer;
        }

        virtual bool OnKeyPressed(const KeyPressedEvent& InEvent)
        {
            (void)InEvent;
            return false;
        }

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
        glm::vec2 m_MousePosition = glm::vec2(0.0f, 0.0f);
        glm::vec2 m_MouseDelta = glm::vec2(0.0f, 0.0f);
    };
}
