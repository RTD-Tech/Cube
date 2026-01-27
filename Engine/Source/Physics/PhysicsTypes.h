#pragma once

#include <vector>

#include "Math/Math.h"

namespace SquareCore
{

    enum class ColliderShape
    {
        BOX,
        CIRCLE,
        CAPSULE,
        POLYGON,
    };

    struct BoxShapeData
    {
        Vec2 halfExtents = Vec2(0.0, 0.0);
    };
    
    struct CircleShapeData
    {
        float radius = 0.5f;
        Vec2 center = Vec2::zero();
    };
    
    struct CapsuleShapeData
    {
        Vec2 center1 = Vec2(0.0f, -0.5f);
        Vec2 center2 = Vec2(0.0f, 0.5f);
        float radius = 0.5f;
    };
    
    struct PolygonShapeData
    {
        std::vector<Vec2> vertices = {};
    };
    
    struct ColliderShapeData
    {
        ColliderShape shape = ColliderShape::BOX;
        BoxShapeData box;
        CircleShapeData circle;
        CapsuleShapeData capsule;
        PolygonShapeData polygon;
    };
    
    struct CollisionInfo
    {
        uint32_t otherEntityId = 0;
        int side; // 0 = top, 1 = right, 2 = bottom, 3 = left
        Vec2 normal;
        Vec2 point;
    };
}
