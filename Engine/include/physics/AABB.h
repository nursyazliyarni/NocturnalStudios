/*!------------------------------------------------------------------------------------------------
    File: AABB.h
    Project: NocturnalStudio

	This file contains AABB struct for bounding box collision and related functions.

    Primary Author:
         Chang Qi Xuan Celest (100%)
		- Created AABB struct and related functions.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#ifndef AABB_H
#define AABB_H

// bounding box collision
#include "math/Vector.h"

struct AABB {
	Vector2D pos; //top-left corner
	Vector2D size; //width and height

	//check if this AABB intersects with another AABB
    bool intersects(const AABB& other) const {
        return (pos.x() + size.x() >= other.pos.x() &&
            other.pos.x() + other.size.x() >= pos.x() &&
            pos.y() + size.y() >= other.pos.y() &&
            other.pos.y() + other.size.y() >= pos.y());
    }

	//check if this AABB contains a point
    bool containsPoint(const Vector2D& point) const {
        return (point.x() >= pos.x() && point.x() <= pos.x() + size.x() &&
            point.y() >= pos.y() && point.y() <= pos.y() + size.y());
    }

};

//sprite to sprite collision
bool checkAABBCollision(const Vector2D& pos1, const Vector2D& size1, const Vector2D& pos2, const Vector2D& size2);
//point to sprite collision
bool isPointInAABB(const Vector2D& point, const Vector2D& boxPos, const Vector2D& boxSize);


#endif