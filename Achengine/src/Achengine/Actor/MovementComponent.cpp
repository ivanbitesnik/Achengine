#include "Achenginepch.h"
#include "MovementComponent.h"

#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/WorldActorCache.h"
#include "Achengine/Collision/CollisionHelpers.h"
#include "Achengine/Globals/GameGlobals.h"

#include <algorithm>

namespace Achengine
{
	namespace
	{
		const bool s_RegisteredMovementClass = UActorComponent::RegisterComponentClass<UMovementComponent>("UMovementComponent", "movement");
	}

	UMovementComponent::UMovementComponent()
	{
	}

	void UMovementComponent::AddInputVector(const glm::vec3& input)
	{
		m_PendingInput += input;
	}

	void UMovementComponent::ClearInputVector()
	{
		m_PendingInput = glm::vec3(0.0f);
	}

	void UMovementComponent::Tick(float DeltaTime)
	{
		if (!GetOwner())
		{
			m_PendingInput = glm::vec3(0.0f);
			m_JumpRequested = false;
			return;
		}

		glm::vec3 planarInput = m_PendingInput;
		m_PendingInput = glm::vec3(0.0f);
		planarInput.y = 0.0f;
		if (glm::length(planarInput) > 1.0f)
		{
			planarInput = glm::normalize(planarInput);
		}

		const glm::vec3 targetPlanarVelocity = planarInput * m_MaxSpeed;
		glm::vec3 currentPlanarVelocity(m_Velocity.x, 0.0f, m_Velocity.z);
		glm::vec3 deltaPlanarVelocity = targetPlanarVelocity - currentPlanarVelocity;
		const float maxPlanarDelta = m_Acceleration * DeltaTime;
		if (glm::length(deltaPlanarVelocity) > maxPlanarDelta && maxPlanarDelta > 0.0f)
		{
			deltaPlanarVelocity = glm::normalize(deltaPlanarVelocity) * maxPlanarDelta;
		}

		currentPlanarVelocity += deltaPlanarVelocity;
		if (glm::length(planarInput) < 0.0001f)
		{
			const float damping = std::max(0.0f, 1.0f - m_BrakingDeceleration * DeltaTime);
			currentPlanarVelocity *= damping;
		}

		m_Velocity.x = currentPlanarVelocity.x;
		m_Velocity.z = currentPlanarVelocity.z;

		std::vector<AActor*> traceActors;
		if (WorldActorCache* cache = WorldActorCache::Get())
		{
			traceActors.reserve(cache->GetActorCache().size());
			for (AActor* actor : cache->GetActorCache())
			{
				if (actor && actor != GetOwner())
				{
					traceActors.push_back(actor);
				}
			}
		}

		const glm::vec3 traceStart = GetOwner()->GetActorLocation();
		const glm::vec3 traceEnd = traceStart + glm::vec3(0.0f, -m_GroundTraceDistance, 0.0f);
		const FCollisionHitResult groundHit = LineTraceSingle(traceStart, traceEnd, traceActors, this);
		m_IsGrounded = groundHit.Hit;

		if (m_JumpRequested && m_IsGrounded)
		{
			m_Velocity.y = m_JumpImpulse;
			m_IsGrounded = false;
		}
		m_JumpRequested = false;

		const GameGlobals* globals = GameGlobals::GetActiveConst();
		const float gravity = globals ? globals->Gravity : 9.81f;

		if (m_IsGrounded)
		{
			if (m_Velocity.y < 0.0f)
			{
				m_Velocity.y = 0.0f;
			}
		}
		else
		{
			m_Velocity.y -= gravity * DeltaTime;
		}

		GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + m_Velocity * DeltaTime);
	}
}