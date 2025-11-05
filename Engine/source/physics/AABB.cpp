/*!------------------------------------------------------------------------------------------------
    File: AABB.cpp
    Project: NocturnalStudio

	This file contains AABB functions definitions for bounding box collision.

    Primary Author:
         Chang Qi Xuan Celest (100%)
        - Created AABB function definitions.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// bounding box collision
#include "physics/AABB.h"
#include "graphics/DebugDraw.h"

//sprite to sprite collision
bool checkAABBCollision(const Vector2D& pos1, const Vector2D& size1,
    const Vector2D& pos2, const Vector2D& size2)
{
    /*
    // Queue debug boxes for both AABBs
    if (Debug::Drawer::Instance().IsActive()) {
        Debug::Drawer::Instance().AddRect(pos1.x(), pos1.y(), size1.x(), size1.y(), 0xFF0000FF); // red
        Debug::Drawer::Instance().AddRect(pos2.x(), pos2.y(), size2.x(), size2.y(), 0x00FF00FF); // green
    } */

    return AABB{ pos1, size1 }.intersects(AABB{ pos2, size2 });
}


//mouse to sprite collision
bool isPointInAABB(const Vector2D& point, const Vector2D& boxPos, const Vector2D& boxSize)
{
    /*
    if (Debug::Drawer::Instance().IsActive()) {
        Debug::Drawer::Instance().AddRect(boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 0xFFFF00FF); // yellow box
        Debug::Drawer::Instance().AddCircle(point.x(), point.y(), 5.f, 0xFF00FFFF); // cyan point
    } */

    return AABB{ boxPos, boxSize }.containsPoint(point);
}
