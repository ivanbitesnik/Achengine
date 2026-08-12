#pragma once

#include "Achengine/Actor/Actor.h"
#include "Achengine/Globals/GameGlobals.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

namespace Achengine
{
    class CollisionOctree
    {
    public:
        struct FPair
        {
            AActor* A = nullptr;
            AActor* B = nullptr;
        };

        CollisionOctree()
        {
            ResetRoot();
        }

        explicit CollisionOctree(const GameGlobals& globals)
        {
            Configure(globals);
        }

        void Configure(const GameGlobals& globals)
        {
            m_RootSize = globals.CollisionOctreeSize > 0.001f ? globals.CollisionOctreeSize : 0.001f;
            m_MinNodeSize = globals.CollisionOctreeMinNodeSize > 0.001f ? globals.CollisionOctreeMinNodeSize : 0.001f;
            if (m_MinNodeSize > m_RootSize)
            {
                m_MinNodeSize = m_RootSize;
            }

            ResetRoot();
        }

        void SetRootCenter(const glm::vec3& center)
        {
            m_RootCenter = center;
            ResetRoot();
        }

        void Clear()
        {
            ResetRoot();
        }

        bool InsertActor(AActor* actor)
        {
            if (!actor || !m_Root)
            {
                return false;
            }

            const FBounds bounds = actor->GetBounds();
            if (!bounds.IsValid)
            {
                return false;
            }

            if (!IntersectsNode(bounds, m_Root->Center, m_Root->Size))
            {
                return false;
            }

            FEntry entry;
            entry.Actor = actor;
            entry.Bounds = bounds;
            InsertIntoNode(*m_Root, entry);
            return true;
        }

        void Build(const std::vector<AActor*>& actors)
        {
            ResetRoot();
            for (AActor* actor : actors)
            {
                InsertActor(actor);
            }
        }

        std::vector<FPair> CollectCandidatePairs(bool requireAabbOverlap = false) const
        {
            std::vector<FPair> pairs;
            if (!m_Root)
            {
                return pairs;
            }

            std::vector<FEntry> inherited;
            std::unordered_set<uint64_t> seen;
            CollectNodePairs(*m_Root, inherited, requireAabbOverlap, seen, pairs);
            return pairs;
        }

    private:
        struct FEntry
        {
            AActor* Actor = nullptr;
            FBounds Bounds;
        };

        struct FNode
        {
            glm::vec3 Center = glm::vec3(0.0f);
            float Size = 1.0f;
            std::vector<FEntry> Entries;
            std::array<std::unique_ptr<FNode>, 8> Children;

            bool HasChildren() const
            {
                for (const std::unique_ptr<FNode>& child : Children)
                {
                    if (child)
                    {
                        return true;
                    }
                }
                return false;
            }
        };

        static glm::vec3 GetBoundsMin(const FBounds& bounds)
        {
            return bounds.Center - bounds.Extents;
        }

        static glm::vec3 GetBoundsMax(const FBounds& bounds)
        {
            return bounds.Center + bounds.Extents;
        }

        static bool IntersectsBounds(const FBounds& a, const FBounds& b)
        {
            if (!a.IsValid || !b.IsValid)
            {
                return false;
            }

            const glm::vec3 aMin = GetBoundsMin(a);
            const glm::vec3 aMax = GetBoundsMax(a);
            const glm::vec3 bMin = GetBoundsMin(b);
            const glm::vec3 bMax = GetBoundsMax(b);

            return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
                (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
                (aMin.z <= bMax.z && aMax.z >= bMin.z);
        }

        static bool IntersectsNode(const FBounds& bounds, const glm::vec3& nodeCenter, float nodeSize)
        {
            if (!bounds.IsValid)
            {
                return false;
            }

            FBounds nodeBounds;
            nodeBounds.Center = nodeCenter;
            nodeBounds.Extents = glm::vec3(nodeSize * 0.5f);
            nodeBounds.IsValid = true;
            return IntersectsBounds(bounds, nodeBounds);
        }

        static bool FitsInsideNode(const FBounds& bounds, const glm::vec3& nodeCenter, float nodeSize)
        {
            if (!bounds.IsValid)
            {
                return false;
            }

            const glm::vec3 nodeExtents(nodeSize * 0.5f);
            const glm::vec3 nodeMin = nodeCenter - nodeExtents;
            const glm::vec3 nodeMax = nodeCenter + nodeExtents;
            const glm::vec3 boundsMin = GetBoundsMin(bounds);
            const glm::vec3 boundsMax = GetBoundsMax(bounds);

            return (boundsMin.x >= nodeMin.x && boundsMax.x <= nodeMax.x) &&
                (boundsMin.y >= nodeMin.y && boundsMax.y <= nodeMax.y) &&
                (boundsMin.z >= nodeMin.z && boundsMax.z <= nodeMax.z);
        }

        static uint64_t MakePairKey(const AActor* a, const AActor* b)
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

        bool CanSubdivide(const FNode& node) const
        {
            return (node.Size * 0.5f) >= m_MinNodeSize;
        }

        glm::vec3 ComputeChildCenter(const FNode& node, int childIndex) const
        {
            const float quarter = node.Size * 0.25f;
            return glm::vec3(
                node.Center.x + ((childIndex & 1) ? quarter : -quarter),
                node.Center.y + ((childIndex & 2) ? quarter : -quarter),
                node.Center.z + ((childIndex & 4) ? quarter : -quarter));
        }

        int FindContainingChildIndex(const FNode& node, const FBounds& bounds) const
        {
            const float childSize = node.Size * 0.5f;
            for (int i = 0; i < 8; ++i)
            {
                const glm::vec3 childCenter = ComputeChildCenter(node, i);
                if (FitsInsideNode(bounds, childCenter, childSize))
                {
                    return i;
                }
            }

            return -1;
        }

        void InsertIntoNode(FNode& node, const FEntry& entry)
        {
            if (!CanSubdivide(node))
            {
                node.Entries.push_back(entry);
                return;
            }

            const int childIndex = FindContainingChildIndex(node, entry.Bounds);
            if (childIndex < 0)
            {
                node.Entries.push_back(entry);
                return;
            }

            if (!node.Children[childIndex])
            {
                std::unique_ptr<FNode> child(new FNode());
                child->Size = node.Size * 0.5f;
                child->Center = ComputeChildCenter(node, childIndex);
                node.Children[childIndex] = std::move(child);
            }

            InsertIntoNode(*node.Children[childIndex], entry);
        }

        static void AppendPair(
            const FEntry& a,
            const FEntry& b,
            bool requireAabbOverlap,
            std::unordered_set<uint64_t>& seen,
            std::vector<FPair>& outPairs)
        {
            if (!a.Actor || !b.Actor || a.Actor == b.Actor)
            {
                return;
            }

            if (requireAabbOverlap && !IntersectsBounds(a.Bounds, b.Bounds))
            {
                return;
            }

            const uint64_t key = MakePairKey(a.Actor, b.Actor);
            if (seen.find(key) != seen.end())
            {
                return;
            }

            seen.insert(key);

            FPair pair;
            pair.A = a.Actor;
            pair.B = b.Actor;
            outPairs.push_back(pair);
        }

        static void CollectNodePairs(
            const FNode& node,
            const std::vector<FEntry>& inherited,
            bool requireAabbOverlap,
            std::unordered_set<uint64_t>& seen,
            std::vector<FPair>& outPairs)
        {
            for (size_t i = 0; i < node.Entries.size(); ++i)
            {
                for (size_t j = i + 1; j < node.Entries.size(); ++j)
                {
                    AppendPair(node.Entries[i], node.Entries[j], requireAabbOverlap, seen, outPairs);
                }

                for (const FEntry& inheritedEntry : inherited)
                {
                    AppendPair(node.Entries[i], inheritedEntry, requireAabbOverlap, seen, outPairs);
                }
            }

            if (!node.HasChildren())
            {
                return;
            }

            std::vector<FEntry> nextInherited = inherited;
            nextInherited.insert(nextInherited.end(), node.Entries.begin(), node.Entries.end());

            for (const std::unique_ptr<FNode>& child : node.Children)
            {
                if (!child)
                {
                    continue;
                }

                CollectNodePairs(*child, nextInherited, requireAabbOverlap, seen, outPairs);
            }
        }

        void ResetRoot()
        {
            m_Root.reset(new FNode());
            m_Root->Center = m_RootCenter;
            m_Root->Size = m_RootSize;
        }

    private:
        glm::vec3 m_RootCenter = glm::vec3(0.0f);
        float m_RootSize = 10000.0f;
        float m_MinNodeSize = 1.0f;
        std::unique_ptr<FNode> m_Root;
    };
}