/*!------------------------------------------------------------------------------------------------
	File: Collision.cpp
	Project: NocturnalStudio

	This file contains collision checks & responses using AABB wrapper.

	Primary Author:
		 Chang Qi Xuan Celest (100%)
		- Created collision functions using AABB wrapper.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// collision checks & responses

#include "physics/Collision.h"

bool checkCollision(float x1, float y1, float w1, float h1,	float x2, float y2, float w2, float h2) {
	////collision x-axis
	//bool collisionX = x1 + w1 >= x2 && x2 + w2 >= x1;
	////collision y-axis
	//bool collisionY = y1 + h1 >= y2 && y2 + h2 >= y1;
	////collision only if on both axes
	//return collisionX && collisionY;
	
	//Using AABB struct to check for collision
	AABB box1{ Vector2D(x1, y1), Vector2D(w1, h1) };
	AABB box2{ Vector2D(x2, y2), Vector2D(w2, h2) };
	return box1.intersects(box2);
}

bool isPointInBox(const Vector2D& point, const Vector2D& boxPos, const Vector2D& boxSize) {
	//return point.x() >= boxPos.x() && point.x() <= boxPos.x() + boxSize.x() && point.y() >= boxPos.y() && point.y() <= boxPos.y() + boxSize.y();
	
	//Using AABB struct to check if point is in box
	AABB box { 
		boxPos, boxSize
	};
	return box.containsPoint(point);
}
