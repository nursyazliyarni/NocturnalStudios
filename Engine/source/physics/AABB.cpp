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

//sprite to sprite collision
bool checkAABBCollision(const Vector2D& pos1, const Vector2D& size1, const Vector2D& pos2, const Vector2D& size2) {
    return AABB{pos1,size1}.intersects(AABB{pos2,size2});
}

//mouse to sprite collision
bool isPointInAABB(const Vector2D& point, const Vector2D& boxPos, const Vector2D& boxSize) {
    return AABB{boxPos,boxSize}.containsPoint(point);
}
