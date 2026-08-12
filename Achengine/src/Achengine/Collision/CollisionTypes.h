#pragma once

namespace Achengine
{
    enum class ECollisionChannel
    {
        BlockAll,
        BlockAllDynamic,
        BlockAllStatic,
        OverlapAll,
        OverlapAllDynamic,
        OverlapAllStatic,
        Custom
    };

    enum class ECollisionResponse
    {
        ECR_Ignore,
        ECR_Overlap,
        ECR_Block
    };

    enum class ECollisionObjectType
    {
        WorldStatic,
        WorldDynamic,
        Pawn,
        PhysicsBody,
        Vehicle,
        Destructible,
        Custom
    };
}
