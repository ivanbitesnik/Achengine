#pragma once

#include "Achengine/Actor/Actor.h"
#include "Achengine/Collision/CollisionTypes.h"
#include "Achengine/Core/Utilities.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace Achengine
{
    struct FCollisionHitResult
    {
        bool Hit = false;
        float Time = 1.0f;
        float Distance = 0.0f;
        glm::vec3 Location = glm::vec3(0.0f);
        glm::vec3 Normal = glm::vec3(0.0f);
        AActor* HitActor = nullptr;
        UActorComponent* HitComponent = nullptr;
    };

    inline bool BoundsOverlapAabb(const FBounds& a, const FBounds& b)
    {
        if (!a.IsValid || !b.IsValid)
        {
            return false;
        }

        const glm::vec3 aMin = a.Center - a.Extents;
        const glm::vec3 aMax = a.Center + a.Extents;
        const glm::vec3 bMin = b.Center - b.Extents;
        const glm::vec3 bMax = b.Center + b.Extents;

        return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
            (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
            (aMin.z <= bMax.z && aMax.z >= bMin.z);
    }

    inline bool ChannelAllowsObjectType(ECollisionChannel channel, ECollisionObjectType objectType)
    {
        switch (channel)
        {
        case ECollisionChannel::BlockAll:
        case ECollisionChannel::OverlapAll:
            return true;
        case ECollisionChannel::BlockAllDynamic:
        case ECollisionChannel::OverlapAllDynamic:
            return objectType != ECollisionObjectType::WorldStatic;
        case ECollisionChannel::BlockAllStatic:
        case ECollisionChannel::OverlapAllStatic:
            return objectType == ECollisionObjectType::WorldStatic;
        case ECollisionChannel::Custom:
        default:
            return true;
        }
    }

    inline const UActorComponent* GetPrimaryCollisionComponent(const AActor* actor)
    {
        if (!actor)
        {
            return nullptr;
        }

        const std::vector<UActorComponent*>& components = actor->GetActorComponents();
        const UActorComponent* fallback = nullptr;
        for (const UActorComponent* component : components)
        {
            if (!component)
            {
                continue;
            }

            if (!fallback)
            {
                fallback = component;
            }

            if (component->GetBounds().IsValid)
            {
                return component;
            }
        }

        return fallback;
    }

    inline bool CollisionConfigurationAllowsPair(const UActorComponent* compA, const UActorComponent* compB)
    {
        if (!compA || !compB)
        {
            return false;
        }

        if (compA->GetCollisionResponse() == ECollisionResponse::ECR_Ignore ||
            compB->GetCollisionResponse() == ECollisionResponse::ECR_Ignore)
        {
            return false;
        }

        const bool aAllowsB = ChannelAllowsObjectType(compA->GetCollisionChannel(), compB->GetCollisionObjectType());
        const bool bAllowsA = ChannelAllowsObjectType(compB->GetCollisionChannel(), compA->GetCollisionObjectType());
        return aAllowsB && bAllowsA;
    }

    inline uint64_t MakeActorPairKey(const AActor* a, const AActor* b)
    {
        uintptr_t pa = reinterpret_cast<uintptr_t>(a);
        uintptr_t pb = reinterpret_cast<uintptr_t>(b);
        if (pa > pb)
        {
            std::swap(pa, pb);
        }

        uint64_t hashA = static_cast<uint64_t>(pa);
        uint64_t hashB = static_cast<uint64_t>(pb);
        return hashA ^ (hashB + 0x9e3779b97f4a7c15ULL + (hashA << 6) + (hashA >> 2));
    }

    inline bool LineTraceBounds(const glm::vec3& start, const glm::vec3& end, const FBounds& bounds, float& outTime, glm::vec3& outNormal)
    {
        if (!bounds.IsValid)
        {
            return false;
        }

        const glm::vec3 minBounds = bounds.Center - bounds.Extents;
        const glm::vec3 maxBounds = bounds.Center + bounds.Extents;
        const glm::vec3 direction = end - start;

        float tMin = 0.0f;
        float tMax = 1.0f;
        glm::vec3 hitNormal(0.0f);

        for (int axis = 0; axis < 3; ++axis)
        {
            const float dir = direction[axis];
            const float origin = start[axis];
            const float minAxis = minBounds[axis];
            const float maxAxis = maxBounds[axis];

            if (std::abs(dir) < 1e-6f)
            {
                if (origin < minAxis || origin > maxAxis)
                {
                    return false;
                }
                continue;
            }

            const float invDir = 1.0f / dir;
            float t1 = (minAxis - origin) * invDir;
            float t2 = (maxAxis - origin) * invDir;

            glm::vec3 axisNormal(0.0f);
            axisNormal[axis] = (t1 < t2) ? -1.0f : 1.0f;

            if (t1 > t2)
            {
                std::swap(t1, t2);
            }

            if (t1 > tMin)
            {
                tMin = t1;
                hitNormal = axisNormal;
            }
            tMax = std::min(tMax, t2);
            if (tMin > tMax)
            {
                return false;
            }
        }

        if (tMin < 0.0f || tMin > 1.0f)
        {
            return false;
        }

        outTime = tMin;
        outNormal = hitNormal;
        return true;
    }

    inline FCollisionHitResult LineTraceSingle(
        const glm::vec3& start,
        const glm::vec3& end,
        const std::vector<AActor*>& actors,
        const UActorComponent* queryComponent = nullptr)
    {
        FCollisionHitResult bestHit;
        const float segmentLength = glm::length(end - start);
        float bestTime = std::numeric_limits<float>::max();

        for (AActor* actor : actors)
        {
            if (!actor)
            {
                continue;
            }

            UActorComponent* hitComponent = const_cast<UActorComponent*>(GetPrimaryCollisionComponent(actor));
            if (!hitComponent)
            {
                continue;
            }

            if (queryComponent && !CollisionConfigurationAllowsPair(queryComponent, hitComponent))
            {
                continue;
            }

            float hitTime = 0.0f;
            glm::vec3 hitNormal(0.0f);
            if (!LineTraceBounds(start, end, actor->GetBounds(), hitTime, hitNormal))
            {
                continue;
            }

            if (hitTime >= bestTime)
            {
                continue;
            }

            bestTime = hitTime;
            bestHit.Hit = true;
            bestHit.Time = hitTime;
            bestHit.Distance = segmentLength * hitTime;
            bestHit.Location = start + (end - start) * hitTime;
            bestHit.Normal = hitNormal;
            bestHit.HitActor = actor;
            bestHit.HitComponent = hitComponent;
        }

        return bestHit;
    }

    inline FCollisionHitResult SweepAabbSingle(
        const FBounds& movingBounds,
        const glm::vec3& delta,
        const std::vector<AActor*>& actors,
        const UActorComponent* queryComponent = nullptr)
    {
        FCollisionHitResult bestHit;
        if (!movingBounds.IsValid)
        {
            return bestHit;
        }

        const glm::vec3 start = movingBounds.Center;
        const glm::vec3 end = movingBounds.Center + delta;
        const float sweepLength = glm::length(delta);
        float bestTime = std::numeric_limits<float>::max();

        for (AActor* actor : actors)
        {
            if (!actor)
            {
                continue;
            }

            UActorComponent* hitComponent = const_cast<UActorComponent*>(GetPrimaryCollisionComponent(actor));
            if (!hitComponent)
            {
                continue;
            }

            if (queryComponent && !CollisionConfigurationAllowsPair(queryComponent, hitComponent))
            {
                continue;
            }

            const FBounds targetBounds = actor->GetBounds();
            if (!targetBounds.IsValid)
            {
                continue;
            }

            FBounds expandedTarget;
            expandedTarget.IsValid = true;
            expandedTarget.Center = targetBounds.Center;
            expandedTarget.Extents = targetBounds.Extents + movingBounds.Extents;

            float hitTime = 0.0f;
            glm::vec3 hitNormal(0.0f);
            if (!LineTraceBounds(start, end, expandedTarget, hitTime, hitNormal))
            {
                continue;
            }

            if (hitTime >= bestTime)
            {
                continue;
            }

            bestTime = hitTime;
            bestHit.Hit = true;
            bestHit.Time = hitTime;
            bestHit.Distance = sweepLength * hitTime;
            bestHit.Location = start + (end - start) * hitTime;
            bestHit.Normal = hitNormal;
            bestHit.HitActor = actor;
            bestHit.HitComponent = hitComponent;
        }

        return bestHit;
    }
}