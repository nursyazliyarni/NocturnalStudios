// collision checks & responses
#include "math/Vector.h"

#ifndef COLLISION_H
#define COLLISION_H

bool checkCollision(float x1, float y1, float w1, float h1,	float x2, float y2, float w2, float h2);
bool isPointInBox(const Vector2D& point, const Vector2D& boxPos, const Vector2D& boxSize);

#endif
