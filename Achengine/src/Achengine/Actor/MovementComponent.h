#pragma once

#include "Achengine/Actor/ActorComponent.h"

namespace Achengine
{
    class UMovementComponent : public UActorComponent
    {
        public:
            UMovementComponent();

            virtual void Tick(float DeltaTime) override;

            void AddInputVector(const glm::vec3& input);
            void ClearInputVector();

            void SetAcceleration(float value) { m_Acceleration = value; }
            float GetAcceleration() const { return m_Acceleration; }

            void SetMaxSpeed(float value) { m_MaxSpeed = value; }
            float GetMaxSpeed() const { return m_MaxSpeed; }

			void RequestJump() { m_JumpRequested = true; }
			void SetJumpImpulse(float value) { m_JumpImpulse = value; }
			float GetJumpImpulse() const { return m_JumpImpulse; }

            void SetGroundTraceDistance(float value) { m_GroundTraceDistance = value; }
            float GetGroundTraceDistance() const { return m_GroundTraceDistance; }

            bool IsGrounded() const { return m_IsGrounded; }

        protected:
            float m_Acceleration = 5.0f;
            float m_MaxSpeed = 10.0f;
            float m_BrakingDeceleration = 8.0f;
			float m_JumpImpulse = 12.0f;
            float m_GroundTraceDistance = 2.5f;
        private:
            glm::vec3 m_Velocity = glm::vec3(0.0f);
            glm::vec3 m_PendingInput = glm::vec3(0.0f);
            bool m_IsGrounded = false;
			bool m_JumpRequested = false;
    };
}